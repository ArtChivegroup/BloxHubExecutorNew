
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

### Sekarang: Modern Loader (Beralih ke DLL Proxying)
**Import Hijacking TERDETEKSI Hyperion!** Jadi kita akan **beralih ke DLL Proxying**! (lihat [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md) dan [Daftar Bug](../docs/BUGS.md)!

---

## Komponen Utama

### 1. BloxHub.exe (Modern Loader)
**Fungsi**: Gabungan Injector, UI dan TCP Client  
**Tanggung Jawab**:
- **DLL Proxying Setup** (drop `version.dll` ke folder Roblox, rename asli jadi `version_orig.dll`)
- **UI untuk memasukkan script Lua**
- **TCP Client untuk berkomunikasi dengan DLL internal** (Silent Bridge)

### 2. BloxHubInternal.dll (Payload DLL)
**Fungsi**: DLL internal yang diinjek ke Roblox  
**Tanggung Jawab**:
- **PE Header Wiping**: Hapus PE header sendiri dari memory saat attach
- **TCP Listener (Dynamic Port)**: Listen koneksi dari BloxHub.exe di port dinamis per sesi
- **Execution Queue**: Thread-safe queue untuk menyimpan payload Lua
- **Scheduler Hook**: Hook ke Task Scheduler Roblox untuk mengeksekusi payload di main thread (whitelisted anti-tamper)
- **Integrasi LuaVM**: Eksekusi script Lua

---

## Arsitektur Modern Loader + Silent Bridge
```
[BloxHub.exe (Modern Loader)]
    |
    |-- (DLL Proxying: version.dll)
    ↓
[BloxHubInternal.dll di Roblox]
    |
    |-- (TCP Listener: Dynamic Port)
    ↓
[Silent Bridge: Localhost TCP]
    |
    |-- (Script Execution via Scheduler Hook)
    ↓
[Lua Script dijalankan di Main Thread Roblox]
```

---

## Teknik Evasi Hyperion
1. **DLL Proxying**: Tidak memodifikasi RobloxPlayerBeta.exe sama sekali!
2. **CFG Bypass**: Lihat `../EXAMPLE PROJECT/RBX-cfg-bypass-main/!
3. **PE Header Wiping**: DLL menghapus PE header sendiri dari memory saat attach
4. **Dynamic Port Handshake**: Port TCP Silent Bridge dinamis per sesi
5. **Scheduler Hooking**: Eksekusi Lua di main thread Roblox (ter-whitelist anti-tamper)

---

## Tech Stack
- **Bahasa**: C++20
- **Build System**: CMake
- **Library**: pe_bliss (optional, untuk PE parsing), Winsock (TCP)
- **Target**: Windows 10+ x64

---

## Referensi Lainnya
- [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md) - Progres terakhir
- [Daftar Bug](../docs/BUGS.md) - Bug yang sedang terjadi
- [Planning & Roadmap](../docs/PLANNING.md) - Rencana pengembangan selanjutnya
