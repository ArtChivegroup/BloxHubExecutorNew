# BloxHub Executor

Tanggal Checkpoint: 1 Juli 2026

---

## Apa ini?
Executor Roblox dengan **import hijacking** (bypass Hyperion). DLL internal kita terload otomatis saat Roblox berjalan (tidak perlu CreateRemoteThread/manual map yang mudah terdeteksi).

---

## Alur Kerja & Cara Kerja

---

### Bagian 1: BloxHubLoader.exe - Import Hijacking
**Apa yang dilakukan loader?**
1. **Validasi File**: Mengecek apakah file input adalah PE file yang valid (cek signature IMAGE_DOS_HEADER)
2. **Backup Otomatis**: Membuat salinan file `RobloxPlayerBeta.exe` asli menjadi `RobloxPlayerBeta.exe.backup`
3. **Baca PE File**: Menggunakan library `pe_bliss` untuk membaca struktur PE dari Roblox
4. **Tambahkan ke Import Table**:
   - Menambahkan entry baru di Import Table dengan nama DLL `BloxHubInternal.dll`
   - Menambahkan import function `BloxHubInit` (yang diexport oleh DLL kita)
5. **Tambahkan Section Baru**: Membuat section baru `.hydro` untuk menyimpan data import tambahan
6. **Rebuild PE**: Menghitung ulang offset, checksum, dan menyimpan file yang sudah dimodifikasi

**Library yang dipakai oleh loader**:
- `pe_bliss`: Library untuk edit PE file (header, section, import table, dll.)
- `vendor/pe/pe.cpp/hpp`: Wrapper sederhana untuk `pe_bliss` khusus untuk import hijacking

---

### Bagian 2: Roblox Startup - Windows Loader
**Apa yang terjadi ketika Roblox dibuka?**
1. Windows membaca `RobloxPlayerBeta.exe`
2. Windows melihat daftar DLL di **Import Table**
3. Windows memuat **semua DLL tersebut satu per satu** (dari folder executable dulu, baru system32, dll.)
4. Termasuk `BloxHubInternal.dll` (karena kita sudah menambahkannya di Import Table!)
5. Windows memanggil entry point setiap DLL (`DllMain`), lalu memanggil fungsi yang diimport (untuk resolve address)

---

### Bagian 3: BloxHubInternal.dll - Our Code
**Apa yang DLL kita lakukan?**
1. `DllMain` dipanggil dengan alasan `DLL_PROCESS_ATTACH` (saat DLL pertama kali diload)
2. Kita mencatat log ke `%TEMP%\bloxhub_test.txt` untuk bukti DLL berhasil masuk
3. `BloxHubInit` (fungsi yang kita export) dipanggil oleh Windows loader
4. DLL kita tetap berada di memory Roblox, siap untuk dikembangkan fitur executornya (seperti baca/tulis memory, panggil fungsi Roblox, dll.)

---

### Kenapa Ini Bisa Bypass Hyperion?
Hyperion mendeteksi injeksi dengan cara:
- Memantau `CreateRemoteThread`, `WriteProcessMemory`, `VirtualAllocEx` (semua yang kita **TIDAK PAKAI**)
- Memantau manual map (inject tanpa LoadLibrary)
- Memeriksa memory region yang tidak dikenal

Sedangkan kita:
- **Hanya** memodifikasi file di disk (Import Table)
- **Windows sendiri** yang memuat DLL kita (seolah-olah DLL kita bagian dari Roblox asli)
- Ini adalah "trust path" yang tidak mudah terdeteksi sebagai injeksi!

---

## Struktur Proyek Lengkap
```
BloxHub-Executor-New/
├── include/
│   ├── injector.hpp      # Header untuk manual map injector
│   └── offsets.hpp       # Offset Roblox (versi version-1a951716f19e4638)
├── src/
│   ├── injector/
│   │   ├── manual_map.cpp # Manual mapping DLL (untuk test di Notepad)
│   │   └── hijack.cpp     # Thread hijacking (dari EXAMPLE PROJECT)
│   ├── internal/
│   │   └── dllmain.cpp    # DLL internal (log ke %TEMP%\bloxhub_test.txt)
│   ├── BloxHubInjector.cpp # Injector manual map (untuk Notepad/test)
│   └── BloxHubLoader.cpp   # Import hijacking loader (UNTUK ROBLOX - WORKING!)
├── vendor/
│   ├── pe_bliss/          # Library untuk memodifikasi PE file
│   │   ├── entropy.cpp
│   │   ├── entropy.h
│   │   ├── file_version_info.cpp
│   │   ├── file_version_info.h
│   │   ├── message_table.cpp
│   │   ├── message_table.h
│   │   ├── pe_base.cpp
│   │   ├── pe_base.h
│   │   ├── pe_bliss.h
│   │   ├── pe_bliss_resources.h
│   │   ├── pe_bound_import.cpp
│   │   ├── pe_bound_import.h
│   │   ├── pe_checksum.cpp
│   │   ├── pe_checksum.h
│   │   ├── pe_debug.cpp
│   │   ├── pe_debug.h
│   │   ├── pe_directory.cpp
│   │   ├── pe_directory.h
│   │   ├── pe_dotnet.cpp
│   │   ├── pe_dotnet.h
│   │   ├── pe_exception.cpp
│   │   ├── pe_exception.h
│   │   ├── pe_exception_directory.cpp
│   │   ├── pe_exception_directory.h
│   │   ├── pe_exports.cpp
│   │   ├── pe_exports.h
│   │   ├── pe_factory.cpp
│   │   ├── pe_factory.h
│   │   ├── pe_imports.cpp
│   │   ├── pe_imports.h
│   │   ├── pe_load_config.cpp
│   │   ├── pe_load_config.h
│   │   ├── pe_properties.cpp
│   │   ├── pe_properties.h
│   │   ├── pe_properties_generic.cpp
│   │   ├── pe_properties_generic.h
│   │   ├── pe_rebuilder.cpp
│   │   ├── pe_rebuilder.h
│   │   ├── pe_relocations.cpp
│   │   ├── pe_relocations.h
│   │   ├── pe_resource_manager.cpp
│   │   ├── pe_resource_manager.h
│   │   ├── pe_resource_viewer.cpp
│   │   ├── pe_resource_viewer.h
│   │   ├── pe_resources.cpp
│   │   ├── pe_resources.h
│   │   ├── pe_rich_data.cpp
│   │   ├── pe_rich_data.h
│   │   ├── pe_section.cpp
│   │   ├── pe_section.h
│   │   ├── pe_structures.h
│   │   ├── pe_tls.cpp
│   │   ├── pe_tls.h
│   │   ├── resource_bitmap_reader.cpp
│   │   ├── resource_bitmap_reader.h
│   │   ├── resource_bitmap_writer.cpp
│   │   ├── resource_bitmap_writer.h
│   │   ├── resource_cursor_icon_reader.cpp
│   │   ├── resource_cursor_icon_reader.h
│   │   ├── resource_cursor_icon_writer.cpp
│   │   ├── resource_cursor_icon_writer.h
│   │   ├── resource_data_info.cpp
│   │   ├── resource_data_info.h
│   │   ├── resource_internal.h
│   │   ├── resource_message_list_reader.cpp
│   │   ├── resource_message_list_reader.h
│   │   ├── resource_string_table_reader.cpp
│   │   ├── resource_string_table_reader.h
│   │   ├── resource_version_info_reader.cpp
│   │   ├── resource_version_info_reader.h
│   │   ├── resource_version_info_writer.cpp
│   │   ├── resource_version_info_writer.h
│   │   ├── stdint_defs.h
│   │   ├── utils.cpp
│   │   ├── utils.h
│   │   ├── version_info_editor.cpp
│   │   ├── version_info_editor.h
│   │   ├── version_info_viewer.cpp
│   │   ├── version_info_viewer.h
│   │   ├── version_info_types.h
│   │   └── ... (seluruh file pe_bliss)
│   └── pe/
│       ├── pe.cpp          # Wrapper untuk import hijacking
│       └── pe.hpp          # Header wrapper pe
├── CMakeLists.txt          # Build script
└── README.md               # Dokumentasi ini
```

---

## Cara Build (Visual Studio 2022)
1. Buka **Developer Command Prompt for VS2022**
2. Pindah ke direktori proyek:
   ```cmd
   cd "c:\Users\Administrator\Downloads\BloxHubProject\BloxhubDumper\BloxHub Executor\BloxHub-Executor-New"
   ```
3. Buat folder build dan masuk:
   ```cmd
   mkdir build
   cd build
   ```
4. Generate project CMake:
   ```cmd
   cmake ..
   ```
5. Build dalam mode Release:
   ```cmd
   cmake --build . --config Release
   ```

### Hasil Build (di `build/bin/Release/`):
- `BloxHubLoader.exe` - **Import hijacking loader (UNTUK ROBLOX)**
- `BloxHubInternal.dll` - DLL internal kita
- `BloxHubInjector.exe` - Manual map injector (untuk test di Notepad)

---

## Cara Pakai (Untuk Roblox - WORKING!)
1. **Pastikan Roblox tidak berjalan**
2. **Restore file Roblox asli** (jika sudah dimodifikasi):
   - Opsi A: Copy `RobloxPlayerBeta.exe.backup` (dibuat otomatis oleh loader) ke `RobloxPlayerBeta.exe`
   - Opsi B: Buka Bloxstrap → klik **"Verify Files"**
3. **Salin DLL ke folder Roblox**:
   - Copy `BloxHubInternal.dll` ke folder yang sama dengan `RobloxPlayerBeta.exe` (contoh: `C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\`)
4. **Jalankan BloxHubLoader**:
   ```cmd
   cd build/bin/Release
   BloxHubLoader.exe "<path-lengkap-ke-RobloxPlayerBeta.exe>"
   ```
   Contoh:
   ```cmd
   BloxHubLoader.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\RobloxPlayerBeta.exe"
   ```
5. **Buka RobloxPlayerBeta.exe**
6. **Cek log injection**:
   ```cmd
   type %TEMP%\bloxhub_test.txt
   ```

---

## Cara Test (Di Notepad - Aman!)
Kalau mau test basic function tanpa Roblox:
1. Buka **Notepad** (`notepad.exe`)
2. Jalankan injector manual map:
   ```cmd
   cd build/bin/Release
   BloxHubInjector.exe notepad.exe
   ```
3. Cek log:
   ```cmd
   type %TEMP%\bloxhub_test.txt
   ```

---

## Catatan Penting
1. **Backup Otomatis**: `BloxHubLoader` otomatis membuat backup file Roblox asli dengan nama `RobloxPlayerBeta.exe.backup`
2. **Offset**: Offset di `include/offsets.hpp` sesuai dengan versi Roblox `version-1a951716f19e4638`
3. **Lokasi DLL**: `BloxHubInternal.dll` harus selalu berada di **folder yang sama** dengan `RobloxPlayerBeta.exe`
4. **Hyperion**: Kita pakai **import hijacking** karena ini "trust path" yang tidak mudah terdeteksi oleh Hyperion
