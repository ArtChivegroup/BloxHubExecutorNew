# Checkpoint 2026-07-02 - Planning L2: Session Model & Comprehensive Audit

## Tujuan Checkpoint

Checkpoint ini merangkum hasil mega planning dan analisa pada 2026-07-02, mencakup:

- Desain L2 Session Model (state machine loader),
- Audit lengkap 3 EXAMPLE PROJECT referensi,
- String scan import candidate pada RobloxPlayerBeta.exe lokal,
- Update ranking kandidat target DLL dengan data baru (`dsound.dll` ditemukan).

## Executive Summary

Dua temuan utama dari sesi planning ini:

1. **`dsound.dll` ditemukan sebagai kandidat baru** — muncul di string scan `RobloxPlayerBeta.exe` dengan sinyal setara `dxgi.dll`. Pola sideload `dsound.dll` sudah proven di referensi 3LayersPersistence (Spotify hijack). Export count lebih manageable daripada ~720 export `dxgi.dll`.

2. **`pe_patcher` BloxHub arsitekturnya hampir identik dengan `ConvertExecutableToDll` milik 3LayersPersistence** — keduanya pakai struct `EXPORT_ENTRY` yang sama, section `.edata`, dan export forwarding pattern. Ini validasi kuat bahwa arah arsitektur loader sudah benar.

## Milestone Status Update

| Milestone | Status Sebelum | Status Sekarang | Catatan |
|-----------|---------------|-----------------|---------|
| L0 — Documentation Reset | `done` | `done` | - |
| L1 — Loader Baseline Stabilization | `active` | **`done`** | Baseline sudah solid: flow jelas, gap terdokumentasi, checkpoint tersedia |
| L2 — Preflight & Session Model | `next` | **`in_progress`** | State machine sudah didesain detail; siap untuk implementasi |
| L3 — Verify & Restore Hardening | `planned` | `planned` | - |
| L4 — Strategy Selection | `planned` | `planned` | - |
| L5 — Implementation Track | `deferred` | `deferred` | - |

## Hasil Audit EXAMPLE PROJECT

### 3LayersPersistence (Maldev Academy)

**Relevansi: SANGAT TINGGI**

Arsitektur `ConvertExecutableToDll` ([ConvertExeToDll.c](../EXAMPLE%20PROJECT/3LayersPersistence-main/3LayersPersistence/ConvertExeToDll.c)) **mirip dengan `pe_patcher` BloxHub**:

- Keduanya menggunakan struct `EXPORT_ENTRY` yang identik
- Keduanya menggunakan section `.edata` untuk export directory
- Keduanya membangun forwarding export table dari DLL asli
- Keduanya menghitung checksum baru

Fitur 3LayersPersistence yang BELUM ada di `pe_patcher`:
- **Timestamp stomping**: PE timestamp di-set 30 hari lebih tua dari DLL asli
- **Debug directory patching**: timestamp debug entries ikut di-stomp
- **Entry point redirect**: `AddressOfEntryPoint` diarahkan ke `DllMain`
- **Subsystem override**: diubah ke `IMAGE_SUBSYSTEM_WINDOWS_GUI`
- **IMAGE_FILE_DLL flag**: di-set di `FileHeader.Characteristics`

Layer persistence yang didemonstrasikan:
1. WMI permanent event subscription (elevated)
2. COM hijacking via HKCU CLSID (unelevated)
3. **DLL sideloading `dsound.dll` di folder Spotify** (unelevated) — PALING RELEVAN

**Kesimpulan**: `pe_patcher` adalah fondasi yang solid. Fitur tambahan dari 3LayersPersistence bisa diadopsi di milestone L5.

### RBX-cfg-bypass

**Relevansi: RENDAH (hanya untuk manual map track)**

- CFG bypass pasca-Hyperion dengan whitelist insertion + bitmap patch
- Hardcoded offsets per versi Roblox — tidak sustainable
- Hanya berguna jika manual map track butuh CFG bypass yang lebih baru
- **Tidak relevan untuk loader track**

### WebView2Loader-Injection (dianalisa sebelumnya)

**Relevansi: SEDANG**

- Pola `export emulation + loader contract re-implementation`
- Hanya 4 export, tapi perlu re-implement full loader contract
- String scan: 0 match di RobloxPlayerBeta.exe — sinyal lemah
- File ada di folder version Roblox — kemungkinan dynamic load

## Hasil String Scan RobloxPlayerBeta.exe

Target: `c:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\RobloxPlayerBeta.exe`

| DLL Candidate | Exact String Match | Interpretasi Awal |
|---------------|--------------------|--------------------|
| `dxgi.dll` | **1** | Kandidat terkuat |
| `dsound.dll` | **1** | Kandidat baru — setara dxgi |
| `d3d11.dll` | 3 | Perlu verifikasi |
| `d3d9.dll` | 2 | Perlu verifikasi |
| `kernel32.dll` | 1 | Selalu di-import, bukan target |
| `ntdll.dll` | 1 | Selalu di-import, bukan target |
| `user32.dll` | 1 | Selalu di-import, bukan target |
| `WebView2Loader.dll` | **0** | Sinyal lemah |
| `version.dll` | 0 | Sinyal lemah |
| `dinput8.dll` | 0 | Sinyal lemah |
| `dbghelp.dll` | 0 | Sinyal lemah |
| `winmm.dll` | 0 | Sinyal lemah |

**Caveat**: ini string scan mentah, bukan import table parsing. BUT:
- `dxgi` dan `dsound` BUKAN dependency universal Windows executable — kehadiran string exact-nya di binary lebih mungkin berasal dari import table actual
- Perlu `dumpbin /imports` untuk konfirmasi final

## dxgi.dll Export Surface

- Lokasi: `C:\Windows\System32\dxgi.dll`
- Ukuran: 978,256 bytes (~954KB)
- Export: ~720 named exports (RVA 0xC7D40, size 660 bytes)
- Kesimpulan: proxy forwarding 720 export bisa ditangani oleh `pe_patcher` yang sudah ada

## Ranking Final Kandidat Target

1. **`dxgi.dll`** — string exact 1 match, ~720 exports, pe_patcher capable
2. **`dsound.dll`** — string exact 1 match, proven sideload di 3LayersPersistence, exports lebih manageable
3. **`d3d11.dll`** — 3 match, perlu verifikasi lebih lanjut
4. **`WebView2Loader.dll`** — 0 match, kompleksitas tinggi (export emulation, bukan proxy)
5. **`version.dll`** — baseline engineering existing, sinyal 0 match

## L2 Session Model — Design Summary

State machine formal:
```
IDLE → PREFLIGHT → INSTALLED → LAUNCHED → VERIFIED → RESTORED
                    ↘ ROLLBACK      ↘ RESTORE (if timeout)
```

Detail per fase sudah tertulis lengkap di `docs/PLANNING.md` L2 section, mencakup:
- Validasi per fase
- Format session file (JSON)
- Rollback guarantees
- Verify mechanism (Opsi A: file marker untuk prototype awal)
- Timeout dan retry strategy

## Decision Log

### Keputusan di checkpoint ini

1. L1 ditutup sebagai `done` — baseline loader sudah solid
2. L2 dibuka sebagai `in_progress` — state machine sudah didesain, siap implementasi
3. `dsound.dll` resmi masuk sebagai kandidat #2 setara `dxgi.dll`
4. `pe_patcher` divalidasi arsitekturnya oleh referensi 3LayersPersistence — arah engineering sudah benar
5. Fitur tambahan dari 3LayersPersistence (timestamp stomping, debug dir patch) ditunda ke L5

### Keputusan yang belum diambil

1. Target DLL final — butuh verifikasi import table dengan PE parser formal
2. Strategi akhir: proxy/sideload vs PE patch — menunggu L4
3. Mekanisme verify spesifik untuk payload — menunggu L3

## Gap Yang Masih Terbuka

1. **Belum ada bukti import statis** — string scan bukan bukti final, butuh `dumpbin /imports` atau PE parser
2. **Belum ada keputusan target DLL** —dxgi vs dsound vs lainnya
3. **Implementasi L2 belum ada di source code** — baru desain di dokumen
4. **Verify mechanism belum diimplementasi** — bagian dari L3
5. **Naming source masih historis** — `dxgi` di kode padahal target aktif `version`

## Next Steps (Rekomendasi)

1. **Prioritas tertinggi**: dapatkan bukti import table RobloxPlayerBeta.exe — install Windows SDK atau gunakan tool PE parser
2. Mulai implementasi L2 session state machine di `BloxHub.cpp`
3. Parallel: normalisasi naming di source (ganti istilah `dxgi` ke generic `target`)
4. Tentukan verify mechanism untuk L3

## Referensi

- PLANNING.md — detail L2 session model dan referensi eksternal
- ARCHITECTURE.md — kandidat target ranking terbaru
- CHECKPOINT_20260701_LOADER_IMPROVEMENT.md — baseline sebelumnya
- CHECKPOINT_20260702_DLL_TARGET_AUDIT.md — audit DLL target sebelumnya
