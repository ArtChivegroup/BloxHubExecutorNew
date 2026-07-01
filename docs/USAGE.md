# Cara Penggunaan

## Posisi Dokumen

Dokumen ini sekarang mengikuti strategi `loader-first`.

Urutan penggunaan yang benar:

1. gunakan `BloxHub.exe` sebagai baseline loader utama,
2. gunakan `BloxHubLoader.exe` hanya untuk eksperimen patch-import langsung,
3. gunakan `BloxHubInjector.exe` hanya sebagai track riset pembanding.

## Workflow Utama - `BloxHub.exe`

`BloxHub.exe` adalah executable yang saat ini paling mewakili arah proyek.

Flow aktualnya:

1. terima path `RobloxPlayerBeta.exe` atau folder version Roblox,
2. cari payload `version.dll` di folder executable loader,
3. salin `System32\version.dll` menjadi `version_orig.dll` ke folder target,
4. generate proxy DLL baru lewat `pe_patcher`,
5. tulis proxy `version.dll` ke folder Roblox,
6. launch Roblox,
7. tunggu input user,
8. hapus artefak sementara saat cleanup.

### Menjalankan `BloxHub.exe`

1. Buka terminal di folder output build:

```cmd
cd "C:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew\build\bin\Release"
```

2. Jalankan loader dengan path executable atau folder version:

```cmd
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxxxxxxxxxxxx\RobloxPlayerBeta.exe"
```

Atau:

```cmd
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxxxxxxxxxxxx"
```

3. Pastikan file `version.dll` payload berada di folder yang sama dengan `BloxHub.exe`.

4. Loader akan:

- menampilkan direktori Roblox yang dipakai,
- mengecek keberadaan payload,
- membuat `version_orig.dll` jika belum ada,
- membuat proxy `version.dll`,
- meluncurkan Roblox otomatis setelah countdown,
- menunggu Enter untuk cleanup.

### Artefak Yang Disentuh

Pada baseline loader saat ini, file yang paling relevan adalah:

- `RobloxPlayerBeta.exe`
- `version.dll`
- `version_orig.dll`

### Catatan Penting

- Jalur ini adalah prototype, bukan workflow yang sudah dianggap final.
- Cleanup saat ini masih manual dan bergantung pada user menekan Enter.
- Belum ada verifikasi formal bahwa payload termuat sukses setelah launch.

## Workflow Sekunder - `BloxHubLoader.exe`

`BloxHubLoader.exe` adalah PoC untuk patch import table langsung ke executable target.

### Menjalankan `BloxHubLoader.exe`

1. Buka terminal di folder output build:

```cmd
cd "C:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew\build\bin\Release"
```

2. Jalankan patcher:

```cmd
BloxHubLoader.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxxxxxxxxxxxx\RobloxPlayerBeta.exe"
```

### Apa yang Dilakukan PoC Ini

- membuat backup `RobloxPlayerBeta.exe.backup`,
- membuka PE target,
- menambahkan import `BloxHubInternal.dll!BloxHubInit`,
- menyimpan hasil patch ke file target.

### Kapan Dipakai

- saat ingin membandingkan strategi proxy loader dengan patch-import langsung,
- saat ingin memetakan risiko jalur yang lebih dekat ke model Volt-style,
- bukan sebagai workflow utama harian.

## Workflow Pembanding - `BloxHubInjector.exe`

`BloxHubInjector.exe` dipertahankan untuk riset manual map dan CFG bypass.

### Menjalankan `BloxHubInjector.exe`

1. Jalankan Roblox terlebih dahulu.
2. Buka terminal di folder output build:

```cmd
cd "C:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew\build\bin\Release"
```

3. Jalankan injector:

```cmd
BloxHubInjector.exe
```

4. Injector akan menunggu `RobloxPlayerBeta.exe`, lalu mencoba inject `BloxHubInternal.dll`.

### Kapan Dipakai

- saat perlu membedakan bug payload vs bug loader,
- saat perlu baseline pembanding di luar modifikasi file disk,
- bukan sebagai fokus pengembangan utama saat ini.

## Expected Operator Flow

Untuk fase proyek sekarang, urutan kerja yang direkomendasikan adalah:

1. build seluruh target,
2. uji `BloxHub.exe` sebagai baseline loader,
3. catat file yang berubah dan perilaku restore,
4. jika perlu pembanding, baru uji `BloxHubLoader.exe`,
5. gunakan `BloxHubInjector.exe` hanya bila debugging menuntut isolasi masalah payload.

## Troubleshooting

### `BloxHub.exe` tidak menemukan `RobloxPlayerBeta.exe`

Periksa:

- path yang diberikan memang menunjuk ke executable atau folder version yang benar,
- folder version benar-benar berisi `RobloxPlayerBeta.exe`.

### `BloxHub.exe` tidak menemukan `version.dll`

Periksa:

- payload `version.dll` tersedia di folder yang sama dengan executable loader,
- build output menghasilkan artefak DLL di lokasi yang sama.

### Cleanup tidak mengembalikan keadaan folder seperti semula

Ini sesuai gap arsitektural yang masih aktif:

- restore loader belum crash-safe,
- file sisa harus dicek manual jika sesi berhenti di tengah.

### `BloxHubLoader.exe` berhasil patch tetapi runtime tetap gagal

Interpretasi awal:

- sukses patch file tidak sama dengan sukses load runtime,
- pertimbangkan bahwa masalah bisa berasal dari integrity check, bukan dari kegagalan tool patching.

### `BloxHubInjector.exe` sukses inject tetapi tidak ada bukti payload berjalan

Interpretasi awal:

- track manual map masih punya blocker payload dan runtime bypass,
- ini adalah gap jalur sekunder, bukan indikator bahwa baseline loader juga salah.

## Manual Cleanup

Jika sesi uji loader meninggalkan artefak sementara, cek folder Roblox dan hapus file yang relevan sesuai eksperimen aktif, misalnya:

```cmd
del "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxxxxxxxxxxxx\version.dll"
del "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-xxxxxxxxxxxxxxxx\version_orig.dll"
```

Lakukan ini hanya jika memang yakin file tersebut artefak eksperimen. Jika ragu, gunakan verifikasi file dari launcher Roblox atau Bloxstrap.
