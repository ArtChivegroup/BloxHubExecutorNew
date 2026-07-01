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

┌──────────────────────┐
│  BloxHubInjector.exe │  ← jalur uji manual (disarankan)
│  wait game + inject  │
└──────────────────────┘
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

### `BloxHubInjector.exe` (`src/BloxHubInjector.cpp`)

Injector standalone — **workflow uji disarankan:**

1. Tunggu PID dengan `RobloxPlayerBeta.dll` loaded  
2. Delay 2 detik  
3. Panggil `injector::Inject()`  
4. Cek Roblox masih hidup setelah inject  

Tidak launch Roblox — user buka game dulu.

### `pe_patcher` (`src/internal/pe_patcher.cpp`)

- Baca `version.dll` (payload) + System32 DLL asli  
- Bangun export forward table → `*_orig.dll`  
- Output: proxy DLL (mis. `dxgi.dll`) ditulis ke folder Roblox  

### `BloxHubInternal.dll` (`src/internal/dllmain.cpp`)

Payload module stomp (minimal Fase 1):

- `DllMain` — `AllocConsole` + `WriteConsoleA` (`DebugConsoleLog`)  
- **Tidak ada** export `BloxHubInit`, **tidak ada** `WriteLog` / marker `%TEMP%` (dihapus untuk isolasi crash)  
- `offsets.hpp` — siap fase EXECUTE  

### `stomp_inject.cpp` + `tp_execute.cpp`

Digunakan oleh `BloxHub.exe --inject` dan `BloxHubInjector.exe`:

1. `OpenProcess`  
2. `NtCreateSection` + `NtMapViewOfSection` — map `d3d10warp.dll` ke proses Roblox  
3. Stomp: reloc + imports + write image ke stomp base  
4. **Skip TLS + SEH** — CRT TLS dan `RtlAddFunctionTable` dengan alamat injector-local menyebabkan crash  
5. `DllMain` via **`TpExecuteShellcodeSync`** → `CreateRemoteThread` + `WaitForSingleObject`  

`TpExecuteShellcode` (IoCompletion / `ZwSetIoCompletion`) masih ada untuk shellcode umum; **DllMain tidak memakainya** — async dispatch tidak memberi tahu kalau proses crash.

### `offsets.hpp` (`include/offsets.hpp`)

- Offset game dari roblox-dumper (`offsets/raw/`)  
- Copy langsung — tidak perlu blok tambahan untuk injector  

---

## Alur Inject (Jalur Aktif)

```
Roblox in-game
  → BloxHubInjector.exe (Admin)
  → WaitForRobloxGameProcess (RobloxPlayerBeta.dll loaded)
  → Map d3d10warp.dll → stomp BloxHubInternal.dll (reloc + import only)
  → DllMain via CreateRemoteThread sync
  → [BloxHub] DllMain PROCESS_ATTACH (target Fase 1 — console atau Step 4 file)
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
Perbedaan BloxHub: skip TLS/SEH, DllMain sync thread.
