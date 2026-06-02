#include "headers/dashboardwidget.hpp"
#include <QGroupBox>

DashboardWidget::DashboardWidget(ELM327* elm, QWidget* parent)
    : QWidget(parent), m_elm(elm) {

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(15, 15, 15, 15);

    // Title
    QLabel* title = new QLabel("📊 DASHBOARD EN VIVO");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #60CCFF; padding: 5px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Grid for sensor bars
    QGridLayout* grid = new QGridLayout();
    grid->setSpacing(10);

    // Create sensor bars
    createSensorBar("RPM", "RPM", 0, 8000, 5000, 6500,
                    m_barRPM, m_valRPM, grid, 0);
    createSensorBar("VELOCIDAD", "km/h", 0, 260, 120, 180,
                    m_barSpeed, m_valSpeed, grid, 1);
    createSensorBar("TEMP MOTOR", "°C", 40, 130, 100, 120,
                    m_barCoolant, m_valCoolant, grid, 2);
    createSensorBar("CARGA MOTOR", "%", 0, 100, 70, 90,
                    m_barLoad, m_valLoad, grid, 3);
    createSensorBar("ACELERADOR", "%", 0, 100, 75, 90,
                    m_barThrottle, m_valThrottle, grid, 4);
    createSensorBar("MAF", "g/s", 0, 200, 80, 150,
                    m_barMAF, m_valMAF, grid, 5);
    createSensorBar("COMBUSTIBLE", "%", 0, 100, 20, 10,
                    m_barFuelLevel, m_valFuelLevel, grid, 6);
    createSensorBar("AVANCE", "°", -64, 64, 30, 45,
                    m_barTiming, m_valTiming, grid, 7);

    mainLayout->addLayout(grid);

    // Bottom info panel
    QHBoxLayout* bottomLayout = new QHBoxLayout();

    // O2 status
    QGroupBox* o2Box = new QGroupBox("Sensor O2 B1S1");
    o2Box->setStyleSheet("QGroupBox { color: #90EE90; font-weight: bold; border: 1px solid #3C4158; border-radius: 5px; margin-top: 10px; padding-top: 15px; }");
    QVBoxLayout* o2Layout = new QVBoxLayout(o2Box);
    m_lblO2Value = new QLabel("0.000 V");
    m_lblO2Value->setStyleSheet("font-size: 24px; font-weight: bold; color: #90EE90;");
    m_lblO2Value->setAlignment(Qt::AlignCenter);
    m_lblO2Status = new QLabel("ESPERANDO...");
    m_lblO2Status->setStyleSheet("font-size: 14px; color: #8A8FA0;");
    m_lblO2Status->setAlignment(Qt::AlignCenter);
    o2Layout->addWidget(m_lblO2Value);
    o2Layout->addWidget(m_lblO2Status);
    bottomLayout->addWidget(o2Box);

    // Fuel trims
    QGroupBox* ftBox = new QGroupBox("Fuel Trims");
    ftBox->setStyleSheet("QGroupBox { color: #FFB347; font-weight: bold; border: 1px solid #3C4158; border-radius: 5px; margin-top: 10px; padding-top: 15px; }");
    QVBoxLayout* ftLayout = new QVBoxLayout(ftBox);
    m_lblSTFT = new QLabel("STFT B1: 0.0%");
    m_lblSTFT->setStyleSheet("font-size: 14px; color: #DCE1F0;");
    m_lblLTFT = new QLabel("LTFT B1: 0.0%");
    m_lblLTFT->setStyleSheet("font-size: 14px; color: #DCE1F0;");
    ftLayout->addWidget(m_lblSTFT);
    ftLayout->addWidget(m_lblLTFT);
    bottomLayout->addWidget(ftBox);

    // Intake temp & pressure
    QGroupBox* envBox = new QGroupBox("Admisión");
    envBox->setStyleSheet("QGroupBox { color: #60CCFF; font-weight: bold; border: 1px solid #3C4158; border-radius: 5px; margin-top: 10px; padding-top: 15px; }");
    QVBoxLayout* envLayout = new QVBoxLayout(envBox);
    QLabel* lblIntake = new QLabel("Temp. Admisión: -- °C");
    lblIntake->setStyleSheet("font-size: 14px; color: #DCE1F0;");
    QLabel* lblPressure = new QLabel("Presión Colector: -- kPa");
    lblPressure->setStyleSheet("font-size: 14px; color: #DCE1F0;");
    envLayout->addWidget(lblIntake);
    envLayout->addWidget(lblPressure);
    bottomLayout->addWidget(envBox);

    mainLayout->addLayout(bottomLayout);
    mainLayout->addStretch();
}

void DashboardWidget::createSensorBar(const QString& name, const QString& unit,
                                       double minVal, double maxVal,
                                       double warnVal, double critVal,
                                       QProgressBar*& bar, QLabel*& valueLabel,
                                       QGridLayout* grid, int row) {
    Q_UNUSED(minVal)
    Q_UNUSED(maxVal)
    Q_UNUSED(warnVal)
    Q_UNUSED(critVal)

    // Sensor name
    QLabel* nameLabel = new QLabel(name);
    nameLabel->setStyleSheet("font-size: 11px; font-weight: bold; color: #B0B8D0;");
    nameLabel->setFixedWidth(100);

    // Progress bar
    bar = new QProgressBar();
    bar->setRange(0, 1000);
    bar->setValue(0);
    bar->setTextVisible(false);
    bar->setFixedHeight(22);
    bar->setStyleSheet(
        "QProgressBar { background-color: #282C40; border: none; border-radius: 4px; }"
        "QProgressBar::chunk { background-color: #3C8CFF; border-radius: 4px; }"
    );

    // Value label
    valueLabel = new QLabel("-- " + unit);
    valueLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #DCE1F0;");
    valueLabel->setFixedWidth(100);
    valueLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    grid->addWidget(nameLabel, row, 0);
    grid->addWidget(bar, row, 1);
    grid->addWidget(valueLabel, row, 2);
}

void DashboardWidget::updateData(const ELM327::DashboardData& data) {
    auto setBar = [&](QProgressBar* bar, QLabel* label,
                      double value, double maxVal, double minVal,
                      double warnVal, double critVal,
                      const QString& unit, int decimals = 0) {
        int range = static_cast<int>(maxVal - minVal);
        int val = static_cast<int>((value - minVal) / range * 1000);
        bar->setValue(std::max(0, std::min(1000, val)));
        updateBarColor(bar, value, warnVal, critVal);

        QString text;
        if (decimals > 0)
            text = QString::number(value, 'f', decimals) + " " + unit;
        else
            text = QString::number(static_cast<int>(value)) + " " + unit;
        label->setText(text);
    };

    setBar(m_barRPM, m_valRPM, data.rpm, 8000, 0, 5000, 6500, "RPM");
    setBar(m_barSpeed, m_valSpeed, data.speed, 260, 0, 120, 180, "km/h");
    setBar(m_barCoolant, m_valCoolant, data.coolant, 130, 40, 100, 120, "°C");
    setBar(m_barLoad, m_valLoad, data.load, 100, 0, 70, 90, "%");
    setBar(m_barThrottle, m_valThrottle, data.throttle, 100, 0, 75, 90, "%", 1);
    setBar(m_barMAF, m_valMAF, data.maf, 200, 0, 80, 150, "g/s", 1);
    setBar(m_barFuelLevel, m_valFuelLevel, data.fuelLevel, 100, 0, 20, 10, "%", 1);

    // Timing: range -64 to +64, center at 0
    double timingNorm = data.timing + 64.0;
    setBar(m_barTiming, m_valTiming, timingNorm, 128, 0, 90, 110, "°", 1);
}

void DashboardWidget::updateBarColor(QProgressBar* bar, double value,
                                      double warnVal, double critVal) {
    QString color;
    if (value >= critVal)
        color = "#FF3C3C";
    else if (value >= warnVal)
        color = "#FFB347";
    else
        color = "#3C8CFF";

    bar->setStyleSheet(
        QString("QProgressBar { background-color: #282C40; border: none; border-radius: 4px; }"
                "QProgressBar::chunk { background-color: %1; border-radius: 4px; }").arg(color)
    );

    // Update label color too
    if (value >= critVal)
        bar->setStyleSheet(bar->styleSheet().replace("#3C8CFF", "#FF3C3C"));
}
