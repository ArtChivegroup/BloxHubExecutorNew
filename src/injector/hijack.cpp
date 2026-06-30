#include "injector.hpp"
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

// Helper to get module base in target process
static uintptr_t get_module_base_in_target(uint32_t pid, const std::string& module_name) {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);
    if (hSnap == INVALID_HANDLE_VALUE) return 0;

    MODULEENTRY32 me32;
    me32.dwSize = sizeof(MODULEENTRY32);

    uintptr_t base = 0;
    if (Module32First(hSnap, &me32)) {
        do {
            if (_stricmp(me32.szModule, module_name.c_str()) == 0) {
                base = (uintptr_t)me32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnap, &me32));
    }

    CloseHandle(hSnap);
    return base;
}

bool injector::stomp_hijack(uint32_t pid, const ManualMapCtx& ctx) {
    // Open process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!hProcess) return false;

    // Find thread
    HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (hThreadSnap == INVALID_HANDLE_VALUE) {
        CloseHandle(hProcess);
        return false;
    }

    THREADENTRY32 te32;
    te32.dwSize = sizeof(THREADENTRY32);

    uint32_t target_tid = 0;
    if (Thread32First(hThreadSnap, &te32)) {
        do {
            if (te32.th32OwnerProcessID == pid) {
                target_tid = te32.th32ThreadID;
                break;
            }
        } while (Thread32Next(hThreadSnap, &te32));
    }

    CloseHandle(hThreadSnap);

    if (!target_tid) {
        CloseHandle(hProcess);
        return false;
    }

    // Open thread
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, target_tid);
    if (!hThread) {
        CloseHandle(hProcess);
        return false;
    }

    // Suspend thread
    SuspendThread(hThread);

    // Get thread context
    CONTEXT ctx64;
    ctx64.ContextFlags = CONTEXT_FULL;
    GetThreadContext(hThread, &ctx64);

    // Simple shellcode to call BloxHubEntry
    std::vector<uint8_t> shellcode;

    // mov rax, bloxhub_entry
    shellcode.push_back(0x48);
    shellcode.push_back(0xB8);
    uintptr_t entry_addr = ctx.bloxhub_entry;
    shellcode.insert(shellcode.end(), (uint8_t*)&entry_addr, (uint8_t*)&entry_addr + 8);

    // call rax
    shellcode.push_back(0xFF);
    shellcode.push_back(0xD0);

    // mov rax, original_rip
    shellcode.push_back(0x48);
    shellcode.push_back(0xB8);
    uintptr_t orig_rip = ctx64.Rip;
    shellcode.insert(shellcode.end(), (uint8_t*)&orig_rip, (uint8_t*)&orig_rip + 8);

    // jmp rax
    shellcode.push_back(0xFF);
    shellcode.push_back(0xE0);

    // Allocate memory for shellcode in target process (better than using combase.dll tail)
    SIZE_T shellcode_size = shellcode.size();
    PVOID remote_shellcode = VirtualAllocEx(hProcess, NULL, shellcode_size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remote_shellcode) {
        ResumeThread(hThread);
        CloseHandle(hThread);
        CloseHandle(hProcess);
        return false;
    }

    // Write shellcode
    WriteProcessMemory(hProcess, remote_shellcode, shellcode.data(), shellcode_size, NULL);

    // Set thread RIP to shellcode
    ctx64.Rip = (uintptr_t)remote_shellcode;
    SetThreadContext(hThread, &ctx64);

    // Resume thread
    ResumeThread(hThread);

    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}