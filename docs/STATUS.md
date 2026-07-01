# Status Proyek — Baca Ini Dulu

**Terakhir diperbarui:** Juli 2026  
**Roblox versi offset saat ini:** `version-5cf2272675e145f5`  
**Folder offset mentah:** `offsets/raw/offsets.h`

Dokumen ini adalah **satu sumber kebenaran** untuk kondisi proyek sekarang. Kalau bingung, mulai di sini — bukan dari checkpoint lama atau chat AI.

---

## TL;DR — Apa yang Perlu Kamu Tahu

| Pertanyaan | Jawaban singkat |
|------------|-----------------|
| Apa tujuan proyek ini? | Riset loader Windows untuk menjalankan payload di Roblox |
| Apa yang **tidak** jalan? | **Sideload `dxgi.dll`** — diblokir Hyperion sebelum `DllMain` |
| Apa yang dicoba sekarang? | **Module stomp inject** (`--inject`, Riviera-style) |
| Apakah inject sudah terbukti jalan? | **Belum terkonfirmasi** — uji: console `[BloxHub] DllMain PROCESS_ATTACH` di Roblox |
| Versi Roblox harus cocok? | **Ya** — path Bloxstrap harus sama dengan `offsets::roblox_version` |
| Build di mana? | `build\bin\Release\` |
| Perlu Admin? | **Ya** untuk mode `--inject` |

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

```
BloxHub.exe <path_roblox> --inject
```

1. Launch Roblox  
2. Tunggu `RobloxPlayerBeta.dll` loaded  
3. Map `d3d10warp.dll` ke proses → stomp dengan `BloxHubInternal.dll`  
4. Manual map lengkap (reloc, import, SEH, TLS) + **DllMain** via IoCompletion  
5. Bukti Fase 1: console di Roblox (bukan verify `%TEMP%` dulu)  

**Status:** ⚠️ **Injector baru — menunggu uji**  
Injector lama (VirtualAlloc + CFG scan + `BloxHubInit`) diganti Juli 2026.

---

## Artefak Build (Apa File Apa)

| File | Fungsi |
|------|--------|
| `BloxHub.exe` | Launcher utama — sideload **atau** `--inject` |
| `version.dll` | Source payload untuk `pe_patcher` (jadi proxy `dxgi.dll`) |
| `BloxHubInternal.dll` | Payload untuk manual map inject |
| `BloxHubInjector.exe` | Injector standalone |
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

```
PREFLIGHT → LAUNCH → WAIT GAME PID → INJECT → VERIFY
```

- Tidak ada INSTALL/RESTORE file Roblox
- Verify cek `%TEMP%\bloxhub_payload_loaded.txt` dan `bloxhub_test.txt`

---

## Hasil Uji Terakhir

### Inject lama (pre-migrasi) — historis

- VirtualAlloc + CFG scan → false positive `0x800000`  
- `BloxHubInit` thread OK, payload tidak terbukti hidup  

### Inject stomp (saat ini)

- **Belum diuji** setelah migrasi — lihat [`TODO.md`](TODO.md) Step 1  

### Verify `%TEMP%`

- Masih timeout expected — path sandbox Roblox; fix di TODO Step 4–6  

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

```cmd
cd build\bin\Release

REM Wajib: CMD as Administrator
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-5cf2272675e145f5" --inject
```

Cek bukti Fase 1:
- Console di Roblox: `[BloxHub] DllMain PROCESS_ATTACH`
- Log terminal: `Stomp module mapped`, `DllMain dispatched`

---

## Dokumen Lain (Urutan Baca)

1. **STATUS.md** ← kamu di sini  
2. **`TODO.md`** — checklist kerja step-by-step (satu step per sesi)  
3. `USAGE.md` — perintah lengkap + troubleshooting  
4. `BUILD.md` — compile + update offset  
5. `ARCHITECTURE.md` — diagram komponen  
6. `BUGS.md` — daftar masalah terbuka  
7. `PLANNING.md` — prioritas besar (P0–P4)  
8. `../checkpoints/CHECKPOINT_CURRENT.md` — ringkasan sesi terakhir (Checkpoint 01)

---

## Saat Kamu Kembali — 3 Langkah Pertama

1. Baca dokumen ini lagi (5 menit)  
2. Pastikan versi Roblox = versi di `offsets.hpp`  
3. Jalankan `--inject` as Admin — cari **console di Roblox**, bukan hanya log terminal

Jangan utak-atik sideload dulu — itu jalur mati untuk Roblox saat ini.
