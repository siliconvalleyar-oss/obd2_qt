/**
 * @file main.cpp
 * @brief Punto de entrada de la aplicación gráfica OBD-II Qt5.
 *
 * Inicializa QApplication con estilo Fusion oscuro y abre la
 * ventana principal del escáner de diagnóstico automotriz.
 *
 * La aplicación arranca en modo conexión real.
 * Para activar el modo demo (sin hardware), haga clic en
 * el botón "🎮 Demo" de la barra de herramientas.
 */

#include <QApplication>
#include <QSplashScreen>
#include <QPixmap>
#include <QStyleFactory>
#include <QPalette>
#include <QFont>
#include <QDebug>
#include <QTimer>

#include "headers/mainwindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("OBD-II Scanner Profesional");
    app.setApplicationVersion("12.0");
    app.setOrganizationName("Freebuff");

    // Splash screen con logo
    QPixmap splashPix(":/logo");
    QSplashScreen splash(splashPix.scaled(420, 420, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    splash.show();
    app.processEvents();

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

    // Crear ventana principal
    MainWindow window;
    window.setWindowTitle("OBD-II Escáner Profesional v12");
    window.resize(1200, 800);

    // Cerrar splash y mostrar ventana tras 1.5s
    QTimer::singleShot(1500, [&]() {
        splash.close();
        window.show();
    });

    return app.exec();
}
