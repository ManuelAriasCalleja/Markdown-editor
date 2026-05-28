#ifndef TABLEEDIT_H
#define TABLEEDIT_H

#include <QList>
#include <QString>
#include <Qt>

class QTextDocument;

// Preservación de la alineación de columnas de las tablas Markdown.
//
// QTextDocument::toMarkdown() de Qt NO emite los marcadores de alineación
// (`:--`, `:-:`, `--:`): los lee al abrir (alinea las celdas) pero los descarta
// al guardar. Para que la alineación sobreviva al round-trip, reinyectamos los
// marcadores en las líneas separadoras a partir de la alineación real de las
// celdas del documento. Toda la serialización canónica del documento pasa por
// `documentMarkdown()`.
namespace mdtable {

// Alineación horizontal de cada columna de cada tabla del documento, en orden de
// aparición. Cada elemento es la lista de alineaciones (una por columna) de una
// tabla. Solo se conservan los bits horizontales (AlignLeft/HCenter/Right).
QList<QList<Qt::Alignment>> columnAlignments(const QTextDocument *doc);

// Reescribe las líneas separadoras de las tablas de `md` con el marcador de
// alineación de cada columna, según `alignments` (en orden de aparición). Si el
// nº de columnas no cuadra con una tabla, esa se deja intacta. Función pura.
QString injectAlignments(const QString &md,
                         const QList<QList<Qt::Alignment>> &alignments);

// Markdown del documento con la alineación de las tablas preservada.
QString documentMarkdown(const QTextDocument *doc);

} // namespace mdtable

#endif // TABLEEDIT_H
