#include "tasklist.h"

#include <QTextBlock>
#include <QTextEdit>

namespace {

// Bloque de tarea bajo el punto y borde izquierdo del texto del ítem. Devuelve un
// bloque inválido si el punto no cae sobre un ítem de tarea o si el clic está a la
// derecha de ese borde (es decir, sobre el texto, no sobre la casilla).
QTextBlock checkboxBlockAt(QTextEdit *editor, const QPoint &viewportPos)
{
    if (!editor)
        return QTextBlock();
    const QTextBlock block = editor->cursorForPosition(viewportPos).block();
    if (!block.isValid()
        || block.blockFormat().marker() == QTextBlockFormat::MarkerType::NoMarker)
        return QTextBlock();
    // La casilla se dibuja a la izquierda del texto del ítem; el rectángulo del
    // cursor en el inicio del bloque marca el borde izquierdo de ese texto. Un
    // clic más a la izquierda cae sobre la casilla (o la sangría del marcador).
    QTextCursor atStart(block);
    if (viewportPos.x() >= editor->cursorRect(atStart).left())
        return QTextBlock();
    return block;
}

}  // namespace

bool mdtask::isCheckboxAt(QTextEdit *editor, const QPoint &viewportPos)
{
    return checkboxBlockAt(editor, viewportPos).isValid();
}

bool mdtask::toggleCheckboxAt(QTextEdit *editor, const QPoint &viewportPos)
{
    const QTextBlock block = checkboxBlockAt(editor, viewportPos);
    if (!block.isValid())
        return false;

    QTextBlockFormat bf = block.blockFormat();
    bf.setMarker(bf.marker() == QTextBlockFormat::MarkerType::Checked
                     ? QTextBlockFormat::MarkerType::Unchecked
                     : QTextBlockFormat::MarkerType::Checked);
    QTextCursor edit(block);
    edit.setBlockFormat(bf);  // edición deshacible; marca el documento como modificado
    return true;
}
