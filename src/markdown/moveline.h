#ifndef MOVELINE_H
#define MOVELINE_H

/// \file
/// \brief Comandos de línea puros para la vista de código: mover, duplicar, borrar, unir.

#include <QStringList>

namespace mdmoveline {

/// Resultado de un comando de línea: las líneas resultantes y la línea (0-based)
/// donde debe quedar el cursor.
struct Result
{
    QStringList lines;
    int line = 0;
};

/// Mueve la línea `line` una posición hacia arriba (la intercambia con la anterior).
/// No-op (devuelve la entrada tal cual) si `line` es la primera o está fuera de rango.
Result moveUp(const QStringList &lines, int line);

/// Mueve la línea `line` una posición hacia abajo. No-op si es la última o fuera de rango.
Result moveDown(const QStringList &lines, int line);

/// Duplica la línea `line` (inserta una copia justo debajo); el cursor se queda en
/// la original. No-op si `line` está fuera de rango.
Result duplicate(const QStringList &lines, int line);

/// Borra la línea `line`; el cursor pasa a la que ocupa su lugar (o a la última). Si
/// era la única, queda una línea vacía. No-op si `line` está fuera de rango.
Result removeLine(const QStringList &lines, int line);

/// Une la línea `line` con la siguiente (una sola, quitando el espacio inicial de la
/// siguiente y el final de esta). No-op si `line` es la última o está fuera de rango.
Result joinNext(const QStringList &lines, int line);

}  // namespace mdmoveline

#endif  // MOVELINE_H
