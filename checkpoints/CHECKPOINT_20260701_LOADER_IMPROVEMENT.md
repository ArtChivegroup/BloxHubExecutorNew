# Checkpoint 2026-07-01 - Loader Improvement Baseline

## Tujuan Checkpoint

Checkpoint ini adalah baseline resmi untuk fase `loader-first` pada proyek BloxHub Executor.

Dokumen ini dibuat agar AI atau developer berikutnya langsung memahami:

- apa objective aktif proyek,
- source loader apa saja yang sudah dimiliki,
- bagaimana loader saat ini bekerja,
- apa gap yang masih terbuka,
- dan mengapa manual map sekarang diposisikan sebagai jalur sekunder.

## Executive Summary

Fokus proyek resmi sekarang dipindahkan ke `pengembangan loader`, bukan ke penyempurnaan manual map.

Alasan utamanya:

- repositori sudah memiliki baseline loader nyata,
- `BloxHub.exe` sudah punya flow install-launch-cleanup,
- `pe_patcher` sudah mampu membungkus payload DLL menjadi proxy dengan export forwarding,
- `BloxHubLoader.exe` sudah memberi baseline untuk patch-import langsung ke executable target,
- sementara track manual map masih memerlukan pekerjaan runtime yang tidak langsung memajukan desain loader.

Kesimpulan praktis:

- `BloxHub.exe` adalah fondasi utama yang harus dikembangkan,
- `BloxHubLoader.exe` adalah laboratorium PE patching,
- `BloxHubInjector.exe` dipertahankan sebagai pembanding teknis.

## Ruang Lingkup Source Yang Relevan

### Loader modern

- `src/BloxHub.cpp`
- `src/internal/pe_patcher.h`
- `src/internal/pe_patcher.cpp`
- `src/internal/version_proxy.cpp`

### Loader PoC berbasis patching EXE

- `src/BloxHubLoader.cpp`

### Track pembanding

- `src/BloxHubInjector.cpp`
- `src/injector/manual_map.cpp`
- `src/injector/cfg_bypass.cpp`
- `src/internal/dllmain.cpp`

### Build definition

- `CMakeLists.txt`

## Source Audit - Apa Yang Sudah Ada

### 1. `BloxHub.exe` sudah menjadi loader baseline

Fakta dari source:

- menerima path executable Roblox atau folder version,
- melakukan normalisasi target ke `RobloxPlayerBeta.exe`,
- mencari payload `version.dll` di folder executable loader,
- menyalin `System32\version.dll` ke `version_orig.dll`,
- menghasilkan proxy DLL baru dengan `ConvertPayloadToProxy(...)`,
- menulis `version.dll` hasil proxy ke folder Roblox,
- meluncurkan Roblox dengan `CreateProcessW`,
- menunggu Enter dari user,
- mencoba cleanup file artefak yang dipasang.

Interpretasi:

- ini bukan sekadar eksperimen kecil,
- ini sudah berbentuk launcher operasional,
- dan karena itu layak dijadikan fondasi pengembangan resmi.

### 2. `pe_patcher` adalah aset inti yang paling bernilai

Fakta dari source:

- membaca payload DLL dari disk,
- membaca DLL sistem asli,
- membangun tabel export forwarding,
- membuat section `.edata` baru,
- menulis export directory baru,
- menyortir export names,
- memperbarui checksum,
- mengembalikan buffer final siap tulis.

Interpretasi:

- modul ini menghemat pekerjaan besar karena semua logika PE paling mahal sudah ada,
- dan sangat cocok dijadikan mesin utama bagi semua eksperimen loader berbasis proxy.

### 3. `BloxHubLoader.exe` adalah PoC patch-import yang sederhana

Fakta dari source:

- membuat backup `RobloxPlayerBeta.exe.backup`,
- membuka file target sebagai PE,
- menambahkan import `BloxHubInternal.dll!BloxHubInit`,
- menyimpan perubahan ke executable target.

Interpretasi:

- ini jalur paling mudah untuk diskusi patching langsung ke EXE,
- tetapi tidak boleh langsung diasumsikan sebagai loader utama karena risiko integrity check sangat tinggi.

### 4. Manual map tetap ada, tetapi bukan driver roadmap

Fakta dari source:

- `BloxHubInjector.exe` menunggu proses `RobloxPlayerBeta.exe`,
- memuat `BloxHubInternal.dll`,
- melakukan manual map,
- menjalankan CFG bitmap patching,
- memanggil `BloxHubInit`.

Interpretasi:

- track ini penting untuk pembanding dan debugging,
- tetapi pekerjaan berikutnya tidak boleh otomatis dipusatkan ke sini.

## Loader Baseline Saat Ini

### Flow aktual `BloxHub.exe`

```text
input path
    ->
resolve RobloxPlayerBeta.exe
    ->
locate payload version.dll
    ->
copy System32\version.dll -> version_orig.dll
    ->
ConvertPayloadToProxy(...)
    ->
write proxy version.dll
    ->
launch Roblox
    ->
wait for Enter
    ->
delete proxy artifacts
```

### Karakter baseline

- Sudah ada flow operasional end-to-end.
- Sudah ada backup sebagian terhadap DLL sistem yang dipakai.
- Sudah ada proxy generation dinamis.
- Belum ada verify step yang formal.
- Belum ada restore yang tahan crash.
- Belum ada state session yang terdokumentasi secara eksplisit.

## Perbandingan Singkat Dengan Manual Map

| Aspek | Loader track | Manual map track |
|------|--------------|------------------|
| Fokus proyek saat ini | Ya | Tidak |
| Mengubah artefak di disk | Ya | Tidak |
| Membutuhkan bypass runtime intensif | Lebih sedikit | Ya |
| Sudah punya launcher workflow | Ya | Tidak |
| Berguna untuk pembanding teknis | Ya | Ya |
| Menjawab objective user sekarang | Ya | Tidak langsung |

## Gap Utama Yang Harus Diakui

### 1. Target DLL aktif belum dianggap final

`BloxHub.exe` saat ini dibangun di sekitar `version.dll`, tetapi keputusan ini belum final. Loader saat ini harus dipandang sebagai `platform pengembangan`, bukan bukti bahwa target DLL aktif sudah benar untuk strategi akhir.

### 2. Restore masih lemah

Cleanup saat ini bergantung pada alur sukses dan input user. Jika loader atau Roblox berhenti di tengah, rollback tidak dijamin lengkap.

### 3. Verify step belum ada

Belum ada mekanisme resmi yang menyatakan:

- payload termuat,
- sesi install sukses,
- atau restore selesai benar.

### 4. Source masih memuat istilah historis

Masih ada naming dan asumsi lama dari eksperimen sebelumnya. Ini mengurangi keterbacaan dan bisa menyesatkan AI yang baru membaca repo.

### 5. Track patch-import langsung masih sangat eksperimental

`BloxHubLoader.exe` penting sebagai PoC, tetapi terlalu dini untuk dijadikan workflow utama sampai risiko integrity check dipahami lebih jelas.

## Decision Log

### Keputusan aktif

1. Dokumentasi proyek harus membaca repo ini sebagai `loader-first`.
2. `BloxHub.exe` ditetapkan sebagai baseline loader modern yang paling penting.
3. `pe_patcher` diperlakukan sebagai engine inti untuk semua eksperimen proxy/forwarding.
4. `BloxHubLoader.exe` dipertahankan sebagai referensi konsep patching langsung.
5. `BloxHubInjector.exe` dipertahankan sebagai jalur pembanding, bukan driver roadmap.

### Keputusan yang belum diambil

1. Apakah strategi akhir tetap `proxy/sideload`.
2. Apakah proyek akan bergeser ke `PE patch / section rewrite` ala Volt.
3. Target DLL mana yang paling layak untuk loader akhir.

## Objective Fase Berikutnya

Fase berikutnya bukan langsung menulis loader baru dari nol. Objective yang lebih benar adalah:

1. tegaskan baseline loader yang sudah ada,
2. identifikasi semua file dan fase yang disentuh loader,
3. definisikan model session `preflight -> install -> launch -> verify -> restore`,
4. gunakan baseline ini untuk membandingkan strategi Volt-style dengan lebih bersih.

## Rencana Kerja Yang Direkomendasikan

### Fase 1 - Stabilize baseline documentation

- pastikan semua dokumen inti konsisten menyebut loader sebagai prioritas aktif,
- jadikan checkpoint ini sebagai acuan utama.

### Fase 2 - Define loader session model

- tetapkan state minimum,
- tetapkan artefak yang harus dibackup,
- tetapkan kondisi restore minimum.

### Fase 3 - Define verification model

- tentukan bukti minimum bahwa payload berhasil termuat,
- dokumentasikan lokasi log atau indikator validasi.

### Fase 4 - Strategy selection

- bandingkan jalur proxy/sideload dengan jalur patching file ala Volt,
- ambil keputusan setelah baseline loader benar-benar dipahami.

## Non-Goals Saat Checkpoint Ini Dibuat

Item berikut sengaja tidak menjadi fokus aktif checkpoint ini:

- perbaikan penuh payload manual map,
- thread hijacking,
- direct syscalls,
- Lua executor,
- GUI client.

Semua item di atas tetap valid, tetapi tidak membantu menjawab objective aktif proyek secepat pengembangan loader.

## Outcome

Setelah checkpoint ini:

- repo harus dibaca sebagai proyek `loader improvement`,
- manual map tetap ada sebagai pembanding,
- semua dokumentasi inti diarahkan agar AI baru langsung tahu apa yang sudah ada dan apa objective berikutnya.
