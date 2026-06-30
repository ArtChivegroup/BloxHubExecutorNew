cmake_minimum_required(VERSION 3.20)
project(BloxHubExecutor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# BloxHubInternal DLL
add_library(BloxHubInternal SHARED
    src/internal/dllmain.cpp
)
target_include_directories(BloxHubInternal PRIVATE ${CMAKE_SOURCE_DIR}/include)

# BloxHubInjector EXE
add_executable(BloxHubInjector
    src/BloxHubInjector.cpp
    src/injector/manual_map.cpp
)
target_include_directories(BloxHubInjector PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link Windows libraries
target_link_libraries(BloxHubInjector PRIVATE user32 kernel32)
