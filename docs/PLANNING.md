# Planning — Saat Kamu Kembali

**Kondisi saat istirahat:** Juli 2026 — inject stomp stabil (tidak crash)  
**Baca status lengkap:** [`STATUS.md`](STATUS.md)

---

## Sudah Selesai

- State machine loader (preflight / install / launch / verify / restore)  
- Mode `--inject` di `BloxHub.exe`  
- **Injector module stomp** (Riviera-style, tanpa driver)  
- **`BloxHubInjector.exe`** — manual inject, tunggu game loaded  
- Fix crash: skip TLS/SEH, DllMain sync `CreateRemoteThread`  
- Uji manual: 2× inject OK, `DllMain returned`  
- Offset game → `version-5cf2272675e145f5`  
- Dokumentasi + [`TODO.md`](TODO.md)  

---

## P0 — Tutup Fase 1 (Step 1)

Checklist: [`TODO.md`](TODO.md) Step 1.

**Sudah:** tidak crash, `DllMain returned`.  
**Sisa:** konfirmasi console **atau** terima Step 4 sebagai bukti alternatif.

**Definisi selesai Step 1:** Console muncul **atau** file bukti Step 4 + Roblox tidak crash.

---

## P1 — Komunikasi loader ↔ payload

TODO Step 4–6: path absolut `C:\BloxHub\`, verify hijau di `BloxHub.cpp`.

---

## P2 — Execute (offset dari raw)

TODO Step 7–8: baca/tulis memory pakai `offsets.hpp` dari `offsets/raw/`.

---

## P3 — Nasib sideload

Rekomendasi: deprecate atau freeze — Hyperion memblokir.

---

## P4 — Perbaiki `BloxHub.exe --inject` timing

Setelah manual path stabil, samakan wait/delay dengan `BloxHubInjector`.

---

## Goal

> **BloxHub inject `BloxHubInternal.dll` ke Roblox dengan bukti empiris (console / file) dari luar proses.**
