
# Daftar Bug & Masalah

---
## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For architecture details**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **For roadmap & planning**: [PLANNING.md](PLANNING.md)
- **For latest checkpoint**: [../checkpoints/CHECKPOINT_20260701_MANUALMAP.md](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)

---
## Bug Aktif

### 1. CRT Dependency Crash di Manual Map (CRITICAL - NEXT FIX)
**Status**: 🔴 Active
**Deskripsi**: Manual mapper me-resolve import di INJECTOR process, lalu tulis alamat ke Roblox. 
`BloxHubInternal.dll` import VCRUNTIME140.dll + api-ms-win-crt-*.dll yang TIDAK ADA di Roblox.
Akibatnya: IAT berisi alamat sampah → DllMain crash → tidak ada log.
**Solusi**: Rewrite `dllmain.cpp` pure Win32 API (`CreateFileA`, `WriteFile`), flag `/GS- /NODEFAULTLIB`.

### 2. CFG Bitmap Candidate Belum Terverifikasi
**Status**: 🟡 Pending (tergantung fix CRT)
**Deskripsi**: Scanner menemukan kandidat bitmap di RVA `0x1432808` → `0x920000`.
Belum bisa diverifikasi karena DLL crash duluan (bug #1).
**Solusi**: Setelah CRT fix, test ulang untuk verifikasi.

### 3. Hyperion Monitoring CreateRemoteThread
**Status**: ⏸️ Pending (belum terdeteksi, antisipasi)
**Deskripsi**: Hyperion kemungkinan hook `CreateRemoteThread`.
**Solusi**: Ganti dengan Thread Hijacking (suspend main thread → redirect RIP → resume).

---
## Bug Yang Sudah Diperbaiki

### Bug: DLL Proxy — dxgi.dll Signature Ditolak Hyperion
**Status**: ✅ Dibatalkan (bukan bug, by design)
**Alasan**: dxgi.dll signed Microsoft → Hyperion verifikasi signature → proxy unsigned ditolak.

### Bug: DLL Proxy — version.dll Tidak Dicari di App Folder
**Status**: ✅ Dibatalkan (bukan bug)
**Alasan**: Roblox load version.dll dari System32 via absolute path/manifest, abaikan app folder.

### Bug: CFG Bitmap Auto-Scan False Positive
**Status**: ✅ Fixed
**Deskripsi**: Scanner mendeteksi `0x200000002` sebagai bitmap (bukan pointer valid).
**Fix**: Filter page-aligned, AllocationBase, min 1MB address, min 64KB region.

### Bug: LNK Error unresolved external `ReadFileFromDiskW`
**Status**: ✅ Fixed (legacy, dari era DLL Proxying)

### Bug: Restore Gagal — File Locked
**Status**: ✅ Fixed (legacy)
**Fix**: Tambah error checking di BloxHub.cpp.

---
## Penemuan Penting

### 1. Struktur Robust DLL Manual Map
DLL yang di-manual-map HARUS:
- Pure Win32 API saja (kernel32.dll, user32.dll, ntdll.dll)
- TIDAK boleh import CRT (VCRUNTIME, ucrtbase, api-ms-win-crt-*)
- TIDAK boleh import DLL lain yang tidak dijamin ada di target
- Build flag: `/GS- /NODEFAULTLIB`
- Karena kernel32.dll punya alamat sama di injector & target (thanks ASLR per boot session)

### 2. Daftar KnownDLLs Windows
JANGAN target DLL KnownDLLs untuk proxy: winmm.dll, kernel32.dll, user32.dll, gdi32.dll, dll.

### 3. DLL Load Order Roblox
Hanya dxgi.dll yang dicari di app folder. DLL lain (version, dbghelp, dinput8) dari System32.

---
## Referensi
- [Checkpoint Terbaru](../checkpoints/CHECKPOINT_20260701_MANUALMAP.md)
- [Planning & Roadmap](PLANNING.md)
