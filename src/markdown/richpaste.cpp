/// \file
/// \brief Implementación de la conversión HTML→Markdown del portapapeles.

#include "richpaste.h"

#include "tableedit.h"

#include <QTextDocument>

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

}  // namespace mdrichpaste
