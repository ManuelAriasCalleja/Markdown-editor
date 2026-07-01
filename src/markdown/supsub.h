#ifndef SUPSUB_H
#define SUPSUB_H

/// \file
/// \brief Super/subíndice de texto `^x^` / `~x~` (carga, render y serialización fiel).

#include <QString>
#include <QTextFormat>

class QTextDocument;

namespace mdsupsub {

/// Propiedad PROPIA (R9) para distinguir el super/subíndice de TEXTO del de las
/// fórmulas (que usan AlignSuperScript con IsMathProperty). Su valor booleano `true`
/// marca un fragmento que `documentMarkdown` reinyecta como `^x^` / `~x~`. Un `false`
/// (tras quitar el super/sub) lo excluye, sin tener que borrar la propiedad.
constexpr int SupSubProperty = QTextFormat::UserProperty + 20;

/// Carga: envuelve `^x^` (superíndice) y `~x~` (subíndice, una sola `~`, no `~~`) en
/// centinelas PUA para que md4c no los toque (`~` es tachado en GFM). Pura, sobre el
/// markdown fuente.
QString protect(const QString &markdown);

/// Tras `setMarkdown`: convierte los centinelas en fragmentos con
/// AlignSuperScript/AlignSubScript + SupSubProperty, quitando los centinelas. En
/// bloques o spans de código (y fórmulas) los restaura como texto literal `^x^`/`~x~`
/// en vez de aplicar el formato. Idempotente (no quedan centinelas al terminar).
void renderInDocument(QTextDocument *doc);

/// Serialización (sobre un clon): reemplaza cada fragmento con SupSubProperty por un
/// par de centinelas PUA opacos a `toMarkdown` envolviendo su texto.
void replaceWithSentinels(QTextDocument *doc);

/// Restaura en el markdown ya serializado los centinelas PUA a `^x^` / `~x~`.
QString restoreFromSentinels(const QString &markdown);

}  // namespace mdsupsub

#endif  // SUPSUB_H
