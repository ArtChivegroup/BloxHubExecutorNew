# Cara Penggunaan

**Baca dulu:** [`STATUS.md`](STATUS.md) untuk konteks kondisi proyek saat ini.

---

## Prasyarat

- Windows x64  
- Build Release di `build\bin\Release\`  
- **CMD/PowerShell as Administrator** (untuk inject)  
- Path Roblox Bloxstrap **sama versi** dengan `offsets::roblox_version` di `include/offsets.hpp`  
- Artefak inject: `BloxHubInjector.exe` + `BloxHubInternal.dll` di folder yang sama  

---

## Mode 1 — Inject Manual (Disarankan)

Tidak memodifikasi file di folder Roblox. **Roblox harus sudah in-game** sebelum inject.

```cmd
cd build\bin\Release

REM 1. Buka Roblox, masuk game
REM 2. CMD as Administrator:
BloxHubInjector.exe
```

### Alur yang diharapkan

```text
[*] Mode: tunggu RobloxPlayerBeta.dll loaded, lalu stomp inject
[*] Waiting for game process (RobloxPlayerBeta.dll)...
[+] Game process ready (PID: ...)
[*] Tunggu 2 detik sebelum inject...
[*] DLL: ...\BloxHubInternal.dll
[*] Injecting...
[*] Stomp module mapped at ... (C:\Windows\System32\d3d10warp.dll)
[*] RobloxPlayerBeta.dll base: ...
[*] Payload written to stomp region (... bytes)
[*] Calling DllMain at ... (sync thread)
[*] CreateRemoteThread OK
[+] DllMain returned
[+] Injection OK — Roblox masih hidup
[*] Cari console: [BloxHub] DllMain PROCESS_ATTACH
```

### Cek bukti inject (Fase 1)

- Console di proses Roblox (bisa di belakang window game): `[BloxHub] DllMain PROCESS_ATTACH`
- Terminal: `DllMain returned` + `Roblox masih hidup`
- Kalau tidak ada console → lihat TODO Step 4 (`C:\BloxHub\test.txt`)

---

## Mode 2 — Inject via Launcher

Launch Roblox + inject otomatis. Timing bisa lebih riskan daripada manual.

```cmd
cd build\bin\Release

BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-5cf2272675e145f5" --inject
```

Atau path langsung ke exe:

```cmd
BloxHub.exe "C:\...\version-5cf2272675e145f5\RobloxPlayerBeta.exe" --inject
```

### Alur yang diharapkan

```text
[PREFLIGHT] OK
[LAUNCH] Roblox launched (PID: ...)
[+] Roblox game process ready (PID: ...)
[*] Stomp module mapped at: ...
[*] Payload written to stomp region
[*] CreateRemoteThread OK
[+] DllMain returned
[INJECT] OK
[VERIFY] ...                    ← masih sering timeout sampai Step 6
```

---

## Mode 3 — Sideload (Eksperimen / Hampir Pasti Gagal)

```cmd
BloxHub.exe "C:\...\version-xxxxxxxx\RobloxPlayerBeta.exe"
```

Opsional — ganti target DLL (default `dxgi.dll`):

```cmd
BloxHub.exe "C:\...\RobloxPlayerBeta.exe" dsound.dll
```

### Alur

1. Preflight — cek path, `version.dll`, System32 DLL  
2. Install — copy `dxgi_orig.dll`, tulis proxy `dxgi.dll`  
3. Launch Roblox  
4. Verify — tunggu `bloxhub_loaded.txt` (30 detik)  
5. Tekan Enter → restore (hapus `dxgi.dll`, `dxgi_orig.dll`)  

### Kenapa gagal di Roblox modern

Hyperion menolak proxy sebelum `DllMain` jalan. Gejala:

- Verify timeout  
- Tidak ada `%TEMP%\bloxhub_test.txt`  
- Roblox bisa tampilkan "failed to load dxgi.dll"  

**Jangan buang waktu debug sideload dulu** — fokus inject.

---

## Update Offset Setelah Roblox Patch

1. Taruh dump baru di `offsets/raw/`
2. Copy ke project:
   ```cmd
   copy /Y offsets\raw\offsets.h include\offsets.hpp
   ```
3. Rebuild:
   ```cmd
   cmake --build build --config Release
   ```
4. Update path Bloxstrap ke versi baru

---

## Troubleshooting

### `Injection failed` / `OpenProcess` error

Jalankan **as Administrator**. Pastikan Roblox sudah in-game (`RobloxPlayerBeta.dll` loaded).

### Inject OK, Roblox crash

Rebuild terbaru — TLS/SEH harus di-skip di `stomp_inject.cpp`. Cek log tidak ada error sebelum `DllMain returned`.

### Inject OK, tidak crash, tidak ada console

`DllMain` mungkin sudah jalan — console bisa tersembunyi. Lihat [`TODO.md`](TODO.md) Step 4 untuk bukti file.

### Inject "OK" tapi verify timeout

Expected sampai Step 6 — verify masih cek `%TEMP%` user, bukan sandbox Roblox.

### Versi mismatch

Loader print:
```text
[!] WARNING: Roblox path does not match offsets version!
```

Solusi: update `offsets.hpp` atau pakai folder Bloxstrap yang benar.

### Sideload verify timeout, tidak ada `bloxhub_test.txt`

Normal untuk Roblox + Hyperion. Bukan bug loader install — proxy tidak pernah di-load.

### File tertinggal di folder Roblox

Setelah sideload gagal, restore biasanya jalan saat Enter. Kalau sesi putus:

```cmd
del "C:\...\version-xxx\dxgi.dll"
del "C:\...\version-xxx\dxgi_orig.dll"
```

### `BloxHubInternal.dll not found`

Build ulang, pastikan DLL ada di folder yang sama dengan injector.

---

## File Sementara yang Mungkin Muncul

| Lokasi | File | Dari |
|--------|------|------|
| Folder Roblox | `dxgi.dll`, `dxgi_orig.dll` | Sideload install |
| Folder Roblox | `bloxhub_loaded.txt` | Payload sideload (jika jalan) |
| `C:\BloxHub\` | `test.txt` | Payload Step 4+ (belum diimplementasi) |
| `%TEMP%` user | `bloxhub_test.txt` | Legacy — tidak dipakai payload saat ini |
| Folder `BloxHub.exe` | `bloxhub_session.dat` | Session sideload |
