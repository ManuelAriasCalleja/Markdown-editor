/// \file
/// \brief Implementación de `removePreviewBlocks`: borra del documento los bloques de preview.

#include "diagramdoc.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace mddiagram {

void removePreviewBlocks(QTextDocument *doc)
{
    // Recoge las posiciones de los bloques de preview y bórralos de atrás hacia
    // delante, para no invalidar posiciones al ir eliminando.
    QList<int> positions;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().boolProperty(PreviewBlockProperty))
            positions.append(b.position());
    }
    QTextCursor c(doc);
    c.beginEditBlock();
    for (auto it = positions.crbegin(); it != positions.crend(); ++it) {
        QTextCursor del(doc);
        del.setPosition(*it);
        const QTextBlock block = del.block();
        del.movePosition(QTextCursor::StartOfBlock);
        if (block.next().isValid()) {
            // Selecciona el bloque y su separador de párrafo (hasta el inicio del
            // siguiente), para no dejar una línea vacía.
            del.setPosition(block.next().position(), QTextCursor::KeepAnchor);
        } else {
            // Último bloque: come el separador anterior para no dejar línea vacía.
            del.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            if (block.position() > 0) {
                del.setPosition(block.position() - 1);
                del.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
        }
        del.removeSelectedText();
    }
    c.endEditBlock();
}

} // namespace mddiagram
