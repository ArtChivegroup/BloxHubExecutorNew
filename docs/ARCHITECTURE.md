# Arsitektur

**Konteks terkini:** [`STATUS.md`](STATUS.md)

---

## Gambaran Umum

```
┌─────────────────────────────────────────────────────────────┐
│                      BloxHub.exe                            │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐  │
│  │   Sideload   │  │   --inject   │  │  Session/Restore │  │
│  │    mode      │  │     mode     │  │   (sideload)     │  │
│  └──────┬───────┘  └──────┬───────┘  └──────────────────┘  │
└─────────┼─────────────────┼─────────────────────────────────┘
          │                 │
          ▼                 ▼
   pe_patcher +         manual_map.cpp
   version.dll           cfg_bypass.cpp
          │                 │
          ▼                 ▼
   Folder Roblox         BloxHubInternal.dll
   (dxgi.dll proxy)      (di memori Roblox)
          │                 │
          └────────┬────────┘
                   ▼
         RobloxPlayerBeta.exe + RobloxPlayerBeta.dll
                   │
                   ▼
              Hyperion (blokir sideload, tantang inject)
```

---

## Komponen

### `BloxHub.exe` (`src/BloxHub.cpp`)

Launcher dengan dua mode:

| Mode | Flag | Payload | Modifikasi disk Roblox |
|------|------|---------|------------------------|
| Sideload | (default) | `version.dll` → proxy `dxgi.dll` | Ya |
| Inject | `--inject` | `BloxHubInternal.dll` | Tidak |

State machine sideload: `PREFLIGHT → INSTALL → LAUNCH → VERIFY → RESTORE`

### `pe_patcher` (`src/internal/pe_patcher.cpp`)

- Baca `version.dll` (payload) + System32 DLL asli  
- Bangun export forward table → `*_orig.dll`  
- Output: proxy DLL (mis. `dxgi.dll`) ditulis ke folder Roblox  

### `proxy_payload` → `version.dll` (`src/internal/proxy_payload.cpp`)

Kode yang di-patch ke dalam proxy sideload:

- `DllMain` → log, marker verify, load forward DLL di thread terpisah  
- Digunakan hanya jika sideload berhasil (saat ini: tidak di Roblox)

### `BloxHubInternal.dll` (`src/internal/dllmain.cpp`)

Payload manual map:

- Export `BloxHubInit()` — dipanggil injector  
- `DllMain` — tidak dipanggil oleh manual map saat ini  
- `WriteLog` → `bloxhub_test.txt` via `GetTempPath()` **dari dalam proses Roblox**

### `manual_map.cpp` + `cfg_bypass.cpp`

Digunakan oleh `BloxHub.exe --inject` dan `BloxHubInjector.exe`:

1. `OpenProcess`  
2. `VirtualAllocEx` + reloc + resolve imports  
3. `WriteProcessMemory`  
4. CFG bitmap scan/patch  
5. `CreateRemoteThread(BloxHubInit)`  

### `offsets.hpp` (`include/offsets.hpp`)

- Offset game dari roblox-dumper (`offsets/raw/`)  
- `CfgBypass::BitmapPtr` — manual, terpisah dari dump  

---

## Alur Sideload (Tidak Jalan di Hyperion)

```
BloxHub.exe
  → copy System32\dxgi.dll → dxgi_orig.dll
  → ConvertPayloadToProxy(version.dll) → dxgi.dll
  → CreateProcess(RobloxPlayerBeta.exe)
  → Roblox coba load dxgi.dll
  → Hyperion tolak ❌
  → DllMain tidak pernah jalan
  → verify timeout
```

---

## Alur Inject (Jalur Aktif)

```
BloxHub.exe --inject
  → CreateProcess(RobloxPlayerBeta.exe)
  → WaitForRobloxGameProcess (RobloxPlayerBeta.dll loaded)
  → manual_map(BloxHubInternal.dll)
  → CFG bypass (auto-scan bitmap)
  → CreateRemoteThread(BloxHubInit)
  → ??? WriteLog (belum terbukti di %TEMP% user)
  → verify timeout ⚠️
```

---

## File Penting

```
src/
├── BloxHub.cpp
├── BloxHubInjector.cpp
├── injector/
│   ├── manual_map.cpp
│   └── cfg_bypass.cpp
└── internal/
    ├── pe_patcher.cpp
    ├── proxy_payload.cpp    # → version.dll
    └── dllmain.cpp          # → BloxHubInternal.dll

include/
├── offsets.hpp
└── injector.hpp

offsets/raw/
└── offsets.h                # Dump mentah → copy ke include/offsets.hpp
```

## Keputusan Arsitektur yang Sudah Diambil

1. **Sideload bukan jalur utama** untuk Roblox + Hyperion  
2. **Inject terintegrasi** di `BloxHub.exe` via `--inject`  
3. **Verify berbasis file** — desainnya rapuh (path sandbox)  
4. **CFG auto-scan** — sementara, perlu RVA manual  

Detail bug: [`BUGS.md`](BUGS.md)
