#include <Windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    (void)hModule;
    (void)lpReserved;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        OutputDebugStringA("[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!");

        // Step 4: Tulis file ke C:\test_bloxhub.txt
        HANDLE hFile = CreateFileA(
            "C:\\test_bloxhub.txt",
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );
        if (hFile != INVALID_HANDLE_VALUE) {
            const char* msg = "hello from BloxHub!";
            DWORD bytesWritten = 0;
            WriteFile(hFile, msg, static_cast<DWORD>(strlen(msg)), &bytesWritten, nullptr);
            CloseHandle(hFile);
            OutputDebugStringA("[BloxHub] File written to C:\\test_bloxhub.txt!");
        } else {
            OutputDebugStringA("[BloxHub] Failed to create file!");
        }
    }
    return TRUE;
}
