#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <Windows.h>
#include <Shlwapi.h>
#include "internal/pe_patcher.h"

namespace fs = std::filesystem;

static std::string WstrToStr(const std::wstring& ws)
{
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, NULL, 0, NULL, NULL);
    if (len <= 0) return {};
    std::string result(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, &result[0], len, NULL, NULL);
    return result;
}

static std::wstring StrToWstr(const std::string& s)
{
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
    if (len <= 0) return {};
    std::wstring result(len - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &result[0], len);
    return result;
}

enum class SessionState
{
    IDLE,
    PREFLIGHT_OK,
    INSTALLED,
    LAUNCHED,
    VERIFIED,
    RESTORED,
    FAILED
};

struct SessionInfo
{
    SessionState state = SessionState::IDLE;
    std::wstring roblox_dir;
    std::wstring target_dll_name;
    std::wstring payload_path;
    std::wstring system_dll_source;
    std::wstring session_file_path;
    DWORD roblox_pid = 0;

    std::vector<std::wstring> created_files;
    int verify_timeout_sec = 30;

    bool Load(const std::wstring& path)
    {
        session_file_path = path;
        std::ifstream file(path);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line))
        {
            if (line.rfind("STATE=", 0) == 0) state = (SessionState)std::stoi(line.substr(6));
            else if (line.rfind("ROBLOX_DIR=", 0) == 0) roblox_dir = StrToWstr(line.substr(11));
            else if (line.rfind("TARGET_DLL=", 0) == 0) target_dll_name = StrToWstr(line.substr(11));
            else if (line.rfind("PAYLOAD_PATH=", 0) == 0) payload_path = StrToWstr(line.substr(13));
            else if (line.rfind("SYSTEM_DLL=", 0) == 0) system_dll_source = StrToWstr(line.substr(11));
            else if (line.rfind("PID=", 0) == 0) roblox_pid = std::stoul(line.substr(4));
            else if (line.rfind("CREATED_FILE=", 0) == 0) created_files.push_back(StrToWstr(line.substr(13)));
        }
        return true;
    }

    bool Save()
    {
        if (session_file_path.empty()) return false;
        std::ofstream file(session_file_path);
        if (!file.is_open()) return false;

        file << "STATE=" << (int)state << "\n";
        file << "ROBLOX_DIR=" << WstrToStr(roblox_dir) << "\n";
        file << "TARGET_DLL=" << WstrToStr(target_dll_name) << "\n";
        file << "PAYLOAD_PATH=" << WstrToStr(payload_path) << "\n";
        file << "SYSTEM_DLL=" << WstrToStr(system_dll_source) << "\n";
        file << "PID=" << roblox_pid << "\n";
        for (const auto& f : created_files)
            file << "CREATED_FILE=" << WstrToStr(f) << "\n";

        return true;
    }
};

static BOOL WriteFileToDiskW(LPCWSTR pszFileName, const BYTE* pbDataBuffer, DWORD dwDataLength)
{
    HANDLE hFile = CreateFileW(pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    DWORD dwWritten = 0;
    WriteFile(hFile, pbDataBuffer, dwDataLength, &dwWritten, NULL);
    CloseHandle(hFile);
    return (dwWritten == dwDataLength);
}

static bool AttemptDelete(const fs::path& filepath, int maxRetries = 3)
{
    for (int i = 0; i < maxRetries; i++)
    {
        if (!fs::exists(filepath)) return true;
        try
        {
            fs::remove(filepath);
            return true;
        }
        catch (const fs::filesystem_error&)
        {
            if (i < maxRetries - 1) Sleep(1000);
        }
    }
    return !fs::exists(filepath);
}

static std::wstring GetTargetOrigName(const std::wstring& target_dll)
{
    fs::path p(target_dll);
    std::wstring stem = p.stem().wstring();
    return stem + L"_orig" + p.extension().wstring();
}

static bool RunPreflight(SessionInfo& session, const fs::path& roblox_exe, const std::wstring& target_dll_name)
{
    std::cout << "[PREFLIGHT] Validating environment...\n";

    if (!fs::exists(roblox_exe))
    {
        std::cerr << "[!] RobloxPlayerBeta.exe not found at: " << roblox_exe << "\n";
        return false;
    }

    auto roblox_dir = roblox_exe.parent_path();
    session.roblox_dir = roblox_dir.wstring();
    session.target_dll_name = target_dll_name;
    std::wcout << L"[*] Roblox directory: " << roblox_dir.wstring() << L"\n";

    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    fs::path our_dir = fs::path(exe_path).parent_path();

    auto payload = our_dir / "version.dll";
    if (!fs::exists(payload))
    {
        std::cerr << "[!] Payload version.dll not found in loader directory\n";
        return false;
    }
    session.payload_path = payload.wstring();
    std::wcout << L"[*] Payload: " << payload.wstring() << L"\n";

    const wchar_t* windir = _wgetenv(L"WINDIR");
    if (!windir)
    {
        std::cerr << "[!] WINDIR not available\n";
        return false;
    }

    auto system_dll = fs::path(windir) / L"System32" / target_dll_name;
    if (!fs::exists(system_dll))
    {
        std::wcerr << L"[!] System DLL not found: " << system_dll.wstring() << L"\n";
        return false;
    }
    session.system_dll_source = system_dll.wstring();
    std::wcout << L"[*] System DLL: " << system_dll.wstring() << L"\n";

    std::wcout << L"[*] Target DLL: " << target_dll_name << L"\n";

    auto session_file_path = our_dir / L"bloxhub_session.dat";
    session.session_file_path = session_file_path.wstring();

    if (session.Load(session.session_file_path))
    {
        std::wcout << L"[!] Found existing session file (state=" << (int)session.state
                   << L"). Attempting auto-restore from previous session...\n";
        return session.state == SessionState::RESTORED;
    }

    session.state = SessionState::PREFLIGHT_OK;
    session.Save();
    std::cout << "[PREFLIGHT] OK\n";
    return true;
}

static bool RunInstall(SessionInfo& session)
{
    std::cout << "[INSTALL] Installing proxy DLL...\n";

    fs::path roblox_dir(session.roblox_dir);
    fs::path target_dll = roblox_dir / session.target_dll_name;
    fs::path orig_dll = roblox_dir / GetTargetOrigName(session.target_dll_name);

    if (!fs::exists(orig_dll))
    {
        std::wcout << L"[*] Copying " << session.system_dll_source
                   << L" -> " << orig_dll.wstring() << L"\n";
        try
        {
            fs::copy_file(session.system_dll_source, orig_dll, fs::copy_options::overwrite_existing);
        }
        catch (const fs::filesystem_error& e)
        {
            std::cerr << "[!] Copy failed: " << e.what() << "\n";
            return false;
        }
    }
    else
    {
        std::cout << "[*] Original DLL backup already exists, skipping copy\n";
    }

    session.created_files.push_back(orig_dll.wstring());
    std::wstring orig_filename = orig_dll.filename().wstring();

    std::cout << "[*] Generating proxy DLL...\n";
    PBYTE pProxyBuffer = NULL;
    DWORD dwProxySize = 0;
    if (!ConvertPayloadToProxy(
        session.payload_path.c_str(),
        session.system_dll_source.c_str(),
        orig_filename.c_str(),
        &pProxyBuffer,
        &dwProxySize))
    {
        std::cerr << "[!] Proxy generation failed\n";
        return false;
    }

    std::wcout << L"[*] Writing proxy: " << target_dll.wstring() << L" (" << dwProxySize << L" bytes)\n";
    if (!WriteFileToDiskW(target_dll.wstring().c_str(), pProxyBuffer, dwProxySize))
    {
        std::cerr << "[!] Failed to write proxy DLL\n";
        HeapFree(GetProcessHeap(), 0, pProxyBuffer);
        return false;
    }
    HeapFree(GetProcessHeap(), 0, pProxyBuffer);

    session.created_files.push_back(target_dll.wstring());
    session.state = SessionState::INSTALLED;
    session.Save();
    std::cout << "[INSTALL] OK\n";
    return true;
}

static bool RunLaunch(SessionInfo& session, const fs::path& roblox_exe)
{
    std::cout << "[LAUNCH] Starting Roblox...\n";

    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    std::wstring cmd_line = L"\"" + roblox_exe.wstring() + L"\"";

    if (!CreateProcessW(
        roblox_exe.wstring().c_str(),
        NULL,
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        roblox_exe.parent_path().wstring().c_str(),
        &si,
        &pi))
    {
        std::cerr << "[!] Launch failed. Error: " << GetLastError() << "\n";
        return false;
    }

    session.roblox_pid = pi.dwProcessId;
    std::cout << "[+] Roblox launched (PID: " << pi.dwProcessId << ")\n";
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    session.state = SessionState::LAUNCHED;
    session.Save();
    return true;
}

static bool WaitForVerify(SessionInfo& session)
{
    std::cout << "[VERIFY] Waiting for payload load signal...\n";
    std::cout << "[*] Timeout: " << session.verify_timeout_sec << " seconds\n";

    fs::path marker = fs::path(session.roblox_dir) / L"bloxhub_loaded.txt";
    int waited = 0;

    while (waited < session.verify_timeout_sec)
    {
        if (fs::exists(marker))
        {
            std::cout << "[VERIFY] Payload loaded successfully!\n";
            try { fs::remove(marker); }
            catch (...) {}
            session.state = SessionState::VERIFIED;
            session.Save();
            return true;
        }
        Sleep(1000);
        waited++;
    }

    std::cout << "[!] Verify timeout — payload may not have loaded\n";
    return false;
}

static bool RunRestore(SessionInfo& session)
{
    std::cout << "[RESTORE] Cleaning up...\n";
    bool all_ok = true;

    for (const auto& file : session.created_files)
    {
        fs::path fp(file);
        if (!AttemptDelete(fp))
        {
            std::wcerr << L"[!] Failed to delete: " << fp.wstring() << L"\n";
            all_ok = false;
        }
        else
        {
            std::wcout << L"[*] Deleted: " << fp.filename().wstring() << L"\n";
        }
    }

    if (all_ok)
        std::cout << "[RESTORE] Cleanup complete\n";
    else
        std::cout << "[RESTORE] Cleanup partially failed — check files manually\n";

    session.state = all_ok ? SessionState::RESTORED : SessionState::FAILED;
    session.Save();

    try { fs::remove(session.session_file_path); }
    catch (...) {}

    return all_ok;
}

int main(int argc, char* argv[])
{
    std::cout << "BloxHub Modern Loader v2\n";
    std::cout << "------------------------\n\n";

    fs::path roblox_exe;
    std::wstring target_dll_name = L"dxgi.dll";

    if (argc > 1)
    {
        roblox_exe = argv[1];
    }
    else
    {
        std::cout << "[?] Enter path to RobloxPlayerBeta.exe or version folder:\n";
        std::string input;
        std::getline(std::cin, input);
        roblox_exe = input;
    }

    if (argc > 2)
    {
        std::string tgt_arg = argv[2];
        target_dll_name.assign(tgt_arg.begin(), tgt_arg.end());
    }

    if (fs::is_directory(roblox_exe))
    {
        auto exe_in_folder = roblox_exe / "RobloxPlayerBeta.exe";
        if (fs::exists(exe_in_folder))
            roblox_exe = exe_in_folder;
    }

    SessionInfo session;

    bool restore_needed = false;
    bool success = false;

    do
    {
        if (!RunPreflight(session, roblox_exe, target_dll_name))
        {
            success = false;
            break;
        }
        restore_needed = true;

        if (!RunInstall(session))
        {
            success = false;
            break;
        }

        if (!RunLaunch(session, roblox_exe))
        {
            success = false;
            break;
        }

        WaitForVerify(session);

        std::cout << "\n[*] Press Enter to restore files and exit...\n";
        std::cin.get();

        success = RunRestore(session);
        restore_needed = false;

    } while (false);

    if (restore_needed)
    {
        std::cerr << "\n[!] Session incomplete — attempting restore...\n";
        RunRestore(session);
    }

    std::cout << "\n[*] Press Enter to exit...\n";
    std::cin.get();
    return success ? 0 : 1;
}
