#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

#include "elm327.hpp"

/**
 * @brief Panel de estado de conexión (solo lectura).
 *
 * Muestra el estado actual de la comunicación Bluetooth con el ELM327.
 * Los botones de conectar/desconectar están en la barra de herramientas
 * principal (MainWindow). Este panel solo muestra:
 * - Dirección MAC configurada
 * - Estado de conexión (conectado/desconectado)
 * - Protocolo detectado
 * - Último error o mensaje
 *
 * La conexión/desconexión se controla desde la toolbar principal.
 */
class ConnectionPanel : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionPanel(ELM327* elm, QWidget* parent = nullptr);
    void setELM(ELM327* elm) { m_elm = elm; }
    void setMAC(const QString& mac);
    QString getMAC() const { return m_macInput->text().trimmed(); }

public slots:
    /** @brief Actualiza el estado de conexión mostrado. */
    void updateStatus(bool connected, const QString& protocol = "");

signals:
    void connectionStatusChanged(bool connected);

private:
    ELM327* m_elm;
    QLineEdit* m_macInput;
    QLabel* m_lblStatus;
    QLabel* m_lblProtocol;
    QLabel* m_lblLastError;
    QLabel* m_lblCleanupInfo;
    bool m_connected;
};
