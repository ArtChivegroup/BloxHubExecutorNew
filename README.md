
# BloxHub Executor

Executor Roblox dengan fokus stealth dan keandalan.

---

## Dokumentasi Lengkap
- [Arsitektur Sistem](docs/ARCHITECTURE.md) - Penjelasan komponen dan arsitektur "Silent Bridge"
- [Cara Build](docs/BUILD.md) - Langkah-langkah build project
- [Cara Penggunaan](docs/USAGE.md) - Panduan penggunaan
- [Daftar Bug](docs/BUGS.md) - Bug dan masalah yang sedang terjadi
- [Planning & Roadmap](docs/PLANNING.md) - Rencana pengembangan

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
│   └── BloxHubLoader.cpp
├── vendor/
│   ├── pe_bliss/
│   └── pe/
├── docs/                 # Dokumentasi lengkap
├── CMakeLists.txt
└── README.md
```

---

## Quick Start
Lihat [Cara Build](docs/BUILD.md) dan [Cara Penggunaan](docs/USAGE.md).

---

## Catatan Penting
- Testing Notepad dihapus karena tidak relevan
- BloxHubLoader saat ini menyebabkan "Roblox Damaged"
- Fokus pengembangan adalah "Silent Bridge" arsitektur
