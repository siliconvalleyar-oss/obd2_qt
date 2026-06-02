/**
 * @file main.cpp
 * @brief Punto de entrada de la aplicación gráfica OBD-II Qt5.
 *
 * Inicializa QApplication con estilo Fusion oscuro y abre la
 * ventana principal del escáner de diagnóstico automotriz.
 */

#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include <QDebug>

#include "headers/mainwindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("OBD-II Scanner Profesional");
    app.setApplicationVersion("8.0");
    app.setOrganizationName("Freebuff");

    // Estilo Fusion oscuro profesional
    app.setStyle(QStyleFactory::create("Fusion"));

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(30, 34, 48));
    darkPalette.setColor(QPalette::WindowText, QColor(220, 225, 240));
    darkPalette.setColor(QPalette::Base, QColor(20, 22, 32));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 40, 55));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(45, 50, 68));
    darkPalette.setColor(QPalette::ToolTipText, QColor(220, 225, 240));
    darkPalette.setColor(QPalette::Text, QColor(220, 225, 240));
    darkPalette.setColor(QPalette::Button, QColor(45, 50, 68));
    darkPalette.setColor(QPalette::ButtonText, QColor(220, 225, 240));
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Highlight, QColor(60, 140, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::white);
    darkPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(100, 105, 120));
    darkPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(100, 105, 120));

    app.setPalette(darkPalette);

    // Fuente global
    QFont appFont("monospace", 10);
    appFont.setStyleHint(QFont::Monospace);
    app.setFont(appFont);

    MainWindow window;
    window.setWindowTitle("OBD-II Escáner Profesional v8");
    window.resize(1200, 800);
    window.show();

    return app.exec();
}
