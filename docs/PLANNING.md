
# Planning & Roadmap

## Evolusi Arsitektur: Modern Loader Integration
BloxHub akan diadaptasi ke **Modern Loader** yang memisahkan frontend dan backend, serta mengelola payload secara dinamis untuk evasi Hyperion yang lebih baik.

---

## Versi 1.0 - Dasar (Belum Selesai, Perlu Improvement & Testing)
- [x] Setup project CMake
- [ ] Implementasi BloxHubLoader (Import Hijacking) - Perlu fix "Roblox Damaged"
- [ ] Implementasi BloxHubInjector (Manual Map) - Perlu testing & improvement stealth
- [ ] Implementasi BloxHubInternal.dll (logging dasar) - Perlu improve stability

---

## Versi 2.0 - Modern Loader (Tahap 1: Dasar)
- [ ] Gabungkan BloxHubLoader dan BloxHubClient menjadi **BloxHub.exe** (Modern Loader)
- [ ] Implementasi **Smart Import Hijacking**:
  - Auto-detect folder versi Roblox saat ini
  - Backup otomatis RobloxPlayerBeta.exe
  - Generate nama DLL acak (mirip DLL sistem, misal: `msvcp140_app.dll`, `winmm.dll`)
  - Modifikasi Import Table dengan nama DLL acak tersebut
- [ ] Implementasi **Restore/Uninject**:
  - Restore RobloxPlayerBeta.exe dari backup saat Roblox ditutup
  - Hapus FakeDLL.dll dari folder Roblox otomatis
- [ ] Fix bug koneksi TCP listener (DLL side)
- [ ] Implementasi **Dynamic Port Handshake**: Port TCP Silent Bridge dihitung secara dinamis per-sesi

---

## Versi 2.1 - Modern Loader (Tahap 2: Silent Bridge)
- [ ] Implementasi thread-safe **Execution Queue** di DLL
- [ ] Implementasi **Task Scheduler Hook** di DLL
- [ ] Implementasi **PE Header Wiping** di DllMain DLL_PROCESS_ATTACH
- [ ] Implementasi enkripsi payload Lua di memory

---

## Versi 2.2 - Modern Loader (Tahap 3: Dynamic Payload)
- [ ] Implementasi **Dynamic Payload Fetching**:
  - Gunakan WinHTTP/CPR untuk fetch payload dari server
  - Payload dienkripsi unik per-HWID
  - Drop payload hanya sebagai file sementara (temp) dengan nama acak
- [ ] (Opsional) Authentication & Licensing (HWID + Key)

---

## Versi 3.0 - Lua Execution
- [ ] Integrasi dengan LuaVM Roblox
- [ ] Set identity level (6/8)
- [ ] Executor Lua dasar di UI
- [ ] Script hub

---

## Prioritas Tinggi
1. Gabungkan menjadi BloxHub.exe (Modern Loader dasar)
2. Smart Import Hijacking dengan nama DLL acak
3. Restore/Uninject otomatis
4. Fix bug koneksi TCP listener
5. PE Header Wiping di DLL

---

## Prioritas Sedang
1. Task Scheduler Hook
2. Execution Queue thread-safe
3. Dynamic Port Handshake
4. Enkripsi payload Lua

---

## Prioritas Rendah
1. Dynamic Payload Fetching dari server
2. Authentication & Licensing
3. GUI Client (sekarang bisa CLI dulu)
4. Script hub online
5. Auto-update

---

## Arsitektur Modern Loader
```
[ INTERNET / SERVER ]
      │
      ▼ (1. Auth & Fetch Encrypted Payload)
┌─────────────────────────────────┐
│      BloxHub.exe (Loader)       │
│  - Auth HWID                    │
│  - Fetch Payload (Encrypted)    │
│  - Decode Payload -> FakeDLL.dll│
│  - Import Hijack Roblox PE      │
│  - Spawn UI & TCP Client        │
└─────────────┬───────────────────┘
              │ (2. Drop FakeDLL.dll to Roblox Folder)
              │ (3. Execute RobloxPlayerBeta.exe)
              ▼
[ WINDOWS LOADER ]
              │ (4. Loads FakeDLL.dll)
              ▼
┌─────────────────────────────────┐
│   FakeDLL.dll (BloxHubInternal) │
│  - DllMain (Erase PE Header)   │
│  - Scheduler Hook (Polling)     │
│  - TCP Socket Listener (Dyn Port)│
└─────────────┬───────────────────┘
              │ (5. Localhost TCP Bridge)
              ▼
┌─────────────────────────────────┐
│      BloxHub.exe (UI Client)    │
│  - Input Script                 │
│  - Encrypt & Send               │
│  - Receive Logs                 │
└─────────────────────────────────┘
```
