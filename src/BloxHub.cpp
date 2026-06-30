
#include <Windows.h>
#include <iostream>
#include <filesystem>
#include <array>
#include <string>
#include <random>
#include "pe/pe.hpp"

// Generate random DLL name (like system DLL)
std::string GenerateRandomDllName()
{
    const std::array<std::string, 5> prefixes = {
        "msvcp140_", "vcruntime140_", "winmm_", "d3d11_", "dxgi_"
    };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> prefix_dist(0, prefixes.size() - 1);
    std::uniform_int_distribution<> num_dist(1000, 9999);
    return prefixes[prefix_dist(gen)] + std::to_string(num_dist(gen)) + ".dll";
}

int main(int argc, char* argv[])
{
    std::cout << "BloxHub Modern Loader\n";
    std::cout << "------------------------\n\n";

    std::filesystem::path roblox_exe;

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

    // If user gave a directory, auto-add RobloxPlayerBeta.exe
    if (std::filesystem::is_directory(roblox_exe))
    {
        auto exe_in_folder = roblox_exe / "RobloxPlayerBeta.exe";
        if (std::filesystem::exists(exe_in_folder))
        {
            roblox_exe = exe_in_folder;
        }
    }

    if (!std::filesystem::exists(roblox_exe))
    {
        std::cerr << "[!] RobloxPlayerBeta.exe not found at: " << roblox_exe << "\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }

    auto roblox_dir = roblox_exe.parent_path();
    std::cout << "[*] Using Roblox directory: " << roblox_dir << "\n";

    // Generate random DLL name
    auto fake_dll_name = GenerateRandomDllName();
    std::cout << "[*] Generated fake DLL name: " << fake_dll_name << "\n";

    // Get our own executable path to find BloxHubInternal.dll
    char exe_path[MAX_PATH];
    GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    std::filesystem::path our_exe = exe_path;
    auto our_dir = our_exe.parent_path();
    auto internal_dll = our_dir / "BloxHubInternal.dll";
    if (!std::filesystem::exists(internal_dll))
    {
        std::cerr << "[!] BloxHubInternal.dll not found in current directory!\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }
    std::cout << "[*] Found BloxHubInternal.dll: " << internal_dll << "\n";

    // Copy DLL to Roblox directory
    auto fake_dll_path = roblox_dir / fake_dll_name;

    // Check if backup exists, restore first
    auto backup_exe = roblox_dir / "RobloxPlayerBeta.exe.backup";

    try
    {
        if (std::filesystem::exists(backup_exe))
        {
            std::cout << "[*] Restoring original RobloxPlayerBeta.exe from backup...\n";
            std::filesystem::copy_file(backup_exe, roblox_exe, std::filesystem::copy_options::overwrite_existing);
        }
        else
        {
            std::cout << "[*] Creating backup of RobloxPlayerBeta.exe...\n";
            std::filesystem::copy_file(roblox_exe, backup_exe);
        }

        // Copy our DLL
        std::cout << "[*] Copying BloxHubInternal.dll to Roblox directory as " << fake_dll_name << "...\n";
        std::filesystem::copy_file(internal_dll, fake_dll_path, std::filesystem::copy_options::overwrite_existing);

        // Import hijack
        std::cout << "[*] Modifying Import Table...\n";
        pe::pe_t pe(roblox_exe);
        pe.import_dll(fake_dll_name, std::to_array<std::string>({"BloxHubInit"}));
        std::cout << "[+] Success!\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "[!] Error: " << e.what() << "\n";
        std::cout << "[*] Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }

    std::cout << "\n[*] Done! Now you can launch Roblox.\n";
    std::cout << "[*] Press Enter to exit (will restore original when Roblox closes)...\n";
    std::cin.get();

    // Restore
    try
    {
        std::cout << "[*] Restoring original RobloxPlayerBeta.exe...\n";
        std::filesystem::copy_file(backup_exe, roblox_exe, std::filesystem::copy_options::overwrite_existing);
        std::filesystem::remove(fake_dll_path);
        std::cout << "[+] Restored successfully!\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "[!] Error restoring: " << e.what() << "\n";
        std::cin.get();
        return 1;
    }

    return 0;
}
