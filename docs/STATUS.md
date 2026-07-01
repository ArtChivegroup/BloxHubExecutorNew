# Status Proyek — Baca Ini Dulu

**Terakhir diperbarui:** 2 Juli 2026  
**Roblox versi offset saat ini:** `version-5cf2272675e145f5`  
**Folder offset mentah:** `offsets/raw/offsets.h`

Dokumen ini adalah **satu sumber kebenaran** untuk kondisi proyek sekarang. Kalau bingung, mulai di sini — bukan dari checkpoint lama atau chat AI.

---

## TL;DR — Apa yang Perlu Kamu Tahu

| Pertanyaan | Jawaban singkat |
|------------|-----------------|
| Apa tujuan proyek ini? | Riset loader Windows untuk menjalankan payload di Roblox |
| Apa yang **tidak** jalan? | **Sideload `dxgi.dll`** — diblokir Hyperion sebelum `DllMain` |
| Apa yang dicoba sekarang? | **Module stomp inject** (Riviera-style, user-mode) |
| Apakah inject sudah terbukti jalan? | **Fase 1 SELESAI!** — bukti: `[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!` di DebugView |
| Cara uji yang disarankan? | **`BloxHubInjector.exe`** — Roblox in-game dulu, lalu inject as Admin + cek DebugView |
| Versi Roblox harus cocok? | **Ya** — path Bloxstrap harus sama dengan `offsets::roblox_version` |
| Build di mana? | `build\bin\Release\` |
| Perlu Admin? | **Ya** untuk inject |

---

## Dua Jalur Teknis (Jangan Dicampur)

### Jalur A — Sideload / Proxy DLL (default tanpa flag)

```
BloxHub.exe <path_roblox>
```

1. Copy `dxgi.dll` asli → `dxgi_orig.dll` di folder Roblox  
2. Generate proxy `dxgi.dll` dari `version.dll` + `pe_patcher`  
3. Launch Roblox  
4. Tunggu marker `bloxhub_loaded.txt`  
5. Restore (hapus file proxy)

**Status:** ❌ **Gagal di Roblox modern**  
Roblox menolak proxy (`failed to load dxgi.dll`). `DllMain` tidak pernah jalan. Ini batasan Hyperion, bukan bug kecil di kode.

### Jalur B — Module Stomp Inject (yang dipakai sekarang)

**Manual (disarankan):**
```
Roblox in-game → BloxHubInjector.exe → cek DebugView
```

**Via launcher:**
```
BloxHub.exe <path_roblox> --inject
```

1. Tunggu proses game dengan `RobloxPlayerBeta.dll` loaded  
2. Map `d3d10warp.dll` ke proses → stomp dengan `BloxHubInternal.dll`  
3. Manual map: reloc + import (TLS/SEH **di-skip** — fix crash)  
4. **DllMain** via **IoCompletion async** (bukan CreateRemoteThread!)  
5. Bukti Fase 1: **`[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!`** di **DebugView** (wajib Capture Global Win32!)

**Status:** ✅ **Fase 1 SELESAI!**  
Inject stabil, `DllMain` terbukti dipanggil via DebugView.

---

## Artefak Build (Apa File Apa)

| File | Fungsi |
|------|--------|
| `BloxHub.exe` | Launcher utama — sideload **atau** `--inject` |
| `BloxHubInjector.exe` | **Injector manual** — tunggu Roblox in-game, lalu stomp inject |
| `version.dll` | Source payload untuk `pe_patcher` (jadi proxy `dxgi.dll`) |
| `BloxHubInternal.dll` | Payload untuk module stomp inject |
| `include/offsets.hpp` | Offset game Roblox (dari `offsets/raw/`) |

**Penting:** `version.dll` dipakai sideload. `BloxHubInternal.dll` dipakai inject. Jangan tertukar.

---

## State Machine BloxHub (Mode Sideload)

```
PREFLIGHT → INSTALL → LAUNCH → VERIFY → [Enter] → RESTORE
```

- Session file: `bloxhub_session.dat` di folder `BloxHub.exe`
- Verify timeout: 30 detik
- Marker: `bloxhub_loaded.txt` di folder Roblox, fallback `%TEMP%\bloxhub_payload_loaded.txt`

## Mode Inject (Tanpa Modifikasi Folder Roblox)

**BloxHubInjector (manual):**
```
WAIT GAME PID → DELAY 2s → INJECT → cek DebugView
```

**BloxHub.exe --inject:**
```
PREFLIGHT → LAUNCH → WAIT GAME PID → INJECT → VERIFY
```

- Tidak ada INSTALL/RESTORE file Roblox
- Verify cek `%TEMP%\bloxhub_payload_loaded.txt` — **masih broken** sampai Step 6

---

## Hasil Uji Terakhir (Fase 1 Selesai!)

### Test Terakhir (2 Juli 2026 — Sukses!)

**Test:** `BloxHubInjector.exe`, PID 9320, Roblox in-game, Admin, **DebugView Capture Global Win32**

| Cek | Hasil |
|-----|--------|
| Stomp map | ✅ `d3d10warp.dll` mapped |
| Payload write | ✅ ~32 KB |
| IoCompletion | ✅ `ZwSetIoCompletion OK` |
| DllMain | ✅ **Terlihat di DebugView!** `[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!` |
| Roblox crash | ✅ **tidak** |
| DebugView | ✅ **Bukti empiris!** |

**Fix penting untuk sukses:**
1. **`TpDirect` struct di `tp_execute.cpp` diperbaiki** agar match contoh Riviera
2. **Hapus restore section protect** di `stomp_inject.cpp` (karena IoCompletion async)
3. **Tambah `DisableThreadLibraryCalls`** di `dllmain.cpp`
4. **Ganti ke IoCompletion** (bukan CreateRemoteThread!)

---

## Offset — Cara Update Setelah Roblox Patch

1. Dump baru → taruh di `offsets/raw/`
2. Copy ke header:
   ```cmd
   copy /Y offsets\raw\offsets.h include\offsets.hpp
   ```
3. Rebuild:
   ```cmd
   cmake --build build --config Release
   ```
4. Pastikan path Bloxstrap = versi di `offsets::roblox_version`

**Catatan:** `offsets.hpp` = offset **game** saja. Injector stomp tidak butuh offset Hyperion.

---

## Cara Test Cepat (Saat Bangun Lagi)

### 1. Install DebugView
Download **DebugView** (Sysinternals) dari: https://learn.microsoft.com/id-id/sysinternals/downloads/debugview

### 2. Setup DebugView
- Jalankan DebugView **as Administrator**
- Di menu DebugView: **Capture → Capture Global Win32** (centang!)

### 3. Inject Roblox
```cmd
cd build\bin\Release

REM 1. Buka Roblox, masuk game
REM 2. CMD as Administrator:
BloxHubInjector.exe
```

### 4. Cek DebugView
Jika berhasil, akan terlihat:
```text
[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!
```

---

## Dokumen Lain (Urutan Baca)

1. **STATUS.md** ← kamu di sini  
2. **`TODO.md`** — checklist kerja step-by-step (satu step per sesi)  
3. `USAGE.md` — perintah lengkap + troubleshooting  
4. `BUILD.md` — compile + update offset  
5. `ARCHITECTURE.md` — diagram komponen  
6. `BUGS.md` — daftar masalah terbuka  
7. `PLANNING.md` — prioritas besar (P0–P4)  
8. `../checkpoints/CHECKPOINT_CURRENT.md` — ringkasan sesi terakhir (Checkpoint 03 — Fase 1 Selesai!)

---

## Saat Kamu Kembali — 3 Langkah Pertama

1. Baca dokumen ini lagi (5 menit)  
2. Pastikan versi Roblox = versi di `offsets.hpp`  
3. Jalankan `BloxHubInjector.exe` as Admin (Roblox in-game) — **cek DebugView**!

Jangan utak-atik sideload dulu — itu jalur mati untuk Roblox saat ini.
