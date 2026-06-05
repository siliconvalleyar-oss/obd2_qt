/**
 * @file historypanel.hpp
 * @brief Panel de historial con grabación, visualización gráfica y exportación.
 *
 * Permite:
 * - Grabar datos de sensores en tiempo real
 * - Guardar/cargar sesiones históricas (formato CSV)
 * - Visualizar gráficamente cualquier parámetro seleccionado
 * - Ver tabla detallada y estadísticas de cada sesión
 */

#pragma once

#include <QWidget>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QScatterSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <deque>
#include <vector>

#include "elm327.hpp"

QT_CHARTS_USE_NAMESPACE

// ─── Estructura de datos para un punto histórico ────────────────────────────

struct HistoryPoint {
    QDateTime timestamp;
    double rpm;
    double speed;
    double coolant;
    double load;
    double throttle;
    double maf;
    double o2v;
    double stft;
    double ltft;
};

// ─── Marcador / Favorito en la línea de tiempo ──────────────────────────────

struct Bookmark {
    QDateTime timestamp;
    QString label;
    QColor color;
};

// ─── Panel de historial ─────────────────────────────────────────────────────

class HistoryPanel : public QWidget {
    Q_OBJECT
public:
    explicit HistoryPanel(QWidget* parent = nullptr);

    /** Agrega un punto de datos (llamado desde el timer tick). */
    void addDataPoint(double rpm, double speed, double coolant,
                      double load, double throttle, double maf,
                      double o2v, double stft, double ltft);

    /** ¿Está grabando actualmente? */
    bool isRecording() const { return m_recording; }

public slots:
    void startRecording();
    void stopRecording();
    void clearHistory();

    /** Guarda la sesión actual a CSV. */
    void saveSession();
    /** Carga una sesión desde CSV. */
    void loadSession();
    /** Exporta a HTML con gráfico PNG embebido. */
    void exportHTML();

private slots:
    void onParamChanged(int idx);
    void onZoomIn();
    void onZoomOut();
    void addBookmark();
    void onBookmarkClicked(QListWidgetItem* item);

private:
    bool m_recording;
    std::vector<HistoryPoint> m_data;
    std::vector<Bookmark> m_bookmarks;
    int m_selectedParam;  // 0=RPM, 1=Speed, etc.

    // UI
    QPushButton* m_btnRecord;
    QPushButton* m_btnStop;
    QPushButton* m_btnClear;
    QPushButton* m_btnSave;
    QPushButton* m_btnLoad;
    QPushButton* m_btnHtmlExport;
    QPushButton* m_btnZoomIn;
    QPushButton* m_btnZoomOut;
    QPushButton* m_btnAddBookmark;
    QListWidget* m_bookmarkList;
    QLabel* m_lblStatus;
    QLabel* m_lblCount;
    QComboBox* m_cmbParam;
    QTableWidget* m_dataTable;
    QTextEdit* m_statsView;

    // Chart
    QChart* m_chart;
    QChartView* m_chartView;
    QLineSeries* m_series;
    QScatterSeries* m_bookmarkSeries;
    QDateTimeAxis* m_axisX;
    QValueAxis* m_axisY;

    // Zoom
    int m_visibleRange;  // seconds visible in chart

    void setupUI();
    void rebuildChart();
    void updateBookmarkMarkers();
    void updateBookmarkList();
    void updateTable();
    void updateStats();
    double getParam(int idx, const HistoryPoint& p) const;
    double getParam(int idx, const Bookmark& bm) const;
    QString paramName(int idx) const;
    QColor paramColor(int idx) const;
    double paramMin(int idx) const;
    double paramMax(int idx) const;
};
