# BloxHub Executor

Windows x64 loader research project for Roblox (Bloxstrap). Primary path: **manual map inject** (`--inject`). Sideload via `dxgi.dll` proxy is blocked by Hyperion on current Roblox builds.

**Start here:** [`docs/STATUS.md`](docs/STATUS.md)

---

## Quick start

```cmd
cmake -S . -B build
cmake --build build --config Release
cd build\bin\Release

REM Run as Administrator
BloxHub.exe "C:\Users\Administrator\AppData\Local\Bloxstrap\Versions\version-5cf2272675e145f5" --inject
```

Roblox version must match `include/offsets.hpp` (`offsets::roblox_version`).

---

## Repository layout

```text
BloxHubExecutorNew/
├── CMakeLists.txt
├── README.md
├── docs/                 # Documentation
├── include/
│   ├── injector.hpp
│   └── offsets.hpp       # Active game offsets (from offsets/raw)
├── offsets/raw/          # roblox-dumper output
├── src/
│   ├── BloxHub.cpp       # Main loader
│   ├── BloxHubInjector.cpp
│   ├── injector/         # stomp_inject + tp_execute
│   └── internal/         # pe_patcher, payloads
└── checkpoints/          # Session notes (CHECKPOINT_CURRENT.md)
```

---

## Build targets

| Target | Output | Role |
|--------|--------|------|
| `BloxHub` | `BloxHub.exe` | Launcher: sideload or `--inject` |
| `BloxHubInjector` | `BloxHubInjector.exe` | Inject into running Roblox |
| `BloxHubInternal` | `BloxHubInternal.dll` | Manual map payload |
| `proxy_payload` | `version.dll` | Sideload proxy source |

---

## Documentation

| Doc | Purpose |
|-----|---------|
| [`docs/STATUS.md`](docs/STATUS.md) | Current project state |
| [`docs/TODO.md`](docs/TODO.md) | Step-by-step checklist (one step per session) |
| [`docs/USAGE.md`](docs/USAGE.md) | Commands & troubleshooting |
| [`docs/BUILD.md`](docs/BUILD.md) | Build & offset updates |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Components & flows |
| [`docs/BUGS.md`](docs/BUGS.md) | Known issues |
| [`docs/PLANNING.md`](docs/PLANNING.md) | Next priorities |

---

## Project status (summary)

| Item | Status |
|------|--------|
| `--inject` infrastructure | Module stomp + IoCompletion |
| Payload verification | Pending Step 1 test (console) |
| Sideload `dxgi.dll` | Blocked by Hyperion |
| Offsets | `version-5cf2272675e145f5` |

Details: [`docs/STATUS.md`](docs/STATUS.md)

---

## Update offsets after Roblox patch

```cmd
copy /Y offsets\raw\offsets.h include\offsets.hpp
cmake --build build --config Release
```

See [`docs/BUILD.md`](docs/BUILD.md).
