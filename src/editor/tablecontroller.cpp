/// \file
/// \brief Implementación de TableController: edición de la tabla bajo el cursor y estado de sus acciones.

#include "tablecontroller.h"

#include <QAction>
#include <QList>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextTable>

#include "documentio.h"
#include "focuseditor.h"
#include "splitviewcontroller.h"
#include "tablesort.h"

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

void TableController::sortByColumn(bool ascending)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const int cols = table->columns();
    const int rows = table->rows();
    const int bodyCount = rows - 1;  // fila 0 = cabecera, se mantiene fija
    if (bodyCount < 2)
        return;  // 0 o 1 fila de datos: nada que ordenar
    const int col = table->cellAt(cursor).column();

    // Instantánea del contenido de cada celda del cuerpo como fragmento (preserva
    // formato de carácter, fórmulas y el objeto MathObject). Se copia TODO antes de
    // tocar nada, así que sobrescribir después no invalida lo guardado.
    QList<QList<QTextDocumentFragment>> body;
    body.reserve(bodyCount);
    for (int r = 1; r < rows; ++r) {
        QList<QTextDocumentFragment> cells;
        cells.reserve(cols);
        for (int c = 0; c < cols; ++c) {
            const QTextTableCell cell = table->cellAt(r, c);
            QTextCursor cc = cell.firstCursorPosition();
            cc.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
            cells.append(cc.selection());
        }
        body.append(cells);
    }

    // Claves de ordenación: el texto plano de la columna elegida.
    QStringList keys;
    keys.reserve(bodyCount);
    for (int r = 0; r < bodyCount; ++r)
        keys.append(body.at(r).at(col).toPlainText().trimmed());

    const bool numeric = mdtablesort::looksNumeric(keys);
    const QList<int> order = mdtablesort::sortedOrder(keys, numeric, ascending);

    bool changed = false;
    for (int i = 0; i < order.size(); ++i)
        if (order.at(i) != i) {
            changed = true;
            break;
        }
    if (!changed)
        return;  // ya estaba ordenada: no ensuciar el documento

    // Reescribe cada celda del cuerpo con el fragmento de la fila de origen. La
    // alineación es por columna (posicional), así que se conserva al reordenar filas.
    cursor.beginEditBlock();
    for (int i = 0; i < bodyCount; ++i) {
        const int src = order.at(i);
        const int destRow = i + 1;  // +1: saltar la cabecera
        for (int c = 0; c < cols; ++c) {
            const QTextTableCell cell = table->cellAt(destRow, c);
            QTextCursor cc = cell.firstCursorPosition();
            cc.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
            cc.removeSelectedText();
            cc.insertFragment(body.at(src).at(c));
        }
    }
    cursor.endEditBlock();

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
