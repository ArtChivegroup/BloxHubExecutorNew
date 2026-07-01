#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include <Psapi.h>
#include "injector.hpp"

uint32_t FindProcessId(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    uint32_t pid = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_stricmp(pe32.szExeFile, processName.c_str()) == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return pid;
}

int main(int argc, char* argv[]) {
    std::cout << "BloxHub Injector (Manual Map + CFG Bypass)\n";
    std::cout << "-------------------------------------------\n";

    std::string targetProcess = "RobloxPlayerBeta.exe";
    if (argc > 1) {
        targetProcess = argv[1];
    }

    std::cout << "[*] Target process: " << targetProcess << "\n";
    std::cout << "[*] Waiting for process...\n";

    uint32_t pid = 0;
    while (!(pid = FindProcessId(targetProcess))) {
        Sleep(1000);
    }

    std::cout << "[*] Found process! PID: " << pid << "\n";

    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath).substr(0, std::string(exePath).find_last_of("\\/"));
    std::string dllPath = exeDir + "\\BloxHubInternal.dll";

    std::cout << "[*] DLL path: " << dllPath << "\n";
    std::cout << "[*] Injecting with CFG bypass...\n";

    auto status = injector::Inject(pid, dllPath, true);
    if (status != injector::InjectionStatus::SUCCESS) {
        std::cout << "[!] Injection failed!\n";
        std::cin.get();
        return 1;
    }

    std::cout << "[+] Injection successful!\n";
    std::cout << "[*] Check %TEMP%\\bloxhub_test.txt for log\n";
    std::cout << "\n[*] Press Enter to exit...";
    std::cin.get();

    return 0;
}
