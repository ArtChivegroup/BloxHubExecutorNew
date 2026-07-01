# Daftar Bug & Risiko Teknis

## Related Documentation

- Project overview: `../README.md`
- Architecture: `ARCHITECTURE.md`
- Planning: `PLANNING.md`
- Active checkpoint: `../checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`

## Cara Membaca Dokumen Ini

Daftar ini sekarang diprioritaskan untuk `loader-first development`.

Urutannya:

1. bug dan gap yang langsung menghambat loader aktif,
2. risiko arsitektural loader yang belum berubah menjadi bug implementasi,
3. blocker manual map yang tetap dicatat sebagai konteks sekunder.

## Bug Aktif - Loader Track

### 1. Target DLL aktif belum final

Status: `active`

Deskripsi:

- `BloxHub.exe` saat ini mengandalkan payload `version.dll`.
- Hasil eksperimen historis menunjukkan target ini belum tentu ideal untuk Roblox.
- Artinya, baseline loader saat ini masih valid sebagai platform pengembangan, tetapi target pemuatannya belum dianggap keputusan final.

Dampak:

- hasil uji loader bisa bias karena masalah target DLL, bukan masalah workflow loader,
- sulit membedakan `desain loader salah` vs `target DLL salah`.

Kebutuhan:

- dokumentasikan target DLL yang dipakai setiap eksperimen,
- jadikan pemilihan target sebagai keputusan eksplisit, bukan asumsi tersembunyi.

### 2. Restore loader belum crash-safe

Status: `active`

Deskripsi:

- `BloxHub.exe` saat ini melakukan cleanup setelah user menekan Enter.
- Jika proses gagal di tengah, user menutup loader, atau file terkunci, rollback tidak dijamin lengkap.

Dampak:

- folder Roblox bisa tertinggal dalam state parsial,
- risiko kebingungan saat pengujian berikutnya meningkat.

Kebutuhan:

- definisikan state sesi,
- catat file yang dipasang,
- buat aturan restore minimum yang tidak bergantung pada alur sukses penuh.

### 3. Tidak ada fase verify pasca-launch

Status: `active`

Deskripsi:

- Loader saat ini dapat memasang proxy dan menjalankan Roblox,
- tetapi belum memiliki bukti formal bahwa payload benar-benar termuat dan berjalan sesuai harapan.

Dampak:

- sukses launch bisa menjadi false positive,
- debugging akan lambat karena tidak ada checkpoint verifikasi yang tegas.

Kebutuhan:

- tetapkan sinyal verifikasi minimum,
- gunakan log absolut atau indikator sederhana lain sebagai bukti load sukses.

### 4. Terminologi source loader masih membawa jejak eksperimen lama

Status: `active`

Deskripsi:

- beberapa variabel, pesan log, dan asumsi implementasi masih mewarisi istilah historis seperti `dxgi`,
- padahal target aktif saat ini adalah `version.dll`.

Dampak:

- dokumentasi dan source lebih sulit dipahami,
- AI atau developer baru mudah salah menarik kesimpulan.

Kebutuhan:

- normalisasi istilah pada fase implementasi berikutnya,
- pastikan nama simbol dan log mencerminkan strategi loader aktif.

### 5. `BloxHubLoader.exe` berisiko tinggi terhadap integrity check

Status: `active but secondary`

Deskripsi:

- PoC ini memodifikasi executable target langsung dengan menambah import `BloxHubInternal.dll!BloxHubInit`.
- Jalur ini penting sebagai laboratorium PE patching, tetapi secara desain paling rentan terhadap deteksi integritas file.

Dampak:

- bukan kandidat terbaik untuk baseline operasional harian,
- lebih cocok dipakai sebagai pembanding konsep terhadap model Volt-style.

Kebutuhan:

- perlakukan sebagai track eksperimen,
- jangan menyamakan PoC ini dengan loader utama sampai strategi final dipilih.

## Risiko Arsitektural - Loader Track

### 1. Proxy generation sudah matang, tetapi session model belum ada

Status: `risk`

Deskripsi:

- `pe_patcher` sudah cukup kaya untuk membuat proxy DLL,
- tetapi `BloxHub.exe` belum punya state machine sederhana seperti `prepared`, `installed`, `launched`, `verified`, `restored`, `failed`.

Risiko:

- sulit membuat restore yang deterministik,
- sulit mendiagnosis kegagalan antar fase.

### 2. Patch-in-place belum punya pagar pengaman yang cukup

Status: `risk`

Deskripsi:

- repositori sudah memiliki `BloxHubLoader.exe` sebagai baseline patch import langsung,
- tetapi belum ada strategi baku untuk backup multi-file, validasi hasil patch, atau rollback jika patch gagal.

Risiko:

- file target bisa rusak atau tidak sinkron,
- eksperimen jadi mahal untuk diulang.

### 3. `version_proxy.cpp` masih sangat eksperimental

Status: `risk`

Deskripsi:

- payload/proxy saat ini masih berisi logging dan perilaku `DllMain` yang khas fase eksperimen,
- belum didokumentasikan sebagai payload final produksi loader.

Risiko:

- perilaku runtime proxy bisa tercampur antara debugging, stealth, dan eksperimen historis,
- sulit menjadikannya baseline jangka panjang tanpa spesifikasi baru.

## Blocker Sekunder - Manual Map Track

### 1. CRT dependency crash pada `BloxHubInternal.dll`

Status: `secondary active`

Deskripsi:

- payload manual map masih membawa dependensi CRT,
- proses target tidak menjamin keberadaan dependensi tersebut,
- hasilnya bisa berupa crash sebelum log terbentuk.

Catatan:

- ini tetap penting untuk jalur injector,
- tetapi bukan prioritas utama selama fokus proyek berada di loader track.

### 2. CFG bitmap candidate belum tervalidasi penuh

Status: `secondary active`

Deskripsi:

- kandidat bitmap sudah ditemukan,
- tetapi validasi akhirnya masih tertahan oleh stabilitas payload manual map.

### 3. `CreateRemoteThread` tetap menjadi titik risiko

Status: `secondary active`

Deskripsi:

- jalur injector saat ini masih bergantung pada `CreateRemoteThread`,
- ini adalah risiko klasik untuk track bypass runtime.

## Historical Findings Worth Preserving

### 1. KnownDLLs tetap tidak cocok untuk proxy hijack

Implikasi:

- jangan mengarahkan loader ke target yang secara native diambil Windows dari KnownDLLs.

### 2. Integrity dan signature check adalah batas desain, bukan sekadar bug

Implikasi:

- beberapa eksperimen lama gagal bukan karena coding error,
- tetapi karena jalur desainnya memang bertabrakan dengan model verifikasi target.

### 3. Manual map perlu payload yang sangat ketat

Implikasi:

- payload manual map harus pure Win32 API,
- tidak boleh membawa asumsi CRT atau dependency chain sembarangan.

## Closed / Legacy Items

### 1. `ReadFileFromDiskW` unresolved external

Status: `closed`

Catatan: isu legacy pada era proxy generation, sudah tidak menjadi blocker utama.

### 2. False positive pada CFG bitmap auto-scan

Status: `closed`

Catatan: filter scanner sudah diperketat pada jalur manual map.

### 3. Restore gagal karena file locked

Status: `partially improved`

Catatan: sudah ada error handling lebih baik, tetapi restore loader tetap belum dianggap selesai secara arsitektural.

## Referensi

- Active checkpoint: `../checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`
- Planning: `PLANNING.md`
