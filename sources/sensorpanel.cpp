#include "headers/sensorpanel.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QGroupBox>

SensorPanel::SensorPanel(ELM327* elm, QWidget* parent)
    : QWidget(parent), m_elm(elm), m_pollInterval(500),
      m_selectedSensor(0), m_counter(0) {

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Title
    QLabel* title = new QLabel("📈 SENSORES EN TIEMPO REAL - GRÁFICOS");
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #60CCFF; padding: 4px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Controls bar
    QHBoxLayout* controlBar = new QHBoxLayout();

    m_btnStart = new QPushButton("▶ INICIAR");
    m_btnStart->setStyleSheet(
        "QPushButton { background-color: #2D8C2D; color: white; font-weight: bold; "
        "padding: 6px 16px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3AA63A; }"
    );

    m_btnStop = new QPushButton("■ DETENER");
    m_btnStop->setEnabled(false);
    m_btnStop->setStyleSheet(
        "QPushButton { background-color: #8C2D2D; color: white; font-weight: bold; "
        "padding: 6px 16px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #A63A3A; }"
        "QPushButton:disabled { background-color: #3C4158; color: #6A6F80; }"
    );

    QLabel* intervalLabel = new QLabel("Intervalo:");
    intervalLabel->setStyleSheet("color: #B0B8D0;");

    m_cmbInterval = new QComboBox();
    m_cmbInterval->addItems({"250 ms", "500 ms", "1 s", "2 s", "5 s"});
    m_cmbInterval->setCurrentIndex(1);
    m_cmbInterval->setStyleSheet(
        "QComboBox { background-color: #1E2230; color: #DCE1F0; border: 1px solid #3C4158; "
        "border-radius: 4px; padding: 4px 8px; }"
        "QComboBox::drop-down { border: none; }"
        "QComboBox::down-arrow { image: none; border-left: 4px solid transparent; "
        "border-right: 4px solid transparent; border-top: 6px solid #DCE1F0; margin-right: 6px; }"
    );

    controlBar->addWidget(m_btnStart);
    controlBar->addWidget(m_btnStop);
    controlBar->addSpacing(20);
    controlBar->addWidget(intervalLabel);
    controlBar->addWidget(m_cmbInterval);
    controlBar->addStretch();
    mainLayout->addLayout(controlBar);

    // Charts in a scrollable area
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setStyleSheet("background-color: transparent; border: none;");
    scrollArea->setWidgetResizable(true);

    QWidget* chartContainer = new QWidget();
    QVBoxLayout* chartLayout = new QVBoxLayout(chartContainer);
    chartLayout->setSpacing(10);

    // Individual charts
    m_chartRPM = createChart("RPM", QColor(60, 140, 255), 0, 8000);
    m_seriesRPM = new QLineSeries();
    m_seriesRPM->setColor(QColor(60, 140, 255));
    m_chartRPM->addSeries(m_seriesRPM);
    m_seriesRPM->attachAxis(m_chartRPM->axes(Qt::Horizontal).first());
    m_seriesRPM->attachAxis(m_chartRPM->axes(Qt::Vertical).first());
    m_viewRPM = createChartView(m_chartRPM);
    chartLayout->addWidget(m_viewRPM);

    m_chartSpeed = createChart("VELOCIDAD (km/h)", QColor(80, 200, 255), 0, 260);
    m_seriesSpeed = new QLineSeries();
    m_seriesSpeed->setColor(QColor(80, 200, 255));
    m_chartSpeed->addSeries(m_seriesSpeed);
    m_seriesSpeed->attachAxis(m_chartSpeed->axes(Qt::Horizontal).first());
    m_seriesSpeed->attachAxis(m_chartSpeed->axes(Qt::Vertical).first());
    m_viewSpeed = createChartView(m_chartSpeed);
    chartLayout->addWidget(m_viewSpeed);

    m_chartCoolant = createChart("TEMP. MOTOR (°C)", QColor(255, 160, 60), 40, 130);
    m_seriesCoolant = new QLineSeries();
    m_seriesCoolant->setColor(QColor(255, 160, 60));
    m_chartCoolant->addSeries(m_seriesCoolant);
    m_seriesCoolant->attachAxis(m_chartCoolant->axes(Qt::Horizontal).first());
    m_seriesCoolant->attachAxis(m_chartCoolant->axes(Qt::Vertical).first());
    m_viewCoolant = createChartView(m_chartCoolant);
    chartLayout->addWidget(m_viewCoolant);

    m_chartLoad = createChart("CARGA MOTOR (%)", QColor(120, 255, 120), 0, 100);
    m_seriesLoad = new QLineSeries();
    m_seriesLoad->setColor(QColor(120, 255, 120));
    m_chartLoad->addSeries(m_seriesLoad);
    m_seriesLoad->attachAxis(m_chartLoad->axes(Qt::Horizontal).first());
    m_seriesLoad->attachAxis(m_chartLoad->axes(Qt::Vertical).first());
    m_viewLoad = createChartView(m_chartLoad);
    chartLayout->addWidget(m_viewLoad);

    // Aggregated chart with all sensors
    QLabel* aggTitle = new QLabel("📊 TODOS LOS SENSORES (Superpuestos)");
    aggTitle->setStyleSheet("font-size: 13px; font-weight: bold; color: #B0B8D0; padding-top: 8px;");
    chartLayout->addWidget(aggTitle);

    m_chartAll = createChart("Multiparámetro", QColor(60, 140, 255), 0, 100);
    m_seriesAllRPM = new QLineSeries();
    m_seriesAllRPM->setName("RPM (x10⁻²)");
    m_seriesAllRPM->setColor(QColor(60, 140, 255));
    m_seriesAllLoad = new QLineSeries();
    m_seriesAllLoad->setName("Carga %");
    m_seriesAllLoad->setColor(QColor(120, 255, 120));
    m_seriesAllThrottle = new QLineSeries();
    m_seriesAllThrottle->setName("Acelerador %");
    m_seriesAllThrottle->setColor(QColor(255, 180, 40));
    m_chartAll->addSeries(m_seriesAllRPM);
    m_chartAll->addSeries(m_seriesAllLoad);
    m_chartAll->addSeries(m_seriesAllThrottle);
    m_seriesAllRPM->attachAxis(m_chartAll->axes(Qt::Horizontal).first());
    m_seriesAllRPM->attachAxis(m_chartAll->axes(Qt::Vertical).first());
    m_seriesAllLoad->attachAxis(m_chartAll->axes(Qt::Horizontal).first());
    m_seriesAllLoad->attachAxis(m_chartAll->axes(Qt::Vertical).first());
    m_seriesAllThrottle->attachAxis(m_chartAll->axes(Qt::Horizontal).first());
    m_seriesAllThrottle->attachAxis(m_chartAll->axes(Qt::Vertical).first());
    m_chartAll->legend()->show();
    m_chartAll->legend()->setLabelColor(QColor(180, 190, 210));
    m_viewAll = createChartView(m_chartAll);
    m_viewAll->setMinimumHeight(300);
    chartLayout->addWidget(m_viewAll);

    scrollArea->setWidget(chartContainer);
    mainLayout->addWidget(scrollArea, 1);

    // Sensor values table
    QGroupBox* tableBox = new QGroupBox("Valores Actuales");
    tableBox->setStyleSheet("QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; border-radius: 5px; margin-top: 8px; padding-top: 14px; }");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableBox);

    m_sensorTable = new QTableWidget(8, 3);
    m_sensorTable->setHorizontalHeaderLabels({"Sensor", "Valor", "Estado"});
    m_sensorTable->setAlternatingRowColors(true);
    m_sensorTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_sensorTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_sensorTable->setStyleSheet(
        "QTableWidget { background-color: #1A1D2A; color: #DCE1F0; gridline-color: #2A2E3E; border: none; }"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { background-color: #2A2E3E; color: #B0B8D0; padding: 6px; border: none; font-weight: bold; }"
        "QTableWidget::item:alternate { background-color: #222638; }"
    );
    m_sensorTable->setColumnWidth(0, 180);
    m_sensorTable->setColumnWidth(1, 100);
    m_sensorTable->setColumnWidth(2, 120);

    tableLayout->addWidget(m_sensorTable);
    mainLayout->addWidget(tableBox);

    // Timer
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &SensorPanel::onTimerTick);
    connect(m_btnStart, &QPushButton::clicked, this, &SensorPanel::startPolling);
    connect(m_btnStop, &QPushButton::clicked, this, &SensorPanel::stopPolling);
    connect(m_cmbInterval, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        int intervals[] = {250, 500, 1000, 2000, 5000};
        if (idx >= 0 && idx < 5) {
            m_pollInterval = intervals[idx];
            if (m_timer->isActive()) {
                m_timer->setInterval(m_pollInterval);
            }
        }
    });
}

QChart* SensorPanel::createChart(const QString& title, const QColor& color,
                                  double minY, double maxY) {
    QChart* chart = new QChart();
    chart->setTitle(title);
    chart->setTitleBrush(QColor(180, 190, 210));
    chart->setBackgroundBrush(QColor(30, 34, 48));
    chart->setPlotAreaBackgroundBrush(QColor(20, 22, 32));
    chart->setPlotAreaBackgroundVisible(true);
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->hide();
    chart->setMargins(QMargins(5, 5, 5, 5));
    chart->setMinimumHeight(180);

    QValueAxis* axisX = new QValueAxis();
    axisX->setLabelsColor(QColor(120, 125, 140));
    axisX->setGridLineColor(QColor(40, 45, 60));
    axisX->setRange(0, 200);
    axisX->setLabelFormat("%d");
    axisX->hide();

    QValueAxis* axisY = new QValueAxis();
    axisY->setLabelsColor(QColor(120, 125, 140));
    axisY->setGridLineColor(QColor(40, 45, 60));
    axisY->setRange(minY, maxY);
    axisY->setLabelFormat("%.0f");
    axisY->setTitleBrush(QColor(180, 190, 210));
    axisY->setLabelsAngle(-90);

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    return chart;
}

QChartView* SensorPanel::createChartView(QChart* chart) {
    QChartView* view = new QChartView(chart);
    view->setRenderHint(QPainter::Antialiasing);
    view->setBackgroundBrush(QColor(30, 34, 48));
    view->setMinimumHeight(200);
    return view;
}

void SensorPanel::startPolling() {
    m_counter = 0;
    m_rpmHist.clear();
    m_speedHist.clear();
    m_coolantHist.clear();
    m_loadHist.clear();
    m_throttleHist.clear();
    m_mafHist.clear();
    m_o2Hist.clear();

    m_seriesRPM->clear();
    m_seriesSpeed->clear();
    m_seriesCoolant->clear();
    m_seriesLoad->clear();
    m_seriesAllRPM->clear();
    m_seriesAllLoad->clear();
    m_seriesAllThrottle->clear();

    m_btnStart->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_timer->start(m_pollInterval);
}

void SensorPanel::stopPolling() {
    m_timer->stop();
    m_btnStart->setEnabled(true);
    m_btnStop->setEnabled(false);
}

void SensorPanel::setIntervalIndex(int idx) {
    if (idx >= 0 && idx < m_cmbInterval->count()) {
        m_cmbInterval->setCurrentIndex(idx);
    }
}

void SensorPanel::setPollInterval(int ms) {
    m_pollInterval = ms;
    if (m_timer->isActive()) {
        m_timer->setInterval(ms);
    }
}

void SensorPanel::onTimerTick() {
    if (!m_elm) return;

    ELM327::DashboardData d = m_elm->getDashboardFast();
    auto o2 = m_elm->getO2Sensor(1, 1);
    double stft = m_elm->getShortTermTrimBank1();
    double ltft = m_elm->getLongTermTrimBank1();

    // Store history (limit to 200 points)
    auto addPoint = [this](std::deque<double>& hist, double val) {
        hist.push_back(val);
        if (hist.size() > 200) hist.pop_front();
    };

    double rpm = static_cast<double>(d.rpm);
    double speed = static_cast<double>(d.speed);
    double coolant = static_cast<double>(d.coolant);
    double load = static_cast<double>(d.load);
    double throttle = static_cast<double>(d.throttle);
    double maf = static_cast<double>(d.maf);

    addPoint(m_rpmHist, rpm);
    addPoint(m_speedHist, speed);
    addPoint(m_coolantHist, coolant);
    addPoint(m_loadHist, load);
    addPoint(m_throttleHist, throttle);
    addPoint(m_mafHist, maf);
    addPoint(m_o2Hist, o2.voltage);

    // Helper to update a series from deque
    auto updateSeries = [this](QLineSeries* series, std::deque<double>& hist,
                               double scaleFactor = 1.0) {
        series->clear();
        for (size_t i = 0; i < hist.size(); i++) {
            series->append(static_cast<int>(i), hist[i] * scaleFactor);
        }
    };

    updateSeries(m_seriesRPM, m_rpmHist);
    updateSeries(m_seriesSpeed, m_speedHist);
    updateSeries(m_seriesCoolant, m_coolantHist);
    updateSeries(m_seriesLoad, m_loadHist);

    // Aggregated chart: normalize RPM to % (divide by 80)
    updateSeries(m_seriesAllRPM, m_rpmHist, 1.0 / 80.0);
    updateSeries(m_seriesAllLoad, m_loadHist);
    updateSeries(m_seriesAllThrottle, m_throttleHist);

    // Update table
    updateTable(rpm, speed, coolant, load, throttle, maf,
                o2.voltage, stft, ltft);

    m_counter++;
}

void SensorPanel::updateTable(double rpm, double speed, double coolant,
                               double load, double throttle, double maf,
                               double o2v, double stft, double ltft) {
    auto setCell = [this](int row, int col, const QString& text,
                          const QColor& textColor = QColor(220, 225, 240)) {
        QTableWidgetItem* item = new QTableWidgetItem(text);
        item->setForeground(textColor);
        item->setTextAlignment(Qt::AlignCenter);
        m_sensorTable->setItem(row, col, item);
    };

    auto statusText = [](double val, double warnLow, double warnHigh,
                          double critLow, double critHigh) -> QPair<QString, QColor> {
        if (val >= critHigh || (val <= critLow && critLow > 0)) return {"⚠ CRÍTICO", QColor(255, 60, 60)};
        if (val >= warnHigh || val <= warnLow) return {"⚠ ATENCIÓN", QColor(255, 180, 40)};
        return {"✅ NORMAL", QColor(100, 255, 100)};
    };

    setCell(0, 0, "RPM");
    setCell(0, 1, QString::number(static_cast<int>(rpm)) + " RPM");
    auto s = statusText(rpm, 500, 5000, 200, 6500);
    setCell(0, 2, s.first, s.second);

    setCell(1, 0, "Velocidad");
    setCell(1, 1, QString::number(static_cast<int>(speed)) + " km/h");
    s = statusText(speed, 0, 120, 0, 180);
    setCell(1, 2, s.first, s.second);

    setCell(2, 0, "Temp. Motor");
    setCell(2, 1, QString::number(static_cast<int>(coolant)) + " °C");
    s = statusText(coolant, 50, 100, 40, 120);
    setCell(2, 2, s.first, s.second);

    setCell(3, 0, "Carga Motor");
    setCell(3, 1, QString::number(static_cast<int>(load)) + " %");
    s = statusText(load, 0, 70, 0, 90);
    setCell(3, 2, s.first, s.second);

    setCell(4, 0, "Acelerador");
    setCell(4, 1, QString::number(throttle, 'f', 1) + " %");
    s = statusText(throttle, 0, 75, 0, 90);
    setCell(4, 2, s.first, s.second);

    setCell(5, 0, "MAF");
    setCell(5, 1, QString::number(maf, 'f', 1) + " g/s");
    s = statusText(maf, 0, 80, 0, 150);
    setCell(5, 2, s.first, s.second);

    setCell(6, 0, "Sensor O2");
    setCell(6, 1, QString::number(o2v, 'f', 3) + " V");
    if (o2v < 0.1) setCell(6, 2, "POBRE", QColor(255, 100, 100));
    else if (o2v > 0.9) setCell(6, 2, "RICA", QColor(255, 200, 60));
    else setCell(6, 2, "NORMAL", QColor(100, 255, 100));

    setCell(7, 0, "Fuel Trim");
    setCell(7, 1, QString("STFT: %1% / LTFT: %2%")
             .arg(stft, 0, 'f', 1).arg(ltft, 0, 'f', 1));
    if (qAbs(stft) > 15 || qAbs(ltft) > 15)
        setCell(7, 2, "⚠ FUERA RANGO", QColor(255, 180, 40));
    else
        setCell(7, 2, "NORMAL", QColor(100, 255, 100));
}

void SensorPanel::feedDemoData(const ELM327::DashboardData& d, double o2v, double stft, double ltft) {
    double rpm = static_cast<double>(d.rpm);
    double speed = static_cast<double>(d.speed);
    double coolant = static_cast<double>(d.coolant);
    double load = static_cast<double>(d.load);
    double throttle = static_cast<double>(d.throttle);
    double maf = static_cast<double>(d.maf);

    m_counter++;

    // Store history (limit to 200 points)
    auto addPoint = [this](std::deque<double>& hist, double val) {
        hist.push_back(val);
        if (hist.size() > 200) hist.pop_front();
    };

    addPoint(m_rpmHist, rpm);
    addPoint(m_speedHist, speed);
    addPoint(m_coolantHist, coolant);
    addPoint(m_loadHist, load);
    addPoint(m_throttleHist, throttle);
    addPoint(m_mafHist, maf);
    addPoint(m_o2Hist, o2v);

    // Helper to update a series from deque
    auto updateSeries = [this](QLineSeries* series, std::deque<double>& hist,
                               double scaleFactor = 1.0) {
        series->clear();
        for (size_t i = 0; i < hist.size(); i++) {
            series->append(static_cast<int>(i), hist[i] * scaleFactor);
        }
    };

    updateSeries(m_seriesRPM, m_rpmHist);
    updateSeries(m_seriesSpeed, m_speedHist);
    updateSeries(m_seriesCoolant, m_coolantHist);
    updateSeries(m_seriesLoad, m_loadHist);

    // Aggregated chart: normalize RPM to % (divide by 80)
    updateSeries(m_seriesAllRPM, m_rpmHist, 1.0 / 80.0);
    updateSeries(m_seriesAllLoad, m_loadHist);
    updateSeries(m_seriesAllThrottle, m_throttleHist);

    // Update table
    updateTable(rpm, speed, coolant, load, throttle, maf, o2v, stft, ltft);
}

void SensorPanel::onSensorSelectionChanged(int index) {
    m_selectedSensor = index;
}
