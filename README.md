# 🚗 OBD-II Scanner Profesional v8 — Qt5 (qmake)

Escáner de diagnóstico automotriz OBD-II con **interfaz gráfica Qt5** y tema oscuro Fusion. Construido con **qmake** y utiliza **Qt Charts** para visualización de datos en tiempo real.

## ✨ Características

### 📡 Diagnóstico del vehículo
- **Panel de conexión**: Configuración de puerto Bluetooth, dirección MAC del ELM327
- **Lectura de sensores**: RPM, velocidad, temperatura motor, carga, MAF, presión, avance, combustible
- **Sensores de oxígeno**: Voltaje y Fuel Trims por banco
- **Códigos DTC**: Leer, borrar, pendientes (Modo 07)
- **Dashboard gráfico**: Indicadores analógicos con QPainter + Qt Charts
- **Historial**: Seguimiento de lecturas anteriores
- **Scan log**: Registro detallado de comandos y respuestas

### 🎨 Interfaz gráfica Qt5
- Tema oscuro Fusion profesional
- QPalette personalizada con colores de alto contraste
- Múltiples paneles intercambiables (Dashboard, Sensores, DTCs, Scan Log)
- Qt Charts para gráficos de líneas en tiempo real
- Fuente monospace para datos de diagnóstico

## 📦 Requisitos

### Script automático de instalación

```bash
cd scripts
chmod +x install_dependencies_obd2_sfml.sh
sudo ./install_dependencies_obd2_sfml.sh
```

### Instalación manual

```bash
# Ubuntu/Debian
sudo apt install qtbase5-dev libqt5charts5-dev libbluetooth-dev build-essential
```

| Paquete | Versión mínima | Propósito |
|---------|---------------|-----------|
| Qt5 Widgets | 5.9 | Interfaz gráfica (widgets, ventanas) |
| Qt5 Charts | 5.9 | Gráficos QChart en tiempo real |
| libbluetooth-dev | 5.x | Conexión Bluetooth con ELM327 |
| g++ (o clang++) | C++17 | Compilador |

## 🔧 Compilación

```bash
# Desde la raíz del proyecto
cd src/obd2_v8

# Generar Makefile con qmake
qmake obd2_v8.pro

# Compilar
make -j$(nproc)

# Ejecutar
./bin/obd2_v8
```

> **Nota**: Si `qmake` no está disponible, instalar `qtbase5-dev` o probar con `qmake-qt5`.

## 🚀 Uso

```bash
# 1. Conectar el adaptador ELM327 por Bluetooth
# 2. Ejecutar el programa
./bin/obd2_v8

# 3. En la ventana principal:
#    - Connection Panel: configurar MAC y conectar
#    - Dashboard: indicadores en tiempo real
#    - Sensors: lectura de parámetros del motor
#    - DTCs: leer y borrar códigos de error
#    - Scan Log: registro de comandos OBD
#    - History: histórico de lecturas
```

## 🏗️ Estructura del proyecto

```
obd2_v8/
├── obd2_v8.pro             # Proyecto Qt5 (qmake)
├── Makefile                 # Makefile generado por qmake
├── README.md                # Este archivo
├── scripts/
│   └── install_dependencies_obd2_sfml.sh  # Script de instalación de dependencias
├── headers/
│   ├── mainwindow.hpp      # Ventana principal con paneles
│   ├── connectionpanel.hpp # Panel de conexión Bluetooth
│   ├── sensorpanel.hpp     # Panel de sensores en vivo
│   ├── scanlogpanel.hpp    # Panel de log de escaneo
│   ├── dashboardwidget.hpp # Widget de dashboard gráfico
│   ├── dtcpanel.hpp        # Panel de códigos de error
│   ├── historypanel.hpp    # Panel de histórico
│   ├── elm327.hpp          # Driver ELM327 (conexión, comandos)
│   ├── gm_commands.hpp     # Comandos específicos GM
│   ├── logger.hpp          # Logger CSV + Session Logger
│   └── app_icons.hpp       # Iconos de la aplicación
├── sources/
│   ├── mainwindow.cpp
│   ├── connectionpanel.cpp
│   ├── sensorpanel.cpp
│   ├── scanlogpanel.cpp
│   ├── dashboardwidget.cpp
│   ├── dtcpanel.cpp
│   ├── historypanel.cpp
│   ├── elm327.cpp
│   ├── gm_commands.cpp
│   └── logger.cpp
├── main.cpp                # Entry point (QApplication + Fusion dark)
├── bin/                    # Binario compilado
├── obj/                    # Objetos de compilación
├── moc/                    # Archivos MOC generados
└── logs/                   # Logs de sesiones
```

## 📝 Licencia

Código abierto — Proyecto Freebuff.
