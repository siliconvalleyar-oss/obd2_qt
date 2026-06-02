/**
 * @file dtcpanel.hpp
 * @brief Panel de diagnóstico con DTCs, info vehículo y estado.
 *
 * Proporciona:
 * - Tabla de códigos DTC con código, descripción, estado
 * - Sección de información del vehículo (VIN, ECU, Protocolo)
 * - Botones para leer y limpiar DTCs
 * - Modo demo con DTCs simulados
 */

#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

#include "elm327.hpp"

class DtcPanel : public QWidget {
    Q_OBJECT
public:
    explicit DtcPanel(ELM327* elm, QWidget* parent = nullptr);
    void setELM(ELM327* elm) { m_elm = elm; }
    void setDemoDTCs(const std::vector<DTCCode>& dtcs, bool checkEngine);

public slots:
    void refreshDTCs();
    void refreshVehicleInfo();
    void setDemoVehicleInfo();

private slots:
    void onReadDTCs();
    void onClearDTCs();
    void onRefreshVIN();

private:
    ELM327* m_elm;

    // DTC section
    QTableWidget* m_dtcTable;
    QLabel* m_lblDtcCount;
    QLabel* m_lblMILStatus;
    QPushButton* m_btnReadDTCs;
    QPushButton* m_btnClearDTCs;

    // Vehicle info section
    QLabel* m_lblVIN;
    QLabel* m_lblProtocol;
    QLabel* m_lblECUName;
    QLabel* m_lblCalibrationID;
    QPushButton* m_btnRefreshVIN;

    // Status / freeze frame
    QTextEdit* m_statusView;

    void setupUI();
    void setDTCCell(int row, const std::string& code, const std::string& desc, bool isPending);
    void setVehicleInfo(const QString& vin, const QString& protocol,
                        const QString& ecuName, const QString& calibID);
    void setMILStatus(bool milOn, int dtcCount);
};
