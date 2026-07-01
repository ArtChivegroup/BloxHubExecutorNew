# Bug & Risiko Terbuka

**Terakhir diperbarui:** Juli 2026  
**Konteks:** [`STATUS.md`](STATUS.md)

Daftar ini hanya masalah yang **masih relevan**. Bug yang sudah jelas "by design" (sideload + Hyperion) dicatat sebagai batasan, bukan tugas fix acak.

---

## Kritis — Hambat Inject

### B1. Verify gagal meski injector lapor sukses

**Status:** `active` — blocker utama  

**Gejala:**
- `[INJECT] OK`, `BloxHubInit executed successfully!`
- `%TEMP%\bloxhub_test.txt` tidak ada
- Verify timeout 30 detik

**Penyebab yang paling mungkin:**
1. `WriteLog` di dalam proses Roblox pakai `GetTempPath()` sandbox ≠ `%TEMP%` user  
2. Manual map tidak memanggil `DllMain` — marker hanya di `DllMain`, bukan `BloxHubInit`  
3. Kode `BloxHubInit` crash di Roblox sebelum tulis file  
4. Hyperion memblokir eksekusi injected code  

**Yang perlu dilakukan nanti:** verifikasi dengan Procmon; tulis marker ke path absolut; panggil marker dari `BloxHubInit`.

---

### B2. CFG auto-scan tidak stabil

**Status:** `active`  

**Gejala:** Dua run pada PID sama menemukan RVA bitmap berbeda (`0x14F2738` vs `0x12E6128`).

**Dampak:** Patch CFG mungkin salah target → crash atau bypass tidak efektif.

**Yang perlu dilakukan nanti:** Kunci `offsets::CfgBypass::BitmapPtr` manual dari satu scan yang divalidasi; matikan auto-scan atau jadikan fallback terakhir.

---

## Batasan Desain — Bukan Bug Kecil

### B3. Sideload `dxgi.dll` diblokir Hyperion

**Status:** `wontfix` (untuk Roblox saat ini)  

**Gejala:** Install OK, launch OK, tidak ada log, verify timeout, Roblox: "failed to load dxgi.dll".

**Kesimpulan:** Jalur sideload bukan prioritas sampai ada bypass integrity file-level.

---

### B4. Manual map tidak memanggil `DllMain`

**Status:** `active` — desain saat ini  

Injector hanya memanggil export `BloxHubInit`. TLS, CRT init, dan `DllMain` di-skip.

**Dampak:** Perilaku payload berbeda dari load DLL normal; verify yang mengandalkan `DllMain` tidak akurat.

---

## Sedang — UX & Operasional

### B5. Restore sideload belum crash-safe

**Status:** `active`  

Cleanup sideload butuh user tekan Enter. Sesi putus → `dxgi.dll` bisa tertinggal.

Session file `bloxhub_session.dat` ada tapi auto-restore terbatas.

---

### B6. Dokumentasi checkpoint lama membingungkan

**Status:** `mitigated` — lihat `CHECKPOINT_CURRENT.md` dan `STATUS.md`  

Checkpoint Juli 01 masih bilang "loader-first sideload". Kondisi Juli 2026: inject-first.

---

### B7. Offset CFG terpisah dari dump game

**Status:** `active`  

`offsets/raw/offsets.h` dari roblox-dumper = offset game saja.  
`CfgBypass::BitmapPtr` harus diisi manual dari log scan inject.

---

## Rendah / Legacy

### B8. Target DLL sideload belum final

Shortlist historis: `dxgi.dll`, `dsound.dll`. Semua sideload sama-sama kena Hyperion untuk file di folder game.

### B9. Naming historis di source

Payload file = `version.dll`, target sideload = `dxgi.dll`. Membingungkan tapi sudah didokumentasikan di `STATUS.md`.

---

## Matriks: Gejala → Arti

| Gejala | Interpretasi |
|--------|--------------|
| Tidak ada `bloxhub_test.txt` sama sekali | Payload tidak jalan ATAU log di path lain |
| Sideload install OK, verify timeout | Hyperion blokir proxy (normal) |
| `OpenProcess` error 87 | PID stub mati (sudah ada fix wait PID) |
| Inject OK, verify timeout | False positive inject — lihat B1 |
| CFG RVA beda tiap run | False positive scan — lihat B2 |
| Versi path ≠ `offsets.hpp` | Offset game salah — update dump |

---

## Yang Sudah Diperbaiki (Jangan Debug Ulang)

| Masalah | Fix |
|---------|-----|
| Inject ke PID stub yang sudah mati | `WaitForRobloxGameProcess()` |
| Marker ditulis setelah forward load gagal | Marker lebih dulu di `proxy_payload` (sideload) |
| Error message inject tidak jelas | Log per-step di `manual_map.cpp` |
| Offset versi lama | Updated ke `version-5cf2272675e145f5` |
