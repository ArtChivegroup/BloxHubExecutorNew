# Planning — Saat Kamu Kembali

**Kondisi saat istirahat:** Juli 2026  
**Baca status lengkap:** [`STATUS.md`](STATUS.md)

Dokumen ini singkat — tidak ada roadmap 50 halaman. Hanya: apa sudah selesai, apa berikutnya, dalam urutan logis.

---

## Sudah Selesai ✅

- State machine loader (preflight / install / launch / verify / restore)  
- `pe_patcher` — proxy DLL dinamis  
- Mode `--inject` di `BloxHub.exe`  
- Wait PID game (bukan stub launcher)  
- Offset game → `version-5cf2272675e145f5`  
- Dokumentasi dirapikan (`STATUS.md` sebagai entry point)  

---

## Belum Selesai — Urutan Prioritas

### P0 — Buktikan inject benar-benar jalan

Tanpa ini, semua kerja lain meaningless.

1. Procmon: `RobloxPlayerBeta.exe` + `CreateFile` + `bloxhub`  
2. Cari `bloxhub_test.txt` di seluruh `AppData` (bukan hanya `%TEMP%`)  
3. Kalau path sandbox — tulis marker ke path absolut tetap (mis. folder `BloxHub.exe`)  
4. Panggil `WriteVerifyMarker` dari `BloxHubInit`, bukan hanya `DllMain`  

**Definisi selesai:** Verify hijau + file log terbaca dari luar proses Roblox.

---

### P1 — Stabilkan CFG bypass

1. Dari log inject, pilih **satu** RVA bitmap yang konsisten  
2. Isi `offsets::CfgBypass::BitmapPtr`  
3. Matikan atau batasi auto-scan false positive  
4. Uji: inject tidak crash Roblox  

---

### P2 — Rapikan verify & logging

- Satu path marker untuk sideload dan inject  
- Pesan error yang membedakan: inject crash vs path salah vs Hyperion  
- Optional: `OutputDebugString` untuk DebugView  

---

### P3 — Putuskan nasib sideload

Opsi:

- **A)** Deprecate — hapus dari default, sisakan flag `--sideload` untuk lab  
- **B)** Freeze — dokumentasi "tidak didukung" saja  
- **C)** Pivot target `dsound.dll` — eksperimen, ekspektasi rendah (Hyperion sama)  

**Rekomendasi:** Opsi A atau B. Jangan investasi besar di sideload.

---

### P4 — Executor fitur (jauh)

Baru setelah P0–P1 selesai:

- Hook Lua / scheduler  
- IPC loader ↔ payload  
- UI  

Ini di luar scope loader infrastruktur saat ini.

---

## Yang TIDAK Perlu Dikerjakan Dulu

- Named pipe IPC (belum ada kode, hanya rencana lama)  
- Perpanjang timeout verify tanpa fix path log  
- Debug sideload `dxgi.dll` di Roblox production  
- Baca ulang semua checkpoint Juli 01  
- Refactor besar naming `version` vs `dxgi`  

---

## Satu Kalimat Goal

> **BloxHub bisa inject `BloxHubInternal.dll` ke Roblox versi terbaru dengan bukti empiris yang bisa dibaca dari luar proses.**

Semua task di atas melayani kalimat itu.

---

## Referensi Saat Stuck

| Masalah | Baca |
|---------|------|
| Bingung kondisi proyek | `STATUS.md` |
| Cara test | `USAGE.md` |
| Update offset | `BUILD.md` |
| Daftar bug | `BUGS.md` |
| Sesi terakhir | `../checkpoints/CHECKPOINT_CURRENT.md` |
