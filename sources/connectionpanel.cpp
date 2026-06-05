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
    QLabel* title = new QLabel("🔌 CONEXIÓN ELM327");
    title->setStyleSheet("font-size: 16px; font-weight: bold; color: #60CCFF; padding: 8px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Connection group
    QGroupBox* connGroup = new QGroupBox("Bluetooth RFCOMM");
    connGroup->setStyleSheet("QGroupBox { color: #B0B8D0; font-weight: bold; border: 1px solid #3C4158; border-radius: 6px; margin-top: 12px; padding-top: 18px; }");
    QVBoxLayout* connLayout = new QVBoxLayout(connGroup);
    connLayout->setSpacing(10);

    // MAC address input
    QHBoxLayout* macLayout = new QHBoxLayout();
    QLabel* macLabel = new QLabel("Dirección MAC:");
    macLabel->setStyleSheet("color: #B0B8D0; font-size: 11px;");
    macLayout->addWidget(macLabel);

    m_macInput = new QLineEdit("00:1D:A5:07:23:6E");
    m_macInput->setPlaceholderText("XX:XX:XX:XX:XX:XX");
    m_macInput->setStyleSheet(
        "QLineEdit { background-color: #1E2230; color: #DCE1F0; border: 1px solid #3C4158; "
        "border-radius: 4px; padding: 6px; font-family: monospace; }"
        "QLineEdit:focus { border-color: #3C8CFF; }"
    );
    macLayout->addWidget(m_macInput);
    connLayout->addLayout(macLayout);

    // Channel input
    QHBoxLayout* chLayout = new QHBoxLayout();
    QLabel* chLabel = new QLabel("Canal RFCOMM:");
    chLabel->setStyleSheet("color: #B0B8D0; font-size: 11px;");
    chLayout->addWidget(chLabel);

    m_channelInput = new QLineEdit("1");
    m_channelInput->setStyleSheet(
        "QLineEdit { background-color: #1E2230; color: #DCE1F0; border: 1px solid #3C4158; "
        "border-radius: 4px; padding: 6px; }"
        "QLineEdit:focus { border-color: #3C8CFF; }"
    );
    m_channelInput->setFixedWidth(60);
    chLayout->addWidget(m_channelInput);
    chLayout->addStretch();
    connLayout->addLayout(chLayout);

    mainLayout->addWidget(connGroup);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    m_btnConnect = new QPushButton("🔗 CONECTAR");
    m_btnConnect->setStyleSheet(
        "QPushButton { background-color: #2D8C2D; color: white; font-weight: bold; padding: 10px 24px; "
        "border: none; border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { background-color: #3AA63A; }"
        "QPushButton:pressed { background-color: #1E6B1E; }"
        "QPushButton:disabled { background-color: #3C4158; color: #6A6F80; }"
    );
    m_btnConnect->setMinimumHeight(40);
    btnLayout->addWidget(m_btnConnect);

    m_btnDisconnect = new QPushButton("🔌 DESCONECTAR");
    m_btnDisconnect->setStyleSheet(
        "QPushButton { background-color: #8C2D2D; color: white; font-weight: bold; padding: 10px 24px; "
        "border: none; border-radius: 6px; font-size: 13px; }"
        "QPushButton:hover { background-color: #A63A3A; }"
        "QPushButton:pressed { background-color: #6B1E1E; }"
        "QPushButton:disabled { background-color: #3C4158; color: #6A6F80; }"
    );
    m_btnDisconnect->setMinimumHeight(40);
    m_btnDisconnect->setEnabled(false);
    btnLayout->addWidget(m_btnDisconnect);

    mainLayout->addLayout(btnLayout);

    // Status labels
    m_lblStatus = new QLabel("⏹ Desconectado");
    m_lblStatus->setStyleSheet("font-size: 14px; color: #FF6B6B; padding: 6px;");
    m_lblStatus->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_lblStatus);

    m_lblProtocol = new QLabel("");
    m_lblProtocol->setStyleSheet("font-size: 12px; color: #8A8FA0;");
    m_lblProtocol->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_lblProtocol);

    mainLayout->addStretch();

    // Connect signals
    connect(m_btnConnect, &QPushButton::clicked, this, &ConnectionPanel::onConnectClicked);
    connect(m_btnDisconnect, &QPushButton::clicked, this, &ConnectionPanel::onDisconnectClicked);
}

void ConnectionPanel::onConnectClicked() {
    QString mac = m_macInput->text().trimmed();
    bool ok;
    int channel = m_channelInput->text().toInt(&ok);
    if (!ok || channel < 1) channel = 1;

    m_lblStatus->setText("🔄 Conectando...");
    m_lblStatus->setStyleSheet("font-size: 14px; color: #FFB347; padding: 6px;");
    m_btnConnect->setEnabled(false);
    QApplication::processEvents();

    // Recreate ELM327 with new MAC
    // Note: m_elm is owned by MainWindow, we just use it
    if (m_elm) {
        bool success = m_elm->connectBT();
        if (success) {
            m_connected = true;
            m_lblStatus->setText("✅ Conectado a " + mac);
            m_lblStatus->setStyleSheet("font-size: 14px; color: #6BFF6B; padding: 6px;");
            m_lblProtocol->setText("Protocolo: " + QString::fromStdString(m_elm->getProtocol()));
            m_btnConnect->setEnabled(false);
            m_btnDisconnect->setEnabled(true);
            m_macInput->setEnabled(false);
            emit connectionStatusChanged(true);
        } else {
            m_connected = false;
            m_lblStatus->setText("❌ Error de conexión");
            m_lblStatus->setStyleSheet("font-size: 14px; color: #FF6B6B; padding: 6px;");
            m_btnConnect->setEnabled(true);
        }
    }
}

void ConnectionPanel::onDisconnectClicked() {
    if (m_elm) {
        m_elm->disconnect();
    }
    m_connected = false;
    m_lblStatus->setText("⏹ Desconectado");
    m_lblStatus->setStyleSheet("font-size: 14px; color: #8A8FA0; padding: 6px;");
    m_lblProtocol->setText("");
    m_btnConnect->setEnabled(true);
    m_btnDisconnect->setEnabled(false);
    m_macInput->setEnabled(true);
    emit connectionStatusChanged(false);
}
