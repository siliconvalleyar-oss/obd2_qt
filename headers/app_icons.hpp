/**
 * @file app_icons.hpp
 * @brief Iconos vectoriales dibujados con QPainter para la toolbar.
 *
 * Todos los iconos se generan en tiempo de ejecución como QPixmap
 * sin depender de archivos externos ni emojis del sistema.
 */

#pragma once

#include <QIcon>
#include <QPixmap>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QColor>
#include <QPen>
#include <QBrush>

namespace AppIcons {

// Tamaño base del canvas
constexpr int ICON_SZ = 22;
constexpr int HALF = ICON_SZ / 2;

// Colores temáticos
inline QColor clrEnabled()  { return {0xDC, 0xE1, 0xF0}; }  // texto normal
inline QColor clrConnect()  { return {0x3A, 0xA6, 0x3A}; }  // verde conectar
inline QColor clrDisconn()  { return {0xA6, 0x3A, 0x3A}; }  // rojo desconectar
inline QColor clrScan()     { return {0x60, 0xCC, 0xFF}; }  // azul escanear
inline QColor clrStop()     { return {0xFF, 0x6B, 0x6B}; }  // rojo detener
inline QColor clrClear()    { return {0xFF, 0xB3, 0x47}; }  // naranja limpiar
inline QColor clrExport()   { return {0x6B, 0xFF, 0x6B}; }  // verde exportar
inline QColor clrDemo()     { return {0xCC, 0x88, 0xFF}; }  // violeta demo

// Helpers internos

/** Crea un QPixmap listo para pintar con fondo transparente. */
inline QPixmap makeCanvas() {
    QPixmap px(ICON_SZ, ICON_SZ);
    px.fill(Qt::transparent);
    return px;
}

/** Pinta el borde exterior redondeado opcional (para botones seleccionados). */
inline void drawBorder(QPainter& p, bool active) {
    if (active) {
        p.setPen(QPen(QColor(0xCC, 0x88, 0xFF, 120), 1));
        p.setBrush(QColor(0xCC, 0x88, 0xFF, 40));
        p.drawRoundedRect(1, 1, ICON_SZ - 2, ICON_SZ - 2, 3, 3);
    }
}

// Iconos específicos

/**
 * 🔗 Conectar — Símbolo Bluetooth estilizado (doble zigzag + punto).
 */
inline QIcon iconConnect(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrConnect();
    QPen pen(c, 1.6);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Zigzag Bluetooth: dos arcos en espejo
    p.drawLine(11, 3,  6, 11);
    p.drawLine(6, 11, 11, 11);
    p.drawLine(11, 11, 6, 19);
    p.drawLine(11, 3,  16, 11);
    p.drawLine(16, 11, 11, 11);
    p.drawLine(11, 11, 16, 19);

    // Punto representando "señal"
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(11, 2), 1.2, 1.2);

    p.end();
    return QIcon(px);
}

/**
 * 🔌 Desconectar — Enchufe con aspa de desconexión.
 */
inline QIcon iconDisconnect(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrDisconn();
    QPen pen(c, 1.6);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Cuerpo del enchufe (rectángulo pequeño)
    p.drawRect(7, 3, 8, 12);
    // Clavija central
    p.drawLine(11, 15, 11, 19);
    // Clavijas laterales
    p.drawLine(7, 17, 7, 19);
    p.drawLine(15, 17, 15, 19);

    // Aspa de desconexión (X roja)
    QPen xPen(QColor(0xFF, 0x44, 0x44), 2.2);
    xPen.setCapStyle(Qt::RoundCap);
    p.setPen(xPen);
    p.drawLine(2, 2, 20, 20);
    p.drawLine(20, 2, 2, 20);

    p.end();
    return QIcon(px);
}

/**
 * ▶ Escanear — Triángulo de reproducción con ondas de radar.
 */
inline QIcon iconScan(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrScan();
    QPen pen(c, 1.8);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Triángulo "play"
    QPolygonF tri;
    tri << QPointF(6, 4) << QPointF(18, 11) << QPointF(6, 18);
    p.drawPolygon(tri);

    // Ondas de radar a la derecha
    pen.setWidth(1.2);
    p.setPen(pen);
    p.drawArc(15, 6, 6, 10,  -30 * 16, 60 * 16);  // arco pequeño
    p.drawArc(13, 4, 10, 14, -30 * 16, 60 * 16);  // arco grande

    p.end();
    return QIcon(px);
}

/**
 * ■ Detener — Cuadrado de stop con círculo interior.
 */
inline QIcon iconStop(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrStop();
    p.setPen(Qt::NoPen);
    p.setBrush(c);
    p.drawRoundedRect(5, 5, 12, 12, 2, 2);

    p.end();
    return QIcon(px);
}

/**
 * 🗑 Limpiar — Bote de basura estilizado.
 */
inline QIcon iconClear(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrClear();
    QPen pen(c, 1.5);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Tapa
    p.drawLine(5, 5, 17, 5);
    p.drawLine(7, 5, 8, 2);
    p.drawLine(15, 5, 14, 2);
    p.drawLine(8, 2, 14, 2);

    // Cuerpo
    p.drawRect(6, 7, 10, 13);

    // Líneas verticales (costillas del bote)
    p.drawLine(9, 9, 9, 17);
    p.drawLine(13, 9, 13, 17);

    p.end();
    return QIcon(px);
}

/**
 * 💾 Exportar — Flecha hacia abajo sobre base / disquete abstracto.
 */
inline QIcon iconExport(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrExport();
    QPen pen(c, 1.6);
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // Flecha hacia abajo
    p.drawLine(HALF, 2, HALF, 16);
    p.drawLine(HALF - 5, 11, HALF, 16);
    p.drawLine(HALF + 5, 11, HALF, 16);

    // Base (línea horizontal)
    p.drawLine(3, 18, 19, 18);

    p.end();
    return QIcon(px);
}

/**
 * 🎮 Demo — Gamepad estilizado.
 */
inline QIcon iconDemo(bool active = false) {
    QPixmap px = makeCanvas();
    QPainter p(&px);
    p.setRenderHint(QPainter::Antialiasing);
    drawBorder(p, active);

    QColor c = clrDemo();
    QPen pen(c, 1.5);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    if (active) {
        // Activo: gamepad relleno
        p.setBrush(QColor(0xCC, 0x88, 0xFF, 60));
    }

    // Cuerpo del gamepad (forma de U redondeada)
    QPainterPath gp;
    gp.moveTo(4, 10);
    gp.cubicTo(4, 4, 18, 4, 18, 10);
    gp.lineTo(18, 14);
    gp.cubicTo(18, 16, 16, 16, 15, 14);
    gp.lineTo(14, 10);
    gp.moveTo(7, 10);
    gp.lineTo(8, 14);
    gp.cubicTo(8, 16, 6, 16, 6, 14);
    gp.lineTo(4, 10);
    p.drawPath(gp);

    // D-pad (cruz direccional)
    int dpadX = 9, dpadY = 9;
    p.drawLine(dpadX, dpadY - 2, dpadX, dpadY + 2);
    p.drawLine(dpadX - 2, dpadY, dpadX + 2, dpadY);

    // Botones de acción (rellenos para mejor visibilidad)
    p.setBrush(c);
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPointF(15, 8), 1.5, 1.5);
    p.drawEllipse(QPointF(17, 10), 1.5, 1.5);

    p.end();
    return QIcon(px);
}

}  // namespace AppIcons
