
# Arsitektur BloxHub Executor

## Overview
BloxHub Executor adalah executor untuk Roblox yang dirancang dengan arsitektur **Modern Loader + Silent Bridge** untuk evasi Hyperion yang maksimal.

---

## Evolusi Arsitektur

### Sebelumnya
- BloxHubLoader.exe (PE Editor Import Hijacking)
- BloxHubInjector.exe (Manual Map)
- BloxHubClient.exe (TCP Client)
- BloxHubInternal.dll (Payload)

### Sekarang: Modern Loader
Semua komponen digabung menjadi **BloxHub.exe** (The Modern Loader) yang cerdas dan dinamis.

---

## Komponen Utama

### 1. BloxHub.exe (Modern Loader)
**Fungsi**: Gabungan Loader, PE Editor, dan UI Client.  
**Tanggung Jawab**:
- **Authentication & Licensing**: Verifikasi HWID dan Key pengguna ke server (opsional, tahap akhir)
- **Dynamic Payload Fetching**: Download payload terenkripsi dari server (opsional, tahap akhir)
- **Smart Import Hijacking**:
  - Auto-detect folder versi Roblox saat ini
  - Backup otomatis RobloxPlayerBeta.exe
  - Generate nama DLL acak (mirip DLL sistem: `msvcp140_app.dll`, `winmm.dll`, dll)
  - Modifikasi Import Table Roblox dengan nama DLL acak tersebut
- **Restore/Uninject**: Restore RobloxPlayerBeta.exe dari backup dan hapus FakeDLL.dll saat Roblox ditutup
- **UI & TCP Client**: Setelah Roblox berjalan, bertransisi menjadi UI untuk mengirim script dan menerima log via Silent Bridge

### 2. FakeDLL.dll (BloxHubInternal)
**Fungsi**: Payload DLL yang diinjek ke Roblox dengan nama acak.  
**Tanggung Jawab**:
- **PE Header Wiping**: Hapus PE header sendiri dari memory saat DLL_PROCESS_ATTACH
- **TCP Listener (Dynamic Port)**: Listen koneksi dari BloxHub.exe di port dinamis per-sesi
- **Execution Queue**: Thread-safe queue untuk menyimpan payload Lua
- **Scheduler Hook**: Hook ke Task Scheduler Roblox untuk mengeksekusi payload di main thread (whitelisted)
- **Enkripsi Payload**: Payload Lua disimpan terenkripsi di memory

---

## Arsitektur Modern Loader + Silent Bridge
```
[ INTERNET / SERVER ]
      │
      ▼ (1. Auth & Fetch Encrypted Payload) [OPSIONAL - Tahap Akhir]
┌─────────────────────────────────┐
│      BloxHub.exe (Loader)       │
│  - Auth HWID                    │
│  - Fetch Payload (Encrypted)    │
│  - Decode Payload -> FakeDLL.dll│
│  - Smart Import Hijack Roblox   │
│  - Restore/Uninject             │
│  - Spawn UI & TCP Client        │
└─────────────┬───────────────────┘
              │ (2. Drop FakeDLL.dll to Roblox Folder)
              │ (3. Execute RobloxPlayerBeta.exe)
              ▼
[ WINDOWS LOADER ]
              │ (4. Loads FakeDLL.dll via Import Table)
              ▼
┌─────────────────────────────────┐
│   FakeDLL.dll (BloxHubInternal) │
│  - DLL_PROCESS_ATTACH: Wipe PE  │
│  - TCP Listener (Dynamic Port)  │
│  - Task Scheduler Hook          │
│  - Thread-safe Execution Queue  │
│  - LuaVM Integration            │
└─────────────┬───────────────────┘
              │ (5. Localhost TCP Bridge: 127.0.0.1:[Dynamic Port])
              ▼
┌─────────────────────────────────┐
│      BloxHub.exe (UI Client)    │
│  - Input Script Lua             │
│  - Encrypt & Send Payload       │
│  - Receive Logs & Output        │
└─────────────────────────────────┘
```

---

## Teknik Evasi Hyperion
1. **Anti-Disk Scanning**: Payload DLL bernama acak mirip DLL sistem, tidak plain `BloxHubInternal.dll`
2. **PE Header Wiping**: DLL menghapus PE header sendiri dari memory saat attach
3. **Dynamic Port Handshake**: Port TCP Silent Bridge dinamis per-sesi, tidak statis 8888
4. **Scheduler Hooking**: Eksekusi Lua di main thread Roblox (ter-whitelist anti-tamper)
5. **Dynamic Payload Fetching**: Payload tidak ada di disk sebelum dijalankan (opsional, tahap akhir)

---

## Tech Stack
- **Bahasa**: C++20
- **Build System**: CMake
- **Library**: pe_bliss (PE manipulation), Winsock (TCP)
- **Target**: Windows 10+ x64
