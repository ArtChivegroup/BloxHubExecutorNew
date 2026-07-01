# Planning & Roadmap

## Related Documentation

- Project overview: `../README.md`
- Architecture details: `ARCHITECTURE.md`
- Known bugs and risks: `BUGS.md`
- Active checkpoint: `../checkpoints/CHECKPOINT_20260702_PLANNING_L2.md`

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

### Candidate target ranking (updated 2026-07-02)

Untuk fase planning, kandidat target DLL sekarang sebaiknya dibaca seperti ini:

1. `dxgi.dll` **(~720 named exports, 978KB)**
   - punya sinyal paling konkret pada instalasi Roblox lokal saat ini: nama exact muncul di `RobloxPlayerBeta.exe`,
   - system DLL besar dengan banyak export, tapi baseline `pe_patcher` sudah mampu membangun forwarding untuk jumlah export sebanyak ini,
   - perlu diverifikasi apakah ini import statis atau string sisa — string scan saja tidak cukup, butuh parser PE formal,
   - jika terbukti, proxy forwarding 720 export adalah pekerjaan yang bisa ditangani `pe_patcher`.
2. `dsound.dll` **(new — ditemukan di audit string scan 2026-07-02)**
   - nama exact-nya juga muncul di `RobloxPlayerBeta.exe` — sinyal setara dengan `dxgi.dll`,
   - ini adalah kandidat baru yang sebelumnya tidak masuk daftar riset,
   - pola sideload `dsound.dll` sudah proven dari referensi `3LayersPersistence` (Spotify hijack),
   - jika `dxgi.dll` ternyata bukan direct import, `dsound.dll` bisa menjadi kandidat alternatif dengan sinyal setara.
3. `WebView2Loader.dll`
   - ada secara nyata di folder version Roblox lokal,
   - contoh proyek referensi `WebView2Loader-Injection` menunjukkan model `export emulation + loader contract re-implementation` yang lebih kaya daripada proxy biasa,
   - `RobloxPlayerBeta.exe` mengandung string WebView2 API seperti `CreateCoreWebView2Environment`,
   - tetapi belum ada bukti cukup bahwa EXE melakukan import statis exact ke `WebView2Loader.dll`,
   - jika memang relevan, tingkat implementasinya lebih berat daripada proxy sederhana (hanya 4 export tapi perlu re-implement contract lengkap).
4. `version.dll`
   - sudah menjadi baseline implementasi `BloxHub.exe`,
   - tetap berguna untuk mengembangkan workflow loader,
   - tetapi sinyal kecocokan target-nya paling lemah.

**Temuan audit lokal (updated):**

- Binary Roblox lokal: `c:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-1a951716f19e4638\`.
- String scan exact pada `RobloxPlayerBeta.exe`:
  - `dxgi.dll` — 1 match
  - `dsound.dll` — 1 match
  - `d3d11.dll` — 3 match
  - `d3d9.dll` — 2 match
  - `kernel32.dll` — 1 match
  - `ntdll.dll` — 1 match
  - `user32.dll` — 1 match
  - `WebView2Loader.dll` — 0 match
  - `version.dll` — 0 match
  - `dinput8.dll` — 0 match
  - `dbghelp.dll` — 0 match
- `WebView2Loader.dll` sendiri memang ada di folder version tersebut.
- `dxgi.dll` di System32 memiliki ~720 named exports (978KB file) — proxy forwarding dimungkinkan oleh `pe_patcher` yang sudah ada.

Konsekuensi planning:

- jangan menganggap target aktif saat ini sudah benar hanya karena loader bisa dibangun di atasnya,
- **`dxgi.dll` dan `dsound.dll` sekarang dua kandidat teratas** karena sama-sama punya sinyal exact string di binary target,
- `WebView2Loader.dll` tetap kandidat riset menarik, tapi dengan caveat: implementasi lebih berat (export emulation, bukan proxy forwarding),
- `version.dll` tetap baseline engineering — bukan baseline keyakinan target,
- **langkah kritis berikutnya**: buktikan import path dengan PE parser formal (`dumpbin` / `pe-bear`) untuk memastikan mana yang static import vs dynamic load vs string coincidence.

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

Status: `done`

### Milestone L2 - Preflight And Session Model

Tujuan:

- loader punya fase `preflight`,
- validasi path dan payload sebelum install,
- simpan state sesi agar restore tidak bergantung pada memori user.

Deliverables:

- desain state `prepared / installed / launched / verified / restored / failed`,
- daftar file dan artefak yang dicadangkan,
- aturan rollback minimum.

Status: `in_progress`

#### L2 Design: Session State Machine

State formal loader (satu instance per sesi):

```text
[IDLE]
   |
   v
[PREFLIGHT] ──fail──> [FAILED]
   |                    (log error, exit)
   | success
   v
[INSTALLED] ──fail──> [ROLLBACK]
   |                    (restore yang sudah di-copy, lalu exit)
   | success
   v
[LAUNCHED] ──timeout/no-verify──> [RESTORE]
   |                                (cleanup, flag incomplete session)
   | verify ok
   v
[VERIFIED]
   |
   | (user exit / Roblox close)
   v
[RESTORED]  atau  [FAILED]  (jika restore gagal)
```

#### L2 Design: Per-Fase Detail

**Fase PREFLIGHT:**
- Validasi `RobloxPlayerBeta.exe` exists dan readable.
- Validasi payload DLL (`version.dll` atau target lain) exists di folder loader.
- Baca system DLL asli (e.g. `C:\Windows\System32\dxgi.dll`) — pastikan exists dan readable.
- Hitung hash/checksum file target sebelum modifikasi (untuk verifikasi restore nanti).
- Inventaris file yang akan dibuat/modifikasi:
  - `<roblox_dir>\<target>.dll` (proxy hasil generate)
  - `<roblox_dir>\<target>_orig.dll` (copy system DLL asli)
- Tulis session file ke folder loader: `bloxhub_session.json` atau `.bloxhub_state` yang berisi:
  - timestamp
  - roblox_dir
  - payload path
  - target DLL name
  - daftar file pre-existing di roblox_dir (untuk bedakan artefak baru vs bawaan)
  - status: `preflight_ok`

**Fase INSTALL:**
- Jika `<target>_orig.dll` belum ada → copy dari system DLL asli.
- Jika sudah ada (sisa sesi sebelumnya) → skip copy, tapi catat di log.
- Generate proxy via `ConvertPayloadToProxy(...)`.
- Tulis proxy ke `<roblox_dir>\<target>.dll`.
- Update session file: status = `installed`.
- Jika gagal di tengah → ROLLBACK: hapus file yang baru dibuat, pastikan tidak meninggalkan state kotor.

**Fase LAUNCH:**
- `CreateProcessW(RobloxPlayerBeta.exe)` dengan working directory = roblox_dir.
- Simpan PID ke session file.
- Start timer/monitor: tunggu sinyal verify atau timeout.
- Update session file: status = `launched`, pid = <PID>.

**Fase VERIFY:**
- Mekanisme sinyal sukses dari payload (pilih salah satu, dokumentasikan):
  - Opsi A: payload tulis file marker ke lokasi absolut (e.g. `C:\Users\Public\bloxhub_loaded.txt`) — paling sederhana, tapi sangat terdeteksi.
  - Opsi B: payload kirim sinyal via named pipe / local socket ke loader — lebih stealth, perlu infrastructure tambahan.
  - Opsi C: payload inject bukti ke memory/registry yang dibaca loader — variasi lain.
- Default untuk prototype awal: **Opsi A**, karena fokus sekarang adalah membuktikan flow kerja, bukan stealth.
- Timeout verify: misal 30 detik. Jika timeout, anggap session incomplete, tetap launch RESTORE.
- Update session file: status = `verified`.

**Fase RESTORE:**
- Baca session file untuk daftar file yang dibuat.
- Hapus `<target>.dll` (proxy kita).
- Hapus `<target>_orig.dll` (copy system DLL) — hanya jika file ini dibuat oleh sesi KITA (bukan pre-existing dari sesi lain).
- Verifikasi post-restore: bandingkan hash file kunci dengan hash preflight.
- Update session file: status = `restored`.
- Jika gagal hapus (file locked, dll) → status = `failed`, catat file yang masih tersisa.

#### L2 Design: Session File Format

File disimpan di folder executable loader: `<loader_dir>\bloxhub_session.dat`

Format: binary kecil atau JSON plaintext. Untuk prototype, JSON lebih mudah dibaca:

```json
{
  "session_id": "20260702-001",
  "status": "installed",
  "roblox_dir": "C:\\Users\\...\\Versions\\version-xxx",
  "target_dll": "dxgi",
  "payload_path": "C:\\...\\version.dll",
  "system_dll_source": "C:\\Windows\\System32\\dxgi.dll",
  "created_files": [
    "dxgi.dll",
    "dxgi_orig.dll"
  ],
  "pre_existing_files": ["RobloxPlayerBeta.exe", "WebView2Loader.dll", "..."],
  "roblox_pid": 0,
  "timestamp_start": "2026-07-02T12:00:00Z",
  "verify_timeout_sec": 30
}
```

#### L2 Design: Rollback Guarantees

- Jika loader crash sebelum INSTALL selesai: tidak ada file yang tertulis → no-op.
- Jika loader crash setelah INSTALL: saat restart, loader baca session file yang statusnya `installed` → auto-trigger RESTORE.
- Jika Roblox crash: loader tetap bisa RESTORE karena session file independen dari proses Roblox.
- Jika file terkunci: retry 3x dengan delay 1 detik, lalu catat sebagai `failed` dengan daftar file tersisa.

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

## Analisa Referensi Eksternal

### Referensi 1: 3LayersPersistence (Maldev Academy)

Proyek ini sangat relevan untuk arsitektur loader karena:

- **`ConvertExecutableToDll` engine**: EXE membaca dirinya sendiri dari disk, lalu mem-patch menjadi proxy DLL yang meniru export DLL target. Alurnya:
  - Set `IMAGE_FILE_DLL` di Characteristics
  - Redirect `AddressOfEntryPoint` ke `DllMain`
  - Ubah `Subsystem` ke `IMAGE_SUBSYSTEM_WINDOWS_GUI`
  - Baca export DLL asli → bangun forwarded export table → append `.edata` section baru
  - Stomp PE timestamp (30 hari lebih tua dari DLL asli)
  - Stomp debug directory timestamp
  - Update checksum

- **Arsitektur ini MIRIP dengan `pe_patcher` BloxHub** tetapi lebih matang:
  - BloxHub `pe_patcher`: sudah punya `ConvertPayloadToProxy()`, build export forwarding, update checksum, patch `.edata`
  - 3LayersPersistence: menambahkan **timestamp stomping**, **debug directory patching**, dan **entry point redirect**
  - **Kesimpulan**: `pe_patcher` bisa diperkaya dengan teknik timestamp stomping dan debug dir patching dari referensi ini

- **3 layer persistence** yang didemonstrasikan:
  - Layer 1: WMI permanent event subscription (admin required)
  - Layer 2: COM hijacking via HKCU CLSID (no admin)
  - Layer 3: DLL sideloading `dsound.dll` di folder Spotify (no admin)
  - Untuk BloxHub, layer 3 paling relevan: membuktikan bahwa **pola sideload `dsound.dll` sudah proven di production malware**.

- **Mutex guard + registry installation flag**: pola untuk mencegah payload jalan ganda dan skip re-installasi — relevan untuk session model L2.

### Referensi 2: RBX-cfg-bypass

- CFG bypass pasca-Hyperion yang mengklaim masih working di versi Roblox `version-2a06298afe3947ab`.
- Teknik: whitelist insertion + bitmap patch langsung, hindari `insert_set`.
- Menggunakan `CFG_PAGE_HASH_KEY` (`0xF7455279`) dan `CFG_VALIDATION_XOR` (`0xA9`) yang hardcoded.
- **Nilai untuk BloxHub**: jika jalur manual map butuh CFG bypass yang lebih mutakhir, ini bisa jadi referensi. Tapi untuk loader track, tidak langsung relevan karena loader tidak menyentuh runtime injection.
- **Red flag**: versi hardcoded offsets, repo sangat tipis, tidak jelas apakah masih working di versi Roblox terbaru.

### Referensi 3: WebView2Loader-Injection (sudah dianalisa sebelumnya)

- Pola `export emulation + loader contract re-implementation`.
- Referensi arsitektural untuk kandidat target `WebView2Loader.dll`.

## Near-Term Work Queue

1. ~~Jadikan `BloxHub.exe` sebagai loader baseline resmi dalam semua dokumen.~~ (done)
2. ~~Petakan file yang disentuh loader dan perilaku restore saat ini.~~ (done)
3. ~~Pastikan checkpoint baru merangkum `apa yang sudah ada`, `apa yang belum ada`, dan `apa objective berikutnya`.~~ (done)
4. **Selesaikan desain L2 Session Model** — state machine sudah didesain, tinggal diputuskan untuk coding.
5. **Prioritas selanjutnya: buktikan import path target DLL** — perlu tool PE parser formal (`dumpbin` / `pe-bear`) untuk membaca import table `RobloxPlayerBeta.exe` secara proper, bukan cuma string scan.
6. Definisikan bagaimana keberhasilan loader akan diverifikasi (bagian dari L3).
7. Evaluasi strategi Volt-style setelah target DLL confirmed.

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

- Active checkpoint: `../checkpoints/CHECKPOINT_20260702_PLANNING_L2.md`
- Bug list: `BUGS.md`
- Architecture: `ARCHITECTURE.md`
