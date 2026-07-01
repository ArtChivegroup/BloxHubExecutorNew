#include <Windows.h>

static void DebugConsoleLog(const char* msg)
{
    static bool ready = false;
    if (!ready) {
        AllocConsole();
        ready = true;
    }
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE) return;
    DWORD n = 0;
    WriteConsoleA(h, msg, (DWORD)strlen(msg), &n, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DebugConsoleLog("[BloxHub] DllMain PROCESS_ATTACH\n");
    }
    return TRUE;
}
