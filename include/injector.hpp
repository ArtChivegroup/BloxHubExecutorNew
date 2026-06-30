#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace injector {
    // Status hasil injection
    enum class InjectionStatus {
        FAILED = 0,
        SUCCESS
    };

    // Fungsi utama untuk inject DLL
    InjectionStatus Inject(uint32_t processId, const std::string& dllPath);
}
