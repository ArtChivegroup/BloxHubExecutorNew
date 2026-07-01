# BloxHub Executor

Windows x64 loader research project for Roblox (Bloxstrap). Primary path: **module stomp inject** via `BloxHubInjector.exe`. Sideload via `dxgi.dll` proxy is blocked by Hyperion on current Roblox builds.

**Start here:** [`docs/STATUS.md`](docs/STATUS.md)

---

## 🎉 Fase 1 SELESAI!

Bukti: **`[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!`** di **DebugView** (Capture Global Win32).

---

## Quick start

### 1. Install DebugView
Download DebugView (Sysinternals): https://learn.microsoft.com/id-id/sysinternals/downloads/debugview

### 2. Setup DebugView
- Jalankan DebugView **as Administrator**
- Centang: **Capture → Capture Global Win32**

### 3. Build & Inject
```cmd
cmake -S . -B build
cmake --build build --config Release
cd build\bin\Release

REM 1. Buka Roblox, masuk game
REM 2. Jalankan as Administrator:
BloxHubInjector.exe
```

Roblox version harus sesuai dengan `include/offsets.hpp` (`offsets::roblox_version`).

### 4. Cek DebugView!
Berhasil jika terlihat pesan:
```text
[BloxHub] DllMain PROCESS_ATTACH - SUCCESS!
```

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
| Module stomp inject | ✅ **Fase 1 SELESAI! |
| Bukti `DllMain` | ✅ Terlihat di DebugView |
| `BloxHubInjector.exe` manual | ✅ workflow rekomendasi |
| `BloxHub.exe --inject` | ⏸ gunakan manual dulu |
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

