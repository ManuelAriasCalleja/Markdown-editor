#ifndef NAV_H
#define NAV_H

/// \file
/// \brief Lógica pura de navegación por el documento (ir a línea; línea/columna del cursor).

namespace mdnav {

/// Acota un número de línea (1-based) al rango válido [1, blockCount] de un
/// documento con `blockCount` bloques. Si `blockCount` <= 0, devuelve 1.
/// Función pura (sin GUI, testeable aislada).
int clampLine(int line, int blockCount);

}  // namespace mdnav

#endif  // NAV_H
