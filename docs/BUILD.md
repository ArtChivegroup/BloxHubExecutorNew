# Cara Build

## Prasyarat

- Visual Studio 2022 — workload **Desktop development with C++**
- CMake 3.20+
- Windows SDK
- x64

---

## Build Pertama

```cmd
cd "c:\Users\Administrator\Downloads\BLOXHUB NEW EXEC\BloxHubExecutorNew"
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

Atau dari root (sudah ada folder `build`):

```cmd
cmake --build build --config Release
```

---

## Output

```
build\bin\Release\
├── BloxHub.exe
├── BloxHubInjector.exe
├── BloxHubInternal.dll
└── version.dll
```

**Untuk inject:** butuh `BloxHub.exe` + `BloxHubInternal.dll` di folder yang sama.  
**Untuk sideload:** butuh `BloxHub.exe` + `version.dll`.

---

## Rebuild Setelah Ubah Kode

```cmd
cmake --build build --config Release
```

Rebuild satu target:

```cmd
cmake --build build --config Release --target BloxHub
```

---

## Update Offset (Setelah Roblox Patch)

Offset game ada di `include/offsets.hpp`. Sumber mentah: `offsets/raw/offsets.h`.

```cmd
copy /Y offsets\raw\offsets.h include\offsets.hpp
```

Lalu rebuild. Injector stomp tidak membutuhkan offset tambahan di luar dump game.

Versi aktif saat ini: `version-5cf2272675e145f5` (lihat baris `roblox_version` di header).

---

## Target CMake

| Target | Output |
|--------|--------|
| `BloxHub` | `BloxHub.exe` |
| `BloxHubInjector` | `BloxHubInjector.exe` |
| `BloxHubInternal` | `BloxHubInternal.dll` |
| `proxy_payload` | `version.dll` |

---

## Catatan

- **Build sukses ≠ inject sukses** di Roblox  
- Test runtime butuh Admin + versi Roblox yang cocok  
- Lihat [`STATUS.md`](STATUS.md) untuk interpretasi hasil uji  

---

## Troubleshooting Build

| Masalah | Solusi |
|---------|--------|
| CMake generator error | Hapus `build\CMakeCache.txt`, jalankan `cmake ..` lagi |
| MSVC tidak ketemu | Pakai **Developer Command Prompt for VS 2022** |
| Output tidak ada | Cek `build\bin\Release\`, bukan root project |
