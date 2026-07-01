#include <QtTest>

#include <QCoreApplication>
#include <QSettings>
#include <QTextCursor>
#include <QTextEdit>

#include "editorstack.h"
#include "focuseditor.h"
#include "mainwindow.h"
#include "splitviewcontroller.h"

// Integración de los comandos de línea (#17) sobre el editor de código: mover,
// duplicar, borrar y unir, con el cursor donde toca. Friend de MainWindow.
class TestLineCommands : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    void moveLineUpInSource();
    void duplicateLineInSource();
    void deleteLineInSource();
    void joinLinesInSource();
    void noOpInWysiwyg();

private:
    QTextEdit *source(MainWindow &w) { return w.m_stack->split()->sourceEditor(); }
    // Coloca el cursor del editor `ed` en la línea `n` (0-based).
    static void putCursorAtLine(QTextEdit *ed, int n);
};

void TestLineCommands::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestLineCommands::cleanup()
{
    QSettings().clear();
}

void TestLineCommands::putCursorAtLine(QTextEdit *ed, int n)
{
    QTextCursor c = ed->textCursor();
    c.movePosition(QTextCursor::Start);
    for (int i = 0; i < n; ++i)
        c.movePosition(QTextCursor::NextBlock);
    ed->setTextCursor(c);
}

void TestLineCommands::moveLineUpInSource()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);  // el editor activo pasa a ser el fuente
    source(w)->setPlainText(QStringLiteral("a\nb\nc"));
    putCursorAtLine(source(w), 1);  // «b»

    w.m_stack->moveLineUp();
    QCOMPARE(source(w)->toPlainText(), QStringLiteral("b\na\nc"));
    QCOMPARE(source(w)->textCursor().blockNumber(), 0);  // el cursor sigue a «b»
}

void TestLineCommands::duplicateLineInSource()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);
    source(w)->setPlainText(QStringLiteral("uno\ndos"));
    putCursorAtLine(source(w), 0);

    w.m_stack->duplicateLine();
    QCOMPARE(source(w)->toPlainText(), QStringLiteral("uno\nuno\ndos"));
}

void TestLineCommands::deleteLineInSource()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);
    source(w)->setPlainText(QStringLiteral("a\nb\nc"));
    putCursorAtLine(source(w), 1);

    w.m_stack->deleteLine();
    QCOMPARE(source(w)->toPlainText(), QStringLiteral("a\nc"));
}

void TestLineCommands::joinLinesInSource()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);
    source(w)->setPlainText(QStringLiteral("hola  \n   mundo\nfin"));
    putCursorAtLine(source(w), 0);

    w.m_stack->joinLines();
    QCOMPARE(source(w)->toPlainText(), QStringLiteral("hola mundo\nfin"));
}

void TestLineCommands::noOpInWysiwyg()
{
    MainWindow w;
    w.show();
    // En WYSIWYG (editor activo = el visual), los comandos de línea no hacen nada.
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n"));
    const QString before = w.m_stack->editor()->toPlainText();
    w.m_stack->moveLineDown();
    w.m_stack->duplicateLine();
    QCOMPARE(w.m_stack->editor()->toPlainText(), before);
}

QTEST_MAIN(TestLineCommands)
#include "tst_linecommands.moc"
