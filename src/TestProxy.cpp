#include <Windows.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::cout << "Test BloxHub dinput8.dll Proxy\n";
    std::cout << "------------------------------\n\n";

    // Get current directory
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    fs::path our_exe = exe_path;
    auto our_dir = our_exe.parent_path();

    // Path to our dinput8.dll and where to put test files
    auto proxy_dll = our_dir / "dinput8.dll";
    auto test_dir = our_dir / "test_proxy";

    // Check if proxy dll exists
    if (!fs::exists(proxy_dll)) {
        std::cerr << "[!] dinput8.dll not found in current directory!\n";
        return 1;
    }

    // Create test directory
    if (!fs::exists(test_dir)) {
        fs::create_directory(test_dir);
    }

    // Copy system's dinput8.dll to test directory as dinput8_orig.dll
    char system_dir[MAX_PATH];
    GetSystemDirectoryA(system_dir, MAX_PATH);
    auto system_dinput8_dll = fs::path(system_dir) / "dinput8.dll";
    auto test_orig_dll = test_dir / "dinput8_orig.dll";

    if (fs::exists(system_dinput8_dll)) {
        std::cout << "[*] Copying system dinput8.dll to test directory as dinput8_orig.dll...\n";
        fs::copy_file(system_dinput8_dll, test_orig_dll, fs::copy_options::overwrite_existing);
    } else {
        std::cerr << "[!] System dinput8.dll not found!\n";
        return 1;
    }

    // Copy our proxy dll to test directory
    auto test_proxy_dll = test_dir / "dinput8.dll";
    std::cout << "[*] Copying our dinput8.dll proxy to test directory...\n";
        fs::copy_file(proxy_dll, test_proxy_dll, fs::copy_options::overwrite_existing);

    // Try to load our proxy dll
    std::cout << "[*] Trying to load dinput8.dll from test directory...\n";

    HMODULE h_dinput8 = LoadLibraryExA(test_proxy_dll.string().c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (h_dinput8) {
        std::cout << "[+] Successfully loaded dinput8.dll (proxy)! HMODULE: " << h_dinput8 << "\n";
    } else {
        std::cerr << "[!] Failed to load dinput8.dll! Error: " << GetLastError() << "\n";
        return 1;
    }

    auto direct_input8_create = GetProcAddress(h_dinput8, "DirectInput8Create");
    if (!direct_input8_create) {
        std::cerr << "[!] Missing named export DirectInput8Create!\n";
        FreeLibrary(h_dinput8);
        return 1;
    }

    std::cout << "[+] DirectInput8Create export is present.\n";
    FreeLibrary(h_dinput8);

    std::cout << "\n[+] Test passed! Check %TEMP%\\bloxhub_test.txt for log from proxy dll.\n";
    std::cout << "\nTest files are in: " << test_dir << "\n";
    std::cout << "\nPress Enter to exit...\n";
    std::cin.get();

    return 0;
}
