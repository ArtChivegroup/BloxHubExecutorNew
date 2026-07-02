# Rencana Step 5: Log Multi-Baris

## Konteks
Proyek BloxHub Executor — Fase 2 Komunikasi. Setelah Step 4 (write file single line berhasil), sekarang kita perlu menulis 3 log multi-baris ke file untuk memastikan alur inisialisasi berjalan tanpa crash di tengah.

## Apa yang Sudah Berjalan
- ✅ Fase 1 Selesai: Inject berhasil, DllMain berjalan via IoCompletion
- ✅ Step 4: Menulis "hello from BloxHub!" ke C:\test_bloxhub.txt berhasil
- File: [dllmain.cpp](file:///c:\Users\Administrator\Downloads\BLOXHUB%20NEW%20EXEC\BloxHubExecutorNew\src\internal\dllmain.cpp)

## Rencana Perubahan

### 1. Modifikasi dllmain.cpp
- Tambahkan 3 log: "init start", "init mid", "init end" di titik berbeda dalam DLL_PROCESS_ATTACH
- Setiap log ditulis ke file dengan newline (\r\n)
- Gunakan WinAPI yang sama (CreateFileA, WriteFile)
- Tambahkan OutputDebugStringA untuk setiap langkah

### 2. Build Proyek
- Build target BloxHubInternal.dll
- Build target BloxHubInjector.exe (jika perlu)

### 3. Test
- Buka Roblox, masuk ke game
- Jalankan DebugView as Administrator (Capture Global Win32)
- Jalankan BloxHubInjector.exe as Administrator
- Cek:
  - DebugView menampilkan ketiga pesan
  - File C:\test_bloxhub.txt berisi ketiga baris log

### 4. Update Dokumentasi
- Update docs/TODO.md: Step 5 menjadi ✅
- Update checkpoints/CHECKPOINT_CURRENT.md
- Update docs/STATUS.md dan README.md (jika perlu)

## File yang Akan Diubah
1. src/internal/dllmain.cpp
2. docs/TODO.md
3. checkpoints/CHECKPOINT_CURRENT.md

## Kondisi Sukses
- File C:\test_bloxhub.txt berisi 3 baris:
  ```
  init start
  init mid
  init end
  ```
- DebugView menampilkan ketiga pesan
