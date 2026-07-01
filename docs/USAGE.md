
# Cara Penggunaan

---
## Catatan Penting
- **DLL Proxying**: ❌ Dibatalkan (Hyperion signature check)
- **Manual Map + CFG Bypass**: ✅ Metode utama saat ini!

---
## Cara Menggunakan BloxHubInjector.exe (Manual Map)

### Langkah-langkah:
1. **Buka Roblox dulu** (buka normal, login, masuk game)
2. Buka CMD baru, masuk ke folder build:
   ```cmd
   cd "C:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew\build\bin\Release"
   ```
3. **Hapus log lama**:
   ```cmd
   del %TEMP%\bloxhub_test.txt
   ```
4. **Jalankan injector**:
   ```cmd
   BloxHubInjector.exe
   ```
5. Injector akan auto-detect RobloxPlayerBeta.exe dan inject
6. **Cek log**:
   ```cmd
   type %TEMP%\bloxhub_test.txt
   ```

### Output yang Diharapkan:
```
[*] Allocated remote image at: 0x....
[*] DLL written to remote process
[*] Executing CFG bypass...
[*] Found CFG bitmap candidate at RVA 0x1432808 -> 0x920000
[+] CFG: Bitmap patched for region ...
[*] Calling BloxHubInit at ...
[+] BloxHubInit executed successfully!
[+] Injection successful!
```

Log di `%TEMP%\bloxhub_test.txt` seharusnya berisi:
```
[BloxHub] Injected successfully!
[BloxHub] Timestamp: ...
```

---

## Cara Menggunakan BloxHub.exe (DLL Proxy — LEGACY)
1. Jalankan `build/bin/Release/BloxHub.exe`
2. Masukkan path folder Roblox (contoh: `C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638`)
3. Tunggu countdown 3 detik → Roblox terbuka otomatis
4. Setelah selesai, tekan Enter untuk restore file

---

## Troubleshooting

### Tidak ada log setelah inject?
- **Masalah**: CRT dependency crash (VCRUNTIME140.dll tidak ada di Roblox)
- **Fix**: Coming soon — rewrite dllmain.cpp pure Win32 API

### CFG bitmap tidak ditemukan?
- **Masalah**: Offset bitmap berubah antar Roblox version
- **Fix**: Update `offsets.hpp → CfgBypass::BitmapPtr` dengan RVA dari log scanner

### Roblox crash setelah inject?
- Cek apakah ada antivirus blocking
- Cek apakah Hyperion update (offset berubah)
- Coba restart PC dan test ulang

---
## Cara Update Offsets
Saat Roblox update ke versi baru:
1. Jalankan BloxHubInjector.exe — lihat log scanner untuk RVA bitmap baru
2. Update `include/offsets.hpp → CfgBypass::BitmapPtr`
3. Update Lua offsets jika perlu (via roblox-dumper)
4. Rebuild proyek

---
## Cleanup Roblox Folder
Jika ada file sisa dari test DLL Proxying sebelumnya, hapus manual:
```cmd
del "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\version.dll"
del "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\version_orig.dll"
:: ...dan file proxy lainnya
```
Atau buka Bloxstrap → Verify Files.
