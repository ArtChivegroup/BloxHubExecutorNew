#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <Psapi.h>
#include "injector.hpp"

static bool ProcessHasModule(DWORD pid, const wchar_t* moduleName) {
    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!process) return false;

    HMODULE mods[1024];
    DWORD needed = 0;
    bool found = false;
    if (EnumProcessModules(process, mods, sizeof(mods), &needed)) {
        for (unsigned i = 0; i < needed / sizeof(HMODULE); i++) {
            wchar_t path[MAX_PATH] = {};
            if (GetModuleFileNameExW(process, mods[i], path, MAX_PATH) && wcsstr(path, moduleName)) {
                found = true;
                break;
            }
        }
    }
    CloseHandle(process);
    return found;
}

static DWORD FindGamePid() {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe = {};
    pe.dwSize = sizeof(pe);
    DWORD pid = 0;

    if (Process32FirstW(snap, &pe)) {
        do {
            if (_wcsicmp(pe.szExeFile, L"RobloxPlayerBeta.exe") != 0)
                continue;
            if (!ProcessHasModule(pe.th32ProcessID, L"RobloxPlayerBeta.dll"))
                continue;
            pid = pe.th32ProcessID;
            break;
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

static bool ProcessAlive(DWORD pid) {
    HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!h) return false;
    DWORD code = 0;
    GetExitCodeProcess(h, &code);
    CloseHandle(h);
    return code == STILL_ACTIVE;
}

int main(int argc, char* argv[]) {
    std::cout << "BloxHub Injector (manual — buka Roblox + masuk game dulu)\n";
    std::cout << "-----------------------------------------------------------\n";
    std::cout << "[*] Mode: tunggu RobloxPlayerBeta.dll loaded, lalu stomp inject\n\n";

  (void)argc;
  (void)argv;

    std::cout << "[*] Waiting for game process (RobloxPlayerBeta.dll)...\n";

    DWORD pid = 0;
    for (int i = 0; i < 120 && !pid; i++) {
        pid = FindGamePid();
        if (!pid) Sleep(1000);
    }

    if (!pid) {
        std::cout << "[!] Timeout — buka Roblox, masuk place/game, jalankan lagi\n";
        std::cin.get();
        return 1;
    }

    std::cout << "[+] Game process ready (PID: " << pid << ")\n";
    std::cout << "[*] Tunggu 2 detik sebelum inject...\n";
    Sleep(2000);

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath).substr(0, std::string(exePath).find_last_of("\\/"));
    std::string dllPath = exeDir + "\\BloxHubInternal.dll";

    std::cout << "[*] DLL: " << dllPath << "\n";
    std::cout << "[*] Injecting...\n";

    auto status = injector::Inject(pid, dllPath);
    if (status != injector::InjectionStatus::SUCCESS) {
        std::cout << "[!] Injection failed!\n";
        std::cin.get();
        return 1;
    }

  if (!ProcessAlive(pid)) {
        std::cout << "[!] Roblox crash setelah inject — cek build terbaru (TLS/SEH skip)\n";
    } else {
        std::cout << "[+] Injection OK — Roblox masih hidup\n";
        std::cout << "[*] Cari console: [BloxHub] DllMain PROCESS_ATTACH\n";
    }

    std::cout << "\n[*] Press Enter to exit...";
    std::cin.get();
    return 0;
}
