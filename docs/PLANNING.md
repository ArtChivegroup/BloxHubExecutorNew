# Planning — Saat Kamu Kembali

**Kondisi saat istirahat:** Juli 2026 — migrasi injector stomp  
**Baca status lengkap:** [`STATUS.md`](STATUS.md)

---

## Sudah Selesai

- State machine loader (preflight / install / launch / verify / restore)  
- Mode `--inject` di `BloxHub.exe`  
- **Injector module stomp** (Riviera-style, tanpa driver)  
- Offset game → `version-5cf2272675e145f5`  
- Dokumentasi + [`TODO.md`](TODO.md)  

---

## P0 — Buktikan inject (Step 1 TODO)

Checklist: [`TODO.md`](TODO.md) Step 1 — console `DllMain` di Roblox.

**Definisi selesai:** Console muncul + Roblox tidak crash.

---

## P1 — Komunikasi loader ↔ payload

TODO Step 4–6: path absolut `C:\BloxHub\`, verify hijau.

---

## P2 — Execute (offset dari raw)

TODO Step 7–8: baca/tulis memory pakai `offsets.hpp` dari `offsets/raw/`.

---

## P3 — Nasib sideload

Rekomendasi: deprecate atau freeze — Hyperion memblokir.

---

## Goal

> **BloxHub inject `BloxHubInternal.dll` ke Roblox dengan bukti empiris (console / file) dari luar proses.**
