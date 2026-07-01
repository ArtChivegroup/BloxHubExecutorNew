#include "cfg_bypass.h"
#include <cstdio>
#include <Psapi.h>

bool PatchCfgBitmap(HANDLE process, uintptr_t bitmap_addr, uintptr_t regionBase, DWORD regionSize)
{
    if (!bitmap_addr || !regionBase || !regionSize)
        return false;

    uintptr_t bitmap_ptr = 0;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(process, (LPCVOID)bitmap_addr, &bitmap_ptr, sizeof(bitmap_ptr), &bytesRead) || !bitmap_ptr)
    {
        printf("[!] CFG: Failed to read bitmap pointer at 0x%llX\n", (unsigned long long)bitmap_addr);
        return false;
    }
    printf("[*] CFG: Bitmap pointer = 0x%llX\n", (unsigned long long)bitmap_ptr);

    uintptr_t aligned_start = regionBase & ~0xFFFFULL;
    uintptr_t aligned_end = (regionBase + regionSize + 0xFFFFULL) & ~0xFFFFULL;

    for (uintptr_t pg = aligned_start; pg < aligned_end; pg += 0x1000)
    {
        uintptr_t dword_offset = bitmap_ptr + (pg >> 0x13);
        uint32_t current = 0;

        if (!ReadProcessMemory(process, (LPCVOID)dword_offset, &current, sizeof(current), &bytesRead))
            continue;

        uint32_t bit = 1 << ((pg >> 0x10 & 7) % 0x20);
        if (current & bit)
            continue;

        uint32_t updated = current | bit;
        WriteProcessMemory(process, (LPVOID)dword_offset, &updated, sizeof(updated), nullptr);
    }

    printf("[+] CFG: Bitmap patched for region 0x%llX - 0x%llX\n",
        (unsigned long long)regionBase, (unsigned long long)(regionBase + regionSize));
    return true;
}

static bool ExecuteShellcode(HANDLE process, const uint8_t* code, SIZE_T codeSize)
{
    LPVOID remoteCode = VirtualAllocEx(process, nullptr, codeSize,
        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteCode) return false;

    SIZE_T written = 0;
    if (!WriteProcessMemory(process, remoteCode, code, codeSize, &written)) {
        VirtualFreeEx(process, remoteCode, 0, MEM_RELEASE);
        return false;
    }

    HANDLE hThread = CreateRemoteThread(process, nullptr, 0,
        (LPTHREAD_START_ROUTINE)remoteCode, nullptr, 0, nullptr);
    if (!hThread) {
        VirtualFreeEx(process, remoteCode, 0, MEM_RELEASE);
        return false;
    }

    WaitForSingleObject(hThread, 10000);
    CloseHandle(hThread);
    VirtualFreeEx(process, remoteCode, 0, MEM_RELEASE);
    return true;
}

bool BypassCfgForRegion(HANDLE process, const CfgOffsets& offsets, uintptr_t regionBase, DWORD regionSize)
{
    if (!offsets.valid)
    {
        printf("[!] CFG: Offsets not valid, skipping whitelist\n");
        return PatchCfgBitmap(process, offsets.bitmap_ptr, regionBase, regionSize);
    }

    bool ok = true;

    // Layer 1: Patch bitmap directly (fast path)
    ok &= PatchCfgBitmap(process, offsets.bitmap_ptr, regionBase, regionSize);

    // Layer 2: Whitelist via shellcode (insert_set for each page)
    if (offsets.whitelist && offsets.insert_set)
    {
        uintptr_t aligned_start = regionBase & ~0xFFFULL;
        uintptr_t aligned_end = (regionBase + regionSize + 0xFFFULL) & ~0xFFFULL;

        for (uintptr_t pg = aligned_start; pg < aligned_end; pg += 0x1000)
        {
            uint8_t shellcode[256] = {};
            uint8_t* p = shellcode;

            uint32_t hash = HashPage(pg);
            uint8_t validation = ValidationByte(pg);

            // Build whitelist entry on stack and call insert_set
            // sub rsp, 0x40              ; alloc stack space for entry (0x40 bytes)
            *(p++) = 0x48; *(p++) = 0x83; *(p++) = 0xEC; *(p++) = 0x40;

            // Write hash at [rsp+0x18]
            // mov dword ptr [rsp+0x18], hash
            *(p++) = 0xC7; *(p++) = 0x44; *(p++) = 0x24; *(p++) = 0x18;
            *(uint32_t*)p = hash; p += 4;

            // Write validation at [rsp+0x1C]
            // mov byte ptr [rsp+0x1C], validation
            *(p++) = 0xC6; *(p++) = 0x44; *(p++) = 0x24; *(p++) = 0x1C;
            *(p++) = validation;

            // mov rcx, whitelist        ; arg1: whitelist address
            *(p++) = 0x48; *(p++) = 0xB9;
            *(uint64_t*)p = offsets.whitelist; p += 8;

            // lea rdx, [rsp+0x14]       ; arg2: pointer to entry start
            *(p++) = 0x48; *(p++) = 0x8D; *(p++) = 0x54; *(p++) = 0x24; *(p++) = 0x14;

            // lea r8, [rsp+0x18]        ; arg3: pointer to hash
            *(p++) = 0x4C; *(p++) = 0x8D; *(p++) = 0x44; *(p++) = 0x24; *(p++) = 0x18;

            // mov rax, insert_set
            *(p++) = 0x48; *(p++) = 0xB8;
            *(uint64_t*)p = offsets.insert_set; p += 8;

            // call rax
            *(p++) = 0xFF; *(p++) = 0xD0;

            // add rsp, 0x40
            *(p++) = 0x48; *(p++) = 0x83; *(p++) = 0xC4; *(p++) = 0x40;

            // ret
            *(p++) = 0xC3;

            SIZE_T codeSize = p - shellcode;
            if (!ExecuteShellcode(process, shellcode, codeSize))
            {
                printf("[!] CFG: Shellcode exec failed for page 0x%llX\n", (unsigned long long)pg);
                ok = false;
                break;
            }
        }
        printf("[+] CFG: Whitelist complete for %llu pages\n",
            (unsigned long long)((aligned_end - aligned_start) / 0x1000));
    }

    return ok;
}

CfgOffsets FindCfgOffsets(HANDLE process, uintptr_t moduleBase, DWORD moduleSize)
{
    CfgOffsets result = {};
    result.valid = false;

    MODULEINFO mi;
    if (!GetModuleInformation(process, (HMODULE)moduleBase, &mi, sizeof(mi)))
        return result;

    uintptr_t endAddr = moduleBase + moduleSize;

    // The bitmap pointer and whitelist are static globals in RobloxPlayerBeta.dll
    // They're in .rdata section. We use ReadProcessMemory to scan for known patterns.
    // For now, return the bitmap patching capability (always works if we can find bitmap)
    // Offset from RBX-cfg-bypass for reference: Bitmap=0x2B74D0, Whitelist=0x29A710, InsertSet=0xBFD2C0
    
    // Try to use the offsets from offsets.hpp if they're populated
    // Pattern: bitmap ptr usually at fixed RVA in .rdata section
    // We'll rely on user-provided offsets for now

    return result;
}
