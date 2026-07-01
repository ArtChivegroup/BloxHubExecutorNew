#include <Windows.h>
#include <cstdio>
#include <cstring>
#include "winmm_exports.generated.hpp"

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

HMODULE g_winmm_orig = nullptr;

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    // Write log FIRST to confirm DllMain is called!
    WriteLog("winmm.dll proxy: DllMain started!");

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(module);

    // Load winmm_orig.dll ONLY (no fallback!)
    g_winmm_orig = LoadLibraryA("winmm_orig.dll");
    if (!g_winmm_orig) {
        char err_msg[256];
        sprintf_s(err_msg, "Failed to load winmm_orig.dll! Error: %lu", GetLastError());
        WriteLog(err_msg);
        return FALSE;
    }
    WriteLog("winmm_orig.dll loaded successfully!");

    if (!IsRobloxProcess()) {
        WriteLog("winmm.dll EXACT PROXY loaded outside Roblox.");
        return TRUE;
    }

    WriteLog("winmm.dll EXACT PROXY loaded into Roblox!");
    WipePeHeader(module);
    return TRUE;
}
