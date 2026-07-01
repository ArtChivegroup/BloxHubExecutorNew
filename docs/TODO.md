# TODO BloxHub — Satu Step Per Sesi

**Terakhir diperbarui:** Juli 2026  
**Konteks:** [`STATUS.md`](STATUS.md) · [`PLANNING.md`](PLANNING.md)

Dokumen ini adalah **checklist kerja berurutan**. Setiap step punya satu kondisi sukses. Kalau tidak terpenuhi, step itu belum selesai — **jangan lanjut**.

Kerja dilakukan **satu step per sesi**, menunggu arahan eksplisit sebelum lanjut ke step berikutnya.

---

## Alur Besar

```
MASUK ROBLOX ──→ KOMUNIKASI ──→ EXECUTE
```

| Fase | Step | Ringkasan |
|------|------|-----------|
| MASUK ROBLOX | 1–3b | Buktikan payload hidup di proses Roblox |
| KOMUNIKASI | 4–6 | Buktikan loader ↔ payload bisa saling baca sinyal |
| EXECUTE | 7–8 | Baca/tulis memory game di dalam Roblox |

**Progress saat ini:** Inject manual stabil (2× tidak crash, `DllMain returned`) — Step 1 menunggu konfirmasi console di layar.

---

## Fase: MASUK ROBLOX

### Step 1 — Console di DllMain (injector stomp)

**Apa:** Injector stomp (`stomp_inject.cpp`) memanggil **DllMain**. `DebugConsoleLog` di `DLL_PROCESS_ATTACH` print `"[BloxHub] DllMain PROCESS_ATTACH"`. Rebuild full Release, inject.

**Injector:** module stomp `d3d10warp.dll` + manual map (reloc, import) — **TLS/SEH sengaja di-skip** (crash). DllMain via **`CreateRemoteThread` sync** + wait (`TpExecuteShellcodeSync`). **Tanpa** CFG bypass.

**Cara uji (disarankan):** Roblox sudah in-game → `BloxHubInjector.exe` as Admin (bukan `BloxHub.exe --inject` dulu — timing lebih aman).

**Sukses:** Console hitam di Roblox dengan teks `DllMain PROCESS_ATTACH` **atau** minimal Roblox tidak crash + log injector `DllMain returned`.

**Gagal:** Roblox crash setelah inject → cek build terbaru (TLS/SEH harus skip). Tidak ada console tapi tidak crash → lanjut Step 4 (file absolut) sebagai bukti alternatif.

**File:** `src/internal/dllmain.cpp`, `src/injector/stomp_inject.cpp`, `src/injector/tp_execute.cpp`

**Status:** 🔄 inject stabil (2× tidak crash, DllMain returned) — konfirmasi console di layar?

---

### Step 2 — (dibatalkan)

Digabung ke Step 1. Injector stomp memanggil DllMain; tidak ada lagi export `BloxHubInit`.

**Status:** ✅ N/A — merged Step 1

---

### Step 3 / 3b — (dibatalkan)

CFG bypass dihapus. Injector diganti module stomp (Riviera-style).

**Status:** ✅ cancelled — replaced by stomp injector

---

## Fase: KOMUNIKASI

### Step 4 — Tulis file ke path absolut

**Apa:** Hapus `AllocConsole` debug. Di `DllMain`, tulis `"hello"` ke `C:\BloxHub\test.txt`. Rebuild `BloxHubInternal.dll`, inject.

**Sukses:** `C:\BloxHub\test.txt` ada dan isinya `"hello"`.

**Gagal:** File tidak ada → kemungkinan `CreateDirectoryA` / `fopen` gagal di dalam proses Roblox. Coba ganti ke `WriteFile` (WinAPI langsung, tanpa CRT).

**Status:** ⬜ belum dikerjakan

---

### Step 5 — Log multi-baris

**Apa:** Tambah 3 log: `"init start"`, `"init mid"`, `"init end"` di titik berbeda `DllMain`. Inject.

**Sukses:** Ketiga baris ada di file.

**Gagal:** Hanya `"init start"` → code crash di tengah. Lihat baris mana yang hilang = lokasi crash.

**Status:** ⬜ belum dikerjakan

---

### Step 6 — Verify hijau

**Apa:** Ubah verify di `BloxHub.cpp` cek `C:\BloxHub\test.txt` bukan `%TEMP%`. Inject.

**Sukses:** Terminal print `[VERIFY] Payload loaded successfully!`.

**Gagal:** Timeout → cek timing (apakah verify mulai sebelum file ditulis), tambah delay atau polling.

**Status:** ⬜ belum dikerjakan

---

## Fase: EXECUTE

### Step 7 — Baca nilai dari Roblox memory

**Apa:** Di `DllMain`, baca satu offset game (misal `FakeDataModel::Pointer` dari `offsets.hpp`), tulis hasilnya ke log file. Inject.

**Sukses:** File berisi alamat memory yang valid (bukan 0, bukan random).

**Gagal:** Nilai 0 → offset salah atau base address salah. Cek `GetModuleHandleA("RobloxPlayerBeta.dll")` return value.

**Status:** ⬜ belum dikerjakan

---

### Step 8 — Tulis nilai ke Roblox memory

**Apa:** Tulis satu byte ke alamat yang dibaca Step 7 (atau ke allocated buffer), baca kembali, bandingkan. Log kedua nilai.

**Sukses:** Nilai tulis = nilai baca.

**Gagal:** Berbeda → write diblokir Hyperion. Ini masalah baru yang perlu strategi terpisah.

**Status:** ⬜ belum dikerjakan

---

## Catatan Debug Umum

| Gejala | Kemungkinan |
|--------|-------------|
| Inject OK, Roblox crash | TLS/SEH aktif — harus skip; atau payload terlalu berat di DllMain |
| Inject OK, tidak crash, tidak ada console | `AllocConsole` gagal/tersembunyi di belakang Roblox — coba Step 4 (`C:\BloxHub\test.txt`) |
| File di `%TEMP%` tidak ketemu dari luar | `GetTempPath()` di dalam Roblox ≠ `%TEMP%` user — expected sampai Step 6 |
| Log `CreateRemoteThread OK` | Normal — DllMain memang pakai sync thread, bukan IoCompletion |
| Inject ke-2 di PID sama | Bisa jalan (base stomp beda); idealnya sekali per sesi Roblox |

**Build cepat (hanya payload):**
```cmd
cmake --build build --config Release --target BloxHubInternal
```

**Build injector:**
```cmd
cmake --build build --config Release --target BloxHub
```

```cmd
cd build\bin\Release
BloxHubInjector.exe
```

(Buka Roblox, masuk game, baru jalankan injector as Admin.)

---

## Cara Pakai Dokumen Ini

1. Baca `STATUS.md` untuk konteks kondisi proyek.
2. Kerjakan **hanya satu step** per sesi.
3. Update kolom **Status** di dokumen ini setelah step selesai (✅ / ❌ + catatan singkat).
4. Jangan loncat step — kondisi sukses step N adalah prasyarat logis step N+1.
