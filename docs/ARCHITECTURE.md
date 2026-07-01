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
   pe_patcher +         stomp_inject.cpp
   version.dll           tp_execute.cpp
          │                 │
          ▼                 ▼
   Folder Roblox         BloxHubInternal.dll
   (dxgi.dll proxy)      (stomp d3d10warp region)
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

### `BloxHubInternal.dll` (`src/internal/dllmain.cpp`)

Payload module stomp:

- `DllMain` — dipanggil injector (console debug Fase 1)  
- `WriteLog` / marker — sandbox temp Roblox (verify luar belum akurat)  
- `offsets.hpp` — siap fase EXECUTE  

### `stomp_inject.cpp` + `tp_execute.cpp`

Digunakan oleh `BloxHub.exe --inject` dan `BloxHubInjector.exe`:

1. `OpenProcess`  
2. `NtMapViewOfSection` — map `d3d10warp.dll` ke proses Roblox  
3. Stomp: reloc + imports + write image ke stomp base  
4. SEH (`RtlAddFunctionTable`) + TLS callbacks via remote shellcode  
5. `DllMain` via `ZwSetIoCompletion` (fallback `CreateRemoteThread`)  

### `offsets.hpp` (`include/offsets.hpp`)

- Offset game dari roblox-dumper (`offsets/raw/`)  
- Copy langsung — tidak perlu blok tambahan untuk injector  

---

## Alur Inject (Jalur Aktif)

```
BloxHub.exe --inject
  → CreateProcess(RobloxPlayerBeta.exe)
  → WaitForRobloxGameProcess (RobloxPlayerBeta.dll loaded)
  → Map d3d10warp.dll → stomp BloxHubInternal.dll
  → DllMain via IoCompletion
  → Console [BloxHub] DllMain PROCESS_ATTACH (target Fase 1)
```

---

## File Penting

```
src/
├── BloxHub.cpp
├── BloxHubInjector.cpp
├── injector/
│   ├── stomp_inject.cpp
│   ├── tp_execute.cpp
│   └── nt_util.hpp
└── internal/
    ├── dllmain.cpp          # BloxHubInternal
    ├── proxy_payload.cpp    # version.dll
    └── pe_patcher.cpp
include/
├── injector.hpp
└── offsets.hpp
```

---

## Referensi

Injector stomp diadaptasi dari [`example projects/Roblox-Injector-main`](../example%20projects/Roblox-Injector-main) (user-mode, tanpa driver).
