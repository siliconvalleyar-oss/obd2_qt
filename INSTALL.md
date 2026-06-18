# Installation

## Requirements

### Linux (Debian/Ubuntu)
```
sudo apt install build-essential qtbase5-dev qtcharts5-dev libbluetooth-dev
```

### Linux (Fedora/RHEL)
```
sudo dnf install gcc-c++ qt5-qtbase-devel qt5-qtcharts-devel bluez-libs-devel
```

### Windows (MSYS2)
```
pacman -S mingw-w64-x86_64-qt5 mingw-w64-x86_64-qt5-charts
```
A Bluetooth virtual COM port is required (Windows handles SPP via paired devices).

## Build

```bash
qmake obd2_v12.pro
make -j$(nproc)
```

The binary is placed in `bin/obd2_v12`.

## Run

```bash
./bin/obd2_v12
```

## Permissions (Linux Bluetooth)

Add your user to the `bluetooth` group:
```bash
sudo usermod -aG bluetooth $USER
# Log out and back in, or run:
newgrp bluetooth
```

## First Connection

1. Pair the ELM327 via your system Bluetooth settings
2. Note the MAC address (e.g. `00:1D:A5:07:23:6E`)
3. Launch the app, enter the MAC, click **Connect**
4. Once connected, click **Scan** to start monitoring

## Demo Mode (No Hardware)

Click the **Demo** button in the toolbar to simulate sensor readings and faults without an ELM327 adapter.

## Troubleshooting

| Symptom | Fix |
|---|---|
| `socket: Operation not permitted` | Add user to `bluetooth` group |
| `rfcomm: Cannot open device` | Run `sudo rfcomm release all` |
| `connect: Connection refused` | Verify ELM327 is powered and paired |
| `STOPPED` errors | Increase scan interval in Sensor panel |
| Compile error: `bluetooth/bluetooth.h` not found | Install `libbluetooth-dev` |
