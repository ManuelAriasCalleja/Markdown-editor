#ifndef LINEHIGHLIGHT_H
#define LINEHIGHLIGHT_H

/// \file
/// \brief Lógica pura del color de resaltado de la línea actual.

#include <QColor>

namespace mdlinehighlight {

/// Color de fondo para la línea del cursor: mezcla `base` (fondo del editor) hacia
/// `highlight` (color de selección de la paleta) una fracción pequeña `weight`
/// (0–100), para un subrayado sutil que no estorbe la lectura ni pierda contraste.
/// `weight` 0 devuelve `base`; 100 devuelve `highlight`. Función pura (sin GUI).
QColor currentLineColor(const QColor &base, const QColor &highlight, int weight = 18);

}  // namespace mdlinehighlight

#endif  // LINEHIGHLIGHT_H
