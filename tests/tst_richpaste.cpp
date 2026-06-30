#include <QtTest>

#include <QTextCursor>
#include <QTextDocument>
#include <QTextDocumentFragment>

#include "richpaste.h"

class TestRichPaste : public QObject
{
    Q_OBJECT
private slots:
    void emptyStaysEmpty();
    void convertsCharacterFormat();
    void convertsHeadingAndLink();
    void convertsList();
    void hasNoTrailingNewline();
    void dropsUnsupportedStyling();
    void fragmentEmptyStaysEmpty();
    void fragmentConvertsFormat();
    void fragmentSelectionScoped();
    void fragmentHasNoTrailingNewline();
    void fragmentPreservesTableAlignment();
};

void TestRichPaste::emptyStaysEmpty()
{
    QCOMPARE(mdrichpaste::htmlToMarkdown(QString()), QString());
}

void TestRichPaste::convertsCharacterFormat()
{
    const QString md = mdrichpaste::htmlToMarkdown(
        QStringLiteral("<p>texto <b>negrita</b> e <i>cursiva</i></p>"));
    QVERIFY(md.contains(QStringLiteral("**negrita**")));
    QVERIFY(md.contains(QStringLiteral("*cursiva*")));
}

void TestRichPaste::convertsHeadingAndLink()
{
    const QString heading = mdrichpaste::htmlToMarkdown(QStringLiteral("<h1>Título</h1>"));
    QVERIFY(heading.startsWith(QStringLiteral("# Título")));

    const QString link = mdrichpaste::htmlToMarkdown(
        QStringLiteral("<p><a href=\"https://example.com\">sitio</a></p>"));
    QVERIFY(link.contains(QStringLiteral("[sitio](https://example.com)")));
}

void TestRichPaste::convertsList()
{
    const QString md = mdrichpaste::htmlToMarkdown(
        QStringLiteral("<ul><li>uno</li><li>dos</li></ul>"));
    QVERIFY(md.contains(QStringLiteral("uno")));
    QVERIFY(md.contains(QStringLiteral("dos")));
    // Qt serializa las viñetas como «- » (lista Markdown), no como texto plano.
    QVERIFY(md.contains(QStringLiteral("- uno")));
}

void TestRichPaste::hasNoTrailingNewline()
{
    const QString md = mdrichpaste::htmlToMarkdown(QStringLiteral("<p>una línea</p>"));
    QVERIFY(!md.endsWith(u'\n'));
}

void TestRichPaste::dropsUnsupportedStyling()
{
    // Color y fuente no son expresables en Markdown: el texto sobrevive, el estilo
    // se descarta (no se incrusta como en un pegado enriquecido normal).
    const QString md = mdrichpaste::htmlToMarkdown(QStringLiteral(
        "<p style=\"color:red;font-family:Comic Sans\">solo texto</p>"));
    QCOMPARE(md.trimmed(), QStringLiteral("solo texto"));
}

void TestRichPaste::fragmentEmptyStaysEmpty()
{
    QCOMPARE(mdrichpaste::fragmentToMarkdown(QTextDocumentFragment()), QString());
}

void TestRichPaste::fragmentConvertsFormat()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("texto **negrita** y *cursiva*"));
    // El fragmento del documento entero serializa igual que el original.
    const QString md = mdrichpaste::fragmentToMarkdown(QTextDocumentFragment(&doc));
    QVERIFY(md.contains(QStringLiteral("**negrita**")));
    QVERIFY(md.contains(QStringLiteral("*cursiva*")));
}

void TestRichPaste::fragmentSelectionScoped()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Uno\n\nDos\n\n# Tres"));
    // Selecciona solo el primer bloque (el encabezado «Uno»).
    QTextCursor c(&doc);
    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    const QString md = mdrichpaste::fragmentToMarkdown(c.selection());
    QVERIFY(md.contains(QStringLiteral("Uno")));
    QVERIFY(md.startsWith(QStringLiteral("# ")));  // conserva el formato de encabezado
    QVERIFY(!md.contains(QStringLiteral("Tres")));  // no se cuela el resto del documento
}

void TestRichPaste::fragmentHasNoTrailingNewline()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("una línea"));
    const QString md = mdrichpaste::fragmentToMarkdown(QTextDocumentFragment(&doc));
    QVERIFY(!md.endsWith(u'\n'));
}

void TestRichPaste::fragmentPreservesTableAlignment()
{
    // La ruta canónica reinyecta la alineación de columnas que toMarkdown() pierde;
    // copiar como Markdown debe conservarla (al contrario que un toMarkdown directo).
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("| a | b |\n|:--|--:|\n| 1 | 2 |"));
    const QString md = mdrichpaste::fragmentToMarkdown(QTextDocumentFragment(&doc));
    QVERIFY(md.contains(QStringLiteral(":--")) || md.contains(QStringLiteral("--:")));
}

QTEST_MAIN(TestRichPaste)
#include "tst_richpaste.moc"
