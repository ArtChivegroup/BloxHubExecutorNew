#include <iostream>
#include <filesystem>
#include <string>
#include <array>
#include "pe/pe.hpp"

int main(int argc, char* argv[]) {
    std::cout << "BloxHub Loader\n";
    std::cout << "----------------\n";

    if (argc < 2) {
        std::cout << "Usage: BloxHubLoader.exe <path-to-RobloxPlayerBeta.exe>\n";
        return 1;
    }

    std::filesystem::path roblox_path = argv[1];
    std::cout << "Target: " << roblox_path << "\n";

    try {
        // Buat backup file asli
        std::filesystem::path backup_path = roblox_path;
        backup_path += ".backup";
        
        if (!std::filesystem::exists(backup_path)) {
            std::filesystem::copy_file(roblox_path, backup_path);
            std::cout << "Backup created: " << backup_path << "\n";
        } else {
            std::cout << "Backup already exists: " << backup_path << "\n";
        }

        pe::pe_t pe(roblox_path);
        pe.import_dll("BloxHubInternal.dll", std::to_array<std::string>({"BloxHubInit"}));
        std::cout << "Success! Import table modified.\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}