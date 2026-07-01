cmake_minimum_required(VERSION 3.20)
project(BloxHubExecutor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# pe_bliss library
add_library(pe_bliss STATIC
    vendor/pe_bliss/entropy.cpp
    vendor/pe_bliss/file_version_info.cpp
    vendor/pe_bliss/message_table.cpp
    vendor/pe_bliss/pe_base.cpp
    vendor/pe_bliss/pe_bound_import.cpp
    vendor/pe_bliss/pe_checksum.cpp
    vendor/pe_bliss/pe_debug.cpp
    vendor/pe_bliss/pe_directory.cpp
    vendor/pe_bliss/pe_dotnet.cpp
    vendor/pe_bliss/pe_exception.cpp
    vendor/pe_bliss/pe_exception_directory.cpp
    vendor/pe_bliss/pe_exports.cpp
    vendor/pe_bliss/pe_factory.cpp
    vendor/pe_bliss/pe_imports.cpp
    vendor/pe_bliss/pe_load_config.cpp
    vendor/pe_bliss/pe_properties.cpp
    vendor/pe_bliss/pe_properties_generic.cpp
    vendor/pe_bliss/pe_rebuilder.cpp
    vendor/pe_bliss/pe_relocations.cpp
    vendor/pe_bliss/pe_resource_manager.cpp
    vendor/pe_bliss/pe_resource_viewer.cpp
    vendor/pe_bliss/pe_resources.cpp
    vendor/pe_bliss/pe_rich_data.cpp
    vendor/pe_bliss/pe_section.cpp
    vendor/pe_bliss/pe_tls.cpp
    vendor/pe_bliss/resource_bitmap_reader.cpp
    vendor/pe_bliss/resource_bitmap_writer.cpp
    vendor/pe_bliss/resource_cursor_icon_reader.cpp
    vendor/pe_bliss/resource_cursor_icon_writer.cpp
    vendor/pe_bliss/resource_data_info.cpp
    vendor/pe_bliss/resource_message_list_reader.cpp
    vendor/pe_bliss/resource_string_table_reader.cpp
    vendor/pe_bliss/resource_version_info_reader.cpp
    vendor/pe_bliss/resource_version_info_writer.cpp
    vendor/pe_bliss/utils.cpp
    vendor/pe_bliss/version_info_editor.cpp
    vendor/pe_bliss/version_info_viewer.cpp
)
target_include_directories(pe_bliss PUBLIC ${CMAKE_SOURCE_DIR}/vendor/pe_bliss)

# BloxHubInternal DLL (legacy, for manual map testing)
add_library(BloxHubInternal SHARED
    src/internal/dllmain.cpp
)
target_include_directories(BloxHubInternal PRIVATE ${CMAKE_SOURCE_DIR}/include)

# dxgi.dll (Proxy DLL for Roblox)
add_library(dxgi SHARED
    src/internal/dxgi_proxy.cpp
)
target_include_directories(dxgi PRIVATE ${CMAKE_SOURCE_DIR}/include)

# BloxHubInjector EXE
add_executable(BloxHubInjector
    src/BloxHubInjector.cpp
    src/injector/manual_map.cpp
)
target_include_directories(BloxHubInjector PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BloxHubInjector PRIVATE user32 kernel32)

# BloxHubLoader EXE (legacy, import hijack)
add_executable(BloxHubLoader
    src/BloxHubLoader.cpp
    vendor/pe/pe.cpp
)
target_include_directories(BloxHubLoader PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/vendor)
target_link_libraries(BloxHubLoader PRIVATE pe_bliss)

# BloxHub (Modern Loader) EXE
add_executable(BloxHub
    src/BloxHub.cpp
    src/internal/pe_patcher.cpp
)
target_include_directories(BloxHub PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(BloxHub PRIVATE user32 kernel32 shlwapi)

# TestProxy EXE (for testing winmm.dll proxy)
add_executable(TestProxy
    src/TestProxy.cpp
)
target_include_directories(TestProxy PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(TestProxy PRIVATE user32 kernel32)
