#include <QtTest>

#include <QTextCursor>
#include <QTextEdit>

#include "blockconstructs.h"

// Pruebas de los constructos de bloque: las transformaciones de texto puras y
// el alternado completo sobre un QTextEdit (Template Method).
class TestBlockConstructs : public QObject
{
    Q_OBJECT

private slots:
    // --- Funciones puras (mdblock) ---
    void addQuotePrefixesEachLine();
    void removeQuoteStripsPrefix();
    void removeQuoteHandlesBareGreaterThan();
    void quoteRoundTripsAtTextLevel();
    void fenceWrapsInBackticks();

    // --- Alternado sobre el editor ---
    void toggleBlockquoteWrapsAndUnwraps();
    void toggleCodeBlockWrapsAndUnwraps();
    void toggleCodeBlockKeepsContentLiteral();
};

void TestBlockConstructs::addQuotePrefixesEachLine()
{
    QCOMPARE(mdblock::addBlockquoteMarkers(QStringLiteral("a\nb")),
             QStringLiteral("> a\n> b"));
}

void TestBlockConstructs::removeQuoteStripsPrefix()
{
    QCOMPARE(mdblock::removeBlockquoteMarkers(QStringLiteral("> a\n> b")),
             QStringLiteral("a\nb"));
}

void TestBlockConstructs::removeQuoteHandlesBareGreaterThan()
{
    // Sin espacio tras '>'.
    QCOMPARE(mdblock::removeBlockquoteMarkers(QStringLiteral(">x")),
             QStringLiteral("x"));
}

void TestBlockConstructs::quoteRoundTripsAtTextLevel()
{
    const QString original = QStringLiteral("hola\nmundo");
    QCOMPARE(mdblock::removeBlockquoteMarkers(
                 mdblock::addBlockquoteMarkers(original)),
             original);
}

void TestBlockConstructs::fenceWrapsInBackticks()
{
    QCOMPARE(mdblock::fenceCode(QStringLiteral("int x;")),
             QStringLiteral("```\nint x;\n```"));
}

void TestBlockConstructs::toggleBlockquoteWrapsAndUnwraps()
{
    QTextEdit edit;
    edit.setPlainText(QStringLiteral("hola"));
    edit.selectAll();

    Blockquote().toggle(&edit);
    QVERIFY(edit.document()->toMarkdown().contains(QStringLiteral("> hola")));

    // Con el cursor dentro de la cita, alternar de nuevo la quita.
    edit.moveCursor(QTextCursor::Start);
    Blockquote().toggle(&edit);
    const QString md = edit.document()->toMarkdown();
    QVERIFY(!md.contains(QLatin1Char('>')));
    QVERIFY(md.contains(QStringLiteral("hola")));
}

void TestBlockConstructs::toggleCodeBlockWrapsAndUnwraps()
{
    QTextEdit edit;
    edit.setPlainText(QStringLiteral("codigo"));
    edit.selectAll();

    CodeBlock().toggle(&edit);
    QVERIFY(edit.document()->toMarkdown().contains(QStringLiteral("```")));

    edit.moveCursor(QTextCursor::Start);
    CodeBlock().toggle(&edit);
    const QString md = edit.document()->toMarkdown();
    QVERIFY(!md.contains(QStringLiteral("```")));
    QVERIFY(md.contains(QStringLiteral("codigo")));
}

void TestBlockConstructs::toggleCodeBlockKeepsContentLiteral()
{
    // Dentro de un bloque de código, '#' y '*' no deben interpretarse.
    QTextEdit edit;
    edit.setPlainText(QStringLiteral("# no es titulo"));
    edit.selectAll();

    CodeBlock().toggle(&edit);
    const QString md = edit.document()->toMarkdown();
    QVERIFY(md.contains(QStringLiteral("# no es titulo")));
    QVERIFY(md.contains(QStringLiteral("```")));
}

QTEST_MAIN(TestBlockConstructs)
#include "tst_blockconstructs.moc"
