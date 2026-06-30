/// \file
/// \brief Implementación de la conversión HTML→Markdown del portapapeles.

#include "richpaste.h"

#include "tableedit.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>

namespace mdrichpaste {

QString htmlToMarkdown(const QString &html)
{
    if (html.isEmpty())
        return QString();

    // El parser de HTML de Qt llena un QTextDocument con formatos de carácter y
    // de bloque; serializarlo a Markdown descarta lo que Markdown no expresa
    // (colores, fuentes, spans) y deja solo negrita/cursiva/listas/enlaces/etc.
    QTextDocument doc;
    doc.setHtml(html);

    // Ruta de serialización canónica del proyecto (reinyecta alineación de tablas
    // y fórmulas), no `toMarkdown()` directo.
    QString md = mdtable::documentMarkdown(&doc);

    // `toMarkdown()` cierra siempre con un salto de línea; al insertar en el punto
    // del cursor no queremos forzar un párrafo extra.
    while (md.endsWith(u'\n'))
        md.chop(1);
    return md;
}

QString fragmentToMarkdown(const QTextDocumentFragment &fragment)
{
    if (fragment.isEmpty())
        return QString();

    // Volcamos el fragmento en un documento auxiliar para reusar la ruta canónica
    // (que opera sobre un QTextDocument). insertFragment conserva los formatos de
    // carácter del original, incluidas las propiedades de los grupos de math, así
    // que documentMarkdown puede reinyectar `$tex$`.
    QTextDocument doc;
    QTextCursor cursor(&doc);
    cursor.insertFragment(fragment);

    QString md = mdtable::documentMarkdown(&doc);
    while (md.endsWith(u'\n'))  // sin el salto final que añade toMarkdown()
        md.chop(1);
    return md;
}

}  // namespace mdrichpaste
