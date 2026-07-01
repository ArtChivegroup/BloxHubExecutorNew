#include "injector.hpp"
#include "cfg_bypass.h"
#include "offsets.hpp"
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <fstream>
#include <vector>
#include <cstdio>

#define valid_handle(x) (x != INVALID_HANDLE_VALUE && x != nullptr)

static bool TryCfgBitmapFromRva(HANDLE process, uintptr_t robloxBase, uintptr_t rva, uintptr_t* outSlotAddr)
{
    if (!rva || !outSlotAddr) return false;

    uintptr_t slot = robloxBase + rva;
    uintptr_t ptr = 0;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(process, (LPCVOID)slot, &ptr, sizeof(ptr), &bytesRead) || !ptr)
        return false;

    MEMORY_BASIC_INFORMATION mbi;
    if (!VirtualQueryEx(process, (LPCVOID)ptr, &mbi, sizeof(mbi)))
        return false;
    if (mbi.State != MEM_COMMIT || mbi.RegionSize < 0x10000)
        return false;

    *outSlotAddr = slot;
    printf("[*] CFG bitmap slot RVA 0x%llX -> 0x%llX\n",
        (unsigned long long)rva, (unsigned long long)ptr);
    return true;
}

static bool load_file_into_memory(const std::string file_path, std::vector<uint8_t>& image) {
    std::ifstream file(file_path, std::ios::binary | std::ios::ate);
    if (!file)
        return false;

    const auto size = file.tellg();
    file.seekg(0);

    image.resize(size);
    file.read((char*)image.data(), size);

    return true;
}

static uintptr_t GetModuleBase(HANDLE process, const wchar_t* moduleName)
{
    HMODULE hMods[1024];
    DWORD cbNeeded;
    if (EnumProcessModules(process, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            wchar_t szModName[MAX_PATH];
            if (GetModuleFileNameExW(process, hMods[i], szModName, sizeof(szModName) / sizeof(wchar_t)))
            {
                if (wcsstr(szModName, moduleName))
                    return (uintptr_t)hMods[i];
            }
        }
    }
    return 0;
}

static DWORD FindMainThread(DWORD pid)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);
    DWORD mainThread = 0;

    if (Thread32First(hSnapshot, &te32))
    {
        do {
            if (te32.th32OwnerProcessID == pid)
            {
                if (!mainThread || te32.th32ThreadID < mainThread)
                    mainThread = te32.th32ThreadID;
            }
        } while (Thread32Next(hSnapshot, &te32));
    }

    CloseHandle(hSnapshot);
    return mainThread;
}

injector::InjectionStatus injector::Inject(uint32_t pid, const std::string& dll_path, bool enableCfgBypass) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (!valid_handle(process)) {
        printf("[!] OpenProcess(pid=%u) failed: %lu\n", pid, GetLastError());
        return InjectionStatus::FAILED;
    }

    std::vector<uint8_t> image;
    if (!load_file_into_memory(dll_path, image)) {
        printf("[!] Failed to read DLL: %s\n", dll_path.c_str());
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    const auto* dos_header = (PIMAGE_DOS_HEADER)image.data();
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        printf("[!] Invalid DOS header in payload DLL\n");
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    auto* nt_headers = (PIMAGE_NT_HEADERS)(image.data() + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        printf("[!] Invalid PE signature in payload DLL\n");
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    const auto* remote_image = (uint8_t*)VirtualAllocEx(process, 0, nt_headers->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remote_image) {
        printf("[!] VirtualAllocEx failed: %lu\n", GetLastError());
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }
    printf("[*] Allocated remote image at: 0x%p\n", remote_image);

    std::vector<uint8_t> fixed_image(nt_headers->OptionalHeader.SizeOfImage);

    auto* section_header = IMAGE_FIRST_SECTION(nt_headers);
    for (auto idx = 0; idx < nt_headers->FileHeader.NumberOfSections; idx++) {
        memcpy((void*)(fixed_image.data() + section_header->VirtualAddress),
            image.data() + section_header->PointerToRawData,
            section_header->SizeOfRawData);
        section_header++;
    }

    // Apply base relocations
    auto* realloc_block = (PIMAGE_BASE_RELOCATION)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    while (realloc_block->VirtualAddress) {
        auto count = (realloc_block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(uint16_t);
        auto* entries = (uint16_t*)(realloc_block + 1);
        for (auto i = 0; i < count; i++) {
            auto type = entries[i] >> 12;
            auto offset = entries[i] & 0xFFF;
            if (type == IMAGE_REL_BASED_DIR64) {
                uint64_t* patch_addr = (uint64_t*)(fixed_image.data() + realloc_block->VirtualAddress + offset);
                *patch_addr += (uint64_t)remote_image - nt_headers->OptionalHeader.ImageBase;
            }
        }
        realloc_block = (PIMAGE_BASE_RELOCATION)((uint8_t*)realloc_block + realloc_block->SizeOfBlock);
    }

    // Resolve imports
    auto* import_desc = (PIMAGE_IMPORT_DESCRIPTOR)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (import_desc->Name) {
        const char* dll_name = (const char*)(fixed_image.data() + import_desc->Name);
        HMODULE mod = LoadLibraryA(dll_name);
        if (mod) {
            auto* thunk = (PIMAGE_THUNK_DATA)(fixed_image.data() + import_desc->FirstThunk);
            auto* orig_thunk = import_desc->OriginalFirstThunk ? 
                (PIMAGE_THUNK_DATA)(fixed_image.data() + import_desc->OriginalFirstThunk) : thunk;
            while (orig_thunk->u1.AddressOfData) {
                uint64_t func_addr = 0;
                if (IMAGE_SNAP_BY_ORDINAL(orig_thunk->u1.Ordinal)) {
                    func_addr = (uint64_t)GetProcAddress(mod, MAKEINTRESOURCEA(IMAGE_ORDINAL(orig_thunk->u1.Ordinal)));
                } else {
                    auto* import_by_name = (PIMAGE_IMPORT_BY_NAME)(fixed_image.data() + orig_thunk->u1.AddressOfData);
                    func_addr = (uint64_t)GetProcAddress(mod, import_by_name->Name);
                }
                thunk->u1.Function = func_addr;
                thunk++;
                orig_thunk++;
            }
        }
        import_desc++;
    }

    // Write fixed image to target
    if (!WriteProcessMemory(process, (void*)remote_image, fixed_image.data(), fixed_image.size(), nullptr)) {
        printf("[!] WriteProcessMemory failed: %lu\n", GetLastError());
        VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }
    printf("[*] DLL written to remote process\n");

    // ==========================================
    // CFG Bypass (Hyperion Control Flow Guard)
    // ==========================================
    if (enableCfgBypass) {
        printf("[*] Executing CFG bypass (offsets for %s)...\n", offsets::roblox_version);

        uintptr_t robloxBase = GetModuleBase(process, L"RobloxPlayerBeta.dll");
        if (robloxBase) {
            printf("[*] RobloxPlayerBeta.dll base: 0x%llX\n", (unsigned long long)robloxBase);

            IMAGE_DOS_HEADER dosHdr;
            IMAGE_NT_HEADERS ntHdr;
            SIZE_T bytesRead = 0;
            if (ReadProcessMemory(process, (LPCVOID)robloxBase, &dosHdr, sizeof(dosHdr), &bytesRead) &&
                ReadProcessMemory(process, (LPCVOID)(robloxBase + dosHdr.e_lfanew), &ntHdr, sizeof(ntHdr), &bytesRead))
            {
                uintptr_t robloxSize = ntHdr.OptionalHeader.SizeOfImage;
                uintptr_t bitmapAddr = 0;

                if (!TryCfgBitmapFromRva(process, robloxBase, offsets::CfgBypass::BitmapPtr, &bitmapAddr))
                {
                // Smart scan .rdata/.data for CFG bitmap pointer slot
                IMAGE_SECTION_HEADER sections[32];
                if (ReadProcessMemory(process,
                    (LPCVOID)(robloxBase + dosHdr.e_lfanew + sizeof(IMAGE_NT_HEADERS)),
                    sections, sizeof(IMAGE_SECTION_HEADER) * ntHdr.FileHeader.NumberOfSections, &bytesRead))
                {
                    // First try .data section (dynamic globals often in .data)
                    for (WORD s = 0; s < ntHdr.FileHeader.NumberOfSections && !bitmapAddr; s++)
                    {
                        bool isDataSection = (memcmp(sections[s].Name, ".data", 6) == 0) ||
                                             (memcmp(sections[s].Name, ".rdata", 7) == 0);
                        if (!isDataSection) continue;

                        uintptr_t secStart = robloxBase + sections[s].VirtualAddress;
                        DWORD secSize = sections[s].Misc.VirtualSize;
                        printf("[*] Scanning %.*s (0x%llX - 0x%llX)...\n",
                            8, sections[s].Name,
                            (unsigned long long)secStart, (unsigned long long)(secStart + secSize));

                        // Read section in chunks
                        const DWORD CHUNK = 0x10000;
                        std::vector<uint8_t> buf;
                        for (DWORD off = 0; off < secSize && !bitmapAddr; off += CHUNK)
                        {
                            DWORD readSize = min(CHUNK, secSize - off);
                            buf.resize(readSize);
                            if (!ReadProcessMemory(process, (LPCVOID)(secStart + off), buf.data(), readSize, &bytesRead))
                                continue;

                            for (DWORD i = 0; i + 8 <= bytesRead; i += 8)
                            {
                                uint64_t ptr = *(uint64_t*)(buf.data() + i);
                                // Filter: valid page-aligned user-mode address outside Roblox's image
                                if (ptr < 0x100000 || ptr > 0x00007FFFFFFFFFFFULL) continue;
                                if ((ptr & 0xFFF) != 0) continue; // must be page-aligned
                                if (ptr >= robloxBase && ptr < robloxBase + robloxSize) continue;

                                // Must point to committed memory
                                MEMORY_BASIC_INFORMATION mbi;
                                if (!VirtualQueryEx(process, (LPCVOID)ptr, &mbi, sizeof(mbi))) continue;
                                if (mbi.State != MEM_COMMIT) continue;
                                if (mbi.AllocationBase != (LPCVOID)ptr) continue; // must be base of allocation
                                if (!(mbi.Protect & (PAGE_READONLY | PAGE_READWRITE))) continue;

                                // Bitmap must be large enough (at least 64KB for CFG bitmap)
                                if (mbi.RegionSize < 0x10000) continue;

                                // Try reading a few bytes to verify bitmap-like data
                                uint8_t probe[64];
                                if (ReadProcessMemory(process, (LPCVOID)ptr, probe, sizeof(probe), &bytesRead) && bytesRead == sizeof(probe))
                                {
                                    // Check: non-zero and non-ASCII
                                    bool hasData = false, isAscii = true;
                                    for (int k = 0; k < sizeof(probe); k++)
                                    {
                                        if (probe[k] != 0) hasData = true;
                                        if (probe[k] != 0 && (probe[k] < 0x20 || probe[k] > 0x7E)) isAscii = false;
                                    }
                                    if (!hasData || isAscii) continue;

                                    bitmapAddr = secStart + off + i;
                                    printf("[*] Found CFG bitmap candidate at RVA 0x%llX -> 0x%llX (size: 0x%llX)\n",
                                        (unsigned long long)(bitmapAddr - robloxBase),
                                        (unsigned long long)ptr,
                                        (unsigned long long)mbi.RegionSize);
                                }
                            }
                        }
                    }
                }
                }

                if (bitmapAddr)
                {
                    PatchCfgBitmap(process, bitmapAddr,
                        (uintptr_t)remote_image, (DWORD)nt_headers->OptionalHeader.SizeOfImage);
                }
                else
                {
                    printf("[!] Could not find CFG bitmap pointer automatically\n");
                    printf("[*] To fix: find bitmap RVA in RobloxPlayerBeta.dll .rdata/.data\n");
                    printf("[*] Then add to offsets.hpp -> CfgBypass::BitmapPtr\n");
                }
            }
        }
        else
        {
            printf("[!] Could not find RobloxPlayerBeta.dll in target process\n");
        }
    }

    // Find BloxHubInit export
    uintptr_t bloxhub_init_addr = 0;
    if (nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress != 0) {
        auto* export_dir = (PIMAGE_EXPORT_DIRECTORY)(fixed_image.data() + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
        auto* functions = (PDWORD)(fixed_image.data() + export_dir->AddressOfFunctions);
        auto* names = (PDWORD)(fixed_image.data() + export_dir->AddressOfNames);
        auto* ordinals = (PWORD)(fixed_image.data() + export_dir->AddressOfNameOrdinals);

        for (DWORD i = 0; i < export_dir->NumberOfNames; i++) {
            const char* name = (const char*)(fixed_image.data() + names[i]);
            if (strcmp(name, "BloxHubInit") == 0) {
                bloxhub_init_addr = (uintptr_t)remote_image + functions[ordinals[i]];
                break;
            }
        }
    }

    if (!bloxhub_init_addr) {
        printf("[!] BloxHubInit export not found in DLL!\n");
        VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    // Execute BloxHubInit via CreateRemoteThread
    printf("[*] Calling BloxHubInit at 0x%llX\n", (unsigned long long)bloxhub_init_addr);
    HANDLE thread = CreateRemoteThread(process, nullptr, 0,
        (LPTHREAD_START_ROUTINE)bloxhub_init_addr, nullptr, 0, nullptr);
    if (thread) {
        WaitForSingleObject(thread, 5000);
        CloseHandle(thread);
        printf("[+] BloxHubInit executed successfully!\n");
    } else {
        printf("[!] CreateRemoteThread failed: %lu\n", GetLastError());
        VirtualFreeEx(process, (void*)remote_image, 0, MEM_RELEASE);
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    CloseHandle(process);
    return InjectionStatus::SUCCESS;
}
