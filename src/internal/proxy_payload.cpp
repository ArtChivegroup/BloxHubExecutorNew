#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

namespace {

void WriteLog(const char* message)
{
    char temp_path[MAX_PATH] = {};
    GetTempPathA(MAX_PATH, temp_path);
    strcat_s(temp_path, "\\bloxhub_test.txt");

    FILE* file = nullptr;
    fopen_s(&file, temp_path, "a");
    if (!file) return;
    fprintf(file, "[BloxHub] %s\n", message);
    fclose(file);
}

void WriteVerifyMarker(HMODULE module)
{
    wchar_t my_path[MAX_PATH] = {};
    GetModuleFileNameW(module, my_path, MAX_PATH);
    fs::path dll_path(my_path);
    fs::path marker = dll_path.parent_path() / L"bloxhub_loaded.txt";

    HANDLE hFile = CreateFileW(
        marker.wstring().c_str(),
        GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        const char* msg = "BloxHub payload loaded";
        DWORD written = 0;
        WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
        CloseHandle(hFile);
    }
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
    if (!VirtualProtect(module, 0x1000, PAGE_READWRITE, &old_protect)) return;
    SecureZeroMemory(module, 0x1000);
    DWORD ignored = 0;
    VirtualProtect(module, 0x1000, old_protect, &ignored);
}

HMODULE g_forward_module = nullptr;

wchar_t* GetForwardDllName(HMODULE module)
{
    static wchar_t forward_name[MAX_PATH] = {};
    if (forward_name[0]) return forward_name;

    wchar_t my_path[MAX_PATH] = {};
    GetModuleFileNameW(module, my_path, MAX_PATH);
    fs::path dll_path(my_path);
    std::wstring stem = dll_path.stem().wstring();
    std::wstring ext = dll_path.extension().wstring();
    std::wstring fwd = stem + L"_orig" + ext;

    wcscpy_s(forward_name, fwd.c_str());
    return forward_name;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    WriteLog("proxy payload: DllMain started!");

    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;

    DisableThreadLibraryCalls(module);

    wchar_t* forward_dll = GetForwardDllName(module);
    char forward_info[512];
    sprintf_s(forward_info, "Loading forward DLL: %ls", forward_dll);
    WriteLog(forward_info);

    g_forward_module = LoadLibraryW(forward_dll);
    if (!g_forward_module)
    {
        char err_msg[256];
        sprintf_s(err_msg, "Failed to load %ls! Error: %lu", forward_dll, GetLastError());
        WriteLog(err_msg);
        return FALSE;
    }
    WriteLog("Forward DLL loaded successfully!");

    if (IsRobloxProcess())
    {
        WriteLog("Proxy payload: Loaded into Roblox!");
        WriteVerifyMarker(module);
        WipePeHeader(module);
    }
    else
    {
        WriteLog("Proxy payload: Loaded outside Roblox.");
    }
    return TRUE;
}
