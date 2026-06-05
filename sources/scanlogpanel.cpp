#include "headers/scanlogpanel.hpp"
#include <QLabel>
#include <QScrollBar>

ScanLogPanel::ScanLogPanel(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Title
    QLabel* title = new QLabel("📋 LOG DE ESCANEO");
    title->setStyleSheet("font-size: 15px; font-weight: bold; color: #60CCFF; padding: 4px;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Log view
    m_logView = new QPlainTextEdit();
    m_logView->setReadOnly(true);
    m_logView->setStyleSheet(
        "QPlainTextEdit { background-color: #0D0F17; color: #B0B8D0; "
        "border: 1px solid #2A2E3E; border-radius: 5px; "
        "font-family: 'Monospace', 'Courier New', monospace; font-size: 11px; "
        "padding: 8px; }"
        "QPlainTextEdit:focus { border-color: #3C8CFF; }"
    );
    m_logView->setMaximumBlockCount(10000);
    m_logView->setLineWrapMode(QPlainTextEdit::NoWrap);
    mainLayout->addWidget(m_logView, 1);

    // Status info at bottom
    QLabel* infoLabel = new QLabel("Máx 10,000 líneas | Hora UTC");
    infoLabel->setStyleSheet("color: #6A6F80; font-size: 10px;");
    mainLayout->addWidget(infoLabel);

    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(10);

    m_btnClear = new QPushButton("🗑 LIMPIAR");
    m_btnClear->setStyleSheet(
        "QPushButton { background-color: #3C4158; color: #DCE1F0; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #4C5168; }"
    );
    btnLayout->addWidget(m_btnClear);

    m_btnSave = new QPushButton("💾 GUARDAR LOG");
    m_btnSave->setStyleSheet(
        "QPushButton { background-color: #2D5C8C; color: white; font-weight: bold; "
        "padding: 8px 18px; border: none; border-radius: 4px; }"
        "QPushButton:hover { background-color: #3A6CA6; }"
    );
    btnLayout->addWidget(m_btnSave);

    btnLayout->addStretch();

    // Line count
    QLabel* countLabel = new QLabel("0 líneas");
    countLabel->setStyleSheet("color: #6A6F80; font-size: 11px;");
    countLabel->setObjectName("lineCountLabel");
    btnLayout->addWidget(countLabel);

    mainLayout->addLayout(btnLayout);

    // Connect signals
    connect(m_btnClear, &QPushButton::clicked, this, &ScanLogPanel::clearLog);
    connect(m_btnSave, &QPushButton::clicked, this, &ScanLogPanel::onSaveLog);
}

void ScanLogPanel::appendLog(const QString& text, const QColor& color) {
    QString timestamp = QDateTime::currentDateTime().toString("[HH:mm:ss.zzz] ");

    m_logView->appendPlainText("");  // newline
    // Move cursor to end and insert colored text
    QTextCursor cursor = m_logView->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertHtml(
        QString("<span style='color:%1;'>%2%3</span>")
            .arg(color.name(), timestamp, text.toHtmlEscaped())
    );

    // Auto-scroll to bottom
    QScrollBar* scrollBar = m_logView->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());

    // Update line count
    QLabel* countLabel = findChild<QLabel*>("lineCountLabel");
    if (countLabel) {
        countLabel->setText(QString::number(m_logView->blockCount()) + " líneas");
    }
}

void ScanLogPanel::clearLog() {
    m_logView->clear();

    QLabel* countLabel = findChild<QLabel*>("lineCountLabel");
    if (countLabel) {
        countLabel->setText("0 líneas");
    }
}

void ScanLogPanel::onSaveLog() {
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Guardar Log",
        "logs/scan_log_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt",
        "Archivos de texto (*.txt);;Todos los archivos (*)"
    );

    if (filename.isEmpty()) return;

    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << m_logView->toPlainText();
        file.close();
        appendLog("✅ Log guardado: " + filename, QColor(100, 255, 100));
    } else {
        appendLog("❌ Error al guardar log: " + file.errorString(), QColor(255, 60, 60));
    }
}
