# Checkpoint — Kondisi Saat Istirahat

**Tanggal:** Juli 2026  
**Versi Roblox:** `version-5cf2272675e145f5`  
**Offset sumber:** `newRawOffset]/offsets.h` → `include/offsets.hpp`

Dokumen ini menggantikan checkpoint lama sebagai **ringkasan sesi terakhir**. Checkpoint lain di folder ini = historis.

---

## Apa yang Dikerjakan di Sesi Ini

### Diagnosa sideload timeout
- Verify gagal karena `DllMain` proxy tidak pernah jalan  
- Hyperion menolak `dxgi.dll` yang dimodifikasi  
- Bukan masalah urutan `DllMain` saja — proxy tidak masuk proses  

### Perbaikan kode (sudah di-build)
- `proxy_payload.cpp` — marker lebih dulu, forward load di thread  
- `BloxHub.cpp` — mode `--inject`, wait PID game, verify ganda  
- `dllmain.cpp` — marker di `DllMain` (untuk inject belum terpanggil)  
- `offsets.hpp` — update ke versi Roblox baru  
- `manual_map.cpp` — log error per-step, coba `CfgBypass::BitmapPtr`  

### Hasil uji terakhir user

**Sideload:** gagal (expected) — tidak ada log  

**Inject `--inject`:**
```
[+] Roblox game process ready (PID: 8872)
[*] Allocated remote image at: 0x1C0000
[+] BloxHubInit executed successfully!
[INJECT] OK
[VERIFY] timeout — tidak ada bloxhub_test.txt
```

**BloxHubInjector.exe:** sama — inject "sukses", tidak ada log file  

---

## Kesimpulan Sesi

| Area | Status |
|------|--------|
| Build | ✅ |
| Sideload | ❌ mati (Hyperion) |
| Inject infrastruktur | ✅ allocate/write/thread OK |
| Payload hidup di Roblox | ❓ tidak terbukti |
| Verify | ❌ desain/path salah |
| CFG bypass | ⚠️ auto-scan tidak stabil |
| Dokumentasi | ✅ dirapikan |

---

## File Kunci yang Berubah

- `src/BloxHub.cpp`
- `src/internal/proxy_payload.cpp`
- `src/internal/dllmain.cpp`
- `src/injector/manual_map.cpp`
- `include/offsets.hpp`
- `CMakeLists.txt` (BloxHub link injector)
- `docs/*` (rewrite)
- `README.md`

---

## Langkah Pertama Saat Lanjut

1. Baca `docs/STATUS.md`  
2. Jangan fix random — fokus P0 di `docs/PLANNING.md`  
3. Procmon atau cari log di `AppData` sandbox  

---

## Perintah Cepat

```cmd
cd build\bin\Release
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-5cf2272675e145f5" --inject
```

Wajib **Administrator**.

---

## Dokumentasi Baru

| File | Peran |
|------|-------|
| `docs/STATUS.md` | **Mulai di sini** |
| `README.md` | Index singkat |
| `docs/USAGE.md` | Cara pakai |
| `docs/BUGS.md` | Bug terbuka |
| `docs/PLANNING.md` | Prioritas lanjut |

Checkpoint lama dihapus saat repo cleanup (Juli 2026). Lihat `docs/STATUS.md`.
