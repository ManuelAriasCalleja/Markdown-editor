/// \file
/// \brief Implementación de TableController: edición de la tabla bajo el cursor y estado de sus acciones.

#include "tablecontroller.h"

#include <QAction>
#include <QTextEdit>
#include <QTextTable>

#include "documentio.h"
#include "focuseditor.h"
#include "splitviewcontroller.h"

TableController::TableController(QTextEdit *editor, SplitViewController *split,
                                 DocumentIo *documentIo, QObject *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_split(split)
    , m_documentIo(documentIo)
{
}

void TableController::insertRow(bool below)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const QTextTableCell cell = table->cellAt(cursor);
    table->insertRows(cell.row() + (below ? 1 : 0), 1);
    m_editor->setFocus();
}

void TableController::insertColumn(bool right)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const QTextTableCell cell = table->cellAt(cursor);
    table->insertColumns(cell.column() + (right ? 1 : 0), 1);
    m_editor->setFocus();
}

void TableController::deleteRow()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table || table->rows() <= 1)  // dejar al menos la fila de cabecera
        return;
    table->removeRows(table->cellAt(cursor).row(), 1);
    m_editor->setFocus();
}

void TableController::deleteColumn()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table || table->columns() <= 1)
        return;
    table->removeColumns(table->cellAt(cursor).column(), 1);
    m_editor->setFocus();
}

void TableController::alignColumn(Qt::Alignment alignment)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const int col = table->cellAt(cursor).column();

    cursor.beginEditBlock();
    // Se alinean todas las filas (incluida la cabecera) para que la columna se
    // vea homogénea y mdtable detecte la alineación de forma fiable.
    for (int r = 0; r < table->rows(); ++r) {
        const QTextTableCell c = table->cellAt(r, col);
        QTextCursor cc = c.firstCursorPosition();
        cc.setPosition(c.lastCursorPosition().position(), QTextCursor::KeepAnchor);
        QTextBlockFormat bf;
        bf.setAlignment(alignment);
        cc.mergeBlockFormat(bf);
    }
    cursor.endEditBlock();
    // El contenido cambió (la alineación va al Markdown vía mdtable): refresca el
    // estado de "modificado".
    emit modifiedChanged(m_documentIo->isModified());
    m_editor->setFocus();
}

void TableController::updateActions()
{
    // En vista dividida, las acciones de tabla solo cuando el foco está en el
    // WYSIWYG (es donde viven las tablas); con el foco en el fuente, inhabilitadas.
    const bool wysiwygActive = !m_split->sourceMode()
                               && (!m_split->splitMode() || !m_split->sourceEditor()->hasFocus());
    const bool inTable = wysiwygActive && m_editor->textCursor().currentTable() != nullptr;
    for (QAction *a : m_actions)
        a->setEnabled(inTable);
}
