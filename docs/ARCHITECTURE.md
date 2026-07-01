
# Arsitektur BloxHub Executor

---
## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For known bugs**: [BUGS.md](BUGS.md)
- **For roadmap & planning**: [PLANNING.md](PLANNING.md)
- **For latest checkpoint**: [../checkpoints/CHECKPOINT_20260701_MANUALMAP.md](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)

---
## Overview
BloxHub Executor adalah executor untuk Roblox dengan **Manual Map Injection + CFG Bypass**
untuk evasi Hyperion.

---
## Evolusi Arsitektur

### Sebelumnya (DIBATALKAN)
- **DLL Proxying**: Import Hijacking + Dynamic PE Patching
- Gagal karena: Hyperion verifikasi signature DLL grafis (dxgi.dll), DLL lain tidak dicari di app folder

### Sekarang: Manual Map + CFG Bypass
Metode inject DLL langsung ke memori Roblox tanpa LoadLibrary → melewati signature check.
CFG bypass dari RBX-cfg-bypass memungkinkan eksekusi kode.

---
## Komponen Utama

### 1. BloxHubInjector.exe (Injector)
**Fungsi**: CLI injector yang menunggu Roblox, lalu inject DLL via Manual Map.
**Tanggung Jawab**:
- Cari RobloxPlayerBeta.exe via CreateToolhelp32Snapshot
- OpenProcess → VirtualAllocEx → alokasi memori di target
- Manual Map: copy sections, relocate, resolve imports
- CFG Bypass: auto-scan bitmap → patch via WriteProcessMemory
- CreateRemoteThread → panggil BloxHubInit export

### 2. BloxHubInternal.dll (Payload)
**Fungsi**: DLL minimal yang di-inject ke Roblox.
**Tanggung Jawab**:
- Export `BloxHubInit` — dipanggil injector via CreateRemoteThread
- DllMain — logging dasar ke %TEMP%\bloxhub_test.txt
- **WAJIB**: pure Win32 API, tanpa CRT dependency

### 3. cfg_bypass.h / cfg_bypass.cpp (CFG Bypass Module)
**Fungsi**: Core library untuk bypass Hyperion Control Flow Guard.
**Fitur**:
- `PatchCfgBitmap`: Auto-scan .rdata/.data → temukan bitmap pointer → flip bit untuk region DLL
- `BypassCfgForRegion`: Layer 1 (bitmap) + Layer 2 (whitelist via shellcode)
- Filter smart: page-aligned, AllocationBase, min 1MB addr, min 64KB region

### 4. offsets.hpp (Runtime Offsets)
**Fungsi**: Database offset untuk Roblox version-1a951716f19e4638.
**Kategori**:
- Game objects (Player, Workspace, Camera, dll.)
- Lua VM functions (LuauLoad, LuaResume, LuaNewThread)
- **CFG Bypass** (BitmapPtr, Whitelist, InsertSet)

---
## Alur Kerja Manual Map + CFG Bypass

```
[BloxHubInjector.exe]
    │
    ├── 1. Wait for RobloxPlayerBeta.exe
    ├── 2. OpenProcess(PROCESS_ALL_ACCESS)
    ├── 3. Read BloxHubInternal.dll from disk
    │
    ├── 4. Parse PE headers
    ├── 5. VirtualAllocEx → alokasi SizeOfImage di Roblox (RWX)
    │
    ├── 6. Manual Map:
    │     ├── Copy sections (.text, .rdata, .data, etc.)
    │     ├── Apply base relocations (IMAGE_REL_BASED_DIR64)
    │     ├── Resolve imports (kernel32, user32 only!)
    │     └── WriteProcessMemory → tulis ke remote
    │
    ├── 7. CFG Bypass:
    │     ├── Scan .rdata/.data RobloxPlayerBeta.dll
    │     ├── Filter: page-aligned, AllocationBase, min 64KB
    │     ├── Temukan bitmap pointer (RVA 0x1432808 → 0x920000)
    │     └── Patch bitmap bits untuk region DLL kita
    │
    ├── 8. Find BloxHubInit export di DLL
    ├── 9. CreateRemoteThread → panggil BloxHubInit
    │
    ↓
[RobloxPlayerBeta.exe]
    │
    ├── Thread baru execute BloxHubInit
    ├── DllMain dipanggil (DLL_PROCESS_ATTACH)
    ├── WriteLog ke %TEMP%\bloxhub_test.txt
    └── (Future) Hook TaskScheduler → execute Lua
```

---
## Tech Stack
- **Bahasa**: C++20
- **Build System**: CMake
- **Target DLL**: Pure Win32 API (kernel32.dll, user32.dll)
- **Library**: Psapi (module enumeration), TlHelp32 (process/thread snapshots)
- **Referensi**: `EXAMPLE PROJECT/RBX-cfg-bypass-main`
- **Target**: Windows 10+ x64

---
## Struktur File

```
src/
├── BloxHubInjector.cpp     # CLI injector entry point
├── BloxHub.cpp             # DLL Proxy loader (legacy)
├── injector/
│   ├── manual_map.cpp      # Manual mapper + CFG bypass integration
│   ├── cfg_bypass.h        # CFG bypass structures & API
│   └── cfg_bypass.cpp      # Bitmap scanner + patcher
├── internal/
│   ├── dllmain.cpp         # Payload DLL (BloxHubInternal)
│   ├── pe_patcher.h/.cpp   # PE patcher (legacy, dari 3LayersPersistence)
│   └── *_proxy.cpp         # DLL proxy files (legacy)
include/
├── injector.hpp            # Injection API
└── offsets.hpp             # Roblox runtime offsets
```

---
## Referensi
- [Checkpoint Terbaru](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)
- [Daftar Bug](BUGS.md)
