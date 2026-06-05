#pragma once

#include <QWidget>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QSplitter>
#include <QTableWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <deque>

#include "elm327.hpp"

QT_CHARTS_USE_NAMESPACE

class SensorPanel : public QWidget {
    Q_OBJECT
public:
    explicit SensorPanel(ELM327* elm, QWidget* parent = nullptr);
    void setELM(ELM327* elm) { m_elm = elm; }

public slots:
    void startPolling();
    void stopPolling();
    void setPollInterval(int ms);
    void feedDemoData(const ELM327::DashboardData& d, double o2v, double stft, double ltft);
    void setIntervalIndex(int idx);
    int getIntervalIndex() const { return m_cmbInterval->currentIndex(); }

private slots:
    void onTimerTick();
    void onSensorSelectionChanged(int index);

private:
    ELM327* m_elm;
    QTimer* m_timer;
    int m_pollInterval;

    // Selected sensor for detailed view
    int m_selectedSensor;

    // Graph data history
    std::deque<double> m_rpmHist;
    std::deque<double> m_speedHist;
    std::deque<double> m_coolantHist;
    std::deque<double> m_loadHist;
    std::deque<double> m_throttleHist;
    std::deque<double> m_mafHist;
    std::deque<double> m_o2Hist;

    // Charts
    QChart* m_chartRPM;
    QChart* m_chartSpeed;
    QChart* m_chartCoolant;
    QChart* m_chartLoad;
    QChartView* m_viewRPM;
    QChartView* m_viewSpeed;
    QChartView* m_viewCoolant;
    QChartView* m_viewLoad;

    QLineSeries* m_seriesRPM;
    QLineSeries* m_seriesSpeed;
    QLineSeries* m_seriesCoolant;
    QLineSeries* m_seriesLoad;

    // Aggregated chart with ALL sensors overlaid
    QChart* m_chartAll;
    QChartView* m_viewAll;
    QLineSeries* m_seriesAllRPM;
    QLineSeries* m_seriesAllLoad;
    QLineSeries* m_seriesAllThrottle;

    // Table with current sensor values
    QTableWidget* m_sensorTable;
    QLabel* m_lblValues;

    // Controls
    QComboBox* m_cmbInterval;
    QPushButton* m_btnStart;
    QPushButton* m_btnStop;
    QComboBox* m_cmbSensor;

    int m_counter;

    QChart* createChart(const QString& title, const QColor& color,
                        double minY, double maxY);
    QChartView* createChartView(QChart* chart);
    void updateTable(double rpm, double speed, double coolant,
                     double load, double throttle, double maf,
                     double o2v, double stft, double ltft);
};
