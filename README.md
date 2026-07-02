# BloxHub Executor

Windows x64 loader research project for Roblox (Bloxstrap). Primary path: **module stomp inject** via `BloxHubInjector.exe` or `BloxHub.exe --inject`. Sideload via `dxgi.dll` proxy is blocked by Hyperion on current Roblox builds.

**Start here:** [`docs/STATUS.md`](docs/STATUS.md)

---

## 🎉 Fase 1 & Step 4-6 SELESAI!

**Fase 1:** `DllMain` berhasil dipanggil - bukti: **`[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!`** di **DebugView**  
**Step 4:** File write berhasil - bukti: file **dapat dibuat dari payload**  
**Step 5:** Log multi-baris berhasil - bukti: **3 baris log dapat dituliskan**  
**Step 6:** Verify hijau berhasil - bukti: **terminal menampilkan `[VERIFY] Payload loaded successfully!`**

---

## Quick start

### 1. Opsi 1: Menggunakan `BloxHub.exe` (Recommended)
```cmd
cmake -S . -B build
cmake --build build --config Release
cd build\bin\Release

REM Jalankan sebagai Administrator:
BloxHub.exe <path_ke_RobloxPlayerBeta.exe> --inject
```
Perhatikan terminal! Jika berhasil, akan muncul `[VERIFY] Payload loaded successfully!`.

### 2. Opsi 2: Menggunakan `BloxHubInjector.exe` (Manual)
1. Install dan jalankan DebugView **as Administrator**, centang **Capture → Capture Global Win32**
2. Buka Roblox dan masuk ke game
3. Jalankan `BloxHubInjector.exe` **as Administrator**
4. Periksa DebugView dan file `C:\BloxHub\test.txt` (catatan: file akan dihapus jika menggunakan `BloxHub.exe`, tapi tidak jika menggunakan `BloxHubInjector.exe`)

Roblox version harus sesuai dengan `include/offsets.hpp` (`offsets::roblox_version`).

---

## Repository layout

```text
BloxHubExecutorNew/
├── CMakeLists.txt
├── README.md
├── docs/                 # Dokumentasi
├── include/
│   ├── injector.hpp
│   └── offsets.hpp       # Offset game aktif (dari offsets/raw)
├── offsets/raw/          # Output roblox-dumper
├── src/
│   ├── BloxHub.cpp       # Loader utama
│   ├── BloxHubInjector.cpp
│   ├── injector/         # stomp_inject + tp_execute
│   └── internal/         # pe_patcher, payload
└── checkpoints/          # Catatan sesi (CHECKPOINT_CURRENT.md)
```

---

## Build targets

| Target | Output | Peran |
|--------|--------|------|
| `BloxHub` | `BloxHub.exe` | Launcher: sideload atau `--inject` |
| `BloxHubInjector` | `BloxHubInjector.exe` | **Inject manual** (workflow rekomendasi) |
| `BloxHubInternal` | `BloxHubInternal.dll` | Payload stomp |
| `proxy_payload` | `version.dll` | Source proxy sideload |

---

## Documentation (Bahasa Indonesia)

| Dokumen | Tujuan |
|---------|--------|
| [`docs/STATUS.md`](docs/STATUS.md) | Status proyek saat ini |
| [`docs/TODO.md`](docs/TODO.md) | Checklist langkah demi langkah (satu langkah per sesi) |
| [`docs/USAGE.md`](docs/USAGE.md) | Perintah & troubleshooting |
| [`docs/BUILD.md`](docs/BUILD.md) | Build & update offset |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Komponen & alur |
| [`docs/BUGS.md`](docs/BUGS.md) | Masalah yang diketahui |
| [`docs/PLANNING.md`](docs/PLANNING.md) | Prioritas selanjutnya |

---

## Project status (summary)

| Item | Status |
|------|--------|
| Module stomp inject | ✅ **Fase 1 SELESAI!** |
| Bukti `DllMain` | ✅ Terlihat di DebugView |
| **Step 4: File write** | ✅ **SELESAI!** |
| **Step 5: Log multi-baris** | ✅ **SELESAI!** |
| **Step 6: Verify hijau** | ✅ **SELESAI!** |
| `BloxHubInjector.exe` manual | ✅ workflow rekomendasi |
| `BloxHub.exe --inject` | ✅ verify berhasil! |
| Sideload `dxgi.dll` | ❌ diblokir Hyperion |
| Offset | `version-5cf2272675e145f5` |

Detail: [`docs/STATUS.md`](docs/STATUS.md)

---

## Update offset setelah Roblox patch

```cmd
copy /Y offsets\raw\offsets.h include\offsets.hpp
cmake --build build --config Release
```

Lihat [`docs/BUILD.md`](docs/BUILD.md).
