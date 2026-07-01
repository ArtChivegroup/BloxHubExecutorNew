#include <Windows.h>

static void WriteVerifyMarker()
{
    char path[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, path);
    strcat_s(path, "bloxhub_payload_loaded.txt");

    HANDLE hFile = CreateFileA(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return;

    const char* msg = "BloxHub injected";
    DWORD written = 0;
    WriteFile(hFile, msg, (DWORD)strlen(msg), &written, nullptr);
    CloseHandle(hFile);
}

void WriteLog(const char* message) {
    char tempPath[MAX_PATH];
    char logPath[MAX_PATH];
    
    // Dapatkan temp path dan build log path
    GetTempPathA(MAX_PATH, tempPath);
    // Build path manually (tanpa PathCombineA untuk menghindari dependensi Shlwapi)
    strcpy_s(logPath, tempPath);
    strcat_s(logPath, "bloxhub_test.txt");
    
    // Buka file (append mode)
    HANDLE hFile = CreateFileA(logPath, FILE_APPEND_DATA, 0, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile != INVALID_HANDLE_VALUE) {
        SetFilePointer(hFile, 0, nullptr, FILE_END);
        DWORD written = 0;
        WriteFile(hFile, message, (DWORD)strlen(message), &written, nullptr);
        CloseHandle(hFile);
    }
}

// Export BloxHubInit untuk diinvoke dari injector
extern "C" __declspec(dllexport) void BloxHubInit() {
    WriteLog("[BloxHub] Injected successfully!\n");
    WriteLog("[BloxHub] Timestamp: " __DATE__ " " __TIME__ "\n");
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            WriteLog("[BloxHub] DLL_PROCESS_ATTACH called!\n");
            WriteVerifyMarker();
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}
