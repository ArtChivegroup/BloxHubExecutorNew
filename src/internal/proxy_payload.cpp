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

bool WriteMarkerFile(const wchar_t* path)
{
    HANDLE hFile = CreateFileW(
        path, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    const char* msg = "BloxHub payload loaded";
    DWORD written = 0;
    WriteFile(hFile, msg, (DWORD)strlen(msg), &written, NULL);
    CloseHandle(hFile);
    return true;
}

void WriteVerifyMarker(HMODULE module)
{
    wchar_t my_path[MAX_PATH] = {};
    GetModuleFileNameW(module, my_path, MAX_PATH);
    fs::path marker = fs::path(my_path).parent_path() / L"bloxhub_loaded.txt";

    if (WriteMarkerFile(marker.wstring().c_str()))
    {
        char log[512];
        sprintf_s(log, "Verify marker written: %ls", marker.c_str());
        WriteLog(log);
        return;
    }

    wchar_t temp[MAX_PATH] = {};
    GetTempPathW(MAX_PATH, temp);
    fs::path fallback = fs::path(temp) / L"bloxhub_payload_loaded.txt";
    if (WriteMarkerFile(fallback.wstring().c_str()))
    {
        char log[512];
        sprintf_s(log, "Verify marker fallback: %ls", fallback.c_str());
        WriteLog(log);
    }
    else
    {
        WriteLog("Verify marker write failed (both locations)");
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

void LogWin32Error(const char* prefix, DWORD err)
{
    char msg[320];
    sprintf_s(msg, "%s Error: %lu", prefix, err);
    WriteLog(msg);

    wchar_t* buf = nullptr;
    DWORD n = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&buf, 0, NULL);
    if (n && buf)
    {
        char narrow[512] = {};
        WideCharToMultiByte(CP_UTF8, 0, buf, -1, narrow, sizeof(narrow) - 1, NULL, NULL);
        WriteLog(narrow);
        LocalFree(buf);
    }
}

bool GetForwardDllPath(HMODULE module, wchar_t* out, size_t out_chars)
{
    wchar_t my_path[MAX_PATH] = {};
    if (!GetModuleFileNameW(module, my_path, MAX_PATH)) return false;

    fs::path dll_path(my_path);
    fs::path full = dll_path.parent_path() / (dll_path.stem().wstring() + L"_orig" + dll_path.extension().wstring());
    wcsncpy_s(out, out_chars, full.wstring().c_str(), _TRUNCATE);
    return true;
}

HMODULE g_forward_module = nullptr;

struct InitCtx
{
    HMODULE module;
};

DWORD WINAPI InitThread(LPVOID param)
{
    auto* ctx = static_cast<InitCtx*>(param);
    HMODULE module = ctx->module;
    HeapFree(GetProcessHeap(), 0, ctx);

    wchar_t forward_path[MAX_PATH] = {};
    if (!GetForwardDllPath(module, forward_path, MAX_PATH))
    {
        WriteLog("Failed to resolve forward DLL path");
        return 1;
    }

    char forward_info[512];
    sprintf_s(forward_info, "Loading forward DLL: %ls", forward_path);
    WriteLog(forward_info);

    if (GetFileAttributesW(forward_path) == INVALID_FILE_ATTRIBUTES)
    {
        char err_msg[512];
        sprintf_s(err_msg, "Forward DLL missing on disk: %ls", forward_path);
        WriteLog(err_msg);
        LogWin32Error("GetFileAttributes", GetLastError());
        return 1;
    }

    g_forward_module = LoadLibraryExW(forward_path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (!g_forward_module)
    {
        char err_msg[512];
        sprintf_s(err_msg, "Failed to load %ls", forward_path);
        WriteLog(err_msg);
        LogWin32Error("LoadLibraryEx", GetLastError());
        return 1;
    }

    WriteLog("Forward DLL loaded successfully!");
    WipePeHeader(module);
    return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID)
{
    if (reason != DLL_PROCESS_ATTACH)
        return TRUE;

    DisableThreadLibraryCalls(module);
    WriteLog("proxy payload: DllMain started!");

    if (!IsRobloxProcess())
    {
        WriteLog("Proxy payload: Loaded outside Roblox.");
        return TRUE;
    }

    WriteLog("Proxy payload: Loaded into Roblox!");
    WriteVerifyMarker(module);

    auto* ctx = static_cast<InitCtx*>(HeapAlloc(GetProcessHeap(), 0, sizeof(InitCtx)));
    if (!ctx)
    {
        WriteLog("Init thread alloc failed");
        return TRUE;
    }
    ctx->module = module;

    HANDLE hThread = CreateThread(NULL, 0, InitThread, ctx, 0, NULL);
    if (!hThread)
    {
        WriteLog("Init thread create failed");
        HeapFree(GetProcessHeap(), 0, ctx);
        return TRUE;
    }
    CloseHandle(hThread);
    return TRUE;
}
