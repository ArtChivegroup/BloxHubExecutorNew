
# Cara Penggunaan

## Catatan Penting
- **BloxHubLoader (Import Hijacking)**: Saat ini menyebabkan "Roblox Damaged"
- **BloxHubInjector (Manual Map)**: Dapat digunakan untuk testing, tapi perlu perbaikan untuk stealth

## Menggunakan BloxHubInjector (Manual Map)
1. Buka Roblox
2. Jalankan BloxHubInjector.exe (tanpa argumen, default target RobloxPlayerBeta.exe)
3. Periksa log di `%TEMP%\bloxhub_test.txt`

## Menggunakan BloxHubLoader (Import Hijacking - TIDAK DISARANKAN)
1. Pastikan Roblox tidak berjalan
2. Salin BloxHubInternal.dll ke folder Roblox
3. Jalankan:
   ```cmd
   BloxHubLoader.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxx\RobloxPlayerBeta.exe"
   ```
4. Buka Roblox (akan menampilkan "Roblox Damaged")

## Restore Roblox Asli
- Jika menggunakan BloxHubLoader: Hapus RobloxPlayerBeta.exe dan rename RobloxPlayerBeta.exe.backup menjadi RobloxPlayerBeta.exe
- Atau: Buka Bloxstrap → klik "Verify Files"
