# BloxHub Executor

Windows x64 loader research project for Roblox (Bloxstrap). Primary path: **module stomp inject** via `BloxHubInjector.exe`. Sideload via `dxgi.dll` proxy is blocked by Hyperion on current Roblox builds.

**Start here:** [`docs/STATUS.md`](docs/STATUS.md)

---

## Quick start

```cmd
cmake -S . -B build
cmake --build build --config Release
cd build\bin\Release

REM 1. Open Roblox, join a game
REM 2. Run as Administrator:
BloxHubInjector.exe
```

Roblox version must match `include/offsets.hpp` (`offsets::roblox_version`).

Expected: `DllMain returned`, `Injection OK — Roblox masih hidup`, no crash.

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
| `BloxHubInjector` | `BloxHubInjector.exe` | **Manual inject** (recommended test path) |
| `BloxHubInternal` | `BloxHubInternal.dll` | Stomp payload |
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
| Module stomp inject | ✅ stable (2× no crash, `DllMain returned`) |
| Console proof (Step 1) | 🔄 not confirmed on screen |
| `BloxHubInjector.exe` manual | ✅ recommended workflow |
| `BloxHub.exe --inject` | ⏸ use manual inject first |
| Sideload `dxgi.dll` | ❌ blocked by Hyperion |
| Offsets | `version-5cf2272675e145f5` |

Details: [`docs/STATUS.md`](docs/STATUS.md)

---

## Update offsets after Roblox patch

```cmd
copy /Y offsets\raw\offsets.h include\offsets.hpp
cmake --build build --config Release
```

See [`docs/BUILD.md`](docs/BUILD.md).
