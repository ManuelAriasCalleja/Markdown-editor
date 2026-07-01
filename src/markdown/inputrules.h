#ifndef INPUTRULES_H
#define INPUTRULES_H

/// \file
/// \brief Reglas de entrada: reconocer un marcador Markdown de bloque recién tecleado.

#include <QString>

namespace mdinputrules {

/// Tipo de transformación de bloque que dispara un marcador tecleado.
enum class Kind { None, Heading, Quote, BulletList, NumberedList };

/// Regla a aplicar; `level` (1..6) solo se usa con Heading.
struct Rule
{
    Kind kind = Kind::None;
    int level = 0;
};

/// A partir del texto del bloque DESDE SU INICIO hasta el cursor (lo que precede al
/// espacio que se acaba de teclear), devuelve la regla si ese texto es exactamente un
/// marcador de bloque: `#`…`######`, `>`, `-`/`*`/`+`, o `N.`/`N)`. Si no, None.
/// Función pura (sin GUI), probada aislada.
Rule ruleForPrefix(const QString &beforeCursor);

}  // namespace mdinputrules

#endif  // INPUTRULES_H
