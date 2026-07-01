
# Cara Penggunaan

---
## Catatan Penting
- **BloxHubLoader (Import Hijacking)**: ❌ Dibatalkan, menyebabkan "Roblox Damaged"!
- **BloxHub.exe (Modern Dynamic Proxy)**: ✅ Metode utama saat ini!
- **BloxHubInjector (Manual Map)**: ⏸️ Pending

---
## Cara Menggunakan BloxHub.exe (Dynamic DLL Proxy)
1. **Pastikan Roblox tidak berjalan!**
2. Jalankan `build/bin/Release/BloxHub.exe`
3. Masukkan path ke folder Roblox atau ke RobloxPlayerBeta.exe, misal:
   ```
   C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638
   ```
4. Tunggu 3 detik countdown → Roblox akan terbuka otomatis!
5. Periksa log di `%TEMP%\bloxhub_test.txt` untuk melihat apakah DLL proxy berhasil dimuat!
6. Setelah selesai bermain, **kembali ke jendela BloxHub.exe dan tekan Enter** untuk merestore file Roblox!

---
## Cara Mengganti Target Proxy DLL
Untuk mengganti target proxy DLL:
1. Buka `CMakeLists.txt`
2. Ubah target library dari `dxgi` ke DLL yang diinginkan (misal `msvcp140`, `d3d9`, dll.)
3. Buat proxy file baru di `src/internal/` (contoh: `msvcp140_proxy.cpp`)
4. Ubah target di `src/BloxHub.cpp`
5. Build ulang dengan `cmake --build build --config Release`

---
## Restore Roblox Asli
- **Auto-Restore**: Tekan Enter di jendela BloxHub.exe
- **Manual Restore**: Hapus `dxgi.dll` dan `dxgi_orig.dll` dari folder Roblox!

---
## Cara Cek Log
- Proxy DLL log: `%TEMP%\bloxhub_test.txt`
