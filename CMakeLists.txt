cmake_minimum_required(VERSION 3.20)
project(BloxHubExecutor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Payload DLL for manual map inject
add_library(BloxHubInternal SHARED
    src/internal/dllmain.cpp
)
target_include_directories(BloxHubInternal PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Proxy payload for sideload experiments (pe_patcher output source)
add_library(proxy_payload SHARED
    src/internal/proxy_payload.cpp
)
set_target_properties(proxy_payload PROPERTIES OUTPUT_NAME "version")
target_include_directories(proxy_payload PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Main loader: sideload + --inject
add_executable(BloxHub
    src/BloxHub.cpp
    src/internal/pe_patcher.cpp
    src/injector/manual_map.cpp
    src/injector/cfg_bypass.cpp
)
target_include_directories(BloxHub PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/injector
)
target_link_libraries(BloxHub PRIVATE user32 kernel32 shlwapi psapi)

# Standalone injector (Roblox already running)
add_executable(BloxHubInjector
    src/BloxHubInjector.cpp
    src/injector/manual_map.cpp
    src/injector/cfg_bypass.cpp
)
target_include_directories(BloxHubInjector PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/injector
)
target_link_libraries(BloxHubInjector PRIVATE user32 kernel32 psapi)
