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
| Apa yang dicoba sekarang? | **Manual map inject** (`--inject`) |
| Apakah inject sudah terbukti jalan? | **Belum terkonfirmasi** — injector lapor sukses, tapi tidak ada bukti log/marker |
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

### Jalur B — Manual Map Inject (yang dipakai sekarang)

```
BloxHub.exe <path_roblox> --inject
```

1. Launch Roblox  
2. Tunggu proses game yang punya `RobloxPlayerBeta.dll`  
3. Manual map `BloxHubInternal.dll` + CFG bypass  
4. Panggil export `BloxHubInit`  
5. Tunggu marker/log verifikasi  

**Status:** ⚠️ **Infrastruktur jalan, payload belum terbukti**  
Log menunjukkan allocate/write/CFG/thread OK, tapi `%TEMP%\bloxhub_test.txt` tidak muncul → verify timeout.

---

## Artefak Build (Apa File Apa)

| File | Fungsi |
|------|--------|
| `BloxHub.exe` | Launcher utama — sideload **atau** `--inject` |
| `version.dll` | Source payload untuk `pe_patcher` (jadi proxy `dxgi.dll`) |
| `BloxHubInternal.dll` | Payload untuk manual map inject |
| `BloxHubInjector.exe` | Injector standalone |
| `include/offsets.hpp` | Offset game Roblox + slot CFG bypass |

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

## Hasil Uji Terakhir (Fakta, Bukan Spekulasi)

### Sideload `dxgi.dll`

- Install & launch: ✅  
- `bloxhub_test.txt`: ❌ tidak ada  
- Verify: ❌ timeout  
- Kesimpulan: proxy tidak di-load Hyperion  

### Inject `--inject` (PID 8872, `version-5cf2272675e145f5`)

- Wait game process: ✅  
- VirtualAllocEx + WriteProcessMemory: ✅  
- CFG auto-scan: ⚠️ RVA berbeda tiap run (`0x14F2738` vs `0x12E6128`)  
- `BloxHubInit` thread: ✅ selesai dalam 5 detik  
- `bloxhub_test.txt` di `%TEMP%` user: ❌ tidak ada  
- Verify: ❌ timeout  

### Hipotesis utama kenapa verify gagal (belum difix)

1. **Path temp salah** — `WriteLog` di dalam proses Roblox pakai `GetTempPath()` sandbox, bukan `%TEMP%` user  
2. **Manual map tidak panggil `DllMain`** — hanya `BloxHubInit`; marker file hanya ditulis di `DllMain`  
3. **CFG scan false positive** — kandidat bitmap tidak konsisten  
4. **Hyperion** — kode injected crash sebelum tulis file (thread "selesai" ≠ sukses)

---

## Offset — Cara Update Setelah Roblox Patch

1. Dump baru → taruh di `offsets/raw/`
2. Copy ke header:
   ```cmd
   copy /Y offsets\raw\offsets.h include\offsets.hpp
   ```
3. Tambahkan kembali blok `CfgBypass` di akhir `offsets.hpp` (kalau hilang saat copy):
   ```cpp
   namespace CfgBypass {
       inline constexpr uintptr_t BitmapPtr = 0x0;  // isi dari scan manual
       inline constexpr uintptr_t Whitelist = 0x0;
       inline constexpr uintptr_t InsertSet = 0x0;
   }
   ```
4. Rebuild:
   ```cmd
   cmake --build build --config Release
   ```
5. Pastikan path Bloxstrap = versi di `offsets::roblox_version`

**Catatan:** Offset di `offsets.hpp` adalah offset **game** (DataModel, Camera, dll). Offset **CFG Hyperion** terpisah — belum stabil, `BitmapPtr` masih `0x0` (pakai auto-scan).

---

## Cara Test Cepat (Saat Bangun Lagi)

```cmd
cd build\bin\Release

REM Wajib: CMD as Administrator
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-5cf2272675e145f5" --inject
```

Cek:
```cmd
type %TEMP%\bloxhub_test.txt
dir /s /b %USERPROFILE%\AppData\bloxhub_test.txt
```

Kalau inject "OK" tapi file tidak ada di `%TEMP%` — lihat bagian hipotesis di atas.

---

## Dokumen Lain (Urutan Baca)

1. **STATUS.md** ← kamu di sini  
2. `USAGE.md` — perintah lengkap + troubleshooting  
3. `BUILD.md` — compile + update offset  
4. `ARCHITECTURE.md` — diagram komponen  
5. `BUGS.md` — daftar masalah terbuka  
6. `PLANNING.md` — apa yang dikerjakan berikutnya  
7. `../checkpoints/CHECKPOINT_CURRENT.md` — ringkasan sesi terakhir  

**Jangan mulai dari:** checkpoint Juli 01 tentang "loader-first sideload" — sudah outdated. Hyperion mengubah prioritas ke inject.

---

## Saat Kamu Kembali — 3 Langkah Pertama

1. Baca dokumen ini lagi (5 menit)  
2. Pastikan versi Roblox = versi di `offsets.hpp`  
3. Jalankan `--inject` as Admin, catat log terminal + cari `bloxhub_test.txt` di seluruh `AppData`

Jangan utak-atik sideload dulu — itu jalur mati untuk Roblox saat ini.
