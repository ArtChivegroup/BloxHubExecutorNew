
# Planning & Roadmap

## Evolusi Arsitektur: Modern Loader Integration
BloxHub akan diadaptasi ke **Modern Loader** yang memisahkan frontend dan backend, serta mengelola payload secara dinamis untuk evasi Hyperion yang lebih baik.

---

## Versi 1.0 - Dasar (Belum Selesai, Perlu Improvement & Testing)
- [x] Setup project CMake
- [x] Implementasi BloxHubLoader (Import Hijacking) - **DICABAIKAN! Import Hijack TERDETEKSI Hyperion!**
- [x] Implementasi BloxHubInjector (Manual Map) - **SUDAH TESTED BERHASIL DI NOTEPAD!**
- [x] Implementasi BloxHubInternal.dll (logging dasar) - **SUDAH TESTED BERHASIL DI NOTEPAD!**
- [x] Implementasi BloxHub.exe (Modern Loader Tahap 1: Dasar) - **SUDAH TESTED! Backup/Copy/Restore bekerja, tapi Import Hijack gagal di Roblox!**

---

## Versi 2.0 - Modern Loader (Switch ke Manual Map Injection)
- [x] Gabungkan BloxHubLoader dan BloxHubClient menjadi **BloxHub.exe** (Modern Loader Dasar - Backup/Copy DLL/Restore sudah bekerja!)
- [ ] TINGGAL Import Hijacking dan Ganti ke **Manual Map Injection dengan CFG Bypass! (lihat EXAMPLE PROJECT punya CFG Bypass!)
- [ ] Implementasi **Manual Map Injector dengan CFG Bypass di `src/injector/manual_map.cpp`
- [ ] Implementasi **Dynamic Payload Fetching (opsional, nanti)
- [ ] Fix bug koneksi TCP listener (DLL side)
- [ ] Implementasi **Dynamic Port Handshake**: Port TCP Silent Bridge dihitung secara dinamis per sesi

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
