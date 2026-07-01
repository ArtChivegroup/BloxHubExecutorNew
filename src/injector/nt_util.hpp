#pragma once
#include <Windows.h>
#include <winternl.h>
#include <cstdint>

#ifndef STATUS_INFO_LENGTH_MISMATCH
#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)
#endif

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif

namespace nt {

inline bool Ok(NTSTATUS s) { return NT_SUCCESS(s); }

using FnNtReadVirtualMemory = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
using FnNtWriteVirtualMemory = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, SIZE_T, PSIZE_T);
using FnNtProtectVirtualMemory = NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG);
using FnNtCreateSection = NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE);
using FnNtMapViewOfSection = NTSTATUS(NTAPI*)(HANDLE, HANDLE, PVOID*, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, ULONG, ULONG, ULONG);
using FnNtClose = NTSTATUS(NTAPI*)(HANDLE);
using FnNtQueryInformationProcess = NTSTATUS(NTAPI*)(HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
using FnNtQueryObject = NTSTATUS(NTAPI*)(HANDLE, OBJECT_INFORMATION_CLASS, PVOID, ULONG, PULONG);
using FnZwSetIoCompletion = NTSTATUS(NTAPI*)(HANDLE, PVOID, PVOID, PVOID, ULONG);

struct ProcessHandleEntry {
    HANDLE HandleValue;
    ULONG_PTR NamePointer;
    ULONG_PTR TypePointer;
    ULONG HandleAttributes;
    ULONG GrantedAccess;
    ULONG_PTR Object;
};

struct ProcessHandleSnapshot {
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    ProcessHandleEntry Handles[1];
};

inline bool Read(HANDLE proc, PVOID remote, PVOID buf, SIZE_T size) {
    static auto fn = (FnNtReadVirtualMemory)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtReadVirtualMemory");
    SIZE_T n = 0;
    return fn && Ok(fn(proc, remote, buf, size, &n)) && n == size;
}

inline bool Write(HANDLE proc, PVOID remote, const void* buf, SIZE_T size) {
    static auto fn = (FnNtWriteVirtualMemory)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtWriteVirtualMemory");
    SIZE_T n = 0;
    return fn && Ok(fn(proc, remote, const_cast<PVOID>(buf), size, &n)) && n == size;
}

inline bool Protect(HANDLE proc, PVOID base, SIZE_T& size, ULONG prot, PULONG old = nullptr) {
    static auto fn = (FnNtProtectVirtualMemory)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtProtectVirtualMemory");
    ULONG o = 0;
    PVOID b = base;
    return fn && Ok(fn(proc, &b, &size, prot, old ? old : &o));
}

inline bool UnicodeEqLit(const UNICODE_STRING* u, const wchar_t* lit) {
    if (!u || !u->Buffer || !lit) return false;
    const size_t uw = u->Length / sizeof(wchar_t);
    const size_t lw = wcslen(lit);
    if (uw != lw) return false;
    return _wcsnicmp(u->Buffer, lit, uw) == 0;
}

} // namespace nt
