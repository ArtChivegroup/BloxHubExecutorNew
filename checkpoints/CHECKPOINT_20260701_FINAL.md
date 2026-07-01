
# Checkpoint 2026-07-01 - DLL Proxying Dynamic with 3LayersPersistence

---
## 📝 Ringkasan Hari Ini
Kita mulai dari DLL Proxying static dengan .def yang gagal, kemudian pindah ke **Dynamic Proxy Generation** (Runtime PE Patching) menggunakan teknik dari `3LayersPersistence`!

---
## 📊 Semua Percobaan & Hasilnya
| Percobaan | DLL Target | Metode | Hasil | Alasan |
|-----------|------------|--------|-------|--------|
| 1 | `winmm.dll` | Static .def | ❌ Gagal | `winmm.dll` berada di **KnownDLLs** (registry), jadi Windows selalu load dari System32! |
| 2 | `dinput8.dll` | Static .def | ❌ Tidak Load | `dinput8.dll` tidak dicari Roblox di folder aplikasi! |
| 3 | `dbghelp.dll` | Static .def | ❌ Tidak Load | `dbghelp.dll` juga tidak dicari di folder aplikasi! |
| 4 | `version.dll` | Static .def | ❌ Tidak Load | Sama seperti di atas! |
| 5 | `dxgi.dll` | Dynamic PE Patching (3Layers) | ⚠️ Gagal Load | Roblox **mencoba** load `dxgi.dll` dari folder aplikasi tapi proxy kita masih gagal! |

---
## 🛠️ Yang Sudah Dibuat
1. **`pe_patcher.h` & `pe_patcher.cpp`**: Implementasi teknik `ConvertExeToDll` dari 3LayersPersistence!
   - Baca DLL asli dari System32
   - Bangun export table forwarder ke `*_orig.dll`
   - Patch payload DLL kita dengan export table tersebut
   - Hitung PE Checksum, ubah timestamp DLL

2. **Semua Proxy DLL di `src/internal/`**:
   - `winmm_proxy.cpp` (legacy)
   - `dinput8_proxy.cpp` (legacy)
   - `dbghelp_proxy.cpp` (legacy)
   - `version_proxy.cpp` (legacy)
   - `dxgi_proxy.cpp` (current target)

3. **Update `BloxHub.cpp`**:
   - Dynamic proxy DLL generation
   - Copy DLL asli ke `*_orig.dll` di folder Roblox
   - Write proxy DLL yang sudah di-patch ke folder Roblox
   - Auto-launch Roblox after 3 detik countdown
   - Restore file ketika selesai

---
## 🐛 Bug & Keanehan Yang Ditemukan
1. **LNK Error unresolved external `ReadFileFromDiskW`**:
   - Alasan: `ReadFileFromDiskW` dideklarasikan static di dalam fungsi!
   - Fix: Pindahkan deklarasi static ke atas file!

2. **Proxy Static .def Gagal Load**:
   - Alasan: Banyak DLL tidak dicari di folder aplikasi oleh Roblox (meskipun terlihat di daftar modules)!

3. **PE Patcher Ruang Header**:
   - Sudah ditambahkan pengecekan apakah ada ruang di PE header untuk menambah section baru!

---
## 📝 Catatan Penting Dari 3LayersPersistence
1. **KnownDLLs**: JANGAN target DLL yang ada di `HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\KnownDLLs`!
2. **Forwarding Format**: `MODULE.FunctionName` atau `MODULE.#Ordinal`!
3. **Timestamp**: Buat proxy DLL memiliki timestamp lebih tua 30-60 hari dari asli untuk menghindari deteksi!
4. **Export Table Sorting**: Nama di export table harus diurutkan ascending!
5. **Checksum**: Selalu hitung ulang PE checksum setelah memodifikasi!

---
## 🚀 Rencana Selanjutnya
1. **Coba DLL Lain Yang Biasa Di-Sideload**:
   - `msvcp140.dll`
   - `vcruntime140.dll`
   - `d3d9.dll`
   - Atau coba lihat daftar modules Roblox lagi yang bukan KnownDLLs dan coba satu per satu!
2. **Debug Proxy DLL**:
   - Tambahkan lebih banyak logging (misal log ke `C:\bloxhub_debug.txt` agar tidak hilang)
   - Cek apakah `DllMain` dipanggil sama sekali!
3. **Alternatif Lain**:
   - Jika DLL Proxying total gagal, coba teknik Manual Map + CFG Bypass dari `EXAMPLE PROJECT/RBX-cfg-bypass-main`!

---
## 📌 File Yang Berubah Hari Ini
- `src/BloxHub.cpp`: Diupdate untuk dynamic proxy dan auto-launch
- `src/internal/pe_patcher.h` & `pe_patcher.cpp`: Ditambahkan dari 3Layers
- `src/internal/dxgi_proxy.cpp`: Target proxy baru
- `CMakeLists.txt`: Diupdate target proxy ke `dxgi.dll`
