#include <QtTest>

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QTextCursor>
#include <QTextEdit>

#include "editorstack.h"
#include "find.h"
#include "focuseditor.h"
#include "mainwindow.h"

// Caracterización del compositor de QTextEdit::extraSelections (R1) y de sus
// capas (#12 línea actual, #13 coincidencias). Fija la conducta OBSERVABLE
// —qué selecciones hay en el editor según el estado— para que la refactorización
// del único dueño de setExtraSelections no cambie el comportamiento y para que
// las capas coexistan. Es friend de MainWindow para tocar el stack activo.
class TestExtraSelections : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    // R1: el modo foco atenúa fuera del párrafo del cursor.
    void focusModeAddsSelections();
    void focusModeOffClearsSelections();

    // #12: resaltado de la línea actual (capa independiente que coexiste con el foco).
    void currentLineAddsFullWidthSelection();
    void currentLineCoexistsWithFocus();
    void currentLineOffClears();

    // #13: capa de coincidencias de búsqueda (coexiste con las demás).
    void searchMatchesAddSelections();
    void searchMatchesCoexistWithLineAndFocus();
    void searchMatchesClearOnEmpty();

private:
    // ¿Hay alguna ExtraSelection con FullWidthSelection (la de la línea actual)?
    static bool hasFullWidth(const QList<QTextEdit::ExtraSelection> &sels);
    // Coloca el cursor del editor WYSIWYG en el bloque `n` (0-based).
    static void placeCursorInBlock(QTextEdit *ed, int n);
};

void TestExtraSelections::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestExtraSelections::cleanup()
{
    QSettings().clear();
}

void TestExtraSelections::placeCursorInBlock(QTextEdit *ed, int n)
{
    QTextCursor c = ed->textCursor();
    c.movePosition(QTextCursor::Start);
    for (int i = 0; i < n; ++i)
        c.movePosition(QTextCursor::NextBlock);
    ed->setTextCursor(c);
}

void TestExtraSelections::focusModeAddsSelections()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);  // párrafo del medio

    w.m_stack->setTypewriterMode(true);
    // Con el cursor en el párrafo del medio hay texto que atenuar antes y después.
    QVERIFY(!w.m_stack->editor()->extraSelections().isEmpty());
}

void TestExtraSelections::focusModeOffClearsSelections()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);

    w.m_stack->setTypewriterMode(true);
    QVERIFY(!w.m_stack->editor()->extraSelections().isEmpty());
    w.m_stack->setTypewriterMode(false);
    QVERIFY(w.m_stack->editor()->extraSelections().isEmpty());
}

bool TestExtraSelections::hasFullWidth(const QList<QTextEdit::ExtraSelection> &sels)
{
    for (const QTextEdit::ExtraSelection &s : sels)
        if (s.format.boolProperty(QTextFormat::FullWidthSelection))
            return true;
    return false;
}

void TestExtraSelections::currentLineAddsFullWidthSelection()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);

    QVERIFY(!hasFullWidth(w.m_stack->editor()->extraSelections()));  // apagado por defecto
    w.m_stack->setCurrentLineHighlight(true);
    QVERIFY(hasFullWidth(w.m_stack->editor()->extraSelections()));
}

void TestExtraSelections::currentLineCoexistsWithFocus()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);

    // Las dos capas activas a la vez: debe haber la de línea (FullWidth) Y las de
    // atenuación del foco (varias), sin que una borre a la otra (esto es lo que R1
    // garantiza).
    w.m_stack->setCurrentLineHighlight(true);
    w.m_stack->setTypewriterMode(true);
    const QList<QTextEdit::ExtraSelection> sels = w.m_stack->editor()->extraSelections();
    QVERIFY(hasFullWidth(sels));       // capa de línea actual
    QVERIFY(sels.size() >= 2);         // + al menos un tramo de atenuación
}

void TestExtraSelections::currentLineOffClears()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);

    w.m_stack->setCurrentLineHighlight(true);
    QVERIFY(hasFullWidth(w.m_stack->editor()->extraSelections()));
    w.m_stack->setCurrentLineHighlight(false);
    QVERIFY(!hasFullWidth(w.m_stack->editor()->extraSelections()));
}

void TestExtraSelections::searchMatchesAddSelections()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno dos uno\n"));
    placeCursorInBlock(w.m_stack->editor(), 0);

    QCOMPARE(w.m_stack->editor()->extraSelections().size(), 0);
    w.m_stack->setSearchMatches({{0, 3}, {8, 3}});  // las dos apariciones de "uno"
    QCOMPARE(w.m_stack->editor()->extraSelections().size(), 2);
}

void TestExtraSelections::searchMatchesCoexistWithLineAndFocus()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos uno\n\ntres\n"));
    placeCursorInBlock(w.m_stack->editor(), 1);

    w.m_stack->setCurrentLineHighlight(true);
    w.m_stack->setTypewriterMode(true);
    w.m_stack->setSearchMatches({{0, 3}});
    const QList<QTextEdit::ExtraSelection> sels = w.m_stack->editor()->extraSelections();
    // Las tres capas a la vez: línea actual (FullWidth) + coincidencia + atenuación.
    QVERIFY(hasFullWidth(sels));
    QVERIFY(sels.size() >= 3);
}

void TestExtraSelections::searchMatchesClearOnEmpty()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno dos uno\n"));

    w.m_stack->setSearchMatches({{0, 3}});
    QVERIFY(!w.m_stack->editor()->extraSelections().isEmpty());
    w.m_stack->setSearchMatches({});
    QVERIFY(w.m_stack->editor()->extraSelections().isEmpty());
}

QTEST_MAIN(TestExtraSelections)
#include "tst_extraselections.moc"
