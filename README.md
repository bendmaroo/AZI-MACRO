# Blade Ball Clicker

A precision auto-clicker / auto-key-presser with a real-time measured CPS/KPS
HUD overlay, arbitrary key/mouse-button binding, and 5 GUI themes.

Built in **C++** with **Win32 + DirectX11 + Dear ImGui**. This combo was chosen
because at 500 CPS your click interval is 2ms — that's tight enough that
interpreter overhead (Python) or GC pauses (C#/Java) show up as visible jitter.
C++ + `QueryPerformanceCounter` + `SendInput` + Raw Input is what keeps timing
tight and lets you bind literally any mouse button (including X1/X2 side
buttons), not just standard keys.

## Features implemented

- **Real, measured CPS/KPS**, not just the configured target — a rolling
  1-second window counts actual emitted events (`RateCounter.h`), shown live
  in both the main window and the HUD.
- **Hard caps**: 500 CPS / 300 KPS, enforced every tick even if you drag the
  slider — see `kMaxCPS` / `kMaxKPS` in `AutoClicker.h`.
- **Any-input binding**: keyboard keys AND mouse buttons (Left/Right/Middle/
  X1/X2) via Raw Input (`HotkeyManager.cpp`) — click "Rebind" and press
  literally anything.
- **Hold-to-run or toggle** activation modes.
- **HUD overlay**: a separate, borderless, click-through, always-on-top
  window (`Overlay.cpp`) that shows the live rate over your game without
  intercepting your mouse.
- **5 themes** (Neon Violet, Blade Crimson, Cyber Teal, Solar Amber, Minimal
  Slate) — `Themes.cpp`, easy to add more.
- **Settings persistence** to `%APPDATA%\BladeBallClicker\config.json`.
- **Optional humanized jitter** — adds small random timing variance instead
  of perfectly robotic intervals, if you want that.

## Build instructions (Windows only)

You'll need Visual Studio 2022 (Desktop C++ workload) and vcpkg.

```powershell
git clone https://github.com/microsoft/vcpkg
.\vcpkg\bootstrap-vcpkg.bat

# from the BladeBallClicker project folder:
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
```

The exe will land in `build/Release/BladeBallClicker.exe`.

## Project layout

```
src/
  main.cpp            - app window, ImGui UI, wires everything together
  AutoClicker.h/.cpp   - the timing engine (QueryPerformanceCounter loop)
  HotkeyManager.h/.cpp - Raw Input capture for any key/mouse button
  InputSimulator.h/.cpp- SendInput wrappers for clicks/keypresses
  Overlay.h/.cpp       - transparent click-through HUD window
  Themes.h/.cpp        - color theme definitions
  Config.h/.cpp        - JSON settings persistence
  RateCounter.h        - rolling-window real rate measurement
```

## Notes / things worth tuning yourself

- **I could not compile or test this on real Windows hardware** (built this
  in a Linux sandbox) — the code follows standard, well-established Win32/
  ImGui patterns, but budget time for the normal first-build debugging pass
  (missing includes, a vcpkg triplet mismatch, etc. are the likely culprits
  if something doesn't compile clean the first time).
- The `PreciseWaitUntil` hybrid sleep-then-spin approach is standard for
  sub-5ms timing accuracy on Windows, but the exact spin threshold (currently
  1.5ms) can be tuned if you see overshoot/undershoot at your specific CPS.
- At very high CPS (400-500) some games/anti-cheat-adjacent systems rate-limit
  or ignore inputs faster than ~15-20ms apart regardless — that's a target-side
  limitation, not something this tool controls.
- You said macros/autoclickers are allowed in Blade Ball but auto-parry isn't
  — worth double-checking the current server rules before running this,
  since community rules can change and this tool doesn't read game state at
  all (it has no way to auto-react to anything, it only fires at your
  configured rate while your bound trigger is held/toggled).
