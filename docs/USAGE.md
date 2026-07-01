# Cara Penggunaan

**Baca dulu:** [`STATUS.md`](STATUS.md) untuk konteks kondisi proyek saat ini.

---

## Prasyarat

- Windows x64  
- Build Release di `build\bin\Release\`  
- **CMD/PowerShell as Administrator** (untuk inject)  
- Path Roblox Bloxstrap **sama versi** dengan `offsets::roblox_version` di `include/offsets.hpp`  
- Semua artefak (`BloxHub.exe`, `BloxHubInternal.dll`, `version.dll`) di folder yang sama  

---

## Mode 1 — Inject (Disarankan)

Tidak memodifikasi file di folder Roblox.

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
[+] Roblox game process ready (PID: ...)   ← bisa sama atau beda dari launch PID
[*] Allocated remote image at: ...
[*] DLL written to remote process
[*] Executing CFG bypass ...
[+] BloxHubInit executed successfully!
[INJECT] OK
[VERIFY] Payload loaded successfully!      ← ideal; saat ini sering timeout
```

### Cek bukti inject

```cmd
type %TEMP%\bloxhub_test.txt
type %TEMP%\bloxhub_payload_loaded.txt

REM Kalau kosong, cari di seluruh AppData (Roblox sandbox temp):
dir /s /b %USERPROFILE%\AppData\bloxhub_test.txt
```

### Alternatif: injector standalone

Roblox sudah terbuka dulu:

```cmd
BloxHubInjector.exe
```

---

## Mode 2 — Sideload (Eksperimen / Hampir Pasti Gagal)

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
3. Pastikan blok `CfgBypass` masih ada di akhir file (lihat `STATUS.md`)
4. Rebuild:
   ```cmd
   cmake --build build --config Release
   ```
5. Update path Bloxstrap ke versi baru

---

## Troubleshooting

### `Injection failed` / `OpenProcess` error 87

**Sudah diperbaiki sebagian:** loader menunggu proses game dengan `RobloxPlayerBeta.dll`.  
Kalau masih gagal: jalankan **as Administrator**.

### Inject "OK" tapi verify timeout, tidak ada log

Lihat [`STATUS.md`](STATUS.md) — hipotesis utama:

- Log ditulis ke temp sandbox Roblox, bukan `%TEMP%` user  
- `BloxHubInit` crash di dalam proses  
- CFG scan salah target  

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

Build ulang, pastikan DLL ada di folder yang sama dengan `BloxHub.exe`.

---

## File Sementara yang Mungkin Muncul

| Lokasi | File | Dari |
|--------|------|------|
| Folder Roblox | `dxgi.dll`, `dxgi_orig.dll` | Sideload install |
| Folder Roblox | `bloxhub_loaded.txt` | Payload sideload (jika jalan) |
| `%TEMP%` user | `bloxhub_test.txt` | Payload log |
| `%TEMP%` user | `bloxhub_payload_loaded.txt` | Marker verify |
| Folder `BloxHub.exe` | `bloxhub_session.dat` | Session sideload |
