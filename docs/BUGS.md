# Bug & Risiko Terbuka

**Terakhir diperbarui:** Juli 2026  
**Konteks:** [`STATUS.md`](STATUS.md)

---

## Kritis — Hambat Fase 1

### B1. Console belum dikonfirmasi (Step 1 partial)

**Status:** `partial` — inject stabil, bukti visual pending  

**Yang sudah OK (Juli 2026):**
- `BloxHubInjector.exe` × 2 — tidak crash  
- Log: `DllMain returned`, `Injection OK — Roblox masih hidup`  

**Yang belum:**
- User belum konfirmasi console `[BloxHub] DllMain PROCESS_ATTACH` terlihat di layar  

**Langkah:** Konfirmasi console **atau** lanjut Step 4 (`C:\BloxHub\test.txt`) sebagai bukti file.

**Verify `%TEMP%`:** Masih broken by design sampai Step 4–6 (path absolut).

---

## Batasan Desain

### B2. Sideload `dxgi.dll` diblokir Hyperion

**Status:** `wontfix`  

---

### B3. Verify path sandbox

`WriteLog` di Roblox (jika diaktifkan lagi) pakai `GetTempPath()` ≠ `%TEMP%` user loader.

**Fix:** TODO Step 4–6 (`C:\BloxHub\test.txt`).

---

## Sedang

### B4. Restore sideload belum crash-safe

Cleanup sideload butuh Enter; session file terbatas.

---

### B5. `BloxHub.exe --inject` timing

Launch + inject bisa terlalu cepat vs manual inject. Pakai `BloxHubInjector.exe` sampai timing launcher diperbaiki.

---

## Mitigated (Juli 2026)

| Masalah | Fix |
|---------|-----|
| Roblox crash setelah stomp inject | Skip TLS + SEH di `stomp_inject.cpp` |
| DllMain async / crash tidak terdeteksi | `TpExecuteShellcodeSync` — wait thread |
| CFG auto-scan false positive | Injector diganti module stomp — CFG dihapus |
| Manual map skip DllMain | Stomp injector panggil DllMain |
| `BloxHubInit` only entry | Dihapus — DllMain saja |
| Inject ke PID stub | `WaitForRobloxGameProcess()` |

---

## Matriks: Gejala → Arti

| Gejala | Interpretasi |
|--------|--------------|
| `DllMain returned` + Roblox hidup | Payload hidup — Step 1 hampir selesai |
| Tidak ada console, tidak crash | `AllocConsole` tersembunyi/gagal — coba Step 4 |
| Roblox crash setelah inject | TLS/SEH mungkin aktif lagi — cek build |
| `[INJECT] OK`, verify timeout | Expected sampai Step 6 |
| `Stomp target too small` | Payload > d3d10warp — coba modul System32 lain |
| Inject ke-2 same PID | Bisa jalan; ideal sekali per sesi |
