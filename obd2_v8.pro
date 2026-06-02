QT       += core gui widgets charts
CONFIG   += c++17
TEMPLATE  = app
TARGET    = obd2_v8

# Directorios de includes
INCLUDEPATH += . headers

# Sources
SOURCES += \
    main.cpp \
    sources/mainwindow.cpp \
    sources/connectionpanel.cpp \
    sources/sensorpanel.cpp \
    sources/scanlogpanel.cpp \
    sources/dashboardwidget.cpp \
    sources/dtcpanel.cpp \
    sources/historypanel.cpp \
    sources/elm327.cpp \
    sources/gm_commands.cpp \
    sources/logger.cpp

# Headers
HEADERS += \
    headers/mainwindow.hpp \
    headers/connectionpanel.hpp \
    headers/sensorpanel.hpp \
    headers/scanlogpanel.hpp \
    headers/dashboardwidget.hpp \
    headers/dtcpanel.hpp \
    headers/historypanel.hpp \
    headers/elm327.hpp \
    headers/gm_commands.hpp \
    headers/logger.hpp

# Resources
RESOURCES +=

# Release optimizations
QMAKE_CXXFLAGS_RELEASE += -O2
QMAKE_LFLAGS_RELEASE += -s

# Incluir Bluetooth
LIBS += -lbluetooth

# Destino del binario
DESTDIR = bin
OBJECTS_DIR = obj
MOC_DIR = moc
UI_DIR = ui
