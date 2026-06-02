#include "headers/mainwindow.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QFileDialog>
#include <QToolBar>
#include <QSettings>
#include <QCloseEvent>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QPixmap>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <ctime>

#include "headers/app_icons.hpp"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), m_isScanning(false), m_isConnected(false),
      m_demoMode(false), m_demoCounter(0)    , m_demoPhase(DemoPhase::Normal), m_demoPhaseCounter(0),
      m_demoPhaseDuration(200), m_demoCheckEngine(false),
      m_sessMinRPM(99999), m_sessMaxRPM(0), m_sessTotalRPM(0),
      m_sessCountRPM(0), m_sessMaxSpeed(0) {

    // Seed random generator for demo variety
    srand(static_cast<unsigned>(time(nullptr)));

    // Init core components
    m_logger = std::make_unique<Logger>();
    m_sessionLog = std::make_unique<SessionLogger>();
    m_logger->open();
    m_sessionLog->open();

    createWidgets();
    createToolBar();
    createStatusBar();
    connectSignals();

    appendLog("🚗 Aplicación OBD-II iniciada", QColor(100, 200, 255));
    appendLog("Conecte el adaptador ELM327 vía Bluetooth para comenzar", QColor(180, 190, 210));
    appendLog("Seleccione '🎮 Demo' en la barra para probar sin hardware", QColor(140, 145, 160));

    // Restore saved settings
    loadSettings();
}

MainWindow::~MainWindow() {
    saveSettings();
    if (m_elm && m_elm->isConnected()) {
        m_elm->disconnect();
    }
}

void MainWindow::createWidgets() {
    // ELM327 not created yet - will be created on connect
    m_elm = nullptr;
    m_gm = nullptr;

    m_tabs = new QTabWidget();
    m_tabs->setTabPosition(QTabWidget::North);
    m_tabs->setStyleSheet(
        "QTabWidget::pane { background-color: #1A1D2A; border: none; }"
        "QTabBar::tab { background-color: #2A2E3E; color: #B0B8D0; "
        "  padding: 10px 24px; margin: 1px; border-top-left-radius: 6px; border-top-right-radius: 6px; }"
        "QTabBar::tab:selected { background-color: #3C4158; color: #DCE1F0; font-weight: bold; }"
        "QTabBar::tab:hover { background-color: #353A50; }"
    );

    // Tab 1: Dashboard
    m_dashboard = new DashboardWidget(nullptr, this);
    m_tabs->addTab(m_dashboard, "📊 Dashboard");

    // Tab 2: Sensors (real-time graphs)
    m_sensorPanel = new SensorPanel(nullptr, this);
    m_tabs->addTab(m_sensorPanel, "📈 Sensores");

    // Tab 3: Scan log
    m_scanLogPanel = new ScanLogPanel(this);
    m_tabs->addTab(m_scanLogPanel, "📋 Log");

    // Tab 4: Connection
    m_connectionPanel = new ConnectionPanel(nullptr, this);
    m_tabs->addTab(m_connectionPanel, "🔌 Conexión");

    // Tab 5: Diagnostics (DTCs + Vehicle Info)
    m_dtcPanel = new DtcPanel(nullptr, this);
    m_tabs->addTab(m_dtcPanel, "🔧 Diagnóstico");

    // Tab 6: History & Recording
    m_historyPanel = new HistoryPanel(this);
    m_tabs->addTab(m_historyPanel, "📜 Historial");

    setCentralWidget(m_tabs);
}

void MainWindow::createToolBar() {
    QToolBar* toolbar = addToolBar("Principal");
    toolbar->setStyleSheet(
        "QToolBar { background-color: #1E2230; border: none; border-bottom: 1px solid #2A2E3E; spacing: 8px; padding: 4px; }"
        "QToolButton { color: #DCE1F0; font-weight: bold; padding: 6px 14px; "
        "  border: none; border-radius: 4px; margin: 2px; }"
        "QToolButton:hover { background-color: #353A50; }"
        "QToolButton:disabled { color: #6A6F80; }"
    );
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));

    m_actConnect = toolbar->addAction(AppIcons::iconConnect(), " Conectar");
    m_actConnect->setToolTip("Conectar al ELM327 vía Bluetooth");

    m_actDisconnect = toolbar->addAction(AppIcons::iconDisconnect(), " Desconectar");
    m_actDisconnect->setToolTip("Desconectar ELM327");
    m_actDisconnect->setEnabled(false);

    toolbar->addSeparator();

    m_actStartScan = toolbar->addAction(AppIcons::iconScan(), " Escanear");
    m_actStartScan->setToolTip("Iniciar escaneo de sensores");
    m_actStartScan->setEnabled(false);

    m_actStopScan = toolbar->addAction(AppIcons::iconStop(), " Detener");
    m_actStopScan->setToolTip("Detener escaneo");
    m_actStopScan->setEnabled(false);

    toolbar->addSeparator();

    m_actClearLog = toolbar->addAction(AppIcons::iconClear(), " Limpiar Log");
    m_actClearLog->setToolTip("Limpiar el log de escaneo");

    m_actExport = toolbar->addAction(AppIcons::iconExport(), " Exportar CSV");
    m_actExport->setToolTip("Exportar datos a CSV");
    m_actExport->setEnabled(false);

    toolbar->addSeparator();

    m_actScreenshot = toolbar->addAction("📷");
    m_actScreenshot->setText(" Captura");
    m_actScreenshot->setToolTip("Guardar captura de pantalla");

    m_actReport = toolbar->addAction("📋");
    m_actReport->setText(" Reporte");
    m_actReport->setToolTip("Exportar reporte diagnóstico HTML");
    m_actReport->setEnabled(false);

    toolbar->addSeparator();

    m_actResetSettings = toolbar->addAction("⚙");
    m_actResetSettings->setText(" Reset");
    m_actResetSettings->setToolTip("Restablecer configuración por defecto");

    toolbar->addSeparator();

    m_actDemo = toolbar->addAction(AppIcons::iconDemo(false), " Demo");
    m_actDemo->setToolTip("Activar modo demo con datos simulados (sin ELM327)");
    m_actDemo->setCheckable(true);
}

void MainWindow::createStatusBar() {
    QStatusBar* sb = statusBar();
    sb->setStyleSheet(
        "QStatusBar { background-color: #181A28; border-top: 1px solid #2A2E3E; color: #8A8FA0; padding: 2px 8px; }"
        "QStatusBar::item { border: none; }"
    );

    m_lblStatus = new QLabel("⏹ Desconectado");
    m_lblStatus->setStyleSheet("color: #FF6B6B; font-weight: bold; padding: 2px 10px;");
    sb->addWidget(m_lblStatus);

    m_lblProtocol = new QLabel("");
    m_lblProtocol->setStyleSheet("color: #6A6F80; padding: 2px 10px;");
    sb->addWidget(m_lblProtocol);

    m_lblRPM = new QLabel("RPM: ---");
    m_lblRPM->setStyleSheet("color: #60CCFF; padding: 2px 10px;");
    sb->addPermanentWidget(m_lblRPM);

    m_lblSpeed = new QLabel("SPD: ---");
    m_lblSpeed->setStyleSheet("color: #90EE90; padding: 2px 10px;");
    sb->addPermanentWidget(m_lblSpeed);

    m_lblMaxRPM = new QLabel("RPM máx: ---");
    m_lblMaxRPM->setStyleSheet("color: #FFB347; padding: 2px 10px;");
    sb->addPermanentWidget(m_lblMaxRPM);

    // Poll timer for status bar updates
    m_lblCheckEngine = new QLabel("");
    m_lblCheckEngine->setStyleSheet("padding: 2px 10px;");
    sb->addPermanentWidget(m_lblCheckEngine);

    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(1500);
}

void MainWindow::connectSignals() {
    // Toolbar actions
    connect(m_actConnect, &QAction::triggered, this, &MainWindow::onConnect);
    connect(m_actDisconnect, &QAction::triggered, this, &MainWindow::onDisconnect);
    connect(m_actStartScan, &QAction::triggered, this, &MainWindow::onStartScan);
    connect(m_actStopScan, &QAction::triggered, this, &MainWindow::onStopScan);
    connect(m_actClearLog, &QAction::triggered, this, &MainWindow::onClearLog);
    connect(m_actExport, &QAction::triggered, this, &MainWindow::onExportCSV);
    connect(m_actScreenshot, &QAction::triggered, this, &MainWindow::onScreenshot);
    connect(m_actReport, &QAction::triggered, this, &MainWindow::onExportReport);
    connect(m_actResetSettings, &QAction::triggered, this, &MainWindow::onResetSettings);
    connect(m_actDemo, &QAction::triggered, this, &MainWindow::onDemo);

    // Poll timer
    connect(m_pollTimer, &QTimer::timeout, this, &MainWindow::onTimerTick);

    // Connection panel
    connect(m_connectionPanel, &ConnectionPanel::connectionStatusChanged,
            this, &MainWindow::onConnectionStatusChanged);
}

void MainWindow::onConnect() {
    // If demo mode is active, turn it off first
    if (m_demoMode) {
        m_actDemo->setChecked(false);
        onDemo();
    }

    // Use saved MAC as default
    QSettings settings("Freebuff", "OBD2-Scanner");
    QString defaultMac = settings.value("connection/mac", "00:1D:A5:07:23:6E").toString();

    QString mac = QInputDialog::getText(this, "Conexión ELM327",
                                         "Dirección MAC del ELM327:",
                                         QLineEdit::Normal,
                                         defaultMac);
    if (mac.isEmpty()) return;

    appendLog("🔗 Conectando a " + mac + "...", QColor(255, 180, 40));

    m_elm = std::make_unique<ELM327>(mac.toStdString(), 1);

    if (m_elm->connectBT()) {
        m_gm = std::make_unique<GMCommands>(m_elm.get());
        m_isConnected = true;

        m_lblStatus->setText("✅ Conectado");
        m_lblStatus->setStyleSheet("color: #6BFF6B; font-weight: bold; padding: 2px 10px;");
        m_lblProtocol->setText("Protocolo: " + QString::fromStdString(m_elm->getProtocol()));

        // Reset session statistics
        m_sessMinRPM = 99999;
        m_sessMaxRPM = 0;
        m_sessTotalRPM = 0;
        m_sessCountRPM = 0;
        m_sessMaxSpeed = 0;

        m_actConnect->setEnabled(false);
        m_actDisconnect->setEnabled(true);
        m_actStartScan->setEnabled(true);
        m_actExport->setEnabled(true);
        m_actReport->setEnabled(true);

        m_dashboard->setELM(m_elm.get());
        m_sensorPanel->setELM(m_elm.get());
        m_connectionPanel->setELM(m_elm.get());
        m_dtcPanel->setELM(m_elm.get());

        m_pollTimer->start();

        // Save MAC to settings
        {
            QSettings s("Freebuff", "OBD2-Scanner");
            s.setValue("connection/mac", mac);
        }
        m_connectionPanel->setMAC(mac);

        appendLog("✅ Conectado exitosamente a " + mac, QColor(100, 255, 100));
        appendLog(QString("📊 Protocolo: ") + QString::fromStdString(m_elm->getProtocol()), QColor(180, 190, 210));

        // Read vehicle info
        m_dtcPanel->refreshVehicleInfo();

        // Show dashboard tab
        m_tabs->setCurrentIndex(0);
    } else {
        m_isConnected = false;
        m_lblStatus->setText("❌ Error");
        m_lblStatus->setStyleSheet("color: #FF6B6B; font-weight: bold; padding: 2px 10px;");

        appendLog("❌ Error de conexión. Verifique:", QColor(255, 60, 60));
        appendLog("  - Bluetooth encendido", QColor(255, 150, 100));
        appendLog("  - MAC correcta: " + mac, QColor(255, 150, 100));
        appendLog("  - ELM327 encendido y emparejado", QColor(255, 150, 100));

        m_elm.reset();
        m_gm.reset();
    }
}

void MainWindow::onDisconnect() {
    if (m_elm) {
        m_elm->disconnect();
        m_elm.reset();
        m_gm.reset();
    }
    m_isConnected = false;
    m_isScanning = false;
    m_demoMode = false;

    m_pollTimer->stop();

    m_lblStatus->setText("⏹ Desconectado");
    m_lblStatus->setStyleSheet("color: #8A8FA0; font-weight: bold; padding: 2px 10px;");
    m_lblProtocol->setText("");
    m_lblRPM->setText("RPM: ---");
    m_lblSpeed->setText("SPD: ---");
    m_lblMaxRPM->setText("RPM máx: ---");

    // Reset session stats
    m_sessMinRPM = 99999;
    m_sessMaxRPM = 0;
    m_sessTotalRPM = 0;
    m_sessCountRPM = 0;
    m_sessMaxSpeed = 0;

    m_actConnect->setEnabled(true);
    m_actDisconnect->setEnabled(false);
    m_actStartScan->setEnabled(false);
    m_actStopScan->setEnabled(false);
    m_actExport->setEnabled(false);
    m_actReport->setEnabled(false);
    m_actDemo->setChecked(false);
    m_actDemo->setIcon(AppIcons::iconDemo(false));
    m_actDemo->setEnabled(true);

    m_dashboard->setELM(nullptr);
    m_sensorPanel->setELM(nullptr);
    m_connectionPanel->setELM(nullptr);
    m_dtcPanel->setELM(nullptr);

    m_sensorPanel->stopPolling();

    appendLog("🔌 Desconectado", QColor(255, 180, 40));
}

void MainWindow::onStartScan() {
    if (!m_elm || !m_isConnected) {
        appendLog("⚠ No hay conexión. Conecte primero.", QColor(255, 180, 40));
        return;
    }

    m_isScanning = true;
    m_actStartScan->setEnabled(false);
    m_actStopScan->setEnabled(true);

    // Switch to sensors tab and start polling
    m_tabs->setCurrentIndex(1);
    m_sensorPanel->startPolling();

    appendLog("▶ Escaneo iniciado - monitoreando sensores...", QColor(100, 200, 255));
}

void MainWindow::onStopScan() {
    m_isScanning = false;
    m_sensorPanel->stopPolling();
    m_actStartScan->setEnabled(true);
    m_actStopScan->setEnabled(false);

    appendLog("■ Escaneo detenido", QColor(255, 180, 40));
}

void MainWindow::onClearLog() {
    m_scanLogPanel->clearLog();
    appendLog("🗑 Log limpiado", QColor(180, 190, 210));
}

void MainWindow::onExportCSV() {
    if (!m_elm) {
        appendLog("⚠ No hay datos para exportar", QColor(255, 180, 40));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this, "Exportar datos CSV", "logs/export_",
        "CSV (*.csv);;Todos los archivos (*)"
    );

    if (filename.isEmpty()) return;
    m_elm->logAllSensorsRaw(filename.toStdString());
    appendLog("💾 Datos exportados a " + filename, QColor(100, 200, 255));
}

void MainWindow::onScreenshot() {
    QString filename = QFileDialog::getSaveFileName(
        this, "Guardar captura de pantalla",
        "screenshots/screenshot_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".png",
        "PNG (*.png);;JPEG (*.jpg *.jpeg);;Todos los archivos (*)"
    );

    if (filename.isEmpty()) return;

    QPixmap pixmap = this->grab();
    if (pixmap.save(filename)) {
        appendLog("📷 Captura guardada: " + filename, QColor(100, 200, 255));
    } else {
        appendLog("❌ Error al guardar captura", QColor(255, 60, 60));
    }
}

void MainWindow::onExportReport() {
    if (!m_elm && !m_demoMode) {
        appendLog("⚠ No hay datos para generar reporte", QColor(255, 180, 40));
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this, "Exportar reporte diagnóstico",
        "reportes/reporte_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".html",
        "HTML (*.html);;Todos los archivos (*)"
    );
    if (filename.isEmpty()) return;

    // Build HTML report
    QString report;
    report += "<html><head><meta charset='utf-8'><title>Reporte Diagnóstico OBD-II</title>";
    report += "<style>body{background:#1A1D2A;color:#DCE1F0;font-family:sans-serif;padding:20px;}";
    report += "h1{color:#60CCFF;}h2{color:#FFB347;}table{border-collapse:collapse;width:100%;}";
    report += "td,th{border:1px solid #3C4158;padding:8px;text-align:left;}";
    report += "th{background:#2A2E3E;color:#B0B8D0;}</style></head><body>";
    report += "<h1>📋 Reporte de Diagnóstico OBD-II</h1>";
    report += "<p>Generado: " + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "</p>";
    report += "<hr>";

    // Session statistics
    report += "<h2>📊 Estadísticas de Sesión</h2>";
    report += "<table><tr><th>Métrica</th><th>Valor</th></tr>";
    report += QString("<tr><td>RPM Máximo</td><td>%1</td></tr>").arg(m_sessMaxRPM);
    report += QString("<tr><td>RPM Mínimo</td><td>%1</td></tr>").arg(m_sessMinRPM < 99999 ? QString::number(m_sessMinRPM) : "—");
    report += QString("<tr><td>RPM Promedio</td><td>%1</td></tr>")
        .arg(m_sessCountRPM > 0 ? QString::number(m_sessTotalRPM / m_sessCountRPM) : "—");
    report += QString("<tr><td>Velocidad Máx</td><td>%1 km/h</td></tr>").arg(m_sessMaxSpeed);
    report += "</table><br>";

    // DTCs
    if (!m_demoDTCs.empty()) {
        report += "<h2>🔧 Códigos DTC Detectados</h2><table><tr><th>Código</th><th>Descripción</th></tr>";
        for (const auto& dtc : m_demoDTCs) {
            report += QString("<tr><td style='color:#FFB347;'>%1</td><td>%2</td></tr>")
                .arg(QString::fromStdString(dtc.code), QString::fromStdString(dtc.description));
        }
        report += "</table>";
        if (m_demoCheckEngine) {
            report += "<p style='color:#FF4444;'><b>⚠ MIL: ENCENDIDO</b></p>";
        }
    }

    report += "<hr><p style='color:#6A6F80;'>Freebuff OBD-II Scanner v8.0</p></body></html>";

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << report;
        file.close();
        appendLog("📋 Reporte guardado: " + filename, QColor(100, 200, 255));
    } else {
        appendLog("❌ Error al guardar reporte", QColor(255, 60, 60));
    }
}

void MainWindow::onResetSettings() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Restablecer configuración",
        "¿Está seguro de restablecer toda la configuración a valores por defecto?\n"
        "Se perderán: MAC guardada, tamaño de ventana, modo demo y preferencias.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QSettings settings("Freebuff", "OBD2-Scanner");
        settings.clear();
        settings.sync();

        appendLog("⚙ Configuración restablecida a valores de fábrica", QColor(255, 180, 40));
        appendLog("💡 Reinicie la aplicación para aplicar todos los cambios", QColor(140, 145, 160));

        // Reset local state
        m_connectionPanel->setMAC("00:1D:A5:07:23:6E");
        m_sensorPanel->setIntervalIndex(1);
        m_tabs->setCurrentIndex(0);
        resize(1100, 720);
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "OBD-II Scanner Profesional v8",
        "<h3>OBD-II Escáner Profesional v8.0</h3>"
        "<p>Aplicación gráfica Qt5 para diagnóstico automotriz</p>"
        "<hr>"
        "<p><b>Características:</b></p>"
        "<ul>"
        "<li>Conexión Bluetooth ELM327</li>"
        "<li>Dashboard en vivo con indicadores</li>"
        "<li>Gráficos en tiempo real (QChart)</li>"
        "<li>Log de escaneo detallado</li>"
        "<li>Tabla DTC con +50 códigos en base local</li>"
        "<li>Info vehículo: VIN, ECU, protocolo</li>"
        "<li>Estadísticas de sesión (min/max/avg)</li>"
        "<li>Exportación a CSV y HTML</li>"
        "<li>Captura de pantalla integrada</li>"
        "<li>Modo Demo avanzado con escenarios</li>"
        "<li>Persistencia de configuración (QSettings)</li>"
        "<li>Comandos avanzados GM</li>"
        "</ul>"
        "<hr>"
        "<p style='color:#6A6F80;'>Freebuff Project — Built with Qt 5.15 · C++17</p>"
        "<p style='color:#6A6F80;'>" + QString(__DATE__) + "</p>"
    );
}

void MainWindow::onTimerTick() {
    if (m_demoMode) {
        m_demoCounter++;

        // Advance through demo scenario phases
        advanceDemoPhase();

        ELM327::DashboardData d = generateDemoData(m_demoCounter);
        double o2v = generateDemoO2(m_demoCounter);

        // Fuel trims vary by scenario
        double stft, ltft;
        switch (m_demoPhase) {
        case DemoPhase::FallaSensor:
            stft = 12.0 + 5.0 * sin(m_demoCounter * 0.05);   // rich trim trying to compensate
            ltft = 18.0 + 3.0 * sin(m_demoCounter * 0.01);   // long term maxed out
            break;
        case DemoPhase::RalentiIrregular:
            stft = -8.0 + 6.0 * sin(m_demoCounter * 0.1);    // erratic trims
            ltft = -5.0 + 4.0 * sin(m_demoCounter * 0.02);
            break;
        case DemoPhase::AccelBrusca:
            stft = -5.0 + 2.0 * sin(m_demoCounter * 0.2);    // goes negative (rich)
            ltft = 3.0 + 2.0 * sin(m_demoCounter * 0.01);
            break;
        default:
            stft = 2.0 * sin(m_demoCounter * 0.03);
            ltft = 3.0 * sin(m_demoCounter * 0.01);
            break;
        }

        // Update status bar
        m_lblRPM->setText(QString("RPM: %1").arg(d.rpm));
        m_lblSpeed->setText(QString("SPD: %1 km/h").arg(d.speed));

        // Show current phase in protocol label
        QString phaseStr;
        switch (m_demoPhase) {
        case DemoPhase::Normal:         phaseStr = "DEMO · Normal"; break;
        case DemoPhase::AccelBrusca:    phaseStr = "⚡ DEMO · Acel. Brusca"; break;
        case DemoPhase::PostAccel:      phaseStr = "DEMO · Recuperación"; break;
        case DemoPhase::FallaSensor:    phaseStr = "⚠ DEMO · Falla Sensor"; break;
        case DemoPhase::Recovery:       phaseStr = "DEMO · Recuperando"; break;
        case DemoPhase::RalentiIrregular: phaseStr = "🌀 DEMO · Ral. Irregular"; break;
        case DemoPhase::DTCTrigger:     phaseStr = "🔴 DEMO · DTC Activo"; break;
        }
        m_lblProtocol->setText(phaseStr);

        // Update check engine light
        if (m_demoCheckEngine) {
            m_lblCheckEngine->setText("🔴 CHECK ENGINE");
            m_lblCheckEngine->setStyleSheet(
                "color: #FF4444; font-weight: bold; padding: 2px 10px; "
                "background-color: #442222; border-radius: 3px;");
        } else {
            m_lblCheckEngine->setText("");
        }

        // Update session statistics
        m_sessTotalRPM += d.rpm;
        m_sessCountRPM++;
        if (d.rpm < m_sessMinRPM) m_sessMinRPM = d.rpm;
        if (d.rpm > m_sessMaxRPM) m_sessMaxRPM = d.rpm;
        if (d.speed > m_sessMaxSpeed) m_sessMaxSpeed = d.speed;

        // Update max RPM display
        m_lblMaxRPM->setText(QString("RPM máx: %1").arg(m_sessMaxRPM));

        // Update dashboard
        m_dashboard->updateData(d);

        // Update sensor panel with demo data
        m_sensorPanel->feedDemoData(d, o2v, stft, ltft);

        // Feed data to HistoryPanel (recording only)
        m_historyPanel->addDataPoint(d.rpm, d.speed, d.coolant,
                                      d.load, d.throttle, d.maf,
                                      o2v, stft, ltft);

        // Update DtcPanel with demo DTCs when check engine is on
        if (m_demoCheckEngine) {
            m_dtcPanel->setDemoDTCs(m_demoDTCs, true);
        }

        return;
    }

    if (!m_elm || !m_elm->isConnected()) {
        m_lblStatus->setText("⏹ Desconectado");
        m_lblStatus->setStyleSheet("color: #8A8FA0; font-weight: bold; padding: 2px 10px;");
        return;
    }

    // Update status bar with current values
    int rpm = m_elm->getRPM();
    int speed = m_elm->getSpeed();

    m_lblRPM->setText(QString("RPM: %1").arg(rpm >= 0 ? QString::number(rpm) : "---"));
    m_lblSpeed->setText(QString("SPD: %1 km/h").arg(speed >= 0 ? QString::number(speed) : "---"));

    // Update session statistics
    if (rpm >= 0) {
        m_sessTotalRPM += rpm;
        m_sessCountRPM++;
        if (rpm < m_sessMinRPM) m_sessMinRPM = rpm;
        if (rpm > m_sessMaxRPM) m_sessMaxRPM = rpm;
    }
    if (speed > m_sessMaxSpeed) m_sessMaxSpeed = speed;
    m_lblMaxRPM->setText(QString("RPM máx: %1").arg(m_sessMaxRPM > 0 ? QString::number(m_sessMaxRPM) : "---"));

    // Update dashboard if visible
    ELM327::DashboardData d;
    d.valid = false;
    if (m_tabs->currentIndex() == 0) {
        d = m_elm->getDashboardFast();
        if (d.valid) {
            m_dashboard->updateData(d);
        }
    }

    // Feed data to HistoryPanel (only reads extra OBD when recording)
    if (rpm >= 0) {
        double o2v = -1;
        double stft = 0, ltft = 0;

        // Only fetch expensive OBD-II data when actually recording
        if (m_historyPanel->isRecording()) {
            FuelTrim ft = m_elm->getAllFuelTrims();
            if (ft.available) {
                stft = ft.shortTermBank1;
                ltft = ft.longTermBank1;
            }
            std::vector<OxygenSensor> o2sensors = m_elm->getOxygenSensors();
            if (!o2sensors.empty() && o2sensors[0].voltage >= 0) {
                o2v = o2sensors[0].voltage;
            }
        }

        m_historyPanel->addDataPoint(
            d.valid ? static_cast<double>(d.rpm) : static_cast<double>(rpm),
            d.valid ? static_cast<double>(d.speed) : static_cast<double>(speed),
            d.valid ? static_cast<double>(d.coolant) : 0,
            d.valid ? static_cast<double>(d.load) : 0,
            d.valid ? d.throttle : 0,
            d.valid ? d.maf : 0,
            o2v, stft, ltft
        );
    }
}

void MainWindow::onConnectionStatusChanged(bool connected) {
    if (!connected && m_isConnected) {
        onDisconnect();
    }
}

void MainWindow::onDemo() {
    if (m_actDemo->isChecked()) {
        // Start demo mode
        m_demoMode = true;
        m_demoCounter = 0;
        m_demoPhase = DemoPhase::Normal;
        m_demoPhaseCounter = 0;
        m_demoPhaseDuration = 50 + rand() % 30;
        m_demoCheckEngine = false;
        m_demoDTCs.clear();

        m_lblStatus->setText("🎮 Demo activo");
        m_lblStatus->setStyleSheet("color: #FFB347; font-weight: bold; padding: 2px 10px;");
        m_lblProtocol->setText("Modo: DEMO");
        m_lblRPM->setText("RPM: ---");
        m_lblSpeed->setText("SPD: ---");
        m_lblCheckEngine->setText("");

        m_actConnect->setEnabled(false);
        m_actDisconnect->setEnabled(false);
        m_actStartScan->setEnabled(false);
        m_actStopScan->setEnabled(false);        m_actExport->setEnabled(false);
        m_actReport->setEnabled(true);
        m_dashboard->setELM(nullptr);
        m_sensorPanel->setELM(nullptr);
        m_sensorPanel->stopPolling();

        // Start poll timer for demo data generation (faster for smooth animations)
        m_pollTimer->setInterval(300);
        m_pollTimer->start();

        // Switch to dashboard tab
        m_tabs->setCurrentIndex(0);

        m_actDemo->setIcon(AppIcons::iconDemo(true));

        {
            QSettings s("Freebuff", "OBD2-Scanner");
            s.setValue("demo/enabled", true);
        }

        // Set demo vehicle info in DtcPanel
        m_dtcPanel->setELM(nullptr);
        m_dtcPanel->setDemoVehicleInfo();

        appendLog("🎮 Demo avanzado activado", QColor(255, 180, 40));
        appendLog("📊 Escenarios: Normal • Acel. Brusca • Falla Sensor • Ral. Irregular • DTC", QColor(180, 190, 210));
        appendLog("💡 DTCs visibles en pestaña Diagnóstico · CHECK ENGINE en barra de estado", QColor(140, 145, 160));
    } else {
        // Stop demo mode
        m_demoMode = false;
        m_pollTimer->stop();

        m_lblStatus->setText("⏹ Demo detenido");
        m_lblStatus->setStyleSheet("color: #8A8FA0; font-weight: bold; padding: 2px 10px;");
        m_lblProtocol->setText("");
        m_lblRPM->setText("RPM: ---");
        m_lblSpeed->setText("SPD: ---");
        m_lblCheckEngine->setText("");
        m_demoDTCs.clear();
        m_demoCheckEngine = false;

        m_actConnect->setEnabled(true);
        m_actDisconnect->setEnabled(false);
        m_actStartScan->setEnabled(false);
        m_actStopScan->setEnabled(false);        m_actExport->setEnabled(false);
        m_actReport->setEnabled(false);
        {
            QSettings s("Freebuff", "OBD2-Scanner");
            s.setValue("demo/enabled", false);
        }
        m_actDemo->setIcon(AppIcons::iconDemo(false));

        appendLog("🎮 Demo desactivado", QColor(255, 180, 40));
    }
}

void MainWindow::advanceDemoPhase() {
    // Cycle through phases based on duration
    m_demoPhaseCounter++;

    if (m_demoPhaseCounter < m_demoPhaseDuration) return;

    // Store previous phase for transition logic
    DemoPhase prev = m_demoPhase;
    m_demoPhaseCounter = 0;

    switch (prev) {
    case DemoPhase::Normal:
        // 30% chance of aceleacion brusca, otherwise keep normal
        if (rand() % 10 < 3) {
            m_demoPhase = DemoPhase::AccelBrusca;
            m_demoPhaseDuration = 15 + rand() % 10;  // ~5-8 seg
            appendLog("⚡ ACELERACIÓN BRUSCA — A fondo!", QColor(255, 180, 40));
        } else {
            m_demoPhaseDuration = 50 + rand() % 40;
        }
        break;

    case DemoPhase::AccelBrusca:
        m_demoPhase = DemoPhase::PostAccel;
        m_demoPhaseDuration = 20 + rand() % 15;
        appendLog("↘ Desaceleración — recuperando marcha", QColor(100, 200, 255));
        break;

    case DemoPhase::PostAccel:
        // 40% chance of fault after acceleration
        if (rand() % 10 < 4) {
            m_demoPhase = DemoPhase::FallaSensor;
            m_demoPhaseDuration = 20 + rand() % 15;
            appendLog("⚠ FALLA INTERMITENTE — Sensor MAF degradado!", QColor(255, 120, 60));
            triggerDemoDTC();
        } else {
            m_demoPhase = DemoPhase::Normal;
            m_demoPhaseDuration = 50 + rand() % 30;
        }
        break;

    case DemoPhase::FallaSensor:
        m_demoPhase = DemoPhase::Recovery;
        m_demoPhaseDuration = 10 + rand() % 10;
        appendLog("🛠 Recuperando sensores — normalizando lecturas", QColor(100, 200, 255));
        break;

    case DemoPhase::Recovery:
        m_demoPhase = DemoPhase::Normal;
        m_demoPhaseDuration = 40 + rand() % 30;
        appendLog("✅ Sensores normalizados", QColor(100, 255, 100));
        break;

    case DemoPhase::RalentiIrregular:
        m_demoPhase = DemoPhase::Normal;
        m_demoPhaseDuration = 40 + rand() % 30;
        appendLog("✅ Ralentí estabilizado", QColor(100, 255, 100));
        break;

    case DemoPhase::DTCTrigger:
        m_demoPhase = DemoPhase::Normal;
        m_demoPhaseDuration = 40 + rand() % 20;
        if (m_demoCheckEngine) {
            appendLog("🔧 CÓDIGOS DTC REGISTRADOS — MIL encendido", QColor(255, 100, 60));
            for (const auto& dtc : m_demoDTCs) {
                appendLog(QString::fromStdString("  " + dtc.code + ": " + dtc.description),
                          QColor(255, 180, 100));
            }
        }
        break;
    }

    // Occasionally trigger ralenti irregular when in normal phase
    if (m_demoPhase == DemoPhase::Normal && rand() % 10 < 2) {
        m_demoPhase = DemoPhase::RalentiIrregular;
        m_demoPhaseDuration = 20 + rand() % 15;
        appendLog("🌀 RALENTÍ IRREGULAR — Motor fallando al ralentí", QColor(255, 180, 40));
        if (!m_demoCheckEngine && rand() % 10 < 4) {
            triggerDemoDTC();
        }
    }

    // Occasionally trigger DTCs (multi-code event)
    if (m_demoPhase == DemoPhase::Normal && !m_demoCheckEngine && rand() % 15 < 1) {
        m_demoPhase = DemoPhase::DTCTrigger;
        m_demoPhaseDuration = 5;
        m_demoDTCs = generateDemoDTCs();
        m_demoCheckEngine = true;
        appendLog("🔴 CHECK ENGINE — Múltiples fallos detectados!", QColor(255, 60, 60));
        // Show DTCs immediately
        for (const auto& dtc : m_demoDTCs) {
            appendLog(QString::fromStdString("  ⚠ " + dtc.code + ": " + dtc.description),
                      QColor(255, 180, 100));
        }
    }
}

bool MainWindow::demoFaultActive() {
    return m_demoPhase == DemoPhase::FallaSensor ||
           m_demoPhase == DemoPhase::RalentiIrregular;
}

void MainWindow::triggerDemoDTC() {
    // Add a single DTC related to the current fault
    std::vector<DTCCode> allDTCs = {
        {"P0101", "Rango/rendimiento del circuito MAF"},
        {"P0102", "Entrada baja del circuito MAF"},
        {"P0171", "Mezcla pobre (banco 1)"},
        {"P0174", "Mezcla pobre (banco 2)"},
        {"P0300", "Fallo de encendido aleatorio/múltiples cilindros"},
        {"P0301", "Fallo de encendido cilindro 1"},
        {"P0302", "Fallo de encendido cilindro 2"},
        {"P0303", "Fallo de encendido cilindro 3"},
        {"P0304", "Fallo de encendido cilindro 4"},
        {"P0420", "Eficiencia catalizador bajo banco 1"},
        {"P0455", "Fuga grande en sistema EVAP"},
        {"P0505", "Fallo sistema control ralentí"},
        {"P0507", "Ralentí más alto de lo esperado"},
        {"P1297", "Fallo conexión turbocompresor"},
        {"P2195", "Sensor O2 señal pobre (banco 1)"},
    };
    m_demoDTCs.push_back(allDTCs[rand() % allDTCs.size()]);
}

std::vector<DTCCode> MainWindow::generateDemoDTCs() {
    std::vector<DTCCode> dtcs;
    int count = 2 + rand() % 4;  // 2-5 codes
    for (int i = 0; i < count; i++) {
        triggerDemoDTC();
    }
    // Remove duplicates
    std::sort(m_demoDTCs.begin(), m_demoDTCs.end(),
              [](const DTCCode& a, const DTCCode& b) { return a.code < b.code; });
    auto last = std::unique(m_demoDTCs.begin(), m_demoDTCs.end(),
                            [](const DTCCode& a, const DTCCode& b) { return a.code == b.code; });
    m_demoDTCs.erase(last, m_demoDTCs.end());
    return m_demoDTCs;
}

ELM327::DashboardData MainWindow::generateDemoData(int counter) {
    ELM327::DashboardData d;
    d.valid = true;

    // Warm-up curve (default coolant unless overridden by phase)
    double warmUp = 1.0 - exp(-counter * 0.005);
    d.coolant = static_cast<int>(30.0 + 60.0 * warmUp + 3.0 * sin(counter * 0.02));
    d.intakeTemp = 25.0 + 15.0 * warmUp + 2.0 * sin(counter * 0.015);
    d.intakePressure = 30 + 50 * (0.5 + 0.5 * sin(counter * 0.04));

    switch (m_demoPhase) {

    // ── Conducción normal ──────────────────────────────────────────────
    case DemoPhase::Normal:
    case DemoPhase::PostAccel: {
        double phase = counter * 0.05;
        double baseRPM = 1500.0 + 2500.0 * (0.5 + 0.5 * sin(phase));
        double noise = 40.0 * sin(counter * 0.37);
        d.rpm = static_cast<int>(baseRPM + noise);
        d.speed = static_cast<int>(20.0 + 80.0 * (0.5 + 0.5 * sin(counter * 0.03)));
        d.load = static_cast<int>(15.0 + 35.0 * std::max(0.0, std::min(1.0, (d.rpm - 800.0) / 5200.0)));
        d.throttle = 12.0 + 30.0 * (0.5 + 0.5 * sin(counter * 0.08 + 0.5));
        d.maf = (d.rpm / 1000.0) * (d.load / 25.0) + 1.5;
        d.timing = 8.0 + 20.0 * (d.rpm / 6500.0);
        break;
    }

    // ── Aceleración brusca ─────────────────────────────────────────────
    case DemoPhase::AccelBrusca: {
        double progress = std::min(1.0, m_demoPhaseCounter / 12.0);
        // RPM jump: 2500 -> 6800 in ~3 seconds
        d.rpm = static_cast<int>(2500 + 4300 * std::min(1.0, progress * 1.5));
        // Speed surge
        d.speed = static_cast<int>(60 + 100 * progress);
        // Throttle wide open
        d.throttle = 70.0 + 25.0 * (1.0 - exp(-progress * 3.0));
        if (d.throttle > 95.0) d.throttle = 95.0;
        // Load spikes
        d.load = static_cast<int>(50 + 45 * progress);
        if (d.load > 90) d.load = 90;
        // MAF surge
        d.maf = (d.rpm / 1000.0) * (d.load / 15.0) + 3.0;
        // Timing retards slightly under load
        d.timing = 10.0 + 15.0 * (d.rpm / 6500.0) - 3.0 * progress;
        break;
    }

    // ── Falla intermitente de sensor ───────────────────────────────────
    case DemoPhase::FallaSensor: {
        double phase = counter * 0.04;
        d.rpm = static_cast<int>(1800.0 + 2000.0 * (0.5 + 0.5 * sin(phase)));
        d.speed = static_cast<int>(30.0 + 50.0 * (0.5 + 0.5 * sin(counter * 0.02)));
        d.load = static_cast<int>(20.0 + 30.0 * (0.5 + 0.5 * sin(phase + 0.5)));
        d.throttle = 15.0 + 25.0 * (0.5 + 0.5 * sin(counter * 0.06));

        // FAULT: MAF drops intermittently (simulating bad sensor)
        if ((counter / 5) % 2 == 0) {
            d.maf = 1.0 + 2.0 * (0.5 + 0.5 * sin(counter * 0.2));
        } else {
            d.maf = (d.rpm / 1000.0) * (d.load / 25.0) + 1.5;
        }

        // FAULT: Coolant erratic (overrides warmup default)
        d.coolant = static_cast<int>(80.0 + 20.0 * sin(counter * 0.15) + 5.0 * sin(counter * 0.7));
        d.timing = 5.0 + 15.0 * (d.rpm / 6500.0) + 5.0 * sin(counter * 0.25);
        break;
    }

    // ── Recuperación de falla ──────────────────────────────────────────
    case DemoPhase::Recovery: {
        double progress = std::min(1.0, m_demoPhaseCounter / 15.0);
        double phase = counter * 0.04;
        d.rpm = static_cast<int>(1500.0 + 2000.0 * (0.5 + 0.5 * sin(phase)));
        d.speed = static_cast<int>(30.0 + 50.0 * (0.5 + 0.5 * sin(counter * 0.02)));
        d.load = static_cast<int>(15.0 + 30.0 * (0.5 + 0.5 * sin(phase)));
        d.throttle = 10.0 + 20.0 * (0.5 + 0.5 * sin(counter * 0.06));
        // MAF normalizes with progress
        d.maf = progress * ((d.rpm / 1000.0) * (d.load / 25.0) + 1.5)
                + (1.0 - progress) * (2.0 + 3.0 * (0.5 + 0.5 * sin(counter * 0.2)));
        // Coolant normalizes (overrides warmup default)
        d.coolant = static_cast<int>(85.0 + 5.0 * sin(counter * 0.02));
        d.timing = 8.0 + 18.0 * (d.rpm / 6500.0);
        break;
    }

    // ── Ralentí irregular ──────────────────────────────────────────────
    case DemoPhase::RalentiIrregular: {
        double hunt = 0.5 + 0.5 * sin(counter * 0.3);
        double misfire = 100.0 * sin(counter * 0.8);
        d.rpm = static_cast<int>(700 + 600 * hunt + misfire);
        if (d.rpm < 400) d.rpm = 400;
        if (d.rpm > 2500) d.rpm = 2500;

        d.speed = static_cast<int>(2.0 + 3.0 * (0.5 + 0.5 * sin(counter * 0.1)));
        d.load = static_cast<int>(25.0 + 30.0 * hunt + 10.0 * sin(counter * 0.5));
        d.throttle = 5.0 + 15.0 * hunt;
        d.maf = (d.rpm / 1000.0) * (d.load / 20.0) + 1.0;
        d.timing = 5.0 + 10.0 * hunt;
        break;
    }

    // ── DTC Trigger ────────────────────────────────────────────────────
    case DemoPhase::DTCTrigger: {
        double phase = counter * 0.06;
        d.rpm = static_cast<int>(1200.0 + 2500.0 * (0.5 + 0.5 * sin(phase)));
        d.speed = static_cast<int>(20.0 + 60.0 * (0.5 + 0.5 * sin(counter * 0.025)));
        d.load = static_cast<int>(20.0 + 40.0 * (0.5 + 0.5 * sin(phase)));
        d.throttle = 10.0 + 30.0 * (0.5 + 0.5 * sin(counter * 0.05));
        d.maf = (d.rpm / 1000.0) * (d.load / 22.0) + 1.5 + 2.0 * sin(counter * 0.15);
        d.timing = 8.0 + 15.0 * (d.rpm / 6500.0);
        break;
    }
    }

    // ── Shared fuel level ──────────────────────────────────────────────
    double fuelStart = 85.0;
    double fuelDrop = counter * 0.001;
    d.fuelLevel = std::max(10.0, fuelStart - fuelDrop);

    return d;
}

double MainWindow::generateDemoO2(int counter) {
    if (demoFaultActive() && (counter / 4) % 3 == 0) {
        // During faults, O2 may get stuck at intermediate value (bad sensor)
        return 0.45 + 0.05 * sin(counter * 0.1);
    }

    if (m_demoPhase == DemoPhase::AccelBrusca) {
        // During acceleration, runs rich
        return 0.85 + 0.1 * sin(counter * 0.5);
    }

    // Normal oscillation
    double slowCycle = sin(counter * 0.04);
    double fastNoise = 0.1 * sin(counter * 0.5);
    double voltage = 0.5 + 0.4 * slowCycle + fastNoise;
    return std::max(0.05, std::min(0.95, voltage));
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::loadSettings() {
    QSettings settings("Freebuff", "OBD2-Scanner");

    // Restore window geometry
    QByteArray geo = settings.value("window/geometry").toByteArray();
    if (!geo.isEmpty()) {
        restoreGeometry(geo);
    } else {
        resize(1100, 720);
    }

    QByteArray state = settings.value("window/state").toByteArray();
    if (!state.isEmpty()) {
        restoreState(state);
    }

    // Restore last MAC
    QString mac = settings.value("connection/mac", "").toString();
    if (!mac.isEmpty()) {
        m_connectionPanel->setMAC(mac);
    }

    // Restore scan interval
    int intervalIdx = settings.value("sensor/interval", 1).toInt();
    m_sensorPanel->setIntervalIndex(intervalIdx);

    // Restore last active tab
    int tabIdx = settings.value("window/activeTab", 0).toInt();
    if (tabIdx >= 0 && tabIdx < m_tabs->count()) {
        m_tabs->setCurrentIndex(tabIdx);
    }

    // Restore demo mode (must be last to avoid UI conflicts)
    bool demoEnabled = settings.value("demo/enabled", false).toBool();
    if (demoEnabled) {
        m_actDemo->setChecked(true);
        onDemo();
    }

    appendLog("⚙ Configuración persistente cargada", QColor(140, 145, 160));
}

void MainWindow::saveSettings() {
    QSettings settings("Freebuff", "OBD2-Scanner");

    // Save window geometry
    settings.setValue("window/geometry", saveGeometry());
    settings.setValue("window/state", saveState());

    // Save last active tab
    settings.setValue("window/activeTab", m_tabs->currentIndex());

    // Save scan interval
    settings.setValue("sensor/interval", m_sensorPanel->getIntervalIndex());

    appendLog("⚙ Configuración guardada", QColor(140, 145, 160));
}

void MainWindow::appendLog(const QString& text, const QColor& color) {
    m_scanLogPanel->appendLog(text, color);
}
