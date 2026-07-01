#include <Windows.h>
#include <cstdio>
#include <cstring>

namespace {

void WriteLog(const char* message)
{
    char temp_path[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, temp_path);
    strcat_s(temp_path, "\\bloxhub_test.txt");

    FILE* file = nullptr;
    fopen_s(&file, temp_path, "a");
    if (!file) {
        return;
    }

    fprintf(file, "[BloxHub] %s\n", message);
    fclose(file);
}

bool IsRobloxProcess()
{
    char process_name[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, process_name, MAX_PATH);
    return strstr(process_name, "RobloxPlayerBeta.exe") != nullptr;
}

void WipePeHeader(HMODULE module)
{
    DWORD old_protect = 0;
    if (!VirtualProtect(module, 0x1000, PAGE_READWRITE, &old_protect)) {
        return;
    }

    SecureZeroMemory(module, 0x1000);
    DWORD ignored = 0;
    VirtualProtect(module, 0x1000, old_protect, &ignored);
}

HMODULE g_d3d9_orig = nullptr;

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    WriteLog("d3d9.dll proxy: DllMain started!");

    if (reason != DLL_PROCESS_ATTACH) {
        return TRUE;
    }

    DisableThreadLibraryCalls(module);

    g_d3d9_orig = LoadLibraryA("d3d9_orig.dll");
    if (!g_d3d9_orig) {
        char err_msg[256];
        sprintf_s(err_msg, "Failed to load d3d9_orig.dll! Error: %lu", GetLastError());
        WriteLog(err_msg);
        return FALSE;
    }
    WriteLog("d3d9_orig.dll loaded successfully!");

    if (IsRobloxProcess()) {
        WriteLog("d3d9.dll proxy: Loaded into Roblox!");
        WipePeHeader(module);
    } else {
        WriteLog("d3d9.dll proxy: Loaded outside Roblox.");
    }
    return TRUE;
}
