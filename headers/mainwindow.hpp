/**
 * @file mainwindow.hpp
 * @brief Ventana principal de la aplicación OBD-II gráfica.
 *
 * Organiza la interfaz en pestañas:
 * - Dashboard: indicadores en vivo
 * - Sensores: gráficos en tiempo real
 * - Log de escaneo: historial de datos
 * - Configuración: ajustes de conexión
 */

#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QToolBar>
#include <QAction>
#include <QTimer>
#include <QLabel>
#include <memory>

#include "elm327.hpp"
#include "gm_commands.hpp"
#include "logger.hpp"
#include "connectionpanel.hpp"
#include "sensorpanel.hpp"
#include "scanlogpanel.hpp"
#include "dashboardwidget.hpp"
#include "dtcpanel.hpp"
#include "historypanel.hpp"

/**
 * @brief Ventana principal con pestañas para el escáner OBD-II.
 *
 * Proporciona una interfaz completamente gráfica con:
 * - Toolbar con acciones de conexión, escaneo y configuración
 * - StatusBar con estado de conexión y datos en vivo
 * - Pestañas: Dashboard, Sensores, Log, Configuración
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onConnect();
    void onDisconnect();
    void onStartScan();
    void onStopScan();
    void onClearLog();
    void onExportCSV();
    void onScreenshot();
    void onExportReport();
    void onResetSettings();
    void onAbout();
    void onTimerTick();
    void onConnectionStatusChanged(bool connected);
    void onDemo();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void createWidgets();
    void createToolBar();
    void createStatusBar();
    void connectSignals();
    void loadSettings();
    void saveSettings();
    void appendLog(const QString& text, const QColor& color = QColor(180, 190, 210));

    // Core components
    std::unique_ptr<ELM327> m_elm;
    std::unique_ptr<GMCommands> m_gm;
    std::unique_ptr<Logger> m_logger;
    std::unique_ptr<SessionLogger> m_sessionLog;

    // UI
    QTabWidget* m_tabs;
    DashboardWidget* m_dashboard;
    SensorPanel* m_sensorPanel;
    ScanLogPanel* m_scanLogPanel;
    ConnectionPanel* m_connectionPanel;
    DtcPanel* m_dtcPanel;
    HistoryPanel* m_historyPanel;

    // Toolbar actions
    QAction* m_actConnect;
    QAction* m_actDisconnect;
    QAction* m_actStartScan;
    QAction* m_actStopScan;
    QAction* m_actClearLog;
    QAction* m_actExport;
    QAction* m_actScreenshot;
    QAction* m_actReport;
    QAction* m_actResetSettings;
    QAction* m_actDemo;

    // Status bar labels
    QLabel* m_lblStatus;
    QLabel* m_lblProtocol;
    QLabel* m_lblRPM;
    QLabel* m_lblSpeed;
    QLabel* m_lblMaxRPM;
    QLabel* m_lblCheckEngine;

    // Timer for periodic data polling
    QTimer* m_pollTimer;
    bool m_isScanning;
    bool m_isConnected;
    bool m_demoMode;
    int m_demoCounter;

    // Session statistics
    int m_sessMinRPM;
    int m_sessMaxRPM;
    int m_sessTotalRPM;
    int m_sessCountRPM;
    int m_sessMaxSpeed;

    // Demo scenarios
    enum class DemoPhase {
        Normal,           // Conducción normal suave
        AccelBrusca,      // Aceleración brusca a fondo
        PostAccel,        // Recuperación tras aceleración
        FallaSensor,      // Falla intermitente de sensor (MAF/O2)
        Recovery,         // Recuperación de falla
        RalentiIrregular, // Ralentí irregular, motor fallando
        DTCTrigger        // Múltiples DTCs, MIL encendido
    };
    DemoPhase m_demoPhase;
    int m_demoPhaseCounter;
    int m_demoPhaseDuration;
    bool m_demoCheckEngine;
    std::vector<DTCCode> m_demoDTCs;

    ELM327::DashboardData generateDemoData(int counter);
    double generateDemoO2(int counter);
    void advanceDemoPhase();
    void triggerDemoDTC();
    bool demoFaultActive();
    std::vector<DTCCode> generateDemoDTCs();
};
