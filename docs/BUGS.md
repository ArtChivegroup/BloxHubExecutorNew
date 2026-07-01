
# Daftar Bug & Masalah

---
## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For architecture details**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **For roadmap & planning**: [PLANNING.md](PLANNING.md)
- **For progress checkpoints**: [../checkpoints/CHECKPOINT_20260701_FINAL.md](../checkpoints/CHECKPOINT_20260701_FINAL.md)

---
## Bug Aktif

### 1. Roblox Damaged (Import Hijacking)
**Status**: ❌ Critical, Dibatalkan!  
**Deskripsi**: Ketika menggunakan BloxHubLoader/BloxHub.exe untuk memodifikasi Import Table RobloxPlayerBeta.exe, Roblox menampilkan pesan "Roblox has missing or damaged files. Please reinstall it." dan tidak bisa dibuka.  
**Alasan**: Hyperion Integrity Check memverifikasi hash file RobloxPlayerBeta.exe!  
**Solusi**: Dibatalkan, pindah ke DLL Proxying!

### 2. Koneksi TCP Gagal (Silent Bridge)
**Status**: ⏸️ Pending (Fokus ke DLL Proxy dulu)
**Deskripsi**: Listener TCP di DLL tidak bisa dihubungi dari client.

### 3. DLL Proxy Gagal Load di Roblox
**Status**: 🔄 Active
**Deskripsi**:
- Static .def: Proxy DLL tidak pernah dimuat oleh Roblox
- Dynamic PE Patching (3Layers): Roblox mencoba load `dxgi.dll` tapi masih gagal!
**Kemungkinan Penyebab**:
  - Kita pilih DLL target yang tidak dicari Roblox di folder aplikasi
  - Proxy DLL masih memiliki bug di export table!
**Langkah Debug**:
  - Tambahkan lebih banyak logging di `DllMain` (ke file yang jelas seperti `C:\bloxhub_debug.txt`)
  - Cek apakah `DllMain` dipanggil sama sekali!
  - Coba DLL target lain yang biasa di-sideload!

---
## Bug Yang Sudah Diperbaiki
### Bug 1: LNK Error unresolved external `ReadFileFromDiskW`
**Status**: ✅ Fixed!
**Alasan**: `ReadFileFromDiskW` dideklarasikan static di dalam fungsi `ConvertPayloadToProxy`!
**Fix**: Pindahkan deklarasi static ke atas file `pe_patcher.cpp`!

---
## Penemuan Penting
### 1. KnownDLLs Registry
JANGAN target DLL yang ada di `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs`! DLL tersebut selalu di-load dari System32 dan tidak bisa di-proxy!
Daftar KnownDLLs yang dicek:
- `winmm.dll` ❌
- `kernel32.dll` ❌
- `user32.dll` ❌
- dll.

### 2. DLL Yang Di-Load Tapi Tidak Di-Search di Folder App
Banyak DLL yang muncul di daftar modules Roblox tapi tidak dicari di folder aplikasi! Kita harus coba satu per satu!

---
## Referensi
- [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701_FINAL.md)
