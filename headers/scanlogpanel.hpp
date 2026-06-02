#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDateTime>
#include <QTextStream>

class ScanLogPanel : public QWidget {
    Q_OBJECT
public:
    explicit ScanLogPanel(QWidget* parent = nullptr);

public slots:
    void appendLog(const QString& text, const QColor& color = QColor(180, 190, 210));
    void clearLog();

private slots:
    void onSaveLog();

private:
    QPlainTextEdit* m_logView;
    QPushButton* m_btnClear;
    QPushButton* m_btnSave;
};
