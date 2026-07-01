
# BloxHub Executor

Executor Roblox dengan **Manual Map Injection + CFG Bypass** untuk evasi Hyperion.

---
## Related Documentation
- **For architecture details**: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- **For build instructions**: [docs/BUILD.md](docs/BUILD.md)
- **For usage guide**: [docs/USAGE.md](docs/USAGE.md)
- **For known bugs**: [docs/BUGS.md](docs/BUGS.md)
- **For roadmap & planning**: [docs/PLANNING.md](docs/PLANNING.md)
- **For latest checkpoint**: [checkpoints/CHECKPOINT_20260701_MANUALMAP.md](checkpoints/CHECKPOINT_20260701_MANUALMAP.md)

---
## Status Terkini

| Metode | Status |
|--------|--------|
| DLL Proxying (Import Hijack + PE Patch) | ❌ **Dibatalkan** — Hyperion signature check |
| **Manual Map + CFG Bypass** | 🔄 **In Progress** — Injection OK, CRT fix pending |

---
## Arsitektur (Manual Map + CFG Bypass)

```
BloxHubInjector.exe
  │
  ├── Wait RobloxPlayerBeta.exe
  ├── Manual Map BloxHubInternal.dll → Roblox process
  ├── CFG Bypass (bitmap patching dari RBX-cfg-bypass)
  └── CreateRemoteThread → BloxHubInit()
```

---
## Struktur Proyek
```
BloxHubExecutorNew/
├── include/
│   ├── injector.hpp        # Injection API
│   └── offsets.hpp         # Roblox runtime offsets (v1a951716f19e4638)
├── src/
│   ├── BloxHubInjector.cpp # CLI Injector entry point
│   ├── BloxHub.cpp         # DLL Proxy loader (legacy)
│   ├── injector/
│   │   ├── manual_map.cpp  # Manual mapper + CFG bypass integration
│   │   ├── cfg_bypass.h    # CFG bypass header
│   │   └── cfg_bypass.cpp  # Bitmap scanner + patcher
│   └── internal/
│       ├── dllmain.cpp     # Payload DLL (BloxHubInternal)
│       ├── pe_patcher.cpp  # PE patcher (legacy)
│       └── *_proxy.cpp     # DLL proxy files (legacy)
├── docs/                   # Dokumentasi
├── checkpoints/            # Progress tracking
├── EXAMPLE PROJECT/        # Referensi (RBX-cfg-bypass, 3LayersPersistence)
├── CMakeLists.txt
└── README.md
```

---
## Quick Start
Lihat [Cara Build](docs/BUILD.md) dan [Cara Penggunaan](docs/USAGE.md).

Untuk update progres terakhir, lihat [Checkpoint Terbaru](checkpoints/CHECKPOINT_20260701_MANUALMAP.md).

---
## Next Step (Prioritas)
1. ⭐ **Rewrite `dllmain.cpp` — Pure Win32 API (no CRT)** — fix CRT dependency crash
2. Verifikasi CFG bitmap patching berfungsi
3. Thread Hijacking (ganti CreateRemoteThread)
4. Direct Syscalls (evasi hook Hyperion)
