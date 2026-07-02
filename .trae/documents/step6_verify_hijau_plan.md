# Rencana Step 6: Verify Hijau

## Konteks
Saat ini, payload kita menulis ke `C:\test_bloxhub.txt`, dan `BloxHub.cpp` memeriksa marker di `%TEMP%\bloxhub_payload_loaded.txt`. Step 6 meminta kita untuk mengubah verifikasi agar memeriksa path absolut seperti `C:\BloxHub\test.txt`.

## Rencana Perubahan

### 1. Update `src/internal/dllmain.cpp`
- Buat direktori `C:\BloxHub` jika belum ada (gunakan WinAPI `CreateDirectoryA`)
- Ubah path penulisan file menjadi `C:\BloxHub\test.txt`
- Tetap tuliskan log multi-baris ke file tersebut

### 2. Update `src/BloxHub.cpp`
- Ubah fungsi `WaitForVerify` untuk memeriksa keberadaan file `C:\BloxHub\test.txt` alih-alih `%TEMP%\bloxhub_payload_loaded.txt`
- Update `ClearVerifyArtifacts` untuk membersihkan file di path baru jika diperlukan

### 3. Build Proyek
- Build `BloxHubInternal.dll`
- Build `BloxHub.exe`

### 4. Test
- Jalankan `BloxHub.exe <path_roblox> --inject`
- Pastikan terminal menampilkan `[VERIFY] Payload loaded successfully!`

## File yang Akan Diubah
1. `src/internal/dllmain.cpp`
2. `src/BloxHub.cpp`
3. `docs/TODO.md` (setelah selesai, tandai Step 6 ✅)
4. `checkpoints/CHECKPOINT_CURRENT.md` (dan buat checkpoint baru)

## Kondisi Sukses
- Terminal BloxHub.exe menampilkan `[VERIFY] Payload loaded successfully!`
