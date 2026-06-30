
# BloxHub Executor

Executor Roblox dengan fokus stealth dan keandalan.

---

## Related Documentation
- **For architecture details**: [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- **For build instructions**: [docs/BUILD.md](docs/BUILD.md)
- **For usage guide**: [docs/USAGE.md](docs/USAGE.md)
- **For known bugs**: [docs/BUGS.md](docs/BUGS.md)
- **For roadmap & planning**: [docs/PLANNING.md](docs/PLANNING.md)
- **For progress checkpoints**: [checkpoints/CHECKPOINT_20260701.md](checkpoints/CHECKPOINT_20260701.md)

---

## Dokumentasi Lengkap
- [Arsitektur Sistem](docs/ARCHITECTURE.md) - Penjelasan komponen dan arsitektur "Silent Bridge"
- [Cara Build](docs/BUILD.md) - Langkah-langkah build project
- [Cara Penggunaan](docs/USAGE.md) - Panduan penggunaan
- [Daftar Bug](docs/BUGS.md) - Bug dan masalah yang sedang terjadi
- [Planning & Roadmap](docs/PLANNING.md) - Rencana pengembangan
- [Checkpoints](checkpoints/) - Catatan progres dan milestone (lihat `checkpoints/CHECKPOINT_20260701.md` untuk update terakhir!)

---

## Struktur Proyek
```
BloxHubExecutorNew/
├── include/
│   ├── injector.hpp      # Header manual map injector
│   └── offsets.hpp       # Offset Roblox (version-1a951716f19e4638)
├── src/
│   ├── injector/
│   │   ├── manual_map.cpp
│   │   └── hijack.cpp
│   ├── internal/
│   │   └── dllmain.cpp
│   ├── BloxHubInjector.cpp
│   ├── BloxHubLoader.cpp
│   └── BloxHub.cpp        # Modern Loader (baru!)
├── vendor/
│   ├── pe_bliss/
│   └── pe/
├── docs/                   # Dokumentasi utama (arsitektur, build, bugs, dll.)
├── checkpoints/            # Catatan progres dan milestone (lihat `checkpoints/CHECKPOINT_20260701.md`!)
├── EXAMPLE PROJECT/        # Contoh referensi (CFG Bypass, dll.)
├── CMakeLists.txt
└── README.md
```

---

## Quick Start
Lihat [Cara Build](docs/BUILD.md) dan [Cara Penggunaan](docs/USAGE.md).

Untuk update progres terakhir, lihat [Checkpoints](checkpoints/CHECKPOINT_20260701.md)!

---

## Catatan Penting
- Import Hijacking dicabut karena terdeteksi Hyperion! Lihat [Daftar Bug](docs/BUGS.md) dan [Checkpoint Terakhir](checkpoints/CHECKPOINT_20260701.md).
- Rencana selanjutnya: Pivot ke DLL Proxying! Lihat [Planning & Roadmap](docs/PLANNING.md).
