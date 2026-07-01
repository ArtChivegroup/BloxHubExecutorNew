# Checkpoint 2026-07-01 #2 — Manual Map + CFG Bypass (Post-DLL Proxy)

---

## Ringkasan Hari Ini
Setelah DLL Proxying mentok (dxgi.dll ditolak signature Hyperion, DLL lain tidak dicari Roblox di app folder),
kita pivot ke **Manual Map + CFG Bypass** dari RBX-cfg-bypass.

---

## Arsitektur Baru

```
BloxHubInjector.exe
  │
  ├── 1. Wait RobloxPlayerBeta.exe
  ├── 2. Baca BloxHubInternal.dll dari disk
  ├── 3. VirtualAllocEx → alokasi di Roblox (PAGE_EXECUTE_READWRITE)
  ├── 4. Manual Map: salin sections, relocate, resolve imports
  ├── 5. WriteProcessMemory → tulis DLL ke Roblox
  ├── 6. CFG Bypass:
  │     ├── Auto-scan .rdata/.data untuk bitmap pointer
  │     ├── Filter: page-aligned, AllocationBase, min 64KB, min 1MB addr
  │     ├── Patch CfgBitmap → flip bit untuk region DLL kita
  │     └── (Whitelist insert_set via shellcode — belum aktif, butuh offsets)
  ├── 7. CreateRemoteThread → panggil BloxHubInit export
  └── 8. (Harusnya) log ke %TEMP%\bloxhub_test.txt
```

---

## Progress Hari Ini

| Fase | Deskripsi | Status |
|------|-----------|--------|
| Fase 1 | cfg_bypass.h/.cpp — CFG bitmap scanner + patcher | ✅ Selesai |
| Fase 2 | manual_map.cpp — integrasi CFG bypass | ✅ Selesai |
| Fase 3 | BloxHubInjector.cpp — CLI injector | ✅ Selesai |
| Fase 4 | CMakeLists.txt — build system update | ✅ Selesai |
| Fase 5 | Build & verifikasi | ✅ Build OK |
| Fase 6 | Test di Roblox | ⚠️ Injection OK, tapi crash |

---

## Hasil Test

```
[*] Allocated remote image at: 0x00000000577C0000
[*] DLL written to remote process
[*] Scanned .rdata (0x7FFAA3490000 - 0x7FFAA3694784)
[*] Scanned .data (0x7FFAA48B0000 - 0x7FFAA48F7002)
[*] Found CFG bitmap candidate at RVA 0x1432808 -> 0x920000 (size: 0x3B000)
[+] CFG: Bitmap patched for region 0x577C0000 - 0x577C8000
[+] BloxHubInit executed successfully!
```

✅ **Manual Map berhasil** — DLL ditulis, CFG bitmap ter-patch, CreateRemoteThread berhasil  
❌ **Tidak ada log** — DllMain crash karena dependency chain

---

## Bug Ditemukan

### 1. CRT Dependency Crash (CRITICAL)
**Deskripsi**: Manual Map tidak me-resolve dependensi CRT (VCRUNTIME140.dll, ucrtbase.dll, api-ms-win-crt-*.dll). Alamat function di-resolve di injector process → ditulis ke IAT di Roblox → alamat invalid → crash.

**Bukti dari dumpbin /imports**:
```
BloxHubInternal.dll imports:
  VCRUNTIME140.dll       (__C_specific_handler, memcpy, memset, strstr)
  api-ms-win-crt-string  (strcat_s)
  api-ms-win-crt-stdio   (fopen_s, fclose, fprintf, sprintf_s)
  api-ms-win-crt-runtime (_initterm, _initterm_e, etc.)
```

RobloxPlayerBeta.dll hanya import ntdll, kernel32, user32 → CRT DLL tidak ada di Roblox process.

**Solusi**: Rewrite `dllmain.cpp` pure Win32 API — tanpa CRT (`/GS- /NODEFAULTLIB`)

### 2. CFG Bitmap Auto-Scan False Positive (FIXED)
**Deskripsi**: Scanner awal mendeteksi `0x200000002` sebagai bitmap pointer (false positive).  
**Fix**: Tambah filter page-aligned (`(ptr & 0xFFF) == 0`), AllocationBase check, min 1MB address.

### 3. CFG Bitmap Candidate Found
**RVA**: `0x1432808` di `.data` section RobloxPlayerBeta.dll  
**Pointer**: `0x920000` (236KB region, page-aligned, AllocationBase, RW)  
**Status**: Kemungkinan besar INI ADALAH bitmap yang benar. Tapi perlu verifikasi setelah CRT fix.

---

## File Baru/Diubah

| File | Aksi |
|------|------|
| `src/injector/cfg_bypass.h` | NEW — CFG bypass header |
| `src/injector/cfg_bypass.cpp` | NEW — Bitmap patcher + whitelist shellcode |
| `src/injector/manual_map.cpp` | MODIFIED — Integrasi CFG, bitmap scanner |
| `src/BloxHubInjector.cpp` | MODIFIED — CFG bypass flag, Psapi include |
| `include/injector.hpp` | MODIFIED — Tambah enableCfgBypass param |
| `include/offsets.hpp` | MODIFIED — Tambah CfgBypass namespace |
| `CMakeLists.txt` | MODIFIED — Tambah cfg_bypass.cpp, psapi |

---

## Rencana Selanjutnya

1. **Rewrite `dllmain.cpp` — Pure Win32 API, tanpa CRT** ⭐ **PRIORITAS TERTINGGI**
   - Hapus semua dependensi CRT: `fopen_s`, `fprintf`, `strcpy_s`, `strcat_s`, `memcpy`
   - Ganti dengan: `CreateFileA`, `WriteFile`, `CloseHandle`, `GetTempPathA`, loop manual
   - Build flag: `/GS- /NODEFAULTLIB` (no CRT init, no security cookie)
   - Entry point: raw `DllMain` tanpa CRT wrapper

2. **Test ulang dengan DLL baru**
   - Harusnya: Manual Map → CFG bypass → DllMain jalan → log muncul

3. **Thread Hijacking** (ganti CreateRemoteThread)
   - Suspend main thread Roblox → set RIP ke DllMain → Resume

4. **Direct Syscalls** (jika Hyperion hook API)
   - Ganti `WriteProcessMemory`/`VirtualAllocEx` dengan syscall langsung

---

## Referensi
- [Checkpoint Sebelumnya](CHECKPOINT_20260701_FINAL.md) — DLL Proxying
- [RBX-cfg-bypass](../EXAMPLE%20PROJECT/RBX-cfg-bypass-main/) — Teknik CFG bypass
