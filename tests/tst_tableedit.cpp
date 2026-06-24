#include <QtTest>

#include <QTextDocument>

#include "markdownrender.h"  // kMarkdownFeatures
#include "tableedit.h"

// Pruebas de la preservación de alineación de columnas de tablas: la inyección
// pura de marcadores y el round-trip a través de un QTextDocument real.
class TestTableEdit : public QObject
{
    Q_OBJECT

private slots:
    void injectNoTablesLeavesTextUntouched();
    void injectWritesAlignmentMarkers();
    void injectSkipsOnColumnMismatch();
    void injectHandlesMultipleTables();
    void documentMarkdownRoundTripsAlignment();
    void documentMarkdownStableOnReopen();
    void preservesHtmlLikeTextWithoutDataLoss();
};

void TestTableEdit::injectNoTablesLeavesTextUntouched()
{
    const QString md = QStringLiteral("# Título\n\nun párrafo\n");
    QCOMPARE(mdtable::injectAlignments(md, {}), md);
}

void TestTableEdit::injectWritesAlignmentMarkers()
{
    // Tal y como emite toMarkdown(): separadora de guiones simples.
    const QString md = QStringLiteral("|A|B|C|\n|-|-|-|\n|1|2|3|\n");
    const QList<QList<Qt::Alignment>> al = {
        {Qt::AlignLeft, Qt::AlignHCenter, Qt::AlignRight}};
    QCOMPARE(mdtable::injectAlignments(md, al),
             QStringLiteral("|A|B|C|\n|---|:-:|--:|\n|1|2|3|\n"));
}

void TestTableEdit::injectSkipsOnColumnMismatch()
{
    // Si el nº de columnas no cuadra, la tabla se deja intacta.
    const QString md = QStringLiteral("|A|B|\n|-|-|\n|1|2|\n");
    const QList<QList<Qt::Alignment>> al = {{Qt::AlignRight}};  // 1 ≠ 2
    QCOMPARE(mdtable::injectAlignments(md, al), md);
}

void TestTableEdit::injectHandlesMultipleTables()
{
    const QString md = QStringLiteral(
        "|A|B|\n|-|-|\n|1|2|\n\ntexto\n\n|C|\n|-|\n|x|\n");
    const QList<QList<Qt::Alignment>> al = {
        {Qt::AlignRight, Qt::AlignLeft}, {Qt::AlignHCenter}};
    QCOMPARE(mdtable::injectAlignments(md, al),
             QStringLiteral("|A|B|\n|--:|---|\n|1|2|\n\ntexto\n\n|C|\n|:-:|\n|x|\n"));
}

void TestTableEdit::documentMarkdownRoundTripsAlignment()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("| A | B | C |\n|:--|:-:|--:|\n| 1 | 2 | 3 |\n"));
    const QString md = mdtable::documentMarkdown(&doc);
    // toMarkdown perdería los marcadores; documentMarkdown los reinyecta.
    QVERIFY(md.contains(QStringLiteral(":-:")));
    QVERIFY(md.contains(QStringLiteral("--:")));
}

void TestTableEdit::documentMarkdownStableOnReopen()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("| A | B |\n|:-:|--:|\n| 1 | 2 |\n"));
    const QString first = mdtable::documentMarkdown(&doc);

    QTextDocument doc2;
    doc2.setMarkdown(first);
    QCOMPARE(mdtable::documentMarkdown(&doc2), first);  // idempotente al reabrir
}

// Con las features del editor (NoHTML), un `<algo>` es texto literal y se conserva.
// Sin NoHTML, Qt lo tomaría por HTML en línea y se tragaría ese texto y el de
// alrededor al cargar (pérdida de datos); ver mdrender::kMarkdownFeatures.
void TestTableEdit::preservesHtmlLikeTextWithoutDataLoss()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("texto <tag> mas texto aqui"),
                    mdrender::kMarkdownFeatures);
    const QString md = mdtable::documentMarkdown(&doc);
    QVERIFY2(md.contains(QStringLiteral("mas texto aqui")),
             qPrintable(QStringLiteral("se perdió contenido: <%1>").arg(md)));
    QVERIFY(md.contains(QStringLiteral("<tag>")));  // el tag sobrevive como literal

    // Idempotente al reabrir con las mismas features.
    QTextDocument doc2;
    doc2.setMarkdown(md, mdrender::kMarkdownFeatures);
    QCOMPARE(mdtable::documentMarkdown(&doc2), md);
}

QTEST_MAIN(TestTableEdit)
#include "tst_tableedit.moc"
