#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>

#include "blockconstructs.h"
#include "mathblocks.h"
#include "tableedit.h"

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
    // --- Casos límite que antes fallaban (regresiones) ---
    void blockquoteQuotesEveryLineOfMultiline();
    void blockquotePreservesFormula();
    void codeBlockOnMiddleLineMakesFenceNotInline();
    void codeBlockOnEmptyParagraphCreatesBlock();
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

void TestBlockConstructs::blockquoteQuotesEveryLineOfMultiline()
{
    // Antes: insertFragment fusionaba el primer bloque y la primera línea
    // perdía la cita ("a" salía sin "> ").
    QTextEdit edit;
    edit.setPlainText(QStringLiteral("a\nb\nc"));
    edit.selectAll();

    Blockquote().toggle(&edit);
    const QString md = edit.document()->toMarkdown();
    QVERIFY2(md.contains(QStringLiteral("> a")), md.toUtf8());
    QVERIFY2(md.contains(QStringLiteral("> b")), md.toUtf8());
    QVERIFY2(md.contains(QStringLiteral("> c")), md.toUtf8());
}

void TestBlockConstructs::blockquotePreservesFormula()
{
    // Regresión: alternar cita sobre un párrafo con una fórmula TeX conservaba el
    // TeX (antes se aplanaba a glifos y se perdía en el guardado).
    QTextEdit edit;
    QTextCursor c = edit.textCursor();
    c.insertText(QStringLiteral("Antes "));
    const QTextCharFormat base = mdmath::mathCharFormat(QStringLiteral("E=mc^2"),
                                                        /*block=*/false);
    for (const mdmath::MathRun &r : mdmath::renderTexAsRuns(QStringLiteral("E=mc^2"), base))
        c.insertText(r.text, r.fmt);
    edit.setCurrentCharFormat(QTextCharFormat());
    c = edit.textCursor();
    c.insertText(QStringLiteral(" despues"));

    edit.selectAll();
    Blockquote().toggle(&edit);

    const QString md = mdtable::documentMarkdown(edit.document());
    QVERIFY2(md.contains(QStringLiteral("$E=mc^2$")), md.toUtf8());
    QVERIFY2(md.contains(QStringLiteral("> ")), md.toUtf8());
}

void TestBlockConstructs::codeBlockOnMiddleLineMakesFenceNotInline()
{
    // Antes: convertir una sola línea en medio de varias daba código en línea
    // (`b`) en vez de una valla, porque el único bloque del fragmento se
    // fusionaba y perdía la valla.
    QTextEdit edit;
    edit.setPlainText(QStringLiteral("a\nb\nc"));

    QTextCursor cur(edit.document());
    cur.setPosition(edit.document()->findBlockByNumber(1).position());
    cur.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    edit.setTextCursor(cur);

    CodeBlock().toggle(&edit);
    const QString md = edit.document()->toMarkdown();
    QVERIFY2(md.contains(QStringLiteral("```")), md.toUtf8());
    QVERIFY2(!md.contains(QStringLiteral("`b`")), md.toUtf8());  // no es código en línea
    QVERIFY(md.contains(QLatin1Char('a')));
    QVERIFY(md.contains(QLatin1Char('c')));
}

void TestBlockConstructs::codeBlockOnEmptyParagraphCreatesBlock()
{
    // Antes: pulsar Block en un párrafo vacío no creaba nada.
    QTextEdit edit;  // documento con un único bloque vacío

    CodeBlock().toggle(&edit);
    QVERIFY(edit.document()->firstBlock().blockFormat().hasProperty(
        QTextFormat::BlockCodeFence));
}

QTEST_MAIN(TestBlockConstructs)
#include "tst_blockconstructs.moc"
