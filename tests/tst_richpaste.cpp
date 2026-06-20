#include <QtTest>

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

QTEST_MAIN(TestRichPaste)
#include "tst_richpaste.moc"
