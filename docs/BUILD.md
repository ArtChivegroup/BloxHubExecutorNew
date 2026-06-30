
# Cara Build

## Prasyarat
- Visual Studio 2022 (dengan C++ Desktop Development workload)
- CMake 3.20+
- Windows SDK

## Langkah-langkah Build

### 1. Buka Developer Command Prompt for VS 2022
Cari di Start Menu: "Developer Command Prompt for VS 2022"

### 2. Pindah ke Direktori Proyek
```cmd
cd "c:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew"
```

### 3. Buat Folder Build
```cmd
mkdir build
cd build
```

### 4. Generate Project CMake
```cmd
cmake ..
```

### 5. Build Project (Release Mode)
```cmd
cmake --build . --config Release
```

## Hasil Build
File executable dan DLL akan berada di:
```
build\bin\Release\
├── BloxHubLoader.exe
├── BloxHubInjector.exe
└── BloxHubInternal.dll
```

## Troubleshooting
- Jika terjadi error tentang CMake version: Pastikan CMake 3.20+ terinstall
- Jika terjadi error tentang Windows SDK: Pastikan Windows SDK terinstall di Visual Studio Installer
