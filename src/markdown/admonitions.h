#ifndef ADMONITIONS_H
#define ADMONITIONS_H

/// \file
/// \brief Admoniciones / «callouts» estilo GitHub: detección de marcadores
///        `[!NOTE]`/`[!TIP]`/… y estilo de «callout» sobre el documento.

#include <QColor>
#include <QList>
#include <QString>

class QTextDocument;
class QTextBlockFormat;

/// Admoniciones / «callouts» estilo GitHub: una cita cuya primera línea es un
/// marcador `[!NOTE]`, `[!TIP]`, `[!IMPORTANT]`, `[!WARNING]` o `[!CAUTION]`.
///
/// El round-trip es transparente: Qt trata el bloque como una cita normal y deja
/// el marcador `[!TIPO]` como texto literal (igual que las notas al pie con
/// `[^id]`), así que **no hace falta tocar la serialización**. Este módulo solo
/// añade el estilo de «callout» (fondo tintado + título en color) al maquetar, y
/// la inserción del esqueleto desde el menú. La parte de detección es pura y se
/// prueba aislada (`tst_admonitions`).
namespace mdadmonition {

/// Un tipo de admonición reconocido.
struct Type {
    QString keyword;  ///< marcador canónico en mayúsculas, p. ej. "NOTE"
    QColor accent;    ///< color del título y del tinte de fondo
};

/// Los cinco tipos reconocidos, en orden de menú.
const QList<Type> &types();

/// Si la primera línea de `blockText` es exactamente un marcador `[!TIPO]` (admite
/// espacios alrededor y cualquier combinación de mayúsculas/minúsculas), devuelve
/// el keyword canónico en mayúsculas; si no, "".
QString markerKeyword(const QString &blockText);

/// Color de acento de un keyword (devuelto por markerKeyword), o un color por
/// defecto si no se reconoce.
QColor accentFor(const QString &keyword);

/// El Markdown de un esqueleto de admonición del tipo dado, listo para insertar:
///   > [!TIPO]
///   >
///   > (contenido)
QString skeleton(const QString &keyword);

/// Da estilo de «callout» a todas las citas del documento que empiezan por un
/// marcador. No cambia ningún texto (idempotente, seguro para el round-trip).
void renderAdmonitionsInDocument(QTextDocument *doc);

/// `QTextDocument::toMarkdown()` escapa el corchete del marcador (`> \[!NOTE]`),
/// lo que GitHub ya no reconoce como admonición. Esta función deshace ese escape
/// en las líneas de cita (`> [!NOTE]`). La aplica la serialización canónica
/// (`mdtable::documentMarkdown`) sobre la salida de `toMarkdown`. Función pura.
QString unescapeMarkers(const QString &markdown);

}  // namespace mdadmonition

#endif  // ADMONITIONS_H
