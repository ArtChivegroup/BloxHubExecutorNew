
# Daftar Bug & Masalah

## Bug Aktif

### 1. Roblox Damaged (Import Hijacking)
**Status**: Critical  
**Deskripsi**: Ketika menggunakan BloxHubLoader untuk memodifikasi RobloxPlayerBeta.exe, Roblox menampilkan pesan "Roblox Damaged" dan tidak bisa dibuka.  
**Kemungkinan Penyebab**:
- Hyperion memverifikasi checksum/integritas file Roblox
- Modifikasi Import Table terdeteksi oleh anti-tamper
- PE header yang diubah tidak sesuai dengan ekspektasi Roblox

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

### 3. DLL Tidak Berjalan di Roblox
**Status**: Active  
**Deskripsi**: DLL berhasil diinjek tapi tidak berjalan sebagaimana mestinya di Roblox (hanya di Notepad).  
**Kemungkinan Penyebab**:
- Hyperion memblokir DLL yang tidak dikenal
- Inisialisasi yang hang di DLL_PROCESS_ATTACH
- Crash karena akses memory yang tidak valid

## Catatan Penting
- Semua testing untuk Notepad dihapus karena tidak relevan dan tidak berhasil
- Fokus sekarang adalah membuat injector yang bekerja untuk Roblox dengan stealth
