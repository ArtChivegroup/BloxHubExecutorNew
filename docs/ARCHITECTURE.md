
# Arsitektur BloxHub Executor

---
## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For known bugs**: [BUGS.md](BUGS.md)
- **For roadmap & planning**: [PLANNING.md](PLANNING.md)
- **For progress checkpoints**: [../checkpoints/CHECKPOINT_20260701_FINAL.md](../checkpoints/CHECKPOINT_20260701_FINAL.md)

---
## Overview
BloxHub Executor adalah executor untuk Roblox yang dirancang dengan **Dynamic DLL Proxying (Runtime PE Patching)** untuk evasi Hyperion!

---
## Evolusi Arsitektur
### Sebelumnya
- BloxHubLoader.exe (PE Editor Import Hijacking) ❌ Terdeteksi Hyperion!
- BloxHubInjector.exe (Manual Map)
- BloxHubInternal.dll (Payload)

### Sekarang: Dynamic DLL Proxying with 3LayersPersistence
**Static .def & Import Hijack gagal!** Jadi kita gunakan teknik **Runtime PE Patching** dari `EXAMPLE PROJECT/3LayersPersistence-main`!

---
## Komponen Utama
### 1. BloxHub.exe (Modern Loader)
**Fungsi**: Loader yang membuat proxy DLL secara runtime dan drop ke folder Roblox!  
**Tanggung Jawab**:
- Baca payload DLL proxy (contoh: `dxgi.dll`) dari folder build
- Baca DLL asli dari System32 (contoh: `C:\Windows\System32\dxgi.dll`)
- Buat export table forwarder ke `*_orig.dll` (contoh: `dxgi_orig.dll`) secara runtime!
- Patch payload DLL dengan export table tersebut!
- Copy DLL asli ke folder Roblox sebagai `*_orig.dll`
- Drop proxy DLL yang sudah di-patch ke folder Roblox!
- Auto-launch Roblox setelah 3 detik countdown!
- Restore file Roblox ketika selesai!

### 2. Payload Proxy DLL (`dxgi.dll` / dll lain)
**Fungsi**: DLL proxy yang dimuat Roblox, kemudian menjalankan payload kita!  
**Tanggung Jawab**:
- `DllMain`: Cek apakah di-load oleh RobloxPlayerBeta.exe
- Load `*_orig.dll` (contoh: `dxgi_orig.dll`)
- Forward semua fungsi ke `*_orig.dll`
- Logging ke file
- PE Header Wiping (jika berhasil dimuat Roblox)

### 3. `pe_patcher.h` & `pe_patcher.cpp`
**Fungsi**: Core library untuk Runtime PE Patching, diadaptasi dari `3LayersPersistence`!  
**Fitur**:
- `ReadFileFromDiskW`: Baca file PE dari disk
- `RvaToFileOffset`: Konversi RVA ke file offset
- `ComputePECheckSum`: Hitung checksum PE
- `BuildExportTableFromDll`: Bangun export table forwarder dari DLL asli
- `PatchExportAddressTable`: Tambahkan section .edata ke payload DLL
- `ConvertPayloadToProxy`: Fungsi utama yang menggabungkan semua langkah di atas!

---
## Alur Kerja Dynamic Proxy
```
[BloxHub.exe]
    |
    |-- 1. Baca payload DLL proxy (dxgi.dll) dari disk
    |-- 2. Baca DLL asli dari System32 (C:\Windows\System32\dxgi.dll)
    |-- 3. BuildExportTableFromDll() → buat export table forward ke dxgi_orig.dll
    |-- 4. PatchExportAddressTable() → patch payload DLL dengan export table baru
    |-- 5. Copy System32 dxgi.dll → Roblox folder/dxgi_orig.dll
    |-- 6. Write proxy dxgi.dll (sudah di-patch) ke Roblox folder
    |-- 7. Tunggu 3 detik countdown
    |-- 8. Auto-launch RobloxPlayerBeta.exe
    ↓
[RobloxPlayerBeta.exe]
    |
    |-- Mencari dxgi.dll di folder aplikasi terlebih dahulu
    |-- Memuat proxy dxgi.dll kita!
    ↓
[Proxy dxgi.dll]
    |
    |-- DllMain dipanggil
    |-- Cek apakah host = RobloxPlayerBeta.exe
    |-- Load dxgi_orig.dll
    |-- Forward semua fungsi ke dxgi_orig.dll
    |-- Log ke %TEMP%\bloxhub_test.txt
    |-- (Jika di Roblox) Wipe PE Header
```

---
## Teknik Dari 3LayersPersistence
1. **Dynamic Export Table Generation**: Tidak perlu .def manual, baca export table DLL asli secara runtime!
2. **Timestamp Spoofing**: Timestamp proxy DLL dibuat lebih tua 30-60 hari dari asli!
3. **Export Sorting**: Nama di export table diurutkan ascending untuk kompatibilitas!
4. **Checksum Recalculation**: Hitung ulang PE checksum setelah memodifikasi!
5. **.edata Section Baru**: Tambahkan section baru untuk export table yang di-generate!

---
## Tech Stack
- **Bahasa**: C++20
- **Build System**: CMake
- **Library**: Shlwapi (PathRemoveFileSpec), Winsock (opsional untuk silent bridge)
- **Referensi**: `EXAMPLE PROJECT/3LayersPersistence-main` & `EXAMPLE PROJECT/RBX-cfg-bypass-main`
- **Target**: Windows 10+ x64

---
## Referensi
- [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701_FINAL.md)
- [Daftar Bug](../docs/BUGS.md)
