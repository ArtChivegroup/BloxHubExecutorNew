# Checkpoint 2026-07-02 - DLL Target Audit

## Tujuan Checkpoint

Checkpoint ini merangkum temuan analisa target DLL untuk loader, khususnya `WebView2Loader.dll` sebagai kandidat riset baru dan pembandingan sinyal load antara kandidat aktif.

Dokumen ini dibuat agar AI atau developer berikutnya langsung paham:

- kandidat target DLL mana yang punya bukti load paling kuat,
- apa temuan dari audit lokal binary Roblox,
- apa referensi teknis yang layak dijadikan acuan untuk masing-masing target.

## Executive Summary

Audit target DLL dilakukan sebagai bagian mega planning untuk memperkuat baseline dokumentasi loader.

Hasil utama:

- `dxgi.dll` punya sinyal load paling konkret: nama exact-nya muncul di `RobloxPlayerBeta.exe`.
- `WebView2Loader.dll` naik menjadi kandidat riset karena: file memang ada di folder Roblox lokal, ada string WebView2 API di binary EXE, dan ada referensi contoh proyek yang menunjukkan pola loader `export emulation + contract re-implementation`.
- `version.dll` tetap baseline implementasi loader yang sudah ada, tetapi sinyal target-nya lemah.

## Audit Local Roblox Binary

### Binary yang diaudit

- `c:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\RobloxPlayerBeta.exe`
- `c:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\WebView2Loader.dll`

### Metode audit

Karena tool PE parser formal tidak tersedia di shell lokal, audit dilakukan dengan metode string scan cepat pada binary untuk mendeteksi nama DLL exact.

Metode ini tidak sempurna, tetapi cukup untuk membedakan sinyal kuat vs lemah tanpa setup tooling.

### Hasil

| DLL Candidate | File ada di folder Roblox | String exact muncul di EXE | String related muncul di EXE |
|---------------|----------------------------|----------------------------|------------------------------|
| `dxgi.dll` | Tidak dicek (system DLL) | **Ya** | - |
| `WebView2Loader.dll` | **Ya** | Tidak | `WebView2`, `Loader`, `CreateCoreWebView2Environment`, `WebView` |
| `version.dll` | Tidak (system DLL) | Tidak | Tidak dicek |
| `dinput8.dll` | Tidak dicek | Tidak | - |
| `dbghelp.dll` | Tidak dicek | Tidak | - |

### Interpretasi

- `dxgi.dll` punya sinyal paling konkret: nama exact-nya muncul di binary, menunjukkan kemungkinan besar binary tersebut statically imports atau directly loads DLL ini.
- `WebView2Loader.dll` punya file nyata di folder Roblox dan ada string related yang menunjukkan API WebView2 dipakai di suatu titik, tetapi tidak ada bukti import statis exact.
- `version.dll` masih baseline implementasi loader yang ada, tetapi bukti target-nya paling lemah dari kandidat lain.

## Referensi Teknis WebView2Loader-Injection

### Sumber

`EXAMPLE PROJECT/WebView2Loader-Injection-main`

### Pola yang ditunjukkan

Proyek ini adalah contoh loader-style hijack berbasis `export emulation + loader contract re-implementation`:

- DLL shim menggantikan `WebView2Loader.dll` asli yang dimuat host.
- Shim tidak hanya forward export, tetapi meniru logika loader lengkap untuk menemukan dan memuat WebView2 runtime yang terpasang di sistem.
- API public seperti `CreateCoreWebView2EnvironmentWithOptions` di-implement ulang agar host tetap berjalan normal.
- Payload dapat berjalan via `DllMain` sambil tetap melayani panggilan dari host.

File inti:

- [WebView2Loader.cpp](../EXAMPLE%20PROJECT/WebView2Loader-Injection-main/WebView2Loader/Module/UD/WebView2Loader.cpp): berisi re-implementation lengkap loader contract WebView2.
- [WebView2Loader.def](../EXAMPLE%20PROJECT/WebView2Loader-Injection-main/WebView2Loader/Module/UD/WebView2Loader.def): export definition untuk memastikan nama/ordinal cocok dengan DLL asli.
- [EntryPoint.cpp](../EXAMPLE%20PROJECT/WebView2Loader-Injection-main/WebView2Loader/Module/EntryPoint.cpp): payload entry point via `DllMain` (saat ini demo level dengan `MessageBoxA`).

### Nilai teknis

Pola ini relevan untuk loader:

- Menunjukkan bahwa untuk target DLL dengan contract kompleks, proxy sederhana tidak cukup.
- Loader harus re-implement logika asli DLL agar aplikasi tetap normal.
- Ini lebih sulit dikembangkan daripada baseline proxy forwarding yang sudah ada di `pe_patcher`, tetapi lebih robust jika target memang dipakai host secara serius.

### Red flag

- Payload masih sangat demo-level (`MessageBoxA` + `std::thread`).
- Ada script signing dengan sertifikat/PFX hardcoded yang terlihat murahan.
- README sangat tipis dan tidak menjelaskan teknis kenapa ini berhasil.

Yang layak dipelajari adalah pola `export emulation + contract re-implementation`, bukan payload atau signing ritual mereka.

## Kandidat Target Ranking

Setelah audit ini, kandidat target loader sebaiknya dibaca dengan urutan prioritas:

### 1. `dxgi.dll` (sinyal kuat)

- punya bukti load paling konkret,
- nama exact muncul di binary `RobloxPlayerBeta.exe`,
- baseline proxy loader yang ada sudah cocok untuk target ini.

### 2. `WebView2Loader.dll` (sinyal sedang, kompleksitas tinggi)

- file memang ada di folder Roblox lokal,
- ada string related WebView2 API di EXE,
- contoh referensi menunjukkan pola loader contract re-implementation yang lebih berat,
- **tetapi belum ada bukti import statis exact**,
- jika benar-benar relevan, tingkat implementasi lebih berat daripada proxy sederhana.

### 3. `version.dll` (baseline implementasi, sinyal lemah)

- sudah menjadi baseline implementasi `BloxHub.exe`,
- tetap berguna untuk mengembangkan workflow loader,
- tetapi bukti target-nya paling lemah dari kandidat lain.

## Rekomendasi Planning

1. Pertahankan `dxgi.dll` sebagai kandidat utama dengan prioritas tertinggi untuk investigasi loader lanjut.
2. Simpan `WebView2Loader.dll` sebagai kandidat riset sekunder, tetapi jangan pivot penuh sebelum ada bukti load path yang lebih kuat atau investigasi export-import yang mendalam.
3. Jika `WebView2Loader.dll` terbukti relevan nanti, loader tidak bisa cuma proxy sederhana—harus re-implement contract loader lengkap seperti contoh referensi.
4. Untuk sekarang, jangan ubah baseline loader hanya karena ada kandidat baru; tetap fokus stabilisasi loader untuk target aktif dulu.

## Gap Yang Diakui

- Audit ini belum memakai tool PE parser formal seperti `dumpbin`, `pe-bear`, atau parser custom, jadi hasilnya cepat tetapi tidak sempurna.
- Audit ini tidak memeriksa dynamic load / `LoadLibrary` path secara mendalam.
- Audit ini tidak memeriksa versi Roblox lain atau instalasi non-Bloxstrap.

Untuk planning, temuan ini cukup. Untuk engineering detail, audit yang lebih mendalam mungkin dibutuhkan nanti.

## Decision Log

### Keputusan saat checkpoint ini

- Kandidat target DLL sekarang resmi diranking: `dxgi.dll` > `WebView2Loader.dll` > `version.dll`.
- Contoh referensi `WebView2Loader-Injection` diakui sebagai pola teknis yang relevan untuk loader, tetapi bukan untuk diadopsi mentah-mentah.
- Dokumentasi inti (`ARCHITECTURE`, `PLANNING`) sudah diperkaya dengan temuan audit ini.

### Keputusan yang belum diambil

- Apakah target akhir loader memang `dxgi.dll` atau tetap `version.dll` atau berpindah ke kandidat lain.
- Apakah loader akhir tetap memakai baseline proxy sederhana atau harus re-implement contract DLL target.

## Outcome

Setelah checkpoint ini:

- kandidat target DLL sudah punya ranking eksplisit berdasarkan temuan audit lokal,
- AI berikutnya bisa langsung baca referensi teknis `WebView2Loader-Injection` sebagai contoh pola loader contract re-implementation,
- dokumentasi planning sudah diperkaya dengan konteks target yang lebih jelas.

