#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

#include "elm327.hpp"

class ConnectionPanel : public QWidget {
    Q_OBJECT
public:
    explicit ConnectionPanel(ELM327* elm, QWidget* parent = nullptr);
    void setELM(ELM327* elm) { m_elm = elm; }
    void setMAC(const QString& mac) { m_macInput->setText(mac); }
    QString getMAC() const { return m_macInput->text().trimmed(); }

signals:
    void connectionStatusChanged(bool connected);

private slots:
    void onConnectClicked();
    void onDisconnectClicked();

private:
    ELM327* m_elm;
    QLineEdit* m_macInput;
    QLineEdit* m_channelInput;
    QPushButton* m_btnConnect;
    QPushButton* m_btnDisconnect;
    QLabel* m_lblStatus;
    QLabel* m_lblProtocol;
    bool m_connected;
};
