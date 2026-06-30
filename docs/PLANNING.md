
# Planning & Roadmap

---

## Related Documentation
- **For project overview**: [../README.md](../README.md)
- **For architecture details**: [ARCHITECTURE.md](ARCHITECTURE.md)
- **For known bugs**: [BUGS.md](BUGS.md)
- **For progress checkpoints**: [../checkpoints/CHECKPOINT_20260701.md](../checkpoints/CHECKPOINT_20260701.md)

---

## Evolusi Arsitektur: Modern Loader Integration
BloxHub akan diadaptasi ke **Modern Loader** yang memisahkan frontend dan backend, serta mengelola payload secara dinamis untuk evasi Hyperion yang lebih baik.

---

## Versi 1.0 - Dasar (Belum Selesai, Perlu Improvement & Testing)
- [x] Setup project CMake
- [x] Implementasi BloxHubLoader (Import Hijacking) - **DICABAIKAN! Import Hijack TERDETEKSI Hyperion! Lihat [Daftar Bug](../docs/BUGS.md) dan [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md)!**
- [x] Implementasi BloxHubInjector (Manual Map) - **SUDAH TESTED BERHASIL DI NOTEPAD!**
- [x] Implementasi BloxHubInternal.dll (logging dasar) - **SUDAH TESTED BERHASIL DI NOTEPAD!**
- [x] Implementasi BloxHub.exe (Modern Loader Tahap 1: Dasar) - **SUDAH TESTED! Backup/Copy/Restore bekerja, tapi Import Hijack gagal di Roblox!**

---

## Versi 2.0 - Modern Loader (Switch ke DLL Proxying!)
- [x] Gabungkan BloxHubLoader dan BloxHubClient menjadi **BloxHub.exe** (Modern Loader Dasar - Backup/Copy DLL/Restore sudah bekerja!)
- [ ] Fase 1: Buat DLL Proxy (`version.dll`) dengan export forwarding ke `version_orig.dll` (lihat [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md))
- [ ] Fase 2: Update `BloxHub.exe` untuk drop DLL Proxy dan restore otomatis
- [ ] Fase 3: Fix Silent Bridge (TCP Listener) di DLL Proxy (Non-Blocking Mode, poll di scheduler hook)
- [ ] Fase 4: Implementasi CFG Bypass (jika perlu, lihat `../EXAMPLE PROJECT/RBX-cfg-bypass-main/`)

---

## Versi 2.1 - Modern Loader (Tahap 2: Silent Bridge)
- [ ] Implementasi thread-safe **Execution Queue** di DLL
- [ ] Implementasi **Task Scheduler Hook** di DLL
- [ ] Implementasi **PE Header Wiping** di DllMain DLL_PROCESS_ATTACH
- [ ] Implementasi enkripsi payload Lua di memory

---

## Versi 2.2 - Modern Loader (Tahap 3: Dynamic Payload)
- [ ] Implementasi **Dynamic Payload Fetching** (opsional):
  - Gunakan WinHTTP/CPR untuk fetch payload dari server
  - Payload dienkripsi unik per-HWID
  - Drop payload hanya sebagai file sementara (temp) dengan nama acak
- [ ] (Opsional) Authentication & Licensing (HWID + Key)

---

## Versi 3.0 - Lua Execution
- [ ] Integrasi dengan LuaVM Roblox
- [ ] Set identity level (6/8)
- [ ] Executor Lua dasar di UI
- [ ] Script hub

---

## Prioritas Tinggi
1. Buat DLL Proxy (`version.dll`)
2. Update `BloxHub.exe` untuk DLL Proxying setup/restore
3. Fix bug koneksi TCP listener (Non-Blocking Mode di scheduler hook)
4. PE Header Wiping di DLL

---

## Prioritas Sedang
1. Task Scheduler Hook
2. Execution Queue thread-safe
3. Dynamic Port Handshake
4. CFG Bypass
5. Enkripsi payload Lua

---

## Prioritas Rendah
1. Dynamic Payload Fetching dari server
2. Authentication & Licensing
3. GUI Client (sekarang bisa CLI dulu)
4. Script hub online
5. Auto-update

---

## Referensi Lainnya
- [Checkpoint Terakhir](../checkpoints/CHECKPOINT_20260701.md) - Rencana detail langkah demi langkah!
- [Daftar Bug](../docs/BUGS.md) - Masalah yang sedang terjadi!
- [Arsitektur Sistem](../docs/ARCHITECTURE.md) - Penjelasan arsitektur!
