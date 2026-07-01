
# Planning & Roadmap

---
## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For architecture details**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **For known bugs**: [BUGS.md](BUGS.md)
- **For latest checkpoint**: [../checkpoints/CHECKPOINT_20260701_MANUALMAP.md](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)

---
## Evolusi Arsitektur

### Fase 1: DLL Proxying (DIBATALKAN)
- [x] Static .def proxy — gagal (KnownDLLs + signature check)
- [x] Dynamic PE Patching (3LayersPersistence) — gagal (dxgi.dll signature diverifikasi Hyperion)
- [x] Test berbagai target DLL — version.dll, dbghelp.dll, dinput8.dll tidak dicari Roblox

**Kesimpulan**: DLL Proxying mentok. Hyperion verifikasi signature DLL grafis, DLL lain tidak dicari.

### Fase 2: Manual Map + CFG Bypass (AKTIF)
- [x] Port RBX-cfg-bypass → cfg_bypass.h/.cpp
- [x] Update manual_map.cpp — integrasi CFG bypass
- [x] Auto-scan bitmap di .rdata/.data (berhasil temukan kandidat)
- [x] CFG bitmap patching via WriteProcessMemory
- [x] Whitelist shellcode injection (framework siap, butuh offsets)
- [ ] **Rewrite dllmain.cpp pure Win32 API (tanpa CRT)** ⭐ NEXT
- [ ] Test ulang dengan DLL baru
- [ ] Thread Hijacking (ganti CreateRemoteThread)

---
## Prioritas TERTINGGI

### 1. Rewrite dllmain.cpp — Pure Win32 API
**Alasan**: CRT dependency crash (VCRUNTIME140.dll tidak ada di Roblox).
**Target**: DLL hanya import kernel32.dll + user32.dll.
**Detail**:
- Hapus `fopen_s`, `fprintf`, `sprintf_s`, `strcpy_s`, `strcat_s`
- Ganti dengan `CreateFileA`, `WriteFile`, `CloseHandle`, loop manual
- Build: `/GS- /NODEFAULTLIB`
- Entry point: raw DllMain tanpa CRT wrapper

### 2. Verifikasi CFG Bitmap
Setelah CRT fix → test ulang → pastikan bitmap ter-patch benar.

### 3. Thread Hijacking
Ganti `CreateRemoteThread` yang dimonitor Hyperion.
- Suspend main thread Roblox
- Backup context (registers)
- Redirect RIP ke BloxHubInit
- Resume thread

---
## Prioritas SEDANG

### 4. CFG Whitelist Insertion (Layer 2)
Setelah bitmap terverifikasi, tambahkan whitelist entry via shellcode insert_set.
Butuh offsets: `Whitelist` dan `InsertSet` di RobloxPlayerBeta.dll.

### 5. Direct Syscalls
Jika Hyperion hook `WriteProcessMemory` / `VirtualAllocEx`:
- Ganti dengan syscall langsung (NtWriteVirtualMemory)
- Teknik: Indirect Syscalls (Hell's Gate / Halo's Gate)

### 6. Scheduler Hook
Setelah DLL berhasil load, hook TaskScheduler untuk eksekusi Lua per-frame.

---
## Prioritas RENDAH

### 7. Lua Execution
- Integrasi Luau VM
- Set identity level (6/8)
- Script hub

### 8. GUI Client
- UI untuk edit/execute script
- Script browser

---
## Versi & Milestone

| Versi | Deskripsi | Status |
|-------|-----------|--------|
| v1.0 | Import Hijacking (BloxHubLoader) | ❌ Dibatalkan |
| v1.1 | DLL Proxying (Static .def) | ❌ Dibatalkan |
| v1.2 | DLL Proxying (Dynamic PE Patch) | ❌ Dibatalkan |
| **v2.0** | **Manual Map + CFG Bypass** | 🔄 In Progress |
| v2.1 | Thread Hijacking | ⏸️ |
| v3.0 | Lua Execution | ⏸️ |

---
## Referensi Lainnya
- [Checkpoint Terbaru](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)
- [Daftar Bug](BUGS.md)
- [Arsitektur Sistem](ARCHITECTURE.md)
