#include <Windows.h>
#include <cstring>

static void WriteLogLine(HANDLE hFile, const char* line)
{
    DWORD bytesWritten = 0;
    WriteFile(hFile, line, static_cast<DWORD>(strlen(line)), &bytesWritten, nullptr);
    WriteFile(hFile, "\r\n", 2, &bytesWritten, nullptr);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    (void)hModule;
    (void)lpReserved;

    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        OutputDebugStringA("[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!");

        // Step 6: Create directory C:\BloxHub if it doesn't exist
        CreateDirectoryA("C:\\BloxHub", nullptr);

        // Write log multi-baris ke C:\BloxHub\test.txt
        HANDLE hFile = CreateFileA(
            "C:\\BloxHub\\test.txt",
            GENERIC_WRITE,
            0,
            nullptr,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            nullptr);
        if (hFile != INVALID_HANDLE_VALUE)
        {
            // Log 1: init start
            OutputDebugStringA("[BloxHub] init start");
            WriteLogLine(hFile, "init start");

            // Log 2: init mid
            OutputDebugStringA("[BloxHub] init mid");
            WriteLogLine(hFile, "init mid");

            // Log 3: init end
            OutputDebugStringA("[BloxHub] init end");
            WriteLogLine(hFile, "init end");

            CloseHandle(hFile);
            OutputDebugStringA("[BloxHub] All logs written to C:\\BloxHub\\test.txt!");
        }
        else
        {
            OutputDebugStringA("[BloxHub] Failed to create file!");
        }
    }
    return TRUE;
}
