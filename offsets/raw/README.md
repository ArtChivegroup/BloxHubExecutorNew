# Raw offsets

Output from [roblox-dumper](https://github.com/nopjo/roblox-dumper) for the target Roblox version.

| File | Purpose |
|------|---------|
| `offsets.h` | Copy into `include/offsets.hpp` |
| `offsets.json` | Reference / tooling |
| `offsets.py`, `offsets.cs` | Reference for other languages |

After Roblox updates, replace these files and run:

```cmd
copy /Y offsets\raw\offsets.h include\offsets.hpp
cmake --build build --config Release
```

Injector **tidak** butuh offset Hyperion. Raw dump dipakai untuk **game API** (DataModel, TaskScheduler, dll) di fase EXECUTE.

Active version is declared in `offsets::roblox_version` inside the header.
