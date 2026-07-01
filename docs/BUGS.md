# Bug & Risiko Terbuka

**Terakhir diperbarui:** Juli 2026  
**Konteks:** [`STATUS.md`](STATUS.md)

---

## Kritis — Hambat Inject

### B1. Payload belum terbukti hidup (Fase 1)

**Status:** `active` — blocker utama  

**Gejala:** Injector lama lapor OK tanpa console/log; verify timeout.

**Setelah migrasi stomp:** Uji Step 1 di [`TODO.md`](TODO.md) — console `DllMain PROCESS_ATTACH` di Roblox.

**Verify `%TEMP%`:** Masih broken by design sampai Step 4–6 (path absolut).

---

## Batasan Desain

### B2. Sideload `dxgi.dll` diblokir Hyperion

**Status:** `wontfix`  

---

### B3. Verify path sandbox

`WriteLog` di Roblox pakai `GetTempPath()` ≠ `%TEMP%` user loader.

**Fix:** TODO Step 4–6 (`C:\BloxHub\test.txt`).

---

## Sedang

### B4. Restore sideload belum crash-safe

Cleanup sideload butuh Enter; session file terbatas.

---

## Mitigated (Juli 2026)

| Masalah | Fix |
|---------|-----|
| CFG auto-scan false positive | Injector diganti module stomp — CFG dihapus |
| Manual map skip DllMain | Stomp injector panggil DllMain lengkap |
| `BloxHubInit` only entry | Dihapus — DllMain saja |
| Inject ke PID stub | `WaitForRobloxGameProcess()` |

---

## Matriks: Gejala → Arti

| Gejala | Interpretasi |
|--------|--------------|
| Tidak ada console di Roblox | DllMain tidak jalan / crash / IoCompletion gagal |
| `[INJECT] OK`, no console | Thread dispatch ≠ payload hidup — cek log stomp |
| Verify timeout | Expected sampai Step 6 |
| `Stomp target too small` | Payload > d3d10warp — coba mshtml.dll |
