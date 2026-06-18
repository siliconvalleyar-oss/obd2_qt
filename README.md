# OBD-II Scanner Profesional v12

Aplicación gráfica Qt5 para diagnóstico automotriz vía ELM327 Bluetooth.

## Características

- Conexión Bluetooth RFCOMM (Linux) / Serial COM (Windows) con ELM327
- Dashboard en vivo con barras de progreso animadas
- Gráficos en tiempo real con Qt Charts (RPM, velocidad, temperatura, carga)
- Lectura y limpieza de códigos DTC (Modo 03/04/07/0A)
- Información del vehículo: VIN, ECU, calibración, protocolo
- Fuel Trims (STFT/LTFT) y sensores de oxígeno
- Comandos GM específicos (Modo 22 UDS)
- Auto-Scan de múltiples módulos CAN
- Grabación de historial con exportación CSV/HTML
- Modo Demo con escenarios de fallas simuladas
- Logging de sesión y exportación de reportes

## Requisitos

### Linux
- Qt 5.15+ (core, gui, widgets, charts)
- libbluetooth-dev (BlueZ)
- Compilador C++17 (g++)

### Windows (MinGW/MSYS2)
- Qt 5.15+ (core, gui, widgets, charts)
- Puerto COM Bluetooth virtual

## Compilación

```bash
qmake obd2_v12.pro
make
./bin/obd2_v12
```

## Uso

1. Ejecute la aplicación
2. Haga clic en "Conectar" e ingrese la dirección MAC del ELM327
3. Una vez conectado, use "Escanear" para monitorear sensores
4. Use la pestaña "Diagnóstico" para leer DTCs e información del vehículo
5. Active "Demo" para probar sin hardware

## Estructura del proyecto

```
├── obd2_v12.pro          # Archivo de proyecto qmake
├── main.cpp               # Punto de entrada
├── headers/               # Archivos de cabecera
│   ├── elm327.hpp         # Comunicación con ELM327
│   ├── mainwindow.hpp     # Ventana principal
│   ├── gm_commands.hpp    # Comandos GM modo 22
│   ├── logger.hpp         # Logging CSV y sesión
│   ├── connectionpanel.hpp
│   ├── sensorpanel.hpp
│   ├── dashboardwidget.hpp
│   ├── dtcpanel.hpp
│   ├── historypanel.hpp
│   ├── scanlogpanel.hpp
│   └── app_icons.hpp      # Iconos vectoriales
├── sources/               # Implementación
│   ├── elm327.cpp
│   ├── mainwindow.cpp
│   ├── gm_commands.cpp
│   ├── logger.cpp
│   ├── connectionpanel.cpp
│   ├── sensorpanel.cpp
│   ├── dashboardwidget.cpp
│   ├── dtcpanel.cpp
│   ├── historypanel.cpp
│   └── scanlogpanel.cpp
├── logs/                  # Logs de sesión
├── bin/                   # Binario compilado
└── README.md
```

## Solución de problemas

- **No conecta**: Verifique Bluetooth encendido, MAC correcta, y que el ELM327 esté encendido
- **Datos intermitentes**: Aumente el intervalo de escaneo en la pestaña Sensores
- **STOPPED frecuente**: El ELM327 entra en STOPPED si se envían comandos muy rápido; el timeout adaptativo se ajusta automáticamente
- **Permisos Linux**: Agregue su usuario al grupo `bluetooth` o ejecute con `sudo`

## Licencia

Freebuff Project
