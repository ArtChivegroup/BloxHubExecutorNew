#include "injector.hpp"
#include "nt_util.hpp"
#include "tp_execute.hpp"
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

constexpr const char* kStompDll = "C:\\Windows\\System32\\d3d10warp.dll";

bool LoadFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return false;
    auto sz = f.tellg();
    f.seekg(0);
    out.resize(static_cast<size_t>(sz));
    f.read(reinterpret_cast<char*>(out.data()), sz);
    return f.good();
}

uintptr_t GetModuleBase(HANDLE process, const wchar_t* name) {
    HMODULE mods[1024];
    DWORD needed = 0;
    if (!EnumProcessModules(process, mods, sizeof(mods), &needed)) return 0;
    for (unsigned i = 0; i < needed / sizeof(HMODULE); i++) {
        wchar_t path[MAX_PATH]{};
        if (GetModuleFileNameExW(process, mods[i], path, MAX_PATH) && wcsstr(path, name))
            return reinterpret_cast<uintptr_t>(mods[i]);
    }
    return 0;
}

bool WaitForHyperion(HANDLE process, uintptr_t* outBase) {
    for (int i = 0; i < 60; i++) {
        uintptr_t base = GetModuleBase(process, L"RobloxPlayerBeta.dll");
        if (base) {
            *outBase = base;
            return true;
        }
        Sleep(50);
    }
    return false;
}

// ponytail: default d3d10warp; switch to mshtml.dll if payload SizeOfImage exceeds stomp target
uintptr_t MapStompModule(HANDLE process, const char* dllPath) {
    HANDLE file = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        printf("[!] CreateFile stomp DLL failed: %lu\n", GetLastError());
        return 0;
    }

    BYTE hdr[4096]{};
    DWORD rd = 0;
    if (!ReadFile(file, hdr, sizeof(hdr), &rd, nullptr)) {
        CloseHandle(file);
        return 0;
    }

    auto* dos = reinterpret_cast<PIMAGE_DOS_HEADER>(hdr);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) { CloseHandle(file); return 0; }
    auto* nt = reinterpret_cast<PIMAGE_NT_HEADERS64>(hdr + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE) { CloseHandle(file); return 0; }

    auto createSec = (nt::FnNtCreateSection)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtCreateSection");
    auto mapView = (nt::FnNtMapViewOfSection)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtMapViewOfSection");
    auto ntClose = (nt::FnNtClose)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtClose");
    if (!createSec || !mapView || !ntClose) { CloseHandle(file); return 0; }

    LARGE_INTEGER secSize{};
    secSize.QuadPart = nt->OptionalHeader.SizeOfImage;
    HANDLE section = nullptr;
    NTSTATUS st = createSec(&section, SECTION_ALL_ACCESS, nullptr, &secSize, PAGE_READONLY, SEC_IMAGE, file);
    CloseHandle(file);
    if (!nt::Ok(st)) {
        printf("[!] NtCreateSection failed: 0x%08lX\n", (unsigned long)st);
        return 0;
    }

    PVOID remote = nullptr;
    SIZE_T viewSize = 0;
    st = mapView(section, process, &remote, 0, 0, nullptr, &viewSize, 2 /*ViewUnmap*/, 0, PAGE_READONLY);
    ntClose(section);
    if (!nt::Ok(st)) {
        printf("[!] NtMapViewOfSection failed: 0x%08lX\n", (unsigned long)st);
        return 0;
    }

    auto* secHdr = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        PVOID secBase = reinterpret_cast<PVOID>(reinterpret_cast<uintptr_t>(remote) + secHdr[i].VirtualAddress);
        SIZE_T secSz = secHdr[i].Misc.VirtualSize;
        SYSTEM_INFO si{};
        GetSystemInfo(&si);
        SIZE_T page = si.dwPageSize;
        secSz = (secSz + page - 1) & ~(page - 1);
        nt::Protect(process, secBase, secSz, PAGE_EXECUTE_READWRITE);
    }
    PVOID hdrBase = remote;
    SIZE_T hdrSz = nt->OptionalHeader.SizeOfHeaders;
    nt::Protect(process, hdrBase, hdrSz, PAGE_EXECUTE_READWRITE);

    printf("[*] Stomp module mapped at 0x%llX (%s)\n", (unsigned long long)reinterpret_cast<uintptr_t>(remote), dllPath);
    return reinterpret_cast<uintptr_t>(remote);
}

void ApplyRelocs(BYTE* image, PIMAGE_NT_HEADERS nt, int64_t delta) {
    auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (!dir.VirtualAddress || !dir.Size) return;
    auto* block = reinterpret_cast<PIMAGE_BASE_RELOCATION>(image + dir.VirtualAddress);
    auto* end = reinterpret_cast<BYTE*>(block) + dir.Size;
    while (reinterpret_cast<BYTE*>(block) < end && block->SizeOfBlock) {
        UINT count = (block->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        auto* entry = reinterpret_cast<WORD*>(block + 1);
        for (UINT i = 0; i < count; i++) {
            if ((entry[i] >> 12) == IMAGE_REL_BASED_DIR64) {
                auto* patch = reinterpret_cast<uint64_t*>(image + block->VirtualAddress + (entry[i] & 0xFFF));
                *patch += static_cast<uint64_t>(delta);
            }
        }
        block = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<BYTE*>(block) + block->SizeOfBlock);
    }
}

bool ApplyImports(BYTE* image, PIMAGE_NT_HEADERS nt) {
    auto& dir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.VirtualAddress || !dir.Size) return true;
    auto* imp = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(image + dir.VirtualAddress);
    while (imp->Name) {
        HMODULE mod = LoadLibraryA(reinterpret_cast<const char*>(image + imp->Name));
        if (!mod) {
            printf("[!] Import load failed: %s\n", reinterpret_cast<const char*>(image + imp->Name));
            return false;
        }
        auto* thunk = reinterpret_cast<PIMAGE_THUNK_DATA64>(image + imp->FirstThunk);
        auto* orig = imp->OriginalFirstThunk
            ? reinterpret_cast<PIMAGE_THUNK_DATA64>(image + imp->OriginalFirstThunk) : thunk;
        while (orig->u1.AddressOfData) {
            FARPROC fn = nullptr;
            if (IMAGE_SNAP_BY_ORDINAL64(orig->u1.Ordinal))
                fn = GetProcAddress(mod, MAKEINTRESOURCEA(IMAGE_ORDINAL64(orig->u1.Ordinal)));
            else {
                auto* byName = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(image + orig->u1.AddressOfData);
                fn = GetProcAddress(mod, byName->Name);
            }
            if (!fn) {
                printf("[!] GetProcAddress failed for import\n");
                return false;
            }
            thunk->u1.Function = reinterpret_cast<ULONGLONG>(fn);
            thunk++;
            orig++;
        }
        imp++;
    }
    return true;
}

bool RunDllMainShellcode(HANDLE process, uintptr_t remoteBase, uintptr_t entry) {
    BYTE sc[] = {
        0x48, 0x83, 0xEC, 0x28,
        0x48, 0xB9, 0, 0, 0, 0, 0, 0, 0, 0,
        0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
        0x4D, 0x31, 0xC0,
        0x48, 0xB8, 0, 0, 0, 0, 0, 0, 0, 0,
        0xFF, 0xD0,
        0x48, 0x83, 0xC4, 0x28,
        0xC3
    };
    memcpy(&sc[6], &remoteBase, sizeof(remoteBase));
    memcpy(&sc[26], &entry, sizeof(entry));
    return injector::TpExecuteShellcodeSync(process, sc, sizeof(sc));
}

bool StompMap(DWORD pid, HANDLE process, const std::string& dllPath, uintptr_t remoteBase) {
    std::vector<uint8_t> raw;
    if (!LoadFile(dllPath, raw)) {
        printf("[!] Failed to read payload: %s\n", dllPath.c_str());
        return false;
    }

    auto* dos = reinterpret_cast<PIMAGE_DOS_HEADER>(raw.data());
    auto* nt = reinterpret_cast<PIMAGE_NT_HEADERS>(raw.data() + dos->e_lfanew);
    DWORD imgSize = nt->OptionalHeader.SizeOfImage;

    auto mapped = std::make_unique<BYTE[]>(imgSize);
    memset(mapped.get(), 0, imgSize);
    memcpy(mapped.get(), raw.data(), nt->OptionalHeader.SizeOfHeaders);

    auto* sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (!sec[i].SizeOfRawData) continue;
        memcpy(mapped.get() + sec[i].VirtualAddress, raw.data() + sec[i].PointerToRawData, sec[i].SizeOfRawData);
    }

    auto* mNt = reinterpret_cast<PIMAGE_NT_HEADERS>(mapped.get() +
        reinterpret_cast<PIMAGE_DOS_HEADER>(mapped.get())->e_lfanew);
    int64_t delta = static_cast<int64_t>(remoteBase) - static_cast<int64_t>(mNt->OptionalHeader.ImageBase);
    ApplyRelocs(mapped.get(), mNt, delta);
    mNt->OptionalHeader.ImageBase = remoteBase;

    if (!ApplyImports(mapped.get(), mNt)) return false;

    if (!nt::Write(process, reinterpret_cast<PVOID>(remoteBase), mapped.get(), imgSize)) {
        printf("[!] Failed to write stomped image\n");
        return false;
    }
    printf("[*] Payload written to stomp region (%u bytes)\n", imgSize);

    // ponytail: skip SEH/TLS — CRT TLS + RtlAddFunctionTable dari injector address = crash Roblox
    // Re-enable when payload needs exceptions and remote ntdll resolve is added.

    uintptr_t entry = remoteBase + nt->OptionalHeader.AddressOfEntryPoint;
    printf("[*] Calling DllMain at 0x%llX (sync thread)\n", (unsigned long long)entry);
    if (!RunDllMainShellcode(process, remoteBase, entry)) {
        printf("[!] DllMain dispatch failed\n");
        return false;
    }
    printf("[+] DllMain returned\n");

    sec = IMAGE_FIRST_SECTION(nt);
    for (WORD i = 0; i < nt->FileHeader.NumberOfSections; i++) {
        if (!sec[i].Misc.VirtualSize) continue;
        PVOID secBase = reinterpret_cast<PVOID>(remoteBase + sec[i].VirtualAddress);
        SIZE_T secSz = sec[i].Misc.VirtualSize;
        DWORD ch = sec[i].Characteristics;
        DWORD prot = PAGE_READONLY;
        if (ch & IMAGE_SCN_MEM_EXECUTE)
            prot = (ch & IMAGE_SCN_MEM_WRITE) ? PAGE_EXECUTE_READWRITE : PAGE_EXECUTE_READ;
        else if (ch & IMAGE_SCN_MEM_WRITE)
            prot = PAGE_READWRITE;
        nt::Protect(process, secBase, secSz, prot);
    }
    return true;
}

} // namespace

injector::InjectionStatus injector::Inject(uint32_t pid, const std::string& dll_path) {
    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!process) {
        printf("[!] OpenProcess(pid=%u) failed: %lu\n", pid, GetLastError());
        return InjectionStatus::FAILED;
    }

    std::vector<uint8_t> pe;
    if (!LoadFile(dll_path, pe)) {
        printf("[!] Failed to read DLL: %s\n", dll_path.c_str());
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    auto* dos = reinterpret_cast<PIMAGE_DOS_HEADER>(pe.data());
    auto* nt = reinterpret_cast<PIMAGE_NT_HEADERS>(pe.data() + dos->e_lfanew);
    DWORD payloadSize = nt->OptionalHeader.SizeOfImage;

    uintptr_t stompBase = MapStompModule(process, kStompDll);
    if (!stompBase) {
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    IMAGE_DOS_HEADER stompDos{};
    IMAGE_NT_HEADERS64 stompNt{};
    if (nt::Read(process, reinterpret_cast<PVOID>(stompBase), &stompDos, sizeof(stompDos)) &&
        nt::Read(process, reinterpret_cast<PVOID>(stompBase + stompDos.e_lfanew), &stompNt, sizeof(stompNt))) {
        if (stompNt.OptionalHeader.SizeOfImage < payloadSize) {
            printf("[!] Stomp target too small: need 0x%X, have 0x%X\n",
                payloadSize, stompNt.OptionalHeader.SizeOfImage);
            CloseHandle(process);
            return InjectionStatus::FAILED;
        }
    }

    uintptr_t hyperionBase = 0;
    if (!WaitForHyperion(process, &hyperionBase)) {
        printf("[!] RobloxPlayerBeta.dll not loaded in target\n");
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }
    printf("[*] RobloxPlayerBeta.dll base: 0x%llX\n", (unsigned long long)hyperionBase);

    if (!StompMap(pid, process, dll_path, stompBase)) {
        CloseHandle(process);
        return InjectionStatus::FAILED;
    }

    DWORD exitCode = 0;
    if (GetExitCodeProcess(process, &exitCode) && exitCode != STILL_ACTIVE)
        printf("[!] Target process exited after inject (code %lu)\n", exitCode);

    CloseHandle(process);
    return InjectionStatus::SUCCESS;
}
