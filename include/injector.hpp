#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace injector {
    enum class InjectionStatus {
        FAILED = 0,
        SUCCESS
    };

    InjectionStatus Inject(uint32_t processId, const std::string& dllPath, bool enableCfgBypass = true);
}
