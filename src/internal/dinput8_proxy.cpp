#include <Windows.h>
#include <cstdio>
#include <cstring>
#include "dinput8_exports.generated.hpp"

namespace {

void WriteLog(const char* message) {
    char temp_path[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, temp_path);
    strcat_s(temp_path, "\\bloxhub_test.txt");

    FILE* file = nullptr;
    fopen_s(&file, temp_path, "a");
    if (!file) {
        return;
    }

    std::fprintf(file, "[BloxHub] %s\n", message);
    std::fclose(file);
}

bool IsRobloxProcess() {
    char process_name[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, process_name, MAX_PATH);
    return std::strstr(process_name, "RobloxPlayerBeta.exe") != nullptr;
}

void WipePeHeader(HMODULE module) {
    DWORD old_protect = 0;
    if (!VirtualProtect(module, 0x1000, PAGE_READWRITE, &old_protect)) {
        return;
    }

    SecureZeroMemory(module, 0x1000);
    DWORD ignored = 0;
    VirtualProtect(module, 0x1000, old_protect, &ignored);
}

HMODULE g_dinput8_orig = nullptr;

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    // Write log FIRST to confirm DllMain is called!
    WriteLog("dinput8.dll proxy: DllMain started!");

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(module);

    // Load dinput8_orig.dll ONLY (no fallback!)
    g_dinput8_orig = LoadLibraryA("dinput8_orig.dll");
    if (!g_dinput8_orig) {
        char err_msg[256];
        sprintf_s(err_msg, "Failed to load dinput8_orig.dll! Error: %lu", GetLastError());
        WriteLog(err_msg);
        return FALSE;
    }
    WriteLog("dinput8_orig.dll loaded successfully!");

    if (!IsRobloxProcess()) {
        WriteLog("dinput8.dll EXACT PROXY loaded outside Roblox.");
        return TRUE;
    }

    WriteLog("dinput8.dll EXACT PROXY loaded into Roblox!");
    WipePeHeader(module);
    return TRUE;
}
