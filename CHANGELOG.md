# Changelog

## v12.1 (2026-06-18)

### Fixed
- **connectBT()**: Increased ATZ timeout 1000ms→2000ms, added `ATH0`, flush buffer after ATZ
- **CAN filters**: `ATCF`/`ATCM` now conditional on protocol 6-9; avoids `?` on non-CAN
- **Dashboard stability**: Increased inter-PID delays 50ms→100ms, each PID timeout 200ms→300ms
- **STOPPED recovery**: Added `ATH0`, `ATAL1`, and conditional CAN filters to `recoverFromStopped()` and `ensureNormalConfig()`
- **Demo mode auto-start**: Removed persistence of `demo/enabled` in `QSettings`; app always starts in real mode
- **Regex warning**: Fixed unescaped `.` in `historypanel.cpp` regex (`\.html` → `\\.html`)

### Removed
- Dead `readRaw()` method from `elm327.cpp`/`elm327.hpp`
- Dead `parseResponse()` declaration (no implementation)
- Dead `lookupDTC()` function from `dtcpanel.cpp`
- Commented-out `isConnected()` in `elm327.cpp`
- Empty conditional block in `mainwindow.cpp:744-747`
- Backup file `sources/elm327.cpp.bak`

### Changed
- `obd2_v12.pro`: Added `headers/app_icons.hpp` to `HEADERS`, commented out empty `RESOURCES`

## v12 (2024)

### Added
- Application renamed to "OBD-II Scanner Profesional v12"
- Full dark Fusion theme with monospace font
- Dashboard live progress bars for 8 sensors
- Real-time Qt Charts (RPM, speed, coolant, load)
- DTC reading/clearing (mode 03/04/07/0A)
- Fuel trims and O2 sensor display
- GM-specific mode 22 commands
- Auto-scan of 9 CAN modules
- Session logging with CSV/HTML export
- Demo mode with 9 scripted fault scenarios

## v10 (2024)

### Added
- Initial ELM327 Bluetooth connection
- Basic OBD-II PID reading (RPM, speed, coolant)
- OBD2 QT app scanner foundation

## v1 (2023)

### Added
- Initial project scaffold
- ELM327 interface prototype
