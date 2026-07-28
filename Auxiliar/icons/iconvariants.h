#ifndef ICONVARIANTS_H
#define ICONVARIANTS_H

/// \file
/// \brief Banco de pruebas: variantes alternativas de los iconos de la barra de
/// formato. No forma parte de la compilación del editor; es material de estudio
/// para elegir un diseño antes de tocar `src/widgets/formaticons.cpp`.

#include <QColor>
#include <QIcon>
#include <QPixmap>

namespace iconvariants {

enum class ListKind { Bullet, Numbered, Task };
enum class FormatKind { Bold, Italic, Underline, Strike };

/// Variantes de los tres iconos de lista.
enum class ListVariant {
    Actual,    ///< el de hoy: 3 filas, dígitos de fuente, casillas pequeñas
    Aire,      ///< 2 filas: marcadores al doble de tamaño, dígitos de fuente
    Trazo,     ///< 3 filas, pero los dígitos dibujados con el mismo trazo que las líneas
    Cifra,     ///< 2 filas + dígitos de trazo + casilla grande (Aire ∩ Trazo)
    Check,     ///< como Cifra, pero las tareas son marcas de verificación sin casilla
};

/// Variantes de los cuatro iconos de formato de carácter.
enum class FormatVariant {
    Actual,    ///< el de hoy: N/C/S/T centradas por caja de línea
    Optico,    ///< misma letra, centrada por su altura real y con hueco para el efecto
};

QPixmap listPixmap(ListVariant v, ListKind k, const QColor &color, int px, qreal dpr);
QPixmap formatPixmap(FormatVariant v, FormatKind k, const QColor &color, int px, qreal dpr);

} // namespace iconvariants

#endif // ICONVARIANTS_H
