#pragma once

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QProgressBar>

#include "elm327.hpp"

/**
 * @brief Widget de dashboard con indicadores visuales.
 *
 * Muestra valores de sensores con:
 * - ProgressBars animados que suben/bajan según datos
 * - Valores digitales grandes
 * - Indicadores de color (verde/amarillo/rojo)
 */
class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(ELM327* elm, QWidget* parent = nullptr);
    void setELM(ELM327* elm) { m_elm = elm; }

public slots:
    void updateData(const ELM327::DashboardData& data);

private:
    void createSensorBar(const QString& name, const QString& unit,
                         double minVal, double maxVal,
                         double warnVal, double critVal,
                         QProgressBar*& bar, QLabel*& valueLabel,
                         QGridLayout* grid, int row);
    void updateBarColor(QProgressBar* bar, double value,
                        double warnVal, double critVal);

    ELM327* m_elm;

    // Sensor bars
    QProgressBar* m_barRPM;
    QProgressBar* m_barSpeed;
    QProgressBar* m_barCoolant;
    QProgressBar* m_barLoad;
    QProgressBar* m_barThrottle;
    QProgressBar* m_barMAF;
    QProgressBar* m_barFuelLevel;
    QProgressBar* m_barTiming;

    QLabel* m_valRPM;
    QLabel* m_valSpeed;
    QLabel* m_valCoolant;
    QLabel* m_valLoad;
    QLabel* m_valThrottle;
    QLabel* m_valMAF;
    QLabel* m_valFuelLevel;
    QLabel* m_valTiming;

    // O2 status
    QLabel* m_lblO2Status;
    QLabel* m_lblO2Value;

    // Fuel trims
    QLabel* m_lblSTFT;
    QLabel* m_lblLTFT;
};
