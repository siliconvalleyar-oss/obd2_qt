#include "headers/dtcpanel.hpp"

#include <QHeaderView>
#include <QScrollBar>
#include <QApplication>
#include <QLabel>

// ============================================================================
// DTC database for demo mode and fallback descriptions
// ============================================================================

static const std::vector<DTCCode> s_dtcDatabase = {
    {"P0100", "Circuito MAF - fallo"},
    {"P0101", "Rango/rendimiento del circuito MAF"},
    {"P0102", "Entrada baja del circuito MAF"},
    {"P0103", "Entrada alta del circuito MAF"},
    {"P0106", "Rango/rendimiento MAP"},
    {"P0110", "Circuito IAT - fallo"},
    {"P0113", "Entrada alta IAT"},
    {"P0115", "Circuito ECT - fallo"},
    {"P0120", "Circuito TPS - fallo"},
    {"P0121", "Rango/rendimiento TPS"},
    {"P0122", "Entrada baja TPS"},
    {"P0123", "Entrada alta TPS"},
    {"P0130", "Circuito sensor O2 B1S1 - fallo"},
    {"P0131", "Sensor O2 B1S1 - voltaje bajo"},
    {"P0132", "Sensor O2 B1S1 - voltaje alto"},
    {"P0133", "Sensor O2 B1S1 - respuesta lenta"},
    {"P0135", "Circuito calefactor O2 B1S1"},
    {"P0136", "Circuito sensor O2 B1S2 - fallo"},
    {"P0141", "Circuito calefactor O2 B1S2"},
    {"P0171", "Mezcla pobre (banco 1)"},
    {"P0172", "Mezcla rica (banco 1)"},
    {"P0174", "Mezcla pobre (banco 2)"},
    {"P0300", "Fallo encendido aleatorio/múltiples cilindros"},
    {"P0301", "Fallo encendido cilindro 1"},
    {"P0302", "Fallo encendido cilindro 2"},
    {"P0303", "Fallo encendido cilindro 3"},
    {"P0304", "Fallo encendido cilindro 4"},
    {"P0305", "Fallo encendido cilindro 5"},
    {"P0306", "Fallo encendido cilindro 6"},
    {"P0325", "Circuito sensor knock - fallo"},
    {"P0335", "Circuito CKP - fallo"},
    {"P0340", "Circuito CMP - fallo"},
    {"P0350", "Circuito bobina encendido - fallo"},
    {"P0400", "Flujo EGR - fallo"},
    {"P0401", "Flujo EGR insuficiente"},
    {"P0420", "Eficiencia catalizador bajo banco 1"},
    {"P0430", "Eficiencia catalizador bajo banco 2"},
    {"P0440", "Sistema EVAP - fallo"},
    {"P0442", "Fuga pequeña EVAP"},
    {"P0455", "Fuga grande EVAP"},
    {"P0456", "Fuga muy pequeña EVAP"},
    {"P0500", "Circuito VSS - fallo"},
    {"P0505", "Sistema control ralentí - fallo"},
    {"P0507", "Ralentí más alto de lo esperado"},
    {"P0562", "Voltaje sistema bajo"},
    {"P0600", "Fallo comunicación serial"},
    {"P0601", "ECU - error checksum"},
    {"P0606", "ECU - fallo interno"},
    {"P0620", "Circuito generador - fallo"},
    {"P0700", "Transmisión - fallo genérico"},
    {"P1297", "Fallo conexión turbocompresor"},
    {"P2195", "Sensor O2 señal pobre B1S1"},
    {"P2196", "Sensor O2 señal rica B1S1"},
    {"P2270", "Sensor O2 señal pobre B1S2"},
    {"P2271", "Sensor O2 señal rica B1S2"},
};

// ============================================================================
// DtcPanel implementation
// ============================================================================

DtcPanel::DtcPanel(ELM327* elm, QWidget* parent)
    : QWidget(parent), m_elm(elm) {
    setupUI();
}

void DtcPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Title ──────────────────────────────────────────────────────────
    QLabel* title = new QLabel("🔧 DIAGNÓSTICO - DTCs E INFORMACIÓN");
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #60CCFF; padding: 4px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // ── DTC Section ────────────────────────────────────────────────────
    QGroupBox* dtcGroup = new QGroupBox("Códigos de Error DTC");
    dtcGroup->setStyleSheet(
        "QGroupBox { color: #FF6B6B; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }");
    QVBoxLayout* dtcLayout = new QVBoxLayout(dtcGroup);
    dtcLayout->setSpacing(8);

    // Status bar for DTCs
    QHBoxLayout* dtcStatusLayout = new QHBoxLayout();
    m_lblMILStatus = new QLabel("MIL: —");
    m_lblMILStatus->setStyleSheet("font-size: 13px; font-weight: bold; color: #8A8FA0;");
    m_lblDtcCount = new QLabel("DTCs: 0");
    m_lblDtcCount->setStyleSheet("font-size: 13px; color: #B0B8D0;");
    dtcStatusLayout->addWidget(m_lblMILStatus);
    dtcStatusLayout->addWidget(m_lblDtcCount);
    dtcStatusLayout->addStretch();
    dtcLayout->addLayout(dtcStatusLayout);

    // DTC table
    m_dtcTable = new QTableWidget(0, 4);
    m_dtcTable->setHorizontalHeaderLabels({"Código", "Descripción", "Tipo", "Estado"});
    m_dtcTable->setAlternatingRowColors(true);
    m_dtcTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dtcTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dtcTable->setStyleSheet(
        "QTableWidget { background-color: #1A1D2A; color: #DCE1F0; "
        "gridline-color: #2A2E3E; border: none; }"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { background-color: #2A2E3E; color: #B0B8D0; "
        "padding: 6px; border: none; font-weight: bold; }"
        "QTableWidget::item:alternate { background-color: #222638; }");
    m_dtcTable->setColumnWidth(0, 100);
    m_dtcTable->setColumnWidth(1, 320);
    m_dtcTable->setColumnWidth(2, 80);
    m_dtcTable->setColumnWidth(3, 90);
    dtcLayout->addWidget(m_dtcTable);

    // DTC buttons
    QHBoxLayout* dtcBtnLayout = new QHBoxLayout();
    m_btnReadDTCs = new QPushButton("📡 LEER DTCs");
    m_btnReadDTCs->setStyleSheet(
        "QPushButton { background-color: #2D5C8C; color: white; font-weight: bold; "
        "padding: 8px 20px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3A6CA6; }");
    m_btnClearDTCs = new QPushButton("🗑 LIMPIAR DTCs");
    m_btnClearDTCs->setStyleSheet(
        "QPushButton { background-color: #8C2D2D; color: white; font-weight: bold; "
        "padding: 8px 20px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #A63A3A; }");
    dtcBtnLayout->addWidget(m_btnReadDTCs);
    dtcBtnLayout->addWidget(m_btnClearDTCs);
    dtcBtnLayout->addStretch();
    dtcLayout->addLayout(dtcBtnLayout);

    mainLayout->addWidget(dtcGroup);

    // ── Vehicle Info Section ────────────────────────────────────────────
    QGroupBox* infoGroup = new QGroupBox("Información del Vehículo");
    infoGroup->setStyleSheet(
        "QGroupBox { color: #60CCFF; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(6);

    QGridLayout* infoGrid = new QGridLayout();
    infoGrid->setSpacing(8);

    auto addInfoRow = [&](int row, const QString& label) -> QLabel* {
        QLabel* lbl = new QLabel(label);
        lbl->setStyleSheet("font-size: 11px; font-weight: bold; color: #8A8FA0;");
        QLabel* val = new QLabel("—");
        val->setStyleSheet("font-size: 12px; color: #DCE1F0; font-family: monospace;");
        val->setTextInteractionFlags(Qt::TextSelectableByMouse);
        infoGrid->addWidget(lbl, row, 0);
        infoGrid->addWidget(val, row, 1);
        return val;
    };

    m_lblVIN = addInfoRow(0, "VIN:");
    m_lblProtocol = addInfoRow(1, "Protocolo:");
    m_lblECUName = addInfoRow(2, "ECU:");
    m_lblCalibrationID = addInfoRow(3, "Calibración ID:");
    infoLayout->addLayout(infoGrid);

    m_btnRefreshVIN = new QPushButton("🔄 OBTENER INFO VEHÍCULO");
    m_btnRefreshVIN->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 8px 20px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C5168; }");
    infoLayout->addWidget(m_btnRefreshVIN);

    mainLayout->addWidget(infoGroup);

    // ── Status / Freeze Frame ───────────────────────────────────────────
    QGroupBox* statusGroup = new QGroupBox("Estado del Sistema");
    statusGroup->setStyleSheet(
        "QGroupBox { color: #90EE90; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);

    m_statusView = new QTextEdit();
    m_statusView->setReadOnly(true);
    m_statusView->setStyleSheet(
        "QTextEdit { background-color: #0D0F17; color: #B0B8D0; "
        "border: 1px solid #2A2E3E; border-radius: 4px; "
        "font-family: monospace; font-size: 11px; padding: 6px; }");
    m_statusView->setMinimumHeight(100);
    m_statusView->setMaximumHeight(140);
    statusLayout->addWidget(m_statusView);

    mainLayout->addWidget(statusGroup);
    mainLayout->addStretch();

    // ── Connect signals ─────────────────────────────────────────────────
    connect(m_btnReadDTCs, &QPushButton::clicked, this, &DtcPanel::onReadDTCs);
    connect(m_btnClearDTCs, &QPushButton::clicked, this, &DtcPanel::onClearDTCs);
    connect(m_btnRefreshVIN, &QPushButton::clicked, this, &DtcPanel::onRefreshVIN);

    // Initial status message
    m_statusView->setHtml(
        "<span style='color:#8A8FA0;'>Sistema listo. Conecte ELM327 o use Demo "
        "para leer DTCs e información del vehículo.</span>");
}

// ─── Helper: set DTC cell ────────────────────────────────────────────────────

void DtcPanel::setDTCCell(int row, const std::string& code,
                           const std::string& desc, bool isPending) {
    auto setItem = [this](int r, int col, const QString& text,
                          const QColor& color = QColor(220, 225, 240)) {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setForeground(color);
        item->setTextAlignment(Qt::AlignCenter);
        m_dtcTable->setItem(r, col, item);
    };

    setItem(row, 0, QString::fromStdString(code), QColor(255, 180, 100));
    setItem(row, 1, QString::fromStdString(desc));
    setItem(row, 2, isPending ? "Pendiente" : "Confirmado",
            isPending ? QColor(255, 200, 60) : QColor(100, 200, 255));
    setItem(row, 3, "⚠ ACTIVO", QColor(255, 100, 60));
}

// ─── Public: set demo DTCs ──────────────────────────────────────────────────

void DtcPanel::setDemoDTCs(const std::vector<DTCCode>& dtcs, bool checkEngine) {
    m_dtcTable->setRowCount(0);

    for (size_t i = 0; i < dtcs.size(); i++) {
        m_dtcTable->insertRow(static_cast<int>(i));
        setDTCCell(static_cast<int>(i), dtcs[i].code, dtcs[i].description, false);
    }

    setMILStatus(checkEngine, static_cast<int>(dtcs.size()));

    if (!dtcs.empty()) {
        m_statusView->setHtml(
            "<span style='color:#FF6B6B;'>⚠ SE DETECTARON "
            + QString::number(dtcs.size())
            + " CÓDIGOS DE ERROR</span><br>"
            "<span style='color:#8A8FA0;'>Use 'Limpiar DTCs' para borrar los códigos. "
            "Verifique el sistema antes de borrar.</span>");
    } else {
        m_statusView->setHtml(
            "<span style='color:#6BFF6B;'>✅ No se detectaron códigos de error</span>");
    }
}

// ─── Public: refresh from real ELM327 ───────────────────────────────────────

void DtcPanel::refreshDTCs() {
    if (!m_elm || !m_elm->isConnected()) {
        m_statusView->setHtml(
            "<span style='color:#FFB347;'>⚠ No hay conexión con ELM327. "
            "Conecte primero o active el modo Demo.</span>");
        return;
    }

    m_statusView->setHtml("<span style='color:#B0B8D0;'>📡 Leyendo DTCs...</span>");
    QApplication::processEvents();

    std::vector<DTCCode> dtcs = m_elm->getDTCs();
    bool milOn = m_elm->checkMIL();

    m_dtcTable->setRowCount(0);

    for (size_t i = 0; i < dtcs.size(); i++) {
        m_dtcTable->insertRow(static_cast<int>(i));
        setDTCCell(static_cast<int>(i), dtcs[i].code, dtcs[i].description, false);
    }

    // Also get pending DTCs (mode 07)
    std::vector<DTCCode> pending = m_elm->getPendingDTCs();
    for (size_t i = 0; i < pending.size(); i++) {
        int row = static_cast<int>(dtcs.size() + i);
        m_dtcTable->insertRow(row);
        setDTCCell(row, pending[i].code, pending[i].description, true);
    }

    setMILStatus(milOn, static_cast<int>(dtcs.size() + pending.size()));

    if (!dtcs.empty() || !pending.empty()) {
        m_statusView->setHtml(
            "<span style='color:#FF6B6B;'>⚠ DTCs detectados: "
            + QString::number(dtcs.size()) + " confirmados, "
            + QString::number(pending.size()) + " pendientes</span>");
    } else {
        m_statusView->setHtml(
            "<span style='color:#6BFF6B;'>✅ Sin DTCs. "
            "Sistema OK.</span>");
    }
}

// ─── Public: refresh vehicle info ──────────────────────────────────────────

void DtcPanel::refreshVehicleInfo() {
    if (!m_elm || !m_elm->isConnected()) return;

    QString vin = QString::fromStdString(m_elm->getVIN());
    QString proto = QString::fromStdString(m_elm->getProtocol());
    QString ecu = QString::fromStdString(m_elm->getECUName());
    QString calib = QString::fromStdString(m_elm->getCalibrationID());

    setVehicleInfo(vin, proto, ecu, calib);
}

// ─── Public: set demo vehicle info ─────────────────────────────────────────

void DtcPanel::setDemoVehicleInfo() {
    setVehicleInfo(
        "W0L0ZBF48X2123456",
        "ISO 15765-4 CAN (11 bit, 500 kbaud)",
        "Bosch MED17.1 — ECU Motor Gasolina",
        "CVN: 0x1A2B3C4D / CALID: 1037398472"
    );
}

// ─── Private: set vehicle info ──────────────────────────────────────────────

void DtcPanel::setVehicleInfo(const QString& vin, const QString& protocol,
                               const QString& ecuName, const QString& calibID) {
    m_lblVIN->setText(vin.isEmpty() || vin == "No disponible" ? "—" : vin);
    m_lblProtocol->setText(protocol.isEmpty() ? "—" : protocol);
    m_lblECUName->setText(ecuName.isEmpty() ? "—" : ecuName);
    m_lblCalibrationID->setText(calibID.isEmpty() ? "—" : calibID);
}

// ─── Private: set MIL status ────────────────────────────────────────────────

void DtcPanel::setMILStatus(bool milOn, int dtcCount) {
    m_lblDtcCount->setText(QString("DTCs: %1").arg(dtcCount));

    if (milOn) {
        m_lblMILStatus->setText("🔴 MIL: ENCENDIDO");
        m_lblMILStatus->setStyleSheet(
            "font-size: 13px; font-weight: bold; color: #FF4444; "
            "background-color: #442222; border-radius: 3px; padding: 2px 6px;");
    } else if (dtcCount > 0) {
        m_lblMILStatus->setText("🟡 MIL: OFF (DTCs históricos)");
        m_lblMILStatus->setStyleSheet(
            "font-size: 13px; font-weight: bold; color: #FFB347;");
    } else {
        m_lblMILStatus->setText("✅ MIL: OFF · SIN ERRORES");
        m_lblMILStatus->setStyleSheet(
            "font-size: 13px; font-weight: bold; color: #6BFF6B;");
    }
}

// ─── Slots ──────────────────────────────────────────────────────────────────

void DtcPanel::onReadDTCs() {
    refreshDTCs();
}

void DtcPanel::onClearDTCs() {
    if (!m_elm || !m_elm->isConnected()) {
        m_statusView->setHtml(
            "<span style='color:#FFB347;'>⚠ No hay conexión para limpiar DTCs. "
            "Conecte ELM327 primero.</span>");
        return;
    }

    m_statusView->setHtml("<span style='color:#B0B8D0;'>🗑 Limpiando DTCs...</span>");
    QApplication::processEvents();

    if (m_elm->clearDTCs()) {
        m_dtcTable->setRowCount(0);
        setMILStatus(false, 0);
        m_statusView->setHtml(
            "<span style='color:#6BFF6B;'>✅ DTCs limpiados exitosamente. "
            "MIL apagado si la falla fue reparada.</span>");
    } else {
        m_statusView->setHtml(
            "<span style='color:#FF6B6B;'>❌ Error al limpiar DTCs. "
            "Verifique conexión e intente de nuevo.</span>");
    }
}

void DtcPanel::onRefreshVIN() {
    if (m_elm && m_elm->isConnected()) {
        m_statusView->setHtml(
            "<span style='color:#B0B8D0;'>🔄 Consultando información del vehículo...</span>");
        QApplication::processEvents();
        refreshVehicleInfo();
        m_statusView->setHtml(
            "<span style='color:#6BFF6B;'>✅ Información del vehículo actualizada</span>");
    } else {
        m_statusView->setHtml(
            "<span style='color:#FFB347;'>⚠ Conecte ELM327 o active Demo "
            "para ver información del vehículo</span>");
    }
}
