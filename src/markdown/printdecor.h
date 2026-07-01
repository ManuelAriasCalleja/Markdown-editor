#ifndef PRINTDECOR_H
#define PRINTDECOR_H

/// \file
/// \brief Lógica pura de la decoración de impresión (número de página).

#include <QString>

namespace mdprintdecor {

/// Texto del número de página en el pie: `N / M` (neutro respecto al idioma, sin
/// palabras, así que no necesita traducción). `page` y `total` son 1-based.
/// Función pura, testeable.
QString pageNumberText(int page, int total);

}  // namespace mdprintdecor

#endif  // PRINTDECOR_H
