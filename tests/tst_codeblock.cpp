#include <QtTest>

#include <QTextBlock>
#include <QTextDocument>

#include "codeblock.h"
#include "markdownrender.h"

// Pruebas de la localización del grupo de un bloque de código (mdcodeblock).
class TestCodeBlock : public QObject
{
    Q_OBJECT
private slots:
    void groupSpansTheFenceAndReadsLanguage();
    void groupTextJoinsLines();
    void invalidOutsideAFence();

private:
    static void load(QTextDocument &doc, const QString &md)
    {
        doc.setMarkdown(mdrender::protect(md), mdrender::kMarkdownFeatures);
        mdrender::renderPasses(&doc);
    }
    // Número del primer bloque cuyo texto empieza por `prefix`.
    static int blockNumStarting(const QTextDocument &doc, const QString &prefix)
    {
        for (QTextBlock b = doc.begin(); b.isValid(); b = b.next())
            if (b.text().startsWith(prefix))
                return b.blockNumber();
        return -1;
    }
};

void TestCodeBlock::groupSpansTheFenceAndReadsLanguage()
{
    QTextDocument doc;
    load(doc, QStringLiteral("Antes\n\n```python\nx = 1\ny = 2\n```\n\nDespués\n"));
    const int inside = blockNumStarting(doc, QStringLiteral("x = 1"));
    QVERIFY(inside >= 0);
    const mdcodeblock::Group g = mdcodeblock::groupAt(&doc, inside);
    QVERIFY(g.valid);
    QCOMPARE(g.language, QStringLiteral("python"));
    // El grupo abarca ambas líneas de código y ninguna de prosa.
    QCOMPARE(blockNumStarting(doc, QStringLiteral("y = 2")), g.lastBlock);
    QCOMPARE(g.firstBlock, inside);
}

void TestCodeBlock::groupTextJoinsLines()
{
    QTextDocument doc;
    load(doc, QStringLiteral("```\nuno\ndos\n```\n"));
    const int inside = blockNumStarting(doc, QStringLiteral("uno"));
    const mdcodeblock::Group g = mdcodeblock::groupAt(&doc, inside);
    QVERIFY(g.valid);
    QCOMPARE(mdcodeblock::groupText(&doc, g), QStringLiteral("uno\ndos"));
}

void TestCodeBlock::invalidOutsideAFence()
{
    QTextDocument doc;
    load(doc, QStringLiteral("Solo prosa, sin código.\n"));
    QVERIFY(!mdcodeblock::groupAt(&doc, 0).valid);
}

QTEST_MAIN(TestCodeBlock)
#include "tst_codeblock.moc"
