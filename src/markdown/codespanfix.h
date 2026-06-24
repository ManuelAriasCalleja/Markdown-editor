#ifndef CODESPANFIX_H
#define CODESPANFIX_H

/// \file
/// \brief Revierte el sobre-escapado que `toMarkdown` aplica al contenido de los
///        code spans en línea, para que no se acumule en cada guardado.

#include <QString>

/// Deshace el sobre-escapado que `QTextDocument::toMarkdown` aplica al CONTENIDO de
/// los code spans en línea. Qt antepone `\` a `\ & < * [ !` dentro del código,
/// aunque en un code span el contenido es literal (CommonMark no procesa escapes
/// ahí). Como `setMarkdown` sí lo re-lee literal, esos `\` se ACUMULAN en cada
/// guardado: `` `a\b` `` → `` `a\\b` `` → … y `` `a&b` `` → `` `a\&b` `` → … (pérdida
/// de integridad: una ruta `` `C:\x` `` se corrompe). Esta función revierte ese
/// escape SOLO dentro de los code spans en línea; respeta el texto normal (donde
/// `\*` sí es un escape legítimo) y los bloques de código vallados (```/~~~, donde
/// Qt no escapa nada). Pura, sin GUI, con `tst_codespanfix`.
namespace mdcodespan {

/// \brief Revierte el escape de Qt dentro de los code spans en línea de `markdown`.
QString unescapeInlineCode(const QString &markdown);

}  // namespace mdcodespan

#endif  // CODESPANFIX_H
