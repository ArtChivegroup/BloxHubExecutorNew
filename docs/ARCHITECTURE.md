# Arsitektur BloxHub Executor

## Dokumentasi Terkait

- Overview proyek: `../README.md`
- Build: `BUILD.md`
- Penggunaan: `USAGE.md`
- Bug dan gap: `BUGS.md`
- Planning: `PLANNING.md`
- Checkpoint aktif: `../checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`

## Ringkasan Eksekutif

Repositori ini sekarang harus dibaca sebagai arsitektur `loader-first` dengan track sekunder `manual-map`.

Ada dua jalur utama:

1. `Loader track`
   - memodifikasi artefak di disk,
   - menyiapkan file yang diperlukan,
   - meluncurkan Roblox,
   - lalu melakukan cleanup atau restore.
2. `Manual map track`
   - menyuntikkan payload langsung ke proses target,
   - menangani mapping, relocations, imports, dan CFG bypass di runtime.

Secara strategis, jalur pertama adalah fokus aktif karena:

- source baseline loader sudah ada,
- jalur ini paling dekat dengan analisis Volt yang sedang dijadikan acuan,
- dan perbaikan manual map saat ini tidak otomatis menyelesaikan kebutuhan pengembangan loader.

## Prinsip Arsitektur

- Perlakukan proyek ini sebagai eksperimen PE/loader Windows x64.
- Pisahkan `launcher logic`, `payload/proxy logic`, `patching logic`, dan `restore logic`.
- Backup dan restore adalah bagian inti arsitektur, bukan fitur tambahan.
- Dokumentasikan kondisi source code aktual, termasuk mismatch akibat eksperimen historis.
- Jaga manual map sebagai baseline pembanding, bukan pusat desain.

## System View

| Track | Entry Point | Teknik utama | Nilai saat ini |
|------|-------------|--------------|----------------|
| Loader modern | `src/BloxHub.cpp` | Proxy install + launch + cleanup | Baseline utama pengembangan |
| Loader PoC | `src/BloxHubLoader.cpp` | Import table patching langsung ke EXE | Referensi konsep patching |
| Manual map | `src/BloxHubInjector.cpp` | Injector + CFG bypass | Pembanding dan fallback riset |

## Komponen Utama

### 1. `BloxHub.exe`

Ini adalah prototype loader modern dan entry point terbaik untuk roadmap baru.

Tanggung jawab aktual:

- menerima path `RobloxPlayerBeta.exe` atau folder version,
- menormalisasi path target,
- mencari payload `version.dll` di folder executable loader,
- menyalin `System32\version.dll` menjadi `version_orig.dll` di folder Roblox,
- memanggil `ConvertPayloadToProxy(...)`,
- menulis hasil proxy sebagai `version.dll`,
- menjalankan Roblox via `CreateProcessW`,
- menunggu input user,
- lalu mencoba menghapus artefak yang dipasang sementara.

Karakter implementasi:

- sudah memiliki flow end-to-end,
- cocok sebagai baseline operasional,
- belum memiliki state management formal,
- restore belum tahan crash,
- verifikasi pasca-launch belum eksplisit.

### 2. `pe_patcher`

Modul ini adalah inti reusable dari track loader.

Fungsi utamanya:

- membaca payload DLL dari disk,
- membaca DLL sistem asli yang akan dijadikan referensi export,
- membangun tabel export forwarding dari DLL asli,
- menambahkan section `.edata` baru ke payload,
- menulis export directory dan forwarder strings,
- mengurutkan export names,
- memperbarui checksum dan timestamp,
- mengembalikan buffer final siap tulis ke disk.

Nilai arsitekturalnya tinggi karena modul ini sudah membuktikan hal-hal yang paling mahal untuk ditulis dari nol:

- parsing PE manual,
- patch metadata PE,
- export forwarding generation,
- checksum update,
- manipulasi section header.

### 3. `version.dll`

Target payload/proxy saat ini untuk `BloxHub.exe`.

Perannya:

- menjadi basis DLL yang dibungkus ulang oleh `pe_patcher`,
- dimuat dari folder loader sebagai payload awal,
- ditulis ke folder Roblox sebagai proxy `version.dll`,
- meneruskan export ke `version_orig.dll`.

Catatan penting:

- jalur ini adalah baseline implementasi yang ada sekarang,
- bukan bukti bahwa `version.dll` sudah pasti target terbaik untuk loader akhir.

### 3a. Kandidat target loader yang sedang dianalisis

Hasil analisa terbaru menambah konteks penting untuk pemilihan target DLL:

- `dxgi.dll` memiliki sinyal paling konkret pada instalasi Roblox lokal saat ini karena nama exact-nya muncul di `RobloxPlayerBeta.exe`.
- `WebView2Loader.dll` juga ada secara nyata di folder version Roblox lokal, dan `RobloxPlayerBeta.exe` mengandung string WebView2 seperti `CreateCoreWebView2Environment`.
- Namun, analisa cepat ini belum membuktikan bahwa `RobloxPlayerBeta.exe` melakukan import statis exact ke `WebView2Loader.dll`.
- `version.dll` tetap berguna sebagai baseline implementasi loader yang sudah ada, tetapi bukti target-nya lebih lemah daripada `dxgi.dll`.

**Referensi teknis WebView2Loader-style hijack:**

Contoh [WebView2Loader-Injection](../EXAMPLE%20PROJECT/WebView2Loader-Injection-main) memberikan referensi pola `DLL hijack via export emulation`:

- DLL shim mengganti `WebView2Loader.dll` asli yang dipakai host,
- tetap mengimplementasikan API WebView2 sehingga aplikasi tidak crash,
- implementasinya cukup kompleks: meniru loader contract lengkap dengan pencarian path runtime, bukan cuma forward sederhana,
- payload dapat berjalan via `DllMain` sambil tetap melayani panggilan API host.

Pola ini relevan untuk loader:

- bukan hanya proxy export yang pasif,
- tetapi `re-implement loader contract host`, sehingga shim tidak hanya meneruskan, tetapi benar-benar menggantikan logika loader asli,
- cocok jika target memang memuat DLL tersebut secara statis atau dinamis,
- risiko: lebih sulit dikembangkan daripada proxy sederhana, karena harus memahami contract lengkap DLL target.

Implikasi arsitektural:

- `dxgi.dll` masih layak diperlakukan sebagai kandidat dengan sinyal direct-load paling kuat,
- `WebView2Loader.dll` naik menjadi kandidat riset nyata dengan contoh referensi yang cukup lengkap,
- `version.dll` tetap baseline engineering, bukan baseline keyakinan target,
- jika target akhir memang menggunakan DLL yang kontrak API-nya rumit, contoh WebView2 menunjukkan bahwa proxy sederhana tidak cukup—harus re-implement contract lengkap.

### 4. `BloxHubLoader.exe`

PoC lama untuk patch import table langsung ke executable target.

Flow aktual:

- membuat backup `RobloxPlayerBeta.exe.backup`,
- membuka target PE,
- menambahkan import `BloxHubInternal.dll!BloxHubInit`,
- lalu menyimpan perubahan ke file target.

Nilai arsitektural:

- sangat sederhana,
- mudah dipahami,
- cocok sebagai fondasi untuk diskusi PE patching langsung,
- tetapi paling rawan terhadap integrity check dan deteksi perubahan file utama.

### 5. `BloxHubInjector.exe`

Track manual map yang tetap dipertahankan sebagai pembanding teknis.

Tanggung jawab:

- menunggu proses `RobloxPlayerBeta.exe`,
- memuat `BloxHubInternal.dll` dari folder executable,
- melakukan manual mapping ke proses target,
- menjalankan CFG bitmap patching,
- memanggil `BloxHubInit`.

Alasan masih dipertahankan:

- membantu membedakan masalah `payload` vs `loader`,
- menyediakan baseline bypass runtime,
- berguna jika perlu validasi fungsi payload secara terpisah dari modifikasi file di disk.

### 6. `BloxHubInternal.dll`

Payload legacy yang paling dekat dengan track manual map.

Perannya di arsitektur saat ini:

- payload uji untuk injector,
- referensi untuk callback `BloxHubInit`,
- aset yang mungkin tetap berguna bila loader akhir nanti memilih model load-DLL alih-alih proxy-only.

## Current Source Flows

### Flow `BloxHub.exe`

```text
argv / stdin
    ->
resolve RobloxPlayerBeta.exe
    ->
resolve loader directory
    ->
locate payload version.dll
    ->
copy System32\version.dll -> version_orig.dll
    ->
ConvertPayloadToProxy(payload, system dll, "version_orig.dll")
    ->
write proxy version.dll into Roblox folder
    ->
CreateProcessW(RobloxPlayerBeta.exe)
    ->
wait for user confirmation
    ->
delete proxy artifacts
```

Interpretasi arsitektural:

- ini adalah flow `install -> launch -> cleanup`,
- bukan patch-in-place pada executable target,
- masih berbasis interaksi manual user,
- belum punya fase `verify` yang memvalidasi keberhasilan load payload.

### Flow `BloxHubLoader.exe`

```text
input RobloxPlayerBeta.exe
    ->
create .backup
    ->
open PE
    ->
add import BloxHubInternal.dll!BloxHubInit
    ->
save modified executable
```

Interpretasi arsitektural:

- ini adalah eksperimen paling langsung ke konsep patching EXE,
- sederhana untuk dipahami,
- tetapi merupakan titik dengan risiko integrity check paling jelas.

### Flow `BloxHubInjector.exe`

```text
wait for RobloxPlayerBeta.exe
    ->
OpenProcess
    ->
read BloxHubInternal.dll
    ->
manual map into target
    ->
patch CFG bitmap
    ->
call BloxHubInit
```

Interpretasi arsitektural:

- kompleksitas dipindahkan ke runtime,
- bergantung pada kestabilan payload dan bypass,
- tidak memenuhi tujuan `loader-first` secara langsung.

## File-Level Ownership

| File | Peran |
|------|-------|
| `src/BloxHub.cpp` | Launcher dan installer proxy |
| `src/internal/pe_patcher.cpp` | Engine patching PE/proxy generation |
| `src/internal/pe_patcher.h` | Kontrak publik generator proxy |
| `src/internal/version_proxy.cpp` | Payload/proxy basis untuk jalur loader saat ini |
| `src/BloxHubLoader.cpp` | PoC patch import langsung ke EXE |
| `src/BloxHubInjector.cpp` | CLI untuk manual map |
| `src/injector/manual_map.cpp` | Engine manual mapping |
| `src/injector/cfg_bypass.cpp` | CFG bypass helpers |

## Loader-First Dependency Model

Urutan baca yang benar untuk pengembangan loader:

1. `src/BloxHub.cpp`
2. `src/internal/pe_patcher.h`
3. `src/internal/pe_patcher.cpp`
4. `src/internal/version_proxy.cpp`
5. `src/BloxHubLoader.cpp`
6. dokumentasi checkpoint loader

Urutan baca untuk track pembanding:

1. `src/BloxHubInjector.cpp`
2. `src/injector/manual_map.cpp`
3. `src/injector/cfg_bypass.cpp`
4. `src/internal/dllmain.cpp`

## Strengths Already Present

- Sudah ada launcher yang dapat resolve target dan launch Roblox.
- Sudah ada engine PE/proxy yang cukup kaya untuk dijadikan fondasi.
- Sudah ada backup dan cleanup dasar.
- Sudah ada dua model loader berbeda untuk dibandingkan.
- Sudah ada track manual map sebagai kontrol teknis.

## Architectural Gaps

- Target DLL aktif masih `version.dll`, padahal validitas target ini belum final.
- Naming di loader masih mewarisi istilah lama seperti `dxgi`.
- Restore belum bersifat transaksional atau tahan crash.
- Tidak ada session state formal seperti `prepared`, `installed`, `launched`, `verified`, `restored`, `failed`.
- Belum ada verifikasi pasca-launch yang membuktikan payload benar-benar termuat.
- Belum ada pemisahan tegas antara dokumen historis dan dokumen aktif sebelum update ini.

## Target Architecture

Fase target untuk loader utama:

1. `Preflight`
   - validasi path,
   - validasi ketersediaan payload,
   - identifikasi versi Roblox,
   - inventaris file yang akan diubah.

2. `Install`
   - backup file,
   - copy atau patch artefak,
   - generate proxy atau modifikasi PE,
   - verifikasi struktur hasil install.

3. `Launch`
   - launch via satu executable resmi,
   - simpan state sesi untuk restore.

4. `Verify`
   - cek bahwa payload benar-benar termuat,
   - log hasil minimal ke lokasi absolut yang mudah dicek.

5. `Restore`
   - rollback aman jika user exit atau proses gagal,
   - memastikan folder target kembali ke keadaan yang diharapkan.

## Why Loader Is The Active Priority

- Jalur ini sudah punya baseline source yang bisa dipoles, bukan nol.
- Pendekatan ini lebih selaras dengan analisis Volt dibanding manual map tradisional.
- Manual map saat ini masih terhambat oleh masalah payload dan runtime bypass yang tidak otomatis memajukan desain loader.
- Dokumentasi dan checkpoint proyek perlu mengarahkan AI ke pengembangan loader, bukan memperpanjang fokus pada injector.
