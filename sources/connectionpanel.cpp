#include "headers/connectionpanel.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QApplication>

ConnectionPanel::ConnectionPanel(ELM327* elm, QWidget* parent)
    : QWidget(parent), m_elm(elm), m_connected(false) {

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel* title = new QLabel("🔌 ESTADO DE CONEXIÓN");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #60CCFF; padding: 8px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Info note: connection is controlled from the toolbar
    QLabel* note = new QLabel("💡 La conexión se controla desde la barra de herramientas superior");
    note->setStyleSheet("font-size: 11px; color: #8A8FA0; padding: 4px; font-style: italic;");
    note->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(note);

    // MAC address (read-only display)
    QGroupBox* infoGroup = new QGroupBox("Dispositivo");
    infoGroup->setStyleSheet(
        "QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }"
    );
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    infoLayout->setSpacing(8);

    // MAC
    QHBoxLayout* macLayout = new QHBoxLayout();
    QLabel* macLabel = new QLabel("Dirección MAC:");
    macLabel->setStyleSheet("color: #B0B8D0; font-size: 11px;");
    macLayout->addWidget(macLabel);

    m_macInput = new QLineEdit("00:1D:A5:07:23:6E");
    m_macInput->setPlaceholderText("XX:XX:XX:XX:XX:XX");
    m_macInput->setReadOnly(true);
    m_macInput->setStyleSheet(
        "QLineEdit { background-color: #1E2230; color: #DCE1F0; border: 1px solid #3C4158; "
        "border-radius: 4px; padding: 6px; font-family: monospace; }"
    );
    macLayout->addWidget(m_macInput);
    infoLayout->addLayout(macLayout);

    mainLayout->addWidget(infoGroup);

    // Status display group
    QGroupBox* statusGroup = new QGroupBox("Estado de comunicación");
    statusGroup->setStyleSheet(
        "QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }"
    );
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setSpacing(10);

    // Status indicator
    m_lblStatus = new QLabel("⏹ Desconectado");
    m_lblStatus->setStyleSheet("font-size: 16px; color: #FF6B6B; padding: 6px;");
    m_lblStatus->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(m_lblStatus);

    // Protocol
    m_lblProtocol = new QLabel("");
    m_lblProtocol->setStyleSheet("font-size: 13px; color: #8A8FA0;");
    m_lblProtocol->setAlignment(Qt::AlignCenter);
    statusLayout->addWidget(m_lblProtocol);

    // Last error message
    m_lblLastError = new QLabel("");
    m_lblLastError->setStyleSheet("font-size: 11px; color: #FF6B6B; padding: 4px;");
    m_lblLastError->setAlignment(Qt::AlignCenter);
    m_lblLastError->setWordWrap(true);
    statusLayout->addWidget(m_lblLastError);

    mainLayout->addWidget(statusGroup);

    // Bluetooth cleanup info
    QGroupBox* cleanupGroup = new QGroupBox("Liberación de puerto");
    cleanupGroup->setStyleSheet(
        "QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; "
        "border-radius: 6px; margin-top: 12px; padding-top: 18px; }"
    );
    QVBoxLayout* cleanupLayout = new QVBoxLayout(cleanupGroup);

    m_lblCleanupInfo = new QLabel(
        "✅ Al conectar se ejecuta automáticamente la limpieza del puerto Bluetooth:\n"
        "  • Libera procesos que usan rfcomm\n"
        "  • Reinicia el stack Bluetooth\n"
        "  • Recarga el módulo rfcomm\n\n"
        "En Linux puede requerir ejecutar con sudo para permisos de modprobe/systemctl.\n"
        "En Windows los puertos COM se gestionan automáticamente."
    );
    m_lblCleanupInfo->setStyleSheet("font-size: 11px; color: #8A8FA0; padding: 6px;");
    m_lblCleanupInfo->setWordWrap(true);
    cleanupLayout->addWidget(m_lblCleanupInfo);

    mainLayout->addWidget(cleanupGroup);
    mainLayout->addStretch();
}

void ConnectionPanel::setMAC(const QString& mac) {
    m_macInput->setText(mac);
}

void ConnectionPanel::updateStatus(bool connected, const QString& protocol) {
    m_connected = connected;
    if (connected) {
        m_lblStatus->setText("✅ Conectado");
        m_lblStatus->setStyleSheet("font-size: 16px; color: #6BFF6B; padding: 6px;");
        m_lblProtocol->setText("Protocolo: " + protocol);
        m_lblLastError->setText("");
    } else {
        m_lblStatus->setText("⏹ Desconectado");
        m_lblStatus->setStyleSheet("font-size: 16px; color: #8A8FA0; padding: 6px;");
        m_lblProtocol->setText("");
        m_lblLastError->setText("");
    }
    emit connectionStatusChanged(connected);
}
