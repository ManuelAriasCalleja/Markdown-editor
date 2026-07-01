#ifndef FORMATICONS_H
#define FORMATICONS_H

/// \file
/// \brief Generación de los iconos monocromos de la barra de formato.

#include <QColor>
#include <QIcon>

/// \brief Generación de los iconos monocromos de la barra de formato (negrita, listas…)
/// y el color de tinta con contraste según el fondo. Son funciones PURAS (solo
/// QPainter sobre un QPixmap), sin estado ni dependencia de MainWindow, extraídas
/// de mainwindow.cpp para aligerarlo. El color se pasa desde fuera (lo deriva el
/// tema), así que los iconos siguen al tema claro/oscuro y al zoom (tamaño en px).
namespace formaticons {

/// \brief Tinta (casi negra o casi blanca) que mejor contrasta con `background`, según
/// la luminancia relativa sRGB. Para que el glifo del icono se lea sobre el fondo
/// de la barra sea cual sea el tema.
QColor contrastingInk(const QColor &background);

enum class ListIconKind { Bullet, Numbered, Task };
/// \brief Icono de un botón de lista (viñeta / numerada / tareas): tres «líneas de
/// texto» con su marcador a la izquierda, en `color`, de `px` px (con `dpr`).
QIcon makeListIcon(ListIconKind kind, const QColor &color, int px, qreal dpr);

enum class FormatIconKind { Bold, Italic, Underline, Strike };
/// \brief Icono de un botón de formato de carácter: la inicial española del efecto
/// pintada con ese mismo efecto (N negrita, C cursiva, S subrayada, T tachada).
QIcon makeFormatIcon(FormatIconKind kind, const QColor &color, int px, qreal dpr);

enum class TableIconKind {
    RowInsert, RowDelete, ColInsert, ColDelete, AlignLeft, AlignCenter, AlignRight
};
/// \brief Icono de un botón de la barra flotante de tabla: barras horizontales
/// (filas) o verticales (columnas) con un «+»/«−» para insertar/eliminar, y las tres
/// clásicas líneas alineadas para la alineación de columna. Monocromo, sigue al tema.
QIcon makeTableIcon(TableIconKind kind, const QColor &color, int px, qreal dpr);

} // namespace formaticons

#endif // FORMATICONS_H
