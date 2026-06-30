#include <Windows.h>
#include <iostream>
#include <string>
#include <TlHelp32.h>
#include "injector.hpp"

// Fungsi untuk mencari PID dari nama proses
uint32_t FindProcessId(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    uint32_t pid = 0;

    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Bandingkan nama (case-insensitive)
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
    std::cout << "BloxHub Executor v1.0\n";
    std::cout << "------------------------\n";

    // Tentukan target proses (default RobloxPlayerBeta.exe)
    std::string targetProcess = "RobloxPlayerBeta.exe";
    if (argc > 1) {
        targetProcess = argv[1];
    }

    std::cout << "[*] Target process: " << targetProcess << "\n";
    std::cout << "[*] Waiting for process...\n";

    // Tunggu proses muncul
    uint32_t pid = 0;
    while (!(pid = FindProcessId(targetProcess))) {
        Sleep(1000);
    }

    std::cout << "[*] Found process! PID: " << pid << "\n";

    // Dapatkan path DLL
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    std::string exeDir = std::string(exePath).substr(0, std::string(exePath).find_last_of("\\/"));
    std::string dllPath = exeDir + "\\BloxHubInternal.dll";

    std::cout << "[*] DLL path: " << dllPath << "\n";
    std::cout << "[*] Injecting...\n";

    // Jalankan injection
    auto status = injector::Inject(pid, dllPath);
    if (status != injector::InjectionStatus::SUCCESS) {
        std::cout << "[!] Injection failed!\n";
        std::cin.get();
        return 1;
    }

    std::cout << "[*] Injection successful!\n";
    std::cout << "[*] Check %TEMP%\\bloxhub_test.txt for log\n";
    std::cout << "\n[*] Press Enter to exit...";
    std::cin.get();

    return 0;
}
