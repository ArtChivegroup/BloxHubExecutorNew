
# Daftar Bug & Masalah

---

## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For architecture details**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **For roadmap & planning**: [PLANNING.md](PLANNING.md)
- **For progress checkpoints**: [../checkpoints/CHECKPOINT_20260701.md](../checkpoints/CHECKPOINT_20260701.md)

---

## Bug Aktif

### 1. Roblox Damaged (Import Hijacking)
**Status**: Critical, Terdeteksi & Tidak Bisa Diperbaiki  
**Deskripsi**: Ketika menggunakan BloxHubLoader/BloxHub.exe untuk memodifikasi Import Table RobloxPlayerBeta.exe, Roblox menampilkan pesan "Roblox has missing or damaged files. Please reinstall it." dan tidak bisa dibuka.  
**Fakta**:
- Backup dan restore otomatis bekerja dengan sempurna!
- Copy DLL ke direktori Roblox berhasil!
- Modifikasi Import Table berhasil (pe_bliss tidak error)!
- Sudah coba ganti nama section dan perbaiki checksum — **tetap saja terdeteksi!**  
**Kesimpulan**: Import Hijacking tidak bisa digunakan untuk melewati Hyperion saat ini!
**Solusi yang Dipilih**: Pivot ke DLL Proxying! Lihat [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md) dan [Planning & Roadmap](../docs/PLANNING.md)!

### 2. Koneksi TCP Gagal (Silent Bridge)
**Status**: Active  
**Deskripsi**: Listener TCP di DLL tidak bisa dihubungi dari client. Error yang terjadi:
- `connect failed` di sisi client
- `recv() failed: 10038` di sisi listener (Socket operation on non-socket)  
**Kemungkinan Penyebab**:
- Socket tidak diinisialisasi dengan benar
- WSAStartup tidak dipanggil sebelum membuat socket
- Listener berhenti sebelum menerima koneksi
- Firewall memblokir koneksi localhost
**Rencana Perbaikan**: Pindahkan inisialisasi socket ke scheduler hook dan set ke Non-Blocking Mode! Lihat [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md)!

### 3. DLL Tidak Berjalan di Roblox
**Status**: Active  
**Deskripsi**: DLL berhasil diinjek tapi tidak berjalan sebagaimana mestinya di Roblox (hanya di Notepad).  
**Kemungkinan Penyebab**:
- Hyperion memblokir DLL yang tidak dikenal
- Inisialisasi yang hang di DLL_PROCESS_ATTACH
- Crash karena akses memory yang tidak valid

## Catatan Penting
- ✅ **BloxHub.exe (Modern Loader Dasar) Sudah Berjalan**: Backup, copy DLL, dan restore bekerja!
- ❌ **Import Hijack Tidak Bisa Lewati Hyperion**: Ini adalah masalah utama!
- 📝 **Fokus Sekarang**: Pivot ke DLL Proxying!
- 📌 **Referensi Lainnya**:
  - [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md)
  - [Arsitektur Sistem](../docs/ARCHITECTURE.md)
  - [Planning & Roadmap](../docs/PLANNING.md)
