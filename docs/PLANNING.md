# Planning & Roadmap

## Related Documentation

- Project overview: `../README.md`
- Architecture details: `ARCHITECTURE.md`
- Known bugs and risks: `BUGS.md`
- Active checkpoint: `../checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`

## Planning Position

Roadmap proyek resmi sekarang adalah `loader-first roadmap`.

Manual map tidak dihapus dari repositori, tetapi perannya diperkecil menjadi:

- baseline pembanding perilaku payload,
- jalur riset sekunder,
- fallback jika perlu memisahkan masalah runtime injection dari masalah loader di disk.

Artinya, planning tidak lagi disusun dengan asumsi "manual map dulu, loader nanti". Urutan yang benar sekarang adalah:

1. matangkan loader,
2. dokumentasikan gap loader,
3. gunakan manual map hanya bila membantu validasi atau pembandingan.

## Baseline Yang Sudah Dimiliki

### Loader baseline

- `BloxHub.exe` sudah memiliki flow `resolve target -> install proxy -> launch -> cleanup`.
- `pe_patcher` sudah bisa membangun export-forwarding proxy dari payload DLL.
- `BloxHubLoader.exe` sudah membuktikan konsep patch import langsung ke executable target.

### Manual map baseline

- `BloxHubInjector.exe` sudah dapat melakukan manual mapping.
- CFG bitmap patching sudah terintegrasi.
- Jalur ini tetap berguna untuk eksperimen terpisah, tetapi bukan milestone aktif utama.

## Decision Framework

Semua pengembangan berikutnya dievaluasi dengan pertanyaan sederhana:

1. apakah perubahan ini membuat loader utama lebih jelas, lebih dapat diuji, atau lebih aman di-restore,
2. apakah perubahan ini membantu memilih strategi loader akhir,
3. jika tidak, apakah pekerjaan itu sebaiknya ditunda ke track manual map atau fase setelah loader stabil.

## Loader Strategy Options

### Option A: Proxy / sideload style

Karakter:

- install file pendukung ke folder Roblox,
- gunakan DLL target yang dapat dimuat native oleh Windows,
- jalankan Roblox lewat launcher,
- restore file setelah sesi selesai.

Kelebihan:

- baseline source sudah ada,
- paling cepat dikembangkan dari kondisi repo saat ini,
- cocok untuk membangun workflow installer, launcher, dan restore.

Kekurangan:

- target DLL aktif `version.dll` belum terbukti final,
- perilaku load order Roblox harus terus diverifikasi.

### Option B: PE patch / import patch / section rewrite

Karakter:

- memodifikasi `RobloxPlayerBeta.exe` atau artefak PE terkait,
- menambah import, section, atau metadata baru,
- berpotensi lebih dekat dengan pola Volt.

Kelebihan:

- lebih dekat dengan model loader tanpa injector eksternal,
- bisa menjadi jalur pengembangan jangka menengah jika proxy target tidak stabil.

Kekurangan:

- lebih tinggi risiko integrity check,
- memerlukan restore yang lebih kuat,
- lebih berbahaya untuk file target jika patching gagal.

### Working assumption saat ini

Strategi termurah untuk dikembangkan dari source yang sudah ada adalah:

1. pertahankan `BloxHub.exe` sebagai baseline operasional,
2. pertajam state machine dan restore,
3. dokumentasikan keputusan target DLL,
4. gunakan `BloxHubLoader.exe` sebagai laboratorium konsep patching langsung,
5. bandingkan hasilnya dengan analisis Volt sebelum memilih strategi akhir.

### Candidate target ranking saat ini

Untuk fase planning, kandidat target DLL sekarang sebaiknya dibaca seperti ini:

1. `dxgi.dll`
   - punya sinyal paling konkret pada instalasi Roblox lokal saat ini,
   - nama exact-nya muncul di `RobloxPlayerBeta.exe`,
   - tetap perlu dijaga sebagai kandidat utama untuk investigasi lanjut.
2. `WebView2Loader.dll`
   - ada secara nyata di folder version Roblox lokal,
   - contoh proyek referensi `WebView2Loader-Injection` menunjukkan model `export emulation + loader contract re-implementation` yang lebih kaya daripada proxy biasa,
   - `RobloxPlayerBeta.exe` mengandung string WebView2 API seperti `CreateCoreWebView2Environment`,
   - contoh menunjukkan pola: bukan cuma forward export, tetapi re-implement full contract loader agar host tetap berjalan normal sambil payload termuat,
   - tetapi belum ada bukti cukup bahwa EXE melakukan import statis exact ke `WebView2Loader.dll`,
   - jika memang relevan, tingkat implementasinya lebih berat daripada proxy sederhana.
3. `version.dll`
   - sudah menjadi baseline implementasi `BloxHub.exe`,
   - tetap berguna untuk mengembangkan workflow loader,
   - tetapi sinyal kecocokan target-nya lebih lemah.

**Temuan audit lokal:**

- Binary Roblox lokal tersedia di `c:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\`.
- `WebView2Loader.dll` memang ada di folder tersebut.
- String scan cepat menunjukkan: `dxgi.dll` exact string muncul di `RobloxPlayerBeta.exe`, tetapi `WebView2Loader.dll` exact string tidak ditemukan.
- `RobloxPlayerBeta.exe` mengandung string umum seperti `WebView2`, `Loader`, `CreateCoreWebView2Environment`, menunjukkan kemungkinan dynamic load atau indirect reference.

Konsekuensi planning:

- jangan menganggap target aktif saat ini sudah benar hanya karena loader bisa dibangun di atasnya,
- jangan langsung pivot penuh ke `WebView2Loader.dll` sebelum ada bukti load path yang lebih kuat atau investigasi export-import mendalam,
- pertahankan `dxgi.dll` dan `WebView2Loader.dll` sebagai dua kandidat riset utama di atas `version.dll`,
- untuk `WebView2Loader.dll`, jika benar-benar dipakai, loader tidak bisa cuma proxy sederhana—harus re-implement contract loader seperti contoh referensi.

## Active Milestones

### Milestone L0 - Documentation Reset

Tujuan:

- semua dokumen inti harus mengarah ke `loader improvement`,
- AI langsung paham objective proyek,
- checkpoint historis tetap tersimpan tetapi tidak mendikte roadmap aktif.

Status: `done`

### Milestone L1 - Loader Baseline Stabilization

Tujuan:

- definisikan flow loader resmi,
- dokumentasikan file yang disentuh,
- rapikan istilah historis yang membingungkan,
- nyatakan gap restore, verify, dan target DLL secara eksplisit.

Deliverables:

- baseline documentation lengkap,
- checkpoint loader improvement,
- daftar gap teknis yang terurut.

Status: `active`

### Milestone L2 - Preflight And Session Model

Tujuan:

- loader punya fase `preflight`,
- validasi path dan payload sebelum install,
- simpan state sesi agar restore tidak bergantung pada memori user.

Deliverables:

- desain state `prepared / installed / launched / verified / restored / failed`,
- daftar file dan artefak yang dicadangkan,
- aturan rollback minimum.

Status: `next`

### Milestone L3 - Verify And Restore Hardening

Tujuan:

- loader bisa membuktikan payload termuat,
- restore tetap mungkin setelah gagal di tengah proses,
- ada logging minimum untuk audit sesi.

Deliverables:

- verifikasi pasca-launch,
- restore checklist,
- strategi handling bila Roblox atau loader crash.

Status: `planned`

### Milestone L4 - Strategy Selection

Tujuan:

- memutuskan apakah jalur utama tetap `proxy/sideload`,
- atau dipindahkan ke `PE patch / section rewrite` ala Volt.

Keputusan ini baru diambil setelah:

- baseline loader sekarang dibersihkan,
- risiko restore dipahami,
- dan keterbatasan target DLL aktif terdokumentasi jelas.

Status: `planned`

### Milestone L5 - Implementation Track

Pilihan implementasi setelah L4:

- lanjutkan modern proxy loader, atau
- bangun loader ala Volt berbasis patching file yang lebih agresif.

Status: `deferred until strategy decision`

## Near-Term Work Queue

1. Jadikan `BloxHub.exe` sebagai loader baseline resmi dalam semua dokumen.
2. Petakan file yang disentuh loader dan perilaku restore saat ini.
3. Pastikan checkpoint baru merangkum `apa yang sudah ada`, `apa yang belum ada`, dan `apa objective berikutnya`.
4. Definisikan bagaimana keberhasilan loader akan diverifikasi.
5. Baru setelah itu evaluasi strategi Volt-style secara lebih agresif.

## Comparison Against Manual Map

| Aspek | Loader track | Manual map track |
|------|--------------|------------------|
| Fokus aktif | Ya | Tidak |
| Baseline source tersedia | Ya | Ya |
| Menjawab objective user saat ini | Ya | Tidak langsung |
| Bergantung bypass runtime | Lebih sedikit | Sangat tinggi |
| Membantu desain workflow install/restore | Ya | Tidak |
| Berguna sebagai pembanding teknis | Ya | Ya |

## Success Criteria

Planning fase loader dianggap bergerak benar jika:

- dokumen utama konsisten menyebut loader sebagai fokus aktif,
- AI baru bisa mengidentifikasi file inti loader tanpa membaca seluruh histori repo,
- gap teknis loader terurut berdasarkan prioritas,
- dan keputusan berikutnya dapat dibandingkan langsung terhadap baseline loader, bukan terhadap asumsi lama manual map.

## Deferred Work

Item berikut sengaja tidak menjadi prioritas aktif selama fase loader:

- rewrite payload manual map hanya untuk memulihkan track injector,
- thread hijacking untuk injector,
- direct syscall untuk injector,
- integrasi Lua executor,
- GUI client.

Item-item ini tetap valid, tetapi baru masuk lagi setelah loader track memiliki bentuk yang jelas atau bila dibutuhkan sebagai riset pendukung.

## References

- Active checkpoint: `../checkpoints/CHECKPOINT_20260701_LOADER_IMPROVEMENT.md`
- Bug list: `BUGS.md`
- Architecture: `ARCHITECTURE.md`
