#include "headers/historypanel.hpp"

#include <QHeaderView>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QApplication>
#include <QFileInfo>
#include <QRegularExpression>
#include <QPixmap>
#include <QInputDialog>
#include <QLineEdit>
#include <limits>

// ─── Parameter metadata ─────────────────────────────────────────────────────

struct ParamInfo {
    QString name;
    QString unit;
    double minY;
    double maxY;
    QColor color;
};

static const ParamInfo s_params[] = {
    {"RPM",            "RPM",     0,    8000, QColor(60,  140, 255)},
    {"Velocidad",      "km/h",    0,    260,  QColor(80,  200, 255)},
    {"Temperatura",    "°C",      30,   130,  QColor(255, 160, 60) },
    {"Carga Motor",    "%",       0,    100,  QColor(120, 255, 120)},
    {"Acelerador",     "%",       0,    100,  QColor(255, 180, 40) },
    {"MAF",            "g/s",     0,    200,  QColor(200, 100, 255)},
    {"Sensor O2",      "V",       0,    1.0,  QColor(100, 255, 200)},
    {"STFT",           "%",       -25,  25,   QColor(255, 100, 100)},
    {"LTFT",           "%",       -25,  25,   QColor(255, 200, 100)},
};
constexpr int NUM_PARAMS = sizeof(s_params) / sizeof(s_params[0]);

// ============================================================================
// HistoryPanel implementation
// ============================================================================

HistoryPanel::HistoryPanel(QWidget* parent)
    : QWidget(parent), m_recording(false), m_selectedParam(0), m_visibleRange(60) {
    setupUI();
}

void HistoryPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // ── Title ───────────────────────────────────────────────────────────
    QLabel* title = new QLabel("📜 HISTORIAL Y GRABACIÓN DE SENSORES");
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #60CCFF; padding: 4px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // ── Controls bar ────────────────────────────────────────────────────
    QHBoxLayout* ctrlBar = new QHBoxLayout();
    ctrlBar->setSpacing(8);

    m_btnRecord = new QPushButton("🔴 GRABAR");
    m_btnRecord->setStyleSheet(
        "QPushButton { background-color: #CC3333; color: white; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #DD4444; }");
    m_btnStop = new QPushButton("⏹ DETENER");
    m_btnStop->setEnabled(false);
    m_btnStop->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:enabled { background-color: #8C2D2D; color: white; }"
        "QPushButton:hover:enabled { background-color: #A63A3A; }");
    m_btnClear = new QPushButton("🗑 LIMPIAR");
    m_btnClear->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C5168; }");

    ctrlBar->addWidget(m_btnRecord);
    ctrlBar->addWidget(m_btnStop);
    ctrlBar->addWidget(m_btnClear);
    ctrlBar->addSpacing(10);

    m_btnSave = new QPushButton("💾 GUARDAR");
    m_btnSave->setStyleSheet(
        "QPushButton { background-color: #2D5C8C; color: white; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3A6CA6; }");
    m_btnLoad = new QPushButton("📂 CARGAR");
    m_btnLoad->setStyleSheet(
        "QPushButton { background-color: #5C3C8C; color: white; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #6C4CA6; }");
    m_btnHtmlExport = new QPushButton("📄 HTML");
    m_btnHtmlExport->setStyleSheet(
        "QPushButton { background-color: #2D6C5C; color: white; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3A8C6C; }");

    ctrlBar->addWidget(m_btnSave);
    ctrlBar->addWidget(m_btnLoad);
    ctrlBar->addWidget(m_btnHtmlExport);
    ctrlBar->addStretch();

    m_lblCount = new QLabel("0 pts  |  —");
    m_lblCount->setStyleSheet("color: #8A8FA0; font-size: 12px; padding: 6px;");
    ctrlBar->addWidget(m_lblCount);

    m_lblStatus = new QLabel("");
    m_lblStatus->setStyleSheet("font-weight: bold; padding: 6px;");
    ctrlBar->addWidget(m_lblStatus);

    mainLayout->addLayout(ctrlBar);

    // ── Parameter selector + zoom ───────────────────────────────────────
    QHBoxLayout* viewBar = new QHBoxLayout();

    QLabel* paramLabel = new QLabel("Parámetro:");
    paramLabel->setStyleSheet("color: #B0B8D0; font-size: 11px;");

    m_cmbParam = new QComboBox();
    for (int i = 0; i < NUM_PARAMS; i++) {
        m_cmbParam->addItem(s_params[i].name);
    }
    m_cmbParam->setCurrentIndex(0);
    m_cmbParam->setStyleSheet(
        "QComboBox { background-color: #1E2230; color: #DCE1F0; border: 1px solid #3C4158; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QComboBox::drop-down { border: none; }");

    m_btnZoomIn = new QPushButton("🔍+");
    m_btnZoomIn->setFixedWidth(36);
    m_btnZoomIn->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 4px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C5168; }");
    m_btnZoomOut = new QPushButton("🔍−");
    m_btnZoomOut->setFixedWidth(36);
    m_btnZoomOut->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 4px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C5168; }");

    m_btnAddBookmark = new QPushButton("⭐ MARCADOR");
    m_btnAddBookmark->setEnabled(false);
    m_btnAddBookmark->setStyleSheet(
        "QPushButton { background-color: #8C6C2D; color: white; font-weight: bold; "
        "padding: 6px 14px; border: none; border-radius: 4px; font-size: 11px; }"
        "QPushButton:hover:enabled { background-color: #A68A3A; }"
        "QPushButton:disabled { background-color: #3C4158; color: #6A6F80; }");

    viewBar->addWidget(paramLabel);
    viewBar->addWidget(m_cmbParam);
    viewBar->addSpacing(10);
    viewBar->addWidget(m_btnZoomIn);
    viewBar->addWidget(m_btnZoomOut);
    viewBar->addSpacing(10);
    viewBar->addWidget(m_btnAddBookmark);
    viewBar->addStretch();
    mainLayout->addLayout(viewBar);

    // ── Chart ───────────────────────────────────────────────────────────
    m_chart = new QChart();
    m_chart->setTitle("Seleccione un parámetro y grabe datos");
    m_chart->setTitleBrush(QColor(180, 190, 210));
    m_chart->setBackgroundBrush(QColor(30, 34, 48));
    m_chart->setPlotAreaBackgroundBrush(QColor(20, 22, 32));
    m_chart->setPlotAreaBackgroundVisible(true);
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chart->legend()->hide();
    m_chart->setMargins(QMargins(5, 5, 5, 5));

    m_axisX = new QDateTimeAxis();
    m_axisX->setLabelsColor(QColor(120, 125, 140));
    m_axisX->setGridLineColor(QColor(40, 45, 60));
    m_axisX->setFormat("HH:mm:ss");
    m_axisX->setTitleText("Tiempo");
    m_axisX->setTitleBrush(QColor(180, 190, 210));

    m_axisY = new QValueAxis();
    m_axisY->setLabelsColor(QColor(120, 125, 140));
    m_axisY->setGridLineColor(QColor(40, 45, 60));
    m_axisY->setRange(0, 8000);
    m_axisY->setTitleText("RPM");
    m_axisY->setTitleBrush(QColor(180, 190, 210));

    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_series = new QLineSeries();
    m_series->setColor(QColor(60, 140, 255));
    m_series->setPen(QPen(QColor(60, 140, 255), 1.5));
    m_chart->addSeries(m_series);
    m_series->attachAxis(m_axisX);
    m_series->attachAxis(m_axisY);

    // Bookmark scatter series (circle markers with bold border on the chart)
    m_bookmarkSeries = new QScatterSeries();
    m_bookmarkSeries->setMarkerSize(12);
    m_bookmarkSeries->setColor(QColor(255, 200, 60));
    m_bookmarkSeries->setBorderColor(QColor(200, 150, 30));
    m_bookmarkSeries->setPen(QPen(QColor(180, 130, 20), 2));
    m_chart->addSeries(m_bookmarkSeries);
    m_bookmarkSeries->attachAxis(m_axisX);
    m_bookmarkSeries->attachAxis(m_axisY);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setBackgroundBrush(QColor(30, 34, 48));
    m_chartView->setMinimumHeight(250);
    mainLayout->addWidget(m_chartView, 2);

    // ── Bookmark list (compact horizontal bar) ─────────────────────────
    QHBoxLayout* bmBar = new QHBoxLayout();
    bmBar->setSpacing(6);
    QLabel* bmLabel = new QLabel("⭐ Marcadores:");
    bmLabel->setStyleSheet("color: #FFD060; font-size: 11px; font-weight: bold;");
    m_bookmarkList = new QListWidget();
    m_bookmarkList->setMaximumHeight(60);
    m_bookmarkList->setStyleSheet(
        "QListWidget { background-color: #1A1D2A; color: #DCE1F0; "
        "border: 1px solid #2A2E3E; border-radius: 4px; font-size: 11px; }"
        "QListWidget::item { padding: 2px 6px; }"
        "QListWidget::item:hover { background-color: #2A2E3E; }"
        "QListWidget::item:selected { background-color: #3A4A3A; }");
    QPushButton* btnRemoveBookmark = new QPushButton("✕");
    btnRemoveBookmark->setFixedSize(26, 26);
    btnRemoveBookmark->setToolTip("Eliminar marcador seleccionado");
    btnRemoveBookmark->setStyleSheet(
        "QPushButton { background-color: #5C2D2D; color: white; font-weight: bold; "
        "padding: 2px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #7A3A3A; }");

    bmBar->addWidget(bmLabel);
    bmBar->addWidget(m_bookmarkList, 1);
    bmBar->addWidget(btnRemoveBookmark);
    mainLayout->addLayout(bmBar);

    // ── Splitter: table + stats ─────────────────────────────────────────
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(10);

    // Data table
    QGroupBox* tableGroup = new QGroupBox("Datos Registrados");
    tableGroup->setStyleSheet(
        "QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 5px; margin-top: 8px; padding-top: 14px; }");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableGroup);

    m_dataTable = new QTableWidget(0, 10);
    m_dataTable->setHorizontalHeaderLabels(
        {"Hora", "RPM", "Vel.", "Temp.", "Carga", "Acel.", "MAF", "O2", "STFT", "LTFT"});
    m_dataTable->setAlternatingRowColors(true);
    m_dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_dataTable->setStyleSheet(
        "QTableWidget { background-color: #1A1D2A; color: #DCE1F0; "
        "gridline-color: #2A2E3E; border: none; font-size: 10px; }"
        "QTableWidget::item { padding: 2px 4px; }"
        "QHeaderView::section { background-color: #2A2E3E; color: #B0B8D0; "
        "padding: 4px; border: none; font-weight: bold; font-size: 10px; }"
        "QTableWidget::item:alternate { background-color: #222638; }");
    m_dataTable->setColumnWidth(0, 70);
    for (int c = 1; c < 10; c++) m_dataTable->setColumnWidth(c, 55);

    tableLayout->addWidget(m_dataTable);
    bottomLayout->addWidget(tableGroup, 3);

    // Statistics
    QGroupBox* statsGroup = new QGroupBox("Estadísticas");
    statsGroup->setStyleSheet(
        "QGroupBox { color: #90EE90; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 5px; margin-top: 8px; padding-top: 14px; }");
    QVBoxLayout* statsLayout = new QVBoxLayout(statsGroup);

    m_statsView = new QTextEdit();
    m_statsView->setReadOnly(true);
    m_statsView->setStyleSheet(
        "QTextEdit { background-color: #0D0F17; color: #B0B8D0; "
        "border: none; font-family: monospace; font-size: 10px; padding: 4px; }");
    m_statsView->setMinimumWidth(180);
    statsLayout->addWidget(m_statsView);

    bottomLayout->addWidget(statsGroup, 1);

    mainLayout->addLayout(bottomLayout, 1);

    // ── Connect signals ─────────────────────────────────────────────────
    connect(m_btnRecord, &QPushButton::clicked, this, &HistoryPanel::startRecording);
    connect(m_btnStop, &QPushButton::clicked, this, &HistoryPanel::stopRecording);
    connect(m_btnClear, &QPushButton::clicked, this, &HistoryPanel::clearHistory);
    connect(m_btnSave, &QPushButton::clicked, this, &HistoryPanel::saveSession);
    connect(m_btnLoad, &QPushButton::clicked, this, &HistoryPanel::loadSession);
    connect(m_btnHtmlExport, &QPushButton::clicked, this, &HistoryPanel::exportHTML);
    connect(m_btnZoomIn, &QPushButton::clicked, this, &HistoryPanel::onZoomIn);
    connect(m_btnZoomOut, &QPushButton::clicked, this, &HistoryPanel::onZoomOut);
    connect(m_btnAddBookmark, &QPushButton::clicked, this, &HistoryPanel::addBookmark);
    connect(m_bookmarkList, &QListWidget::itemClicked, this, &HistoryPanel::onBookmarkClicked);
    connect(btnRemoveBookmark, &QPushButton::clicked, this, [this]() {
        int row = m_bookmarkList->currentRow();
        if (row >= 0 && row < static_cast<int>(m_bookmarks.size())) {
            m_bookmarks.erase(m_bookmarks.begin() + row);
            updateBookmarkList();
            rebuildChart();
        }
    });
    connect(m_cmbParam, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &HistoryPanel::onParamChanged);

    // Initial state
    updateStats();
}

// ─── Add data point (called from MainWindow timer tick) ─────────────────────

void HistoryPanel::addDataPoint(double rpm, double speed, double coolant,
                                 double load, double throttle, double maf,
                                 double o2v, double stft, double ltft) {
    if (!m_recording) return;

    HistoryPoint pt;
    pt.timestamp = QDateTime::currentDateTime();
    pt.rpm = rpm;
    pt.speed = speed;
    pt.coolant = coolant;
    pt.load = load;
    pt.throttle = throttle;
    pt.maf = maf;
    pt.o2v = o2v;
    pt.stft = stft;
    pt.ltft = ltft;

    m_data.push_back(pt);

    // Update chart incrementally
    double val = getParam(m_selectedParam, pt);
    qint64 ts = pt.timestamp.toMSecsSinceEpoch();

    m_series->append(ts, val);

    // Trim visible range
    if (!m_data.empty()) {
        qint64 cutoff = m_data.back().timestamp.toMSecsSinceEpoch() - m_visibleRange * 1000;
        while (m_series->count() > 0 && m_series->at(0).x() < cutoff) {
            m_series->remove(0);
        }
    }

    // Auto-scroll X axis
    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(ts - m_visibleRange * 1000),
                      QDateTime::fromMSecsSinceEpoch(ts + 2000));

    // Update counter
    m_lblCount->setText(QString("%1 pts  |  %2 seg")
                        .arg(m_data.size())
                        .arg(QString::number(m_visibleRange)));

    // Periodically update table & stats (every 20 points)
    if (m_data.size() % 20 == 0) {
        updateTable();
        updateStats();
    }
}

// ─── Recording controls ─────────────────────────────────────────────────────

void HistoryPanel::startRecording() {
    if (m_recording) return;

    // If data exists, ask if user wants to clear
    if (!m_data.empty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Nueva grabación",
            "¿Limpiar los datos actuales antes de grabar?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) return;
        if (reply == QMessageBox::Yes) {
            clearHistory();
        }
    }

    m_recording = true;
    m_btnRecord->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_btnClear->setEnabled(false);
    m_btnSave->setEnabled(false);
    m_btnLoad->setEnabled(false);
    m_btnAddBookmark->setEnabled(true);

    m_lblStatus->setText("🔴 GRABANDO...");
    m_lblStatus->setStyleSheet("color: #FF4444; font-weight: bold; padding: 6px;");

    m_chart->setTitle("Grabando... — " + s_params[m_selectedParam].name);
}

void HistoryPanel::stopRecording() {
    if (!m_recording) return;

    m_recording = false;
    m_btnRecord->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_btnClear->setEnabled(true);
    m_btnSave->setEnabled(true);
    m_btnLoad->setEnabled(true);
    m_btnAddBookmark->setEnabled(false);

    m_lblStatus->setText(QString("⏹ Grabado: %1 puntos").arg(m_data.size()));
    m_lblStatus->setStyleSheet("color: #FFB347; font-weight: bold; padding: 6px;");

    m_chart->setTitle(QString("📊 %1 — %2 puntos · %3")
                      .arg(s_params[m_selectedParam].name)
                      .arg(m_data.size())
                      .arg(m_data.empty() ? "—" :
                           m_data.front().timestamp.toString("HH:mm:ss") + " – " +
                           m_data.back().timestamp.toString("HH:mm:ss")));

    // Full table and stats update
    updateTable();
    updateStats();
}

void HistoryPanel::clearHistory() {
    m_data.clear();
    m_bookmarks.clear();
    m_series->clear();
    m_bookmarkSeries->clear();

    m_chart->setTitle("Seleccione un parámetro y grabe datos");
    m_lblCount->setText("0 pts  |  —");

    m_dataTable->setRowCount(0);
    updateBookmarkList();
    updateStats();

    m_lblStatus->setText("");
    m_btnSave->setEnabled(false);
}

// ─── Save/Load ──────────────────────────────────────────────────────────────

void HistoryPanel::saveSession() {
    if (m_data.empty()) {
        QMessageBox::information(this, "Guardar", "No hay datos para guardar.");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(
        this, "Guardar historial",
        "historial/session_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".csv",
        "CSV (*.csv);;Todos los archivos (*)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir el archivo:\n" + file.errorString());
        return;
    }

    QTextStream out(&file);
    // Header
    out << "# OBD-II History v1.1\n";
    out << "# Date: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    out << "# Points: " << m_data.size() << "\n";
    out << "# Bookmarks: " << m_bookmarks.size() << "\n";
    for (const auto& bm : m_bookmarks) {
        out << "# bookmark: " << bm.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << "|" << bm.label << "\n";
    }
    out << "timestamp,rpm,speed,coolant,load,throttle,maf,o2_voltage,stft,ltft\n";

    for (const auto& p : m_data) {
        out << p.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz") << ","
            << QString::number(p.rpm, 'f', 1) << ","
            << QString::number(p.speed, 'f', 1) << ","
            << QString::number(p.coolant, 'f', 1) << ","
            << QString::number(p.load, 'f', 1) << ","
            << QString::number(p.throttle, 'f', 1) << ","
            << QString::number(p.maf, 'f', 2) << ","
            << QString::number(p.o2v, 'f', 3) << ","
            << QString::number(p.stft, 'f', 1) << ","
            << QString::number(p.ltft, 'f', 1) << "\n";
    }

    file.close();
    m_lblStatus->setText("✅ Guardado: " + filename.section('/', -1));
    m_lblStatus->setStyleSheet("color: #6BFF6B; font-weight: bold; padding: 6px;");
}

void HistoryPanel::loadSession() {
    QString filename = QFileDialog::getOpenFileName(
        this, "Cargar historial",
        "historial/",
        "CSV (*.csv);;Todos los archivos (*)");

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir el archivo:\n" + file.errorString());
        return;
    }

    // Ask about current data
    if (!m_data.empty()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Cargar historial",
            "¿Reemplazar los datos actuales con los del archivo?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            file.close();
            return;
        }
    }

    clearHistory();

    QTextStream in(&file);
    int lineNum = 0;
    int loaded = 0;
    int errors = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNum++;

        if (line.isEmpty()) continue;

        // Parse bookmark lines: # bookmark: timestamp|label
        if (line.startsWith("# bookmark: ")) {
            QString data = line.mid(12).trimmed();
            int sep = data.indexOf('|');
            if (sep > 0) {
                Bookmark bm;
                bm.timestamp = QDateTime::fromString(data.left(sep), "yyyy-MM-dd HH:mm:ss.zzz");
                if (!bm.timestamp.isValid()) {
                    bm.timestamp = QDateTime::fromString(data.left(sep), Qt::ISODate);
                }
                if (bm.timestamp.isValid()) {
                    bm.label = data.mid(sep + 1);
                    bm.color = QColor(255, 200, 60);
                    m_bookmarks.push_back(bm);
                }
            }
            continue;
        }

        if (line.startsWith('#')) continue;
        if (line.startsWith("timestamp")) continue;  // header

        QStringList parts = line.split(',');
        if (parts.size() < 10) {
            errors++;
            continue;
        }

        HistoryPoint pt;
        pt.timestamp = QDateTime::fromString(parts[0], "yyyy-MM-dd HH:mm:ss.zzz");
        if (!pt.timestamp.isValid()) {
            pt.timestamp = QDateTime::fromString(parts[0], Qt::ISODate);
        }
        if (!pt.timestamp.isValid()) {
            errors++;
            continue;
        }

        pt.rpm = parts[1].toDouble();
        pt.speed = parts[2].toDouble();
        pt.coolant = parts[3].toDouble();
        pt.load = parts[4].toDouble();
        pt.throttle = parts[5].toDouble();
        pt.maf = parts[6].toDouble();
        pt.o2v = parts[7].toDouble();
        pt.stft = parts[8].toDouble();
        pt.ltft = parts[9].toDouble();

        m_data.push_back(pt);
        loaded++;
    }

    file.close();

    m_lblStatus->setText(QString("📂 Cargados %1 puntos, %2 marcadores (%3 errores)")
                         .arg(loaded).arg(m_bookmarks.size()).arg(errors));
    m_lblStatus->setStyleSheet("color: #6BFF6B; font-weight: bold; padding: 6px;");

    updateBookmarkList();
    rebuildChart();
    updateTable();
    updateStats();
    m_btnSave->setEnabled(true);
}

// ─── Export to HTML with embedded chart PNG ───────────────────────────────

void HistoryPanel::exportHTML() {
    if (m_data.empty()) {
        QMessageBox::information(this, "Exportar HTML", "No hay datos para exportar.");
        return;
    }

    QString baseName = QFileDialog::getSaveFileName(
        this, "Exportar historial a HTML",
        "historial/reporte_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".html",
        "HTML (*.html);;Todos los archivos (*)");

    if (baseName.isEmpty()) return;

    // Capture chart as PNG (native resolution, no side effects)
    QPixmap chartPixmap = m_chartView->grab();

    QString pngPath = baseName;
    pngPath.replace(QRegularExpression("\.html$", QRegularExpression::CaseInsensitiveOption), ".png");
    if (!chartPixmap.save(pngPath, "PNG")) {
        QMessageBox::warning(this, "Error", "No se pudo guardar la imagen del gráfico:\n" + pngPath);
        return;
    }

    // Calculate stats for the full report
    double minV[NUM_PARAMS], maxV[NUM_PARAMS], sumV[NUM_PARAMS];
    for (int i = 0; i < NUM_PARAMS; i++) {
        minV[i] = 1e18; maxV[i] = -1e18; sumV[i] = 0;
    }
    for (const auto& p : m_data) {
        for (int i = 0; i < NUM_PARAMS; i++) {
            double v = getParam(i, p);
            if (v < minV[i]) minV[i] = v;
            if (v > maxV[i]) maxV[i] = v;
            sumV[i] += v;
        }
    }
    double n = static_cast<double>(m_data.size());

    qint64 durSec = m_data.front().timestamp.secsTo(m_data.back().timestamp);
    QString durStr = QString("%1h %2m %3s")
        .arg(durSec / 3600).arg((durSec % 3600) / 60).arg(durSec % 60);

    // Build the HTML document
    QString html;
    html += "<!DOCTYPE html>\n<html lang='es'>\n<head>\n";
    html += "<meta charset='utf-8'>\n";
    html += "<title>Reporte Historial OBD-II</title>\n";
    html += "<style>\n";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }\n";
    html += "body { background: #13151F; color: #C8CDE0; font-family: 'Segoe UI', sans-serif; padding: 30px; }\n";
    html += ".header { text-align: center; padding: 20px; background: #1A1D2A; border-radius: 10px; margin-bottom: 24px; border: 1px solid #2A2E3E; }\n";
    html += "h1 { color: #60CCFF; font-size: 24px; margin-bottom: 6px; }\n";
    html += ".subtitle { color: #6A6F80; font-size: 13px; }\n";
    html += ".chart-wrap { text-align: center; margin-bottom: 24px; }\n";
    html += ".chart-wrap img { max-width: 100%; border-radius: 8px; border: 1px solid #2A2E3E; box-shadow: 0 4px 12px rgba(0,0,0,0.4); }\n";
    html += "h2 { color: #FFB347; font-size: 18px; margin: 20px 0 10px; padding-bottom: 6px; border-bottom: 1px solid #2A2E3E; }\n";
    html += "table { width: 100%; border-collapse: collapse; margin-bottom: 20px; font-size: 13px; }\n";
    html += "th { background: #2A2E3E; color: #B0B8D0; padding: 8px 12px; text-align: left; font-weight: bold; }\n";
    html += "td { padding: 6px 12px; border-bottom: 1px solid #1E2230; }\n";
    html += "tr:hover { background: #1E2230; }\n";
    html += ".stat-table td:first-child { font-weight: bold; }\n";
    html += ".stat-table td:nth-child(2) { color: #8A8FA0; text-align: right; }\n";
    html += ".stat-table td:nth-child(3) { color: #DCE1F0; text-align: right; font-weight: bold; }\n";
    html += ".stat-table td:nth-child(4) { color: #6A6F80; text-align: right; }\n";
    html += ".data-table { font-size: 11px; }\n";
    html += ".data-table td { font-family: 'Consolas', monospace; }\n";
    html += ".footer { text-align: center; color: #4A4F60; font-size: 11px; margin-top: 30px; padding-top: 15px; border-top: 1px solid #1E2230; }\n";
    html += ".badge { display: inline-block; padding: 2px 10px; border-radius: 4px; font-size: 11px; margin: 0 4px; }\n";
    html += ".badge-pts { background: #1A3A50; color: #60CCFF; }\n";
    html += ".badge-dur { background: #3A3020; color: #FFB347; }\n";
    html += ".badge-param { background: #2A3A20; color: #90EE90; }\n";
    html += "</style>\n</head>\n<body>\n";

    // Header
    html += "<div class='header'>\n";
    html += "<h1>📊 Reporte de Historial OBD-II</h1>\n";
    html += "<p class='subtitle'>Generado: "
            + QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") + "</p>\n";
    html += "<p class='subtitle'>";
    html += "<span class='badge badge-pts'>" + QString::number(m_data.size()) + " puntos</span>";
    html += "<span class='badge badge-dur'>" + durStr + "</span>";
    html += "<span class='badge badge-param'>" + s_params[m_selectedParam].name + "</span>";
    html += "</p>\n";
    html += "</div>\n";

    // Embedded chart image
    // Use relative path (just the filename) since HTML and PNG are in the same dir
    QString pngRel = QFileInfo(pngPath).fileName();
    html += "<div class='chart-wrap'>\n";
    html += "<img src='" + pngRel + "' alt='Gráfico "
            + s_params[m_selectedParam].name + "' loading='lazy'>\n";
    html += "</div>\n";

    // Bookmarks section
    if (!m_bookmarks.empty()) {
        html += "<h2>⭐ Marcadores</h2>\n";
        html += "<table class='stat-table'>\n";
        html += "<tr><th>#</th><th>Hora</th><th>Descripción</th></tr>\n";
        for (size_t i = 0; i < m_bookmarks.size(); i++) {
            const auto& bm = m_bookmarks[i];
            html += "<tr>";
            html += "<td style='color:" + bm.color.name() + ";'>" + QString::number(i + 1) + "</td>";
            html += "<td>" + bm.timestamp.toString("HH:mm:ss.zzz") + "</td>";
            html += "<td>" + bm.label.toHtmlEscaped() + "</td>";
            html += "</tr>\n";
        }
        html += "</table>\n";
    }

    // Statistics table
    html += "<h2>📈 Estadísticas por Sensor</h2>\n";
    html += "<table class='stat-table'>\n";
    html += "<tr><th>Sensor</th><th>Mínimo</th><th>Máximo</th><th>Promedio</th></tr>\n";

    for (int i = 0; i < NUM_PARAMS; i++) {
        html += "<tr>";
        html += "<td style='color:" + paramColor(i).name() + ";'>" + s_params[i].name + "</td>";
        html += "<td>" + QString::number(minV[i], 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1) + " " + s_params[i].unit + "</td>";
        html += "<td>" + QString::number(maxV[i], 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1) + " " + s_params[i].unit + "</td>";
        html += "<td>" + QString::number(sumV[i] / n, 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1) + " " + s_params[i].unit + "</td>";
        html += "</tr>\n";
    }
    html += "</table>\n";

    // Data table (first 100 rows to keep file size reasonable)
    html += "<h2>📋 Datos Registrados</h2>\n";
    html += "<p style='color:#6A6F80;font-size:12px;margin-bottom:8px;'>";
    int showRows = qMin(static_cast<int>(m_data.size()), 100);
    if (m_data.size() > 100) {
        html += "Mostrando las primeras 100 filas de " + QString::number(m_data.size()) + " total.</p>\n";
    } else {
        html += "Total: " + QString::number(m_data.size()) + " filas.</p>\n";
    }
    html += "<table class='data-table'>\n";
    html += "<tr><th>#</th><th>Hora</th><th>RPM</th><th>Vel.</th><th>Temp.</th>"
            "<th>Carga</th><th>Acel.</th><th>MAF</th><th>O₂</th><th>STFT</th><th>LTFT</th></tr>\n";

    for (int i = 0; i < showRows; i++) {
        const auto& p = m_data[i];
        auto cell = [&](const QString& v, const QColor& c) -> QString {
            return "<td style='color:" + c.name() + ";'>" + v + "</td>";
        };
        html += "<tr><td style='color:#4A4F60;'>" + QString::number(i + 1) + "</td>";
        html += "<td>" + p.timestamp.toString("HH:mm:ss.zzz") + "</td>";
        html += cell(QString::number(static_cast<int>(p.rpm)), paramColor(0));
        html += cell(QString::number(static_cast<int>(p.speed)), paramColor(1));
        html += cell(QString::number(static_cast<int>(p.coolant)), paramColor(2));
        html += cell(QString::number(static_cast<int>(p.load)), paramColor(3));
        html += cell(QString::number(p.throttle, 'f', 1), paramColor(4));
        html += cell(QString::number(p.maf, 'f', 1), paramColor(5));
        html += cell(QString::number(p.o2v, 'f', 3), paramColor(6));
        html += cell(QString::number(p.stft, 'f', 1), paramColor(7));
        html += cell(QString::number(p.ltft, 'f', 1), paramColor(8));
        html += "</tr>\n";
    }
    html += "</table>\n";

    // Footer
    html += "<div class='footer'>\n";
    html += "<p>Freebuff OBD-II Scanner v8.0 · Historial generado por la aplicación</p>\n";
    html += "<p>Datos: " + m_data.front().timestamp.toString("yyyy-MM-dd HH:mm")
            + " — " + m_data.back().timestamp.toString("HH:mm") + "</p>\n";
    html += "</div>\n";

    html += "</body>\n</html>\n";

    // Write the HTML file
    QFile file(baseName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo guardar el archivo HTML:\n" + file.errorString());
        return;
    }
    QTextStream out(&file);
    out << html;
    file.close();

    m_lblStatus->setText("📄 HTML exportado: " + QFileInfo(baseName).fileName());
    m_lblStatus->setStyleSheet("color: #6BFF6B; font-weight: bold; padding: 6px;");
}

// ─── Bookmark methods ──────────────────────────────────────────────────────

void HistoryPanel::addBookmark() {
    if (!m_recording || m_data.empty()) return;

    // Get the latest data point timestamp as bookmark position
    QDateTime now = m_data.back().timestamp;

    // Ask for a label
    bool ok = false;
    QString label = QInputDialog::getText(
        this, "⭐ Nuevo marcador",
        "Descripción del evento:",
        QLineEdit::Normal,
        "Evento " + QString::number(m_bookmarks.size() + 1),
        &ok);

    if (!ok || label.trimmed().isEmpty()) return;

    Bookmark bm;
    bm.timestamp = now;
    bm.label = label.trimmed();
    // Cycle through colors for visual variety
    static const QColor colors[] = {
        QColor(255, 200, 60),   // gold
        QColor(255, 100, 100),  // red
        QColor(100, 200, 255),  // cyan
        QColor(120, 255, 120),  // green
        QColor(255, 150, 200),  // pink
    };
    bm.color = colors[m_bookmarks.size() % 5];

    m_bookmarks.push_back(bm);

    updateBookmarkList();
    updateBookmarkMarkers();

    m_lblStatus->setText(QString("⭐ Marcador: %1").arg(bm.label));
    m_lblStatus->setStyleSheet("color: #FFD060; font-weight: bold; padding: 6px;");
}

void HistoryPanel::onBookmarkClicked(QListWidgetItem* item) {
    if (!item) return;
    int row = m_bookmarkList->row(item);
    if (row < 0 || row >= static_cast<int>(m_bookmarks.size())) return;

    const Bookmark& bm = m_bookmarks[row];
    qint64 ts = bm.timestamp.toMSecsSinceEpoch();
    qint64 halfRange = (m_visibleRange * 1000) / 2;

    // Center chart on the bookmark timestamp
    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(ts - halfRange),
                      QDateTime::fromMSecsSinceEpoch(ts + halfRange));

    m_lblStatus->setText(QString("📍 Ir a: %1 — %2")
                         .arg(bm.timestamp.toString("HH:mm:ss"))
                         .arg(bm.label));
    m_lblStatus->setStyleSheet("color: #FFD060; font-weight: bold; padding: 6px;");
}

void HistoryPanel::updateBookmarkMarkers() {
    m_bookmarkSeries->clear();

    for (const auto& bm : m_bookmarks) {
        double val = getParam(m_selectedParam, bm);
        qint64 ts = bm.timestamp.toMSecsSinceEpoch();
        m_bookmarkSeries->append(ts, val);
    }
}

void HistoryPanel::updateBookmarkList() {
    m_bookmarkList->clear();

    for (size_t i = 0; i < m_bookmarks.size(); i++) {
        const auto& bm = m_bookmarks[i];
        QString text = bm.timestamp.toString("HH:mm:ss") + "  " + bm.label;

        QListWidgetItem* item = new QListWidgetItem(text);
        item->setForeground(bm.color);
        m_bookmarkList->addItem(item);
    }

    if (!m_bookmarks.empty()) {
        m_bookmarkList->scrollToBottom();
    }
}

// Helper: get parameter value by index from a Bookmark (interpolates from m_data)
double HistoryPanel::getParam(int idx, const Bookmark& bm) const {
    // Find the closest data point to the bookmark timestamp
    qint64 bmTs = bm.timestamp.toMSecsSinceEpoch();
    qint64 bestDelta = std::numeric_limits<qint64>::max();
    double bestVal = 0;

    for (const auto& p : m_data) {
        qint64 delta = std::abs(p.timestamp.toMSecsSinceEpoch() - bmTs);
        if (delta < bestDelta) {
            bestDelta = delta;
            bestVal = getParam(idx, p);
        }
    }
    return bestVal;
}

// ─── Chart helpers ──────────────────────────────────────────────────────────

void HistoryPanel::rebuildChart() {
    m_series->clear();

    if (m_data.empty()) {
        m_chart->setTitle("Sin datos — Cargue o grabe una sesión");
        return;
    }

    m_chart->setTitle(QString("📊 %1 — %2 puntos")
                      .arg(s_params[m_selectedParam].name)
                      .arg(m_data.size()));

    // Update axis ranges
    m_axisY->setRange(paramMin(m_selectedParam), paramMax(m_selectedParam));
    m_axisY->setTitleText(s_params[m_selectedParam].name + " (" +
                          s_params[m_selectedParam].unit + ")");

    // Determine visible range
    qint64 startTs = m_data.front().timestamp.toMSecsSinceEpoch();
    qint64 endTs = m_data.back().timestamp.toMSecsSinceEpoch();
    qint64 duration = endTs - startTs;

    if (duration > m_visibleRange * 1000) {
        startTs = endTs - m_visibleRange * 1000;
    }

    m_axisX->setRange(QDateTime::fromMSecsSinceEpoch(startTs),
                      QDateTime::fromMSecsSinceEpoch(endTs + 1000));

    // Fill series
    for (const auto& p : m_data) {
        double val = getParam(m_selectedParam, p);
        m_series->append(p.timestamp.toMSecsSinceEpoch(), val);
    }

    m_lblCount->setText(QString("%1 pts  |  %2 seg · %3")
                        .arg(m_data.size())
                        .arg(QString::number(m_visibleRange))
                        .arg(s_params[m_selectedParam].name));

    // Overlay bookmark markers
    updateBookmarkMarkers();
}

void HistoryPanel::onParamChanged(int idx) {
    if (idx < 0 || idx >= NUM_PARAMS) return;
    m_selectedParam = idx;

    m_series->setColor(paramColor(idx));
    QPen pen(paramColor(idx), 1.5);
    m_series->setPen(pen);

    m_axisY->setRange(paramMin(idx), paramMax(idx));
    m_axisY->setTitleText(s_params[idx].name + " (" + s_params[idx].unit + ")");

    rebuildChart();
}

void HistoryPanel::onZoomIn() {
    if (m_visibleRange <= 10) return;
    m_visibleRange = qMax(10, m_visibleRange / 2);
    rebuildChart();
}

void HistoryPanel::onZoomOut() {
    if (m_visibleRange >= 3600) return;
    m_visibleRange = qMin(3600, m_visibleRange * 2);
    rebuildChart();
}

// ─── Table & Stats ──────────────────────────────────────────────────────────

void HistoryPanel::updateTable() {
    int maxRows = qMin(static_cast<int>(m_data.size()), 5000);
    m_dataTable->setRowCount(maxRows);

    for (int i = 0; i < maxRows; i++) {
        size_t idx = i;
        if (m_data.size() > 5000) {
            // Show only the last 5000 points
            idx = m_data.size() - 5000 + i;
        }

        const auto& p = m_data[idx];
        auto setItem = [this](int r, int c, const QString& text,
                              const QColor& color = QColor(220, 225, 240)) {
            QTableWidgetItem* item = new QTableWidgetItem(text);
            item->setForeground(color);
            item->setTextAlignment(Qt::AlignCenter);
            m_dataTable->setItem(r, c, item);
        };

        setItem(i, 0, p.timestamp.toString("HH:mm:ss.zzz"));
        setItem(i, 1, QString::number(static_cast<int>(p.rpm)), paramColor(0));
        setItem(i, 2, QString::number(static_cast<int>(p.speed)), paramColor(1));
        setItem(i, 3, QString::number(static_cast<int>(p.coolant)), paramColor(2));
        setItem(i, 4, QString::number(static_cast<int>(p.load)), paramColor(3));
        setItem(i, 5, QString::number(p.throttle, 'f', 1), paramColor(4));
        setItem(i, 6, QString::number(p.maf, 'f', 1), paramColor(5));
        setItem(i, 7, QString::number(p.o2v, 'f', 3), paramColor(6));
        setItem(i, 8, QString::number(p.stft, 'f', 1), paramColor(7));
        setItem(i, 9, QString::number(p.ltft, 'f', 1), paramColor(8));
    }

    // Scroll to bottom
    m_dataTable->verticalScrollBar()->setValue(
        m_dataTable->verticalScrollBar()->maximum());
}

void HistoryPanel::updateStats() {
    if (m_data.empty()) {
        m_statsView->setHtml("<span style='color:#8A8FA0;'>Sin datos grabados</span>");
        return;
    }

    // Calculate min/max/avg for each parameter
    double minV[NUM_PARAMS], maxV[NUM_PARAMS], sumV[NUM_PARAMS];
    for (int i = 0; i < NUM_PARAMS; i++) {
        minV[i] = 1e18;
        maxV[i] = -1e18;
        sumV[i] = 0;
    }

    for (const auto& p : m_data) {
        for (int i = 0; i < NUM_PARAMS; i++) {
            double v = getParam(i, p);
            if (v < minV[i]) minV[i] = v;
            if (v > maxV[i]) maxV[i] = v;
            sumV[i] += v;
        }
    }

    double n = static_cast<double>(m_data.size());

    QString html;
    html += "<table style='width:100%; border-collapse:collapse;'>";
    html += "<tr style='color:#8A8FA0;font-size:9px;'>"
            "<th>Sensor</th><th>Mín</th><th>Máx</th><th>Prom</th></tr>";

    for (int i = 0; i < NUM_PARAMS; i++) {
        QColor c = paramColor(i);
        html += QString("<tr>"
                        "<td style='color:%1;'>%2</td>"
                        "<td style='color:#B0B8D0;text-align:right;'>%3</td>"
                        "<td style='color:#DCE1F0;text-align:right;font-weight:bold;'>%4</td>"
                        "<td style='color:#8A8FA0;text-align:right;'>%5</td>"
                        "</tr>")
            .arg(c.name(), s_params[i].name)
            .arg(QString::number(minV[i], 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1))
            .arg(QString::number(maxV[i], 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1))
            .arg(QString::number(sumV[i] / n, 'f', i == 6 ? 3 : (i == 0 || i == 1) ? 0 : 1));
    }

    html += "</table>";

    // Duration
    qint64 durSec = m_data.front().timestamp.secsTo(m_data.back().timestamp);
    html += QString("<br><span style='color:#6A6F80;font-size:10px;'>"
                    "Duración: %1:%2:%3 · %4 muestras</span>")
        .arg(durSec / 3600, 2, 10, QChar('0'))
        .arg((durSec % 3600) / 60, 2, 10, QChar('0'))
        .arg(durSec % 60, 2, 10, QChar('0'))
        .arg(m_data.size());

    m_statsView->setHtml(html);
}

// ─── Parameter helpers ──────────────────────────────────────────────────────

double HistoryPanel::getParam(int idx, const HistoryPoint& p) const {
    switch (idx) {
    case 0: return p.rpm;
    case 1: return p.speed;
    case 2: return p.coolant;
    case 3: return p.load;
    case 4: return p.throttle;
    case 5: return p.maf;
    case 6: return p.o2v;
    case 7: return p.stft;
    case 8: return p.ltft;
    default: return 0;
    }
}

QString HistoryPanel::paramName(int idx) const {
    if (idx < 0 || idx >= NUM_PARAMS) return "—";
    return s_params[idx].name;
}

QColor HistoryPanel::paramColor(int idx) const {
    if (idx < 0 || idx >= NUM_PARAMS) return QColor(180, 190, 210);
    return s_params[idx].color;
}

double HistoryPanel::paramMin(int idx) const {
    if (idx < 0 || idx >= NUM_PARAMS) return 0;
    return s_params[idx].minY;
}

double HistoryPanel::paramMax(int idx) const {
    if (idx < 0 || idx >= NUM_PARAMS) return 100;
    return s_params[idx].maxY;
}
