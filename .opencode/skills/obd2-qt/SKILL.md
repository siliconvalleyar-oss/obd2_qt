---
name: obd2-qt
description: OBD-II Scanner Profesional v12 — Qt5 C++17 ELM327 automotive diagnostic app with Bluetooth RFCOMM, OBD-II PID reading, GM mode 22 commands, Qt Charts graphing, DTC diagnostic, session logging, and demo mode.
license: MIT
metadata:
  language: C++17
  framework: Qt5
  build: qmake
---

## Overview

Qt5 desktop automotive diagnostic scanner. Connects to ELM327 Bluetooth adapters, reads OBD-II PIDs, displays live gauges and charts, reads/clears DTCs, supports GM-specific mode 22 commands, and includes a fully scripted demo mode.

## Build & Run

```bash
qmake obd2_v12.pro
make clean && make -j$(nproc)
./bin/obd2_v12
```

## Architecture

```
MainWindow (QMainWindow)
 ├── ELM327        — Bluetooth RFCOMM + OBD-II PID I/O
 ├── GMCommands    — Mode 22 (UDS) with fallback
 ├── Logger        — CSV file logger
 ├── SessionLogger — Pipe-delimited session log
 ├── DashboardWidget — Live sensor bars (8 gauges)
 ├── SensorPanel   — Qt Charts real-time graphs
 ├── DtcPanel      — DTC read/clear, freeze frame, monitors
 ├── HistoryPanel  — CSV/HTML export
 ├── ConnectionPanel — MAC, protocol, version status
 └── ScanLogPanel  — Real-time log viewer
```

## Key Files

| File | Purpose |
|---|---|
| `headers/elm327.hpp` | ELM327 interface (~375 lines) |
| `sources/elm327.cpp` | ELM327 implementation (~2485 lines) |
| `headers/gm_commands.hpp` | GM mode 22 interface |
| `sources/gm_commands.cpp` | GM mode 22 + fallback chain |
| `headers/mainwindow.hpp` | Main window + tabs + toolbar |
| `sources/mainwindow.cpp` | UI logic, demo mode, settings |
| `headers/app_icons.hpp` | Runtime QPainter vector icons |
| `obd2_v12.pro` | qmake project file |

## Connection Flow

```
fullCleanup() → socket/COM → ATZ(2000ms) → ATE0, ATH0, ATS0, ATL0
  → flush(\r×3) → ATSP{n} (cached) → ATAT1, ATST10, ATAL1
  → ATCF/ATCM (if protocol 6-9) → detectAndCacheProtocol()
```

## STOPPED Recovery

```
STOPPED detected → penalty += 100ms (max 1000ms)
  → wake(\r) → ATD → (ATZ fallback if ATD fails)
  → flush ×5 → re-apply AT config → retry command
  → penalty decays -25ms after 10 consecutive OK
```

## PID Parsing

```
send("010C", 300ms) → splitResponse() strips \r\n>
  → find() OBD headers ("41", "43", "47", "49", "4A", "62")
  → split into hex bytes → apply SAE J1979 formula
```

## Coding Conventions

- camelCase methods, `m_` member prefix, PascalCase classes
- Error: return `-1`, `"No disponible"`, or empty string; `catch(...)` in parsers
- Debug log: `std::cout << "[monitor] ..."`
- Icons: QPainter vector at runtime (no external assets)
- Dark theme: `#1E2230` bg, `#DCE1F0` text, `#60CCFF` accent

## Common Tasks

### Add a new PID getter
1. Add `getFoo()` declaration in `headers/elm327.hpp` (~line 200)
2. Implement in `sources/elm327.cpp` after line 815: `send("01XX")` → `splitResponse()` → formula
3. Wire into `getDashboardFast()` and `SensorPanel` if needed

### Fix connectivity issue
1. Check `connectBT()` AT command sequence
2. Verify `splitResponse()` handles the response format
3. Check CAN filter (`ATCF`/`ATCM`) only applied for protocols 6-9
4. Add more delay between commands if ELM327 enters STOPPED

### Modify UI panel
1. Edit header in `headers/<panel>.hpp` and source in `sources/<panel>.cpp`
2. UI is built with Qt layouts (QVBoxLayout, QGridLayout, QGroupBox)
3. Styling via Qt style sheets inline
