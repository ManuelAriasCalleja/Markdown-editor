#include <QtTest>

#include <QCoreApplication>
#include <QSettings>
#include <QStringList>
#include <QTextDocumentFragment>
#include <QTextFrame>
#include <QTextTable>

#include "editorstack.h"
#include "focuseditor.h"
#include "mainwindow.h"
#include "tablecontroller.h"
#include "tableedit.h"

// Integración de «ordenar filas por columna» (#10): sobre una tabla real del
// editor WYSIWYG, reordena el cuerpo por la columna del cursor, mantiene la
// cabecera fija y preserva el formato de las celdas. Friend de MainWindow.
class TestTableSortUi : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    void sortsBodyAscendingKeepingHeader();
    void sortsDescending();
    void preservesInlineFormatting();

private:
    static QTextTable *firstTable(const QTextDocument *doc);
    static QString cellText(QTextTable *table, int row, int col);
    static QStringList columnBody(QTextTable *table, int col);  // celdas del cuerpo (sin cabecera)
    static void cursorInCell(MainWindow &w, QTextTable *table, int row, int col);
};

void TestTableSortUi::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestTableSortUi::cleanup()
{
    QSettings().clear();
}

QTextTable *TestTableSortUi::firstTable(const QTextDocument *doc)
{
    for (QTextFrame *f : doc->rootFrame()->childFrames())
        if (auto *t = qobject_cast<QTextTable *>(f))
            return t;
    return nullptr;
}

QString TestTableSortUi::cellText(QTextTable *table, int row, int col)
{
    const QTextTableCell cell = table->cellAt(row, col);
    QTextCursor cc = cell.firstCursorPosition();
    cc.setPosition(cell.lastCursorPosition().position(), QTextCursor::KeepAnchor);
    return cc.selection().toPlainText().trimmed();
}

QStringList TestTableSortUi::columnBody(QTextTable *table, int col)
{
    QStringList out;
    for (int r = 1; r < table->rows(); ++r)
        out << cellText(table, r, col);
    return out;
}

void TestTableSortUi::cursorInCell(MainWindow &w, QTextTable *table, int row, int col)
{
    w.m_stack->editor()->setTextCursor(table->cellAt(row, col).firstCursorPosition());
}

void TestTableSortUi::sortsBodyAscendingKeepingHeader()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral(
        "| N | Nombre |\n|---|--------|\n| 3 | c |\n| 1 | a |\n| 2 | b |\n"));
    QTextTable *table = firstTable(w.m_stack->editor()->document());
    QVERIFY(table);
    cursorInCell(w, table, 1, 0);  // columna N

    w.m_stack->table()->sortByColumn(true);

    // La cabecera se mantiene; el cuerpo queda 1,2,3 y su segunda columna a,b,c.
    QCOMPARE(cellText(table, 0, 0), QStringLiteral("N"));
    QCOMPARE(cellText(table, 0, 1), QStringLiteral("Nombre"));
    QCOMPARE(columnBody(table, 0), (QStringList{"1", "2", "3"}));
    QCOMPARE(columnBody(table, 1), (QStringList{"a", "b", "c"}));  // la fila entera se mueve junta
}

void TestTableSortUi::sortsDescending()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral(
        "| N |\n|---|\n| 3 |\n| 1 |\n| 2 |\n"));
    QTextTable *table = firstTable(w.m_stack->editor()->document());
    QVERIFY(table);
    cursorInCell(w, table, 1, 0);

    w.m_stack->table()->sortByColumn(false);
    QCOMPARE(columnBody(table, 0), (QStringList{"3", "2", "1"}));
}

void TestTableSortUi::preservesInlineFormatting()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral(
        "| N | Nota |\n|---|------|\n| 2 | normal |\n| 1 | **negrita** |\n"));
    QTextTable *table = firstTable(w.m_stack->editor()->document());
    QVERIFY(table);
    cursorInCell(w, table, 1, 0);

    w.m_stack->table()->sortByColumn(true);  // la fila «1 | negrita» pasa a ir primero
    QCOMPARE(columnBody(table, 0), (QStringList{"1", "2"}));

    // El formato en negrita viaja con la fila reordenada (serializa como **…**).
    const QString md = mdtable::documentMarkdown(w.m_stack->editor()->document());
    QVERIFY(md.contains(QStringLiteral("**negrita**")));
}

QTEST_MAIN(TestTableSortUi)
#include "tst_tablesortui.moc"
