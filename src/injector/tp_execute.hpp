#pragma once
#include <Windows.h>
#include <cstddef>
#include <cstdint>

namespace injector {

bool TpExecuteShellcode(DWORD pid, HANDLE process, const void* shellcode, size_t size);
// ponytail: DllMain pakai sync thread — IoCompletion async, proses bisa crash sebelum kita tahu
bool TpExecuteShellcodeSync(HANDLE process, const void* shellcode, size_t size);

} // namespace injector
