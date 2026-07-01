# Cara Build

## Scope

Dokumen ini menjelaskan build baseline untuk seluruh artefak proyek, dengan fokus penggunaan akhir tetap pada `loader-first track`.

## Prasyarat

- Visual Studio 2022 dengan workload `Desktop development with C++`
- CMake 3.20 atau lebih baru
- Windows SDK
- Environment Windows x64

## Toolchain Summary

Proyek menggunakan:

- `CMake` sebagai build system,
- `C++20`,
- `pe_bliss` dan `vendor/pe` sebagai dependency vendor di dalam repo.

## Langkah Build

### 1. Buka terminal developer

Gunakan `Developer Command Prompt for VS 2022` atau terminal lain yang sudah memuat toolchain MSVC.

### 2. Masuk ke root proyek

```cmd
cd "c:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew"
```

### 3. Buat direktori build

```cmd
mkdir build
cd build
```

### 4. Generate project files

```cmd
cmake ..
```

### 5. Build konfigurasi Release

```cmd
cmake --build . --config Release
```

Jika perlu Debug:

```cmd
cmake --build . --config Debug
```

## Artefak Yang Dihasilkan

Secara umum, output runtime berada di:

```text
build\bin\<Config>\
```

Target utama yang didefinisikan di `CMakeLists.txt`:

| Target | Jenis | Peran |
|--------|------|-------|
| `BloxHub` | EXE | Prototype loader modern |
| `BloxHubLoader` | EXE | PoC patch-import langsung ke target PE |
| `BloxHubInjector` | EXE | Injector manual map + CFG bypass |
| `BloxHubInternal` | DLL | Payload legacy untuk track manual map |
| `version` | DLL | Payload/proxy basis untuk `BloxHub.exe` |
| `TestProxy` | EXE | Utility test legacy |

Contoh isi folder output `Release`:

```text
build\bin\Release\
├── BloxHub.exe
├── BloxHubLoader.exe
├── BloxHubInjector.exe
├── BloxHubInternal.dll
├── version.dll
└── TestProxy.exe
```

## Build Notes Per Track

### Loader track

Untuk eksperimen loader aktif, artefak minimum yang biasanya relevan:

- `BloxHub.exe`
- `version.dll`

### PE patch PoC

Untuk eksperimen patch-import langsung:

- `BloxHubLoader.exe`
- `BloxHubInternal.dll`

### Manual map track

Untuk eksperimen injector sekunder:

- `BloxHubInjector.exe`
- `BloxHubInternal.dll`

## Verifikasi Build Minimum

Setelah build selesai, cek bahwa:

- `BloxHub.exe` terbentuk,
- `version.dll` terbentuk,
- `BloxHubLoader.exe` dan `BloxHubInjector.exe` juga tersedia bila ingin pembanding.

Build sukses hanya berarti artefak berhasil dikompilasi. Itu tidak sama dengan validasi runtime terhadap Roblox atau Hyperion.

## Troubleshooting

### CMake tidak ditemukan

Pastikan:

- CMake sudah terpasang,
- executable `cmake` tersedia di `PATH`.

### Error Windows SDK atau toolchain MSVC

Pastikan:

- workload C++ sudah terpasang di Visual Studio Installer,
- Windows SDK tersedia,
- build dijalankan dari terminal developer yang benar.

### Target output tidak muncul semua

Periksa:

- konfigurasi build yang dipakai (`Release` vs `Debug`),
- apakah build berhenti di tengah karena error kompilasi,
- apakah file output dicari di `build\bin\<Config>\`.

### Build sukses tetapi `BloxHub.exe` tidak bisa dipakai untuk uji loader

Periksa:

- `version.dll` tersedia di folder output yang sama,
- path target Roblox diberikan dengan benar saat menjalankan loader,
- jangan menganggap keberhasilan build sebagai bukti keberhasilan strategi loader.
