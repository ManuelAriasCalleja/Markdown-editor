#ifndef SHORTCODES_H
#define SHORTCODES_H

#include <QString>

// Expansión de «shortcodes» `:nombre:` a símbolos (`:alpha:`→α, `:check:`→✓…),
// para escribir símbolos sin abrir el diálogo. Datos puros (sin GUI), para poder
// probarlos aislados; la detección al teclear vive en MainWindow.
namespace mdshortcode {

// Símbolo para `name` (sin los dos puntos), o cadena vacía si no se reconoce.
// Distingue mayúsculas/minúsculas (`:alpha:`→α, `:Omega:`→Ω).
QString expand(const QString &name);

}  // namespace mdshortcode

#endif  // SHORTCODES_H
