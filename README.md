# BloxHub Executor

BloxHub Executor adalah repositori riset Windows untuk eksperimen pemuatan payload ke Roblox. Basis kode saat ini memiliki dua jalur teknis:

1. `Loader track`: modifikasi artefak di disk, proxy generation, launch, lalu restore.
2. `Manual map track`: injector + manual mapping + CFG bypass.

Per 2026-07-01, prioritas proyek resmi dipindahkan ke `loader improvement`. Manual map tetap disimpan sebagai jalur riset pembanding, tetapi bukan fokus pengembangan aktif.

## Executive Summary

Repositori ini tidak lagi boleh dibaca sebagai "manual map project yang kebetulan punya loader". Kondisi terbaru justru kebalikannya:

- `BloxHub.exe` adalah baseline utama untuk pengembangan loader.
- `src/internal/pe_patcher.cpp` adalah aset PE/proxy terpenting yang sudah ada.
- `BloxHubLoader.exe` adalah PoC patch-import langsung ke executable target.
- `BloxHubInjector.exe` adalah track sekunder untuk pembanding teknis dan isolasi masalah payload.

Dokumentasi ini sengaja menekankan state source code apa adanya, gap yang masih terbuka, dan objective baru agar AI atau developer berikutnya langsung paham bahwa pekerjaan aktif sekarang adalah `membangun loader yang lebih matang daripada jalur manual map saat ini`.

## Current Objectives

- Menjadikan alur `backup -> install/patch -> launch -> verify -> restore` sebagai workflow resmi proyek.
- Mengembangkan loader dari baseline source yang sudah ada, bukan mulai dari nol.
- Membandingkan setiap keputusan loader terhadap batasan yang sebelumnya ditemui pada manual map.
- Menjaga manual map sebagai jalur riset cadangan dan baseline perilaku payload/injection.
- Memastikan semua dokumentasi inti merepresentasikan fokus `loader-first`.

## Document Index

- Gambaran arsitektur: `docs/ARCHITECTURE.md`
- Instruksi build: `docs/BUILD.md`
- Cara penggunaan dan workflow: `docs/USAGE.md`
- Bug, gap, dan risiko teknis: `docs/BUGS.md`
- Planning dan roadmap: `docs/PLANNING.md`
- Checkpoint aktif terbaru: `checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`
- Checkpoint historis manual map dan proxy lama: folder `checkpoints/`

## Status Snapshot

| Area | Status | Catatan |
|------|--------|---------|
| Loader improvement | Active priority | Fokus engineering utama |
| `BloxHub.exe` | Existing prototype | Launcher + proxy installer + cleanup dasar |
| `pe_patcher` | Reusable core | Dynamic export-forwarding proxy generator |
| `BloxHubLoader.exe` | Legacy PoC | Import table patching langsung ke EXE target |
| `BloxHubInjector.exe` | Secondary research | Manual map + CFG bypass untuk pembanding |
| `BloxHubInternal.dll` | Legacy payload | Masih dominan kebutuhan jalur manual map |

## What Exists Today

### 1. `BloxHub.exe`

Prototype loader modern yang saat ini:

- menerima path `RobloxPlayerBeta.exe` atau folder version,
- mencari payload `version.dll` di direktori executable loader,
- menyalin `System32\version.dll` menjadi `version_orig.dll` ke folder Roblox,
- membangun proxy DLL baru lewat `ConvertPayloadToProxy(...)`,
- menulis proxy `version.dll` ke folder Roblox,
- meluncurkan Roblox via `CreateProcessW`,
- menunggu input user sebelum cleanup,
- lalu menghapus file hasil instalasi sementara.

Ini adalah baseline terbaik untuk dikembangkan karena sudah memiliki bentuk launcher nyata, walau belum punya state machine, verifikasi pasca-instalasi, atau restore yang crash-safe.

### 2. `BloxHubLoader.exe`

PoC lama untuk patch import table langsung ke `RobloxPlayerBeta.exe`. Flow saat ini:

- membuat backup `RobloxPlayerBeta.exe.backup`,
- membuka target PE,
- menambahkan import `BloxHubInternal.dll!BloxHubInit`,
- lalu menyimpan perubahan ke executable target.

Jalur ini penting sebagai referensi desain PE patching, tetapi secara historis paling dekat dengan risiko integrity check Roblox/Hyperion.

### 3. `BloxHubInjector.exe`

Track sekunder berbasis manual map yang:

- menunggu proses `RobloxPlayerBeta.exe`,
- memuat `BloxHubInternal.dll`,
- melakukan manual mapping,
- menjalankan CFG bitmap patching,
- memanggil `BloxHubInit`.

Track ini tetap bernilai untuk validasi payload dan eksperimen bypass, tetapi tidak lagi menjadi tujuan pengembangan utama.

## Loader-First Interpretation

Saat membaca repositori ini, urutan prioritas yang benar adalah:

1. pahami `BloxHub.exe`,
2. pahami `pe_patcher`,
3. pahami `BloxHubLoader.exe`,
4. baru lihat manual map sebagai konteks historis dan pembanding.

Jika terjadi konflik narasi antara dokumen lama dan source code loader saat ini, anggap `loader-first` sebagai arah resmi terbaru.

## Repository Layout

```text
BloxHubExecutorNew/
├── include/
│   ├── injector.hpp
│   └── offsets.hpp
├── src/
│   ├── BloxHub.cpp
│   ├── BloxHubLoader.cpp
│   ├── BloxHubInjector.cpp
│   ├── injector/
│   │   ├── manual_map.cpp
│   │   ├── cfg_bypass.cpp
│   │   └── cfg_bypass.h
│   └── internal/
│       ├── dllmain.cpp
│       ├── pe_patcher.cpp
│       ├── pe_patcher.h
│       ├── version_proxy.cpp
│       └── *_proxy.cpp
├── docs/
├── checkpoints/
├── EXAMPLE PROJECT/
├── vendor/
├── CMakeLists.txt
└── README.md
```

## Files That Matter For Loader Work

- `src/BloxHub.cpp`: prototype launcher dan installer proxy loader.
- `src/BloxHubLoader.cpp`: baseline patch-import ke executable target.
- `src/internal/pe_patcher.cpp`: engine export-forwarding proxy generation.
- `src/internal/pe_patcher.h`: kontrak `ConvertPayloadToProxy(...)`.
- `src/internal/version_proxy.cpp`: payload/proxy basis yang saat ini digunakan oleh `BloxHub.exe`.
- `CMakeLists.txt`: definisi target build yang menunjukkan mana artefak aktif dan mana yang legacy.

## Current Reality And Constraints

- Dokumentasi lama berat ke manual map dan belum seluruhnya sinkron dengan fokus loader.
- Loader modern masih memakai target `version.dll`, sementara eksperimen lama menunjukkan target ini belum tentu ideal.
- Nama variabel dan pesan log di jalur loader masih menyimpan istilah historis seperti `dxgi`.
- Restore saat ini masih berupa cleanup file sementara, belum rollback yang tahan crash.
- Belum ada fase `verify` yang tegas untuk membuktikan payload benar-benar termuat setelah launch.

## Immediate Development Direction

1. Menetapkan baseline loader resmi dan checkpoint pengembangannya.
2. Membandingkan pendekatan `proxy/sideload` melawan `PE patch / section rewrite` ala Volt.
3. Menyatukan backup, install, launch, verify, dan restore ke satu workflow yang jelas.
4. Mendokumentasikan setiap asset, gap, dan objective supaya AI bisa langsung melanjutkan pekerjaan loader tanpa membaca ulang seluruh histori percobaan manual map.
