# Architecture

## Overview

OBD2_QT is a Qt5 C++17 desktop application for automotive diagnostics via ELM327 Bluetooth.  
It follows a monolithic GUI pattern with three logical layers: **UI panels**, **core communication**, and **logging**.

```
┌─────────────────────────────────────────────────────┐
│                     MainWindow                       │
│  (QMainWindow: toolbar, statusbar, QTabWidget)       │
├──────────┬──────────┬──────────┬──────────┬──────────┤
│Dashboard │ Sensor   │ DTC      │ History  │ Conn.    │
│Widget    │ Panel    │ Panel    │ Panel    │ Panel    │
├──────────┴──────────┴──────────┴──────────┴──────────┤
│                ELM327 (core serial I/O)               │
│         GMCommands (mode 22 wrapper)                  │
├──────────┬────────────────────────────────────────────┤
│ Logger   │ SessionLogger                              │
│ (CSV)    │ (pipe-delimited session log)               │
└──────────┴────────────────────────────────────────────┘
```

## Class Hierarchy

```
QMainWindow
 └── MainWindow
      ├── owns ELM327 (std::unique_ptr)
      ├── owns GMCommands (std::unique_ptr)
      ├── owns Logger (std::unique_ptr)
      ├── owns SessionLogger (std::unique_ptr)
      ├── DashboardWidget (embedded QWidget)
      ├── SensorPanel    (embedded QWidget)
      ├── DtcPanel       (embedded QWidget)
      ├── HistoryPanel   (embedded QWidget)
      ├── ConnectionPanel (embedded QWidget)
      ├── ScanLogPanel   (embedded QWidget)
      └── QTimer m_pollTimer (drives periodic data fetch)
```

## Module Responsibilities

### `ELM327`
- Bluetooth RFCOMM socket (Linux) / COM port (Windows)
- AT command negotiation (`ATZ`, `ATE0`, `ATH0`, `ATSP0`, etc.)
- OBD-II PID request/response cycle (modes 01–0A)
- Adaptive timeout + automatic STOPPED state recovery
- Protocol caching to `protocol.cache` file
- CAN filter/mask conditional on protocol (6–9 only)
- Dashboard multiparameter fetch (`getDashboardFast()`: 10 PIDs)
- Auto-scan of 9 CAN modules
- VIN parsing, DTC read/clear

### `GMCommands`
- Mode 22 (UDS) commands with CAN header `7E0`/`7E8`
- Fallback chain: OBD standard PID → Mode 22 PID
- NRC decode per ISO 14229-1
- PID scan (OBD mode 01 ranges + GM mode 22 known PIDs)

### `Logger` / `SessionLogger`
- CSV file per session (`logs/all_YYYY_MM_DD_HHMM.csv`)
- Pipe-delimited session log (`logs/all_YYYY_MM_DD_HHMMSS.log`)
- Automatic `logs/` directory creation

### `MainWindow`
- Owns all components; connects signals/slots
- `m_pollTimer` fires at ~500ms for period data via `onTimerTick()`
- Toolbar actions: Connect, Scan, Demo, Export, Screenshot, Clear
- Status bar: protocol, RPM, speed, check engine light
- Settings persist/restore via `QSettings`
- Demo mode with scripted fault scenarios

### UI Panels
| Panel | Tab | Purpose |
|---|---|---|
| `DashboardWidget` | Dashboard | Gauges: RPM, speed, coolant, load, throttle, MAF, fuel, timing |
| `SensorPanel` | Sensors | Qt Charts real-time graphs for each sensor |
| `DtcPanel` | Diagnostics | DTC display, clear, vehicle info, freeze frame, monitors |
| `HistoryPanel` | History | CSV/HTML export, session replay |
| `ConnectionPanel` | Config | MAC display, protocol info, version, cleanup |
| `ScanLogPanel` | Log | Real-time scan log viewer |

## Data Flow (Scan Cycle)

```
m_pollTimer (500ms)
    │
    ▼
MainWindow::onTimerTick()
    │
    ├── m_tickCounter = 0 → ELM327::getDashboardFast()
    │                            └── send("PID", 300ms) × 10
    │                                   └── splitResponse()
    │                                          └── parse hex bytes
    │
    ├── m_tickCounter = 1 → getRPM(), getSpeed() (cycle repeat)
    │
    └── emit updateDashboard(d) → bars + charts + status bar
```

## Threading

Single-threaded (Qt main event loop).  
All ELM327 serial I/O is **synchronous** within the event loop.  
`send()` uses `select()` with configurable timeout.  
Long operations (auto-scan, VIN read) block the UI momentarily.

## Connection Lifecycle

```
fullCleanup() → socket() → connect() → ATZ(2000ms)
  → ATE0, ATL0, ATS0, ATH0 → flush (\r ×3)
  → loadCachedProtocol() → ATSP{n} or ATSP0
  → ATAT1, ATST10, ATAL1
  → ATCF/ATCM (if protocol 6-9)
  → detectAndCacheProtocol() (if not cached)
```

## Recovery Flow (STOPPED)

```
send() detects "STOPPED" in response
  → increase penalty +100ms (max 1000ms)
  → recoverFromStopped():
       wake (\r) → ATD → (fallback) ATZ
       → flush buffer ×5 → re-apply AT config
  → retry original command with penalty
  → penalty decays -25ms after 10 consecutive successes
```

## Key Design Decisions

- **Protocol caching**: Avoids ~2-3s `ATSP0` re-detection on reconnect
- **CAN filter conditional**: `ATCF`/`ATCM` only for protocols 6-9 (CAN);
  avoids `?` errors on non-CAN vehicles
- **Adaptive timeout**: Penalty-based system prevents cascading STOPPED
- **`splitResponse()` location-independent**: Uses `find()` not fixed offset
  because CAN headers (ATH1) shift data position
- **Demo mode**: Self-contained simulation with scripted phases (Normal,
  Accel, SensorFault, Overheating, DTCTrigger, etc.)

## File Layout

```
obd2_v12.pro         qmake project
main.cpp             Entry point + dark Fusion theme
headers/             Header files (13 .hpp)
sources/             Implementation files (10 .cpp)
bin/                 Compiled binary
logs/                Session logs (auto-created)
screenshots/         Application screenshots
```
