#include <iostream>
#include <filesystem>
#include <Windows.h>
#include <Shlwapi.h>
#include "internal/pe_patcher.h"

namespace fs = std::filesystem;

BOOL WriteFileToDiskW(LPCWSTR pszFileName, const BYTE* pbDataBuffer, DWORD dwDataLength)
{
    HANDLE hFile = CreateFileW(pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return FALSE;
    DWORD dwWritten = 0;
    WriteFile(hFile, pbDataBuffer, dwDataLength, &dwWritten, NULL);
    CloseHandle(hFile);
    return (dwWritten == dwDataLength);
}

int main(int argc, char* argv[])
{
    std::cout << "BloxHub Modern Loader (version.dll Proxy)\n";
    std::cout << "-----------------------------------------\n\n";

    fs::path roblox_exe;

    if (argc > 1)
    {
        roblox_exe = argv[1];
    }
    else
    {
        std::cout << "[?] Enter full path to RobloxPlayerBeta.exe OR just the version folder:\n";
        std::string input;
        std::getline(std::cin, input);
        roblox_exe = input;
    }

    if (fs::is_directory(roblox_exe))
    {
        auto exe_in_folder = roblox_exe / "RobloxPlayerBeta.exe";
        if (fs::exists(exe_in_folder))
        {
            roblox_exe = exe_in_folder;
        }
    }

    if (!fs::exists(roblox_exe))
    {
        std::cerr << "[!] RobloxPlayerBeta.exe not found at: " << roblox_exe << "\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }

    auto roblox_dir = roblox_exe.parent_path();
    std::wcout << L"[*] Using Roblox directory: " << roblox_dir.wstring() << L"\n";

    wchar_t exe_path[MAX_PATH];
    GetModuleFileNameW(NULL, exe_path, MAX_PATH);
    fs::path our_exe = exe_path;
    auto our_dir = our_exe.parent_path();
    auto payload_dll = our_dir / "version.dll";

    if (!fs::exists(payload_dll))
    {
        std::cerr << "[!] version.dll (payload) not found in current directory!\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }
    std::wcout << L"[*] Found version.dll (payload): " << payload_dll.wstring() << L"\n";

    const wchar_t* windir = _wgetenv(L"WINDIR");
    if (!windir)
    {
        std::cerr << "[!] WINDIR is not available.\n";
        return 1;
    }

    auto system_dxgi = fs::path(windir) / "System32" / "version.dll";
    auto roblox_dxgi_dll = roblox_dir / "version.dll";
    auto roblox_dxgi_orig_dll = roblox_dir / "version_orig.dll";

    try
    {
        if (fs::exists(roblox_dxgi_orig_dll))
        {
            std::cout << "[*] Proxy already installed, skipping copy...\n";
        }
        else
        {
            std::cout << "[*] Copying System32 version.dll to version_orig.dll...\n";
            fs::copy_file(system_dxgi, roblox_dxgi_orig_dll, fs::copy_options::overwrite_existing);

            std::cout << "[*] Generating dynamic proxy DLL...\n";
            PBYTE pProxyBuffer = NULL;
            DWORD dwProxySize = 0;
            if (!ConvertPayloadToProxy(
                payload_dll.wstring().c_str(),
                system_dxgi.wstring().c_str(),
                L"version_orig.dll",
                &pProxyBuffer,
                &dwProxySize
            ))
            {
                std::cerr << "[!] Failed to generate proxy DLL!\n";
                return 1;
            }

            std::wcout << L"[*] Writing proxy version.dll to Roblox directory...\n";
            if (!WriteFileToDiskW(roblox_dxgi_dll.wstring().c_str(), pProxyBuffer, dwProxySize))
            {
                std::cerr << "[!] Failed to write proxy DLL!\n";
                HeapFree(GetProcessHeap(), 0, pProxyBuffer);
                return 1;
            }
            HeapFree(GetProcessHeap(), 0, pProxyBuffer);
        }

        std::cout << "\n[+] Success! DLL proxy installed.\n";
        std::cout << "[*] Launching Roblox automatically in 3 seconds...\n";
        
        // Countdown 3 detik
        for (int i = 3; i > 0; i--) {
            std::cout << "[*] " << i << "...\n";
            Sleep(1000);
        }
        
        // Buka Roblox secara otomatis
        STARTUPINFOW si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));
        
        std::wstring roblox_exe_path = roblox_exe.wstring();
        if (!CreateProcessW(
            roblox_exe_path.c_str(),
            NULL,
            NULL,
            NULL,
            FALSE,
            0,
            NULL,
            NULL,
            &si,
            &pi
        )) {
            std::cerr << "[!] Failed to launch Roblox! Error: " << GetLastError() << "\n";
        } else {
            std::cout << "[+] Roblox launched successfully!\n";
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        
        std::cout << "\n[*] Press Enter to exit and restore original files...\n";
        std::cin.get();

        std::cout << "\n[*] Restoring original files...\n";

        bool restore_ok = true;

        if (fs::exists(roblox_dxgi_dll))
        {
            try
            {
                fs::remove(roblox_dxgi_dll);
                std::cout << "[*] Deleted version.dll (proxy)\n";
            }
            catch (const fs::filesystem_error& e)
            {
                std::cerr << "[!] Failed to delete version.dll: " << e.what() << "\n";
                restore_ok = false;
            }
        }

        if (fs::exists(roblox_dxgi_orig_dll))
        {
            try
            {
                fs::remove(roblox_dxgi_orig_dll);
                std::cout << "[*] Deleted version_orig.dll\n";
            }
            catch (const fs::filesystem_error& e)
            {
                std::cerr << "[!] Failed to delete dxgi_orig.dll: " << e.what() << "\n";
                restore_ok = false;
            }
        }

        if (restore_ok)
        {
            std::cout << "[+] Restored successfully!\n";
        }
        else
        {
            std::cout << "[!] Restore partially failed.\n";
        }

    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "[!] Filesystem error: " << e.what() << "\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }

    return 0;
}
