# TODO

## High Priority

- [ ] Test connectivity with real ELM327 hardware
- [ ] Replace blocking `write()` return-value warnings (`[[nodiscard]]`)
- [ ] Fix member initialization order warning in `mainwindow.hpp` (m_demoCheckEngine reorder)

## Medium Priority

- [ ] Implement multi-PID aggregate requests (mode 01 with multiple PIDs) to reduce command count
- [ ] Add threaded I/O to prevent UI freeze during auto-scan and VIN read
- [ ] Add `-h`/`--help` CLI flag
- [ ] Add `--demo` CLI flag to start directly in demo mode
- [ ] Detect Bluetooth adapter presence before attempting connection
- [ ] Add reconnection logic when Bluetooth disconnects unexpectedly
- [ ] Support for CAN FD (ISO 15765-2) / ISO 13400 (DoIP)

## Low Priority

- [ ] Add unit tests for `splitResponse()`, PID decoders, DTC parsing
- [ ] Internationalization (i18n) / translation files
- [ ] Dark/light theme toggle
- [ ] Package as AppImage / Flatpak
- [ ] Windows MSI installer
- [ ] GUI for PID selection (currently hardcoded)
- [ ] Mobile-responsive layout (Qt Quick?)
- [ ] Support for Bluetooth LE (BLE) ELM327 adapters
- [ ] Add checksum/CRC verification for CAN responses
- [ ] Live graphing for fuel trims and O2 sensors
- [ ] Add `protocol.cache` to `.gitignore`

## Known Issues

- `sources/sensorpanel.cpp:182` — unused parameter `color` in lambda
- `sendRaw()` and `send()` share ~80% identical select()/ReadFile code
- Demo mode DTC database duplicated in `dtcpanel.cpp` static table and `mainwindow.cpp` inline data
