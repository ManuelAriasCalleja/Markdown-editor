#include <QtTest>

#include <QTextBlock>
#include <QTextDocument>

#include "outlinepanel.h"

// Pruebas de la extracción pura de encabezados (mdoutline::headingsOf) que
// alimenta el panel de esquema. Se construye el documento con setMarkdown, que
// es la misma ruta que usa el editor para fijar headingLevel() en los bloques.
class TestOutline : public QObject
{
    Q_OBJECT

private slots:
    void nullDocReturnsEmpty();
    void emptyDocHasNoHeadings();
    void plainTextHasNoHeadings();
    void extractsHeadingsWithLevels();
    void ignoresNonHeadingParagraphs();
    void blockNumberPointsToTheHeading();
};

void TestOutline::nullDocReturnsEmpty()
{
    QVERIFY(mdoutline::headingsOf(nullptr).isEmpty());
}

void TestOutline::emptyDocHasNoHeadings()
{
    QTextDocument doc;
    QVERIFY(mdoutline::headingsOf(&doc).isEmpty());
}

void TestOutline::plainTextHasNoHeadings()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("solo texto\n\notro párrafo"));
    QVERIFY(mdoutline::headingsOf(&doc).isEmpty());
}

void TestOutline::extractsHeadingsWithLevels()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Uno\n\n## Dos\n\n### Tres"));

    const QList<OutlineHeading> h = mdoutline::headingsOf(&doc);
    QCOMPARE(h.size(), 3);
    QCOMPARE(h[0].level, 1);
    QCOMPARE(h[0].text, QStringLiteral("Uno"));
    QCOMPARE(h[1].level, 2);
    QCOMPARE(h[1].text, QStringLiteral("Dos"));
    QCOMPARE(h[2].level, 3);
    QCOMPARE(h[2].text, QStringLiteral("Tres"));
}

void TestOutline::ignoresNonHeadingParagraphs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Título\n\npárrafo normal\n\n## Sección"));

    const QList<OutlineHeading> h = mdoutline::headingsOf(&doc);
    QCOMPARE(h.size(), 2);
    QCOMPARE(h[0].text, QStringLiteral("Título"));
    QCOMPARE(h[1].text, QStringLiteral("Sección"));
}

void TestOutline::blockNumberPointsToTheHeading()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Alfa\n\npárrafo\n\n## Beta"));

    // El blockNumber guardado debe resolver al bloque correcto (es lo que usa la
    // navegación: findBlockByNumber para llevar el cursor allí).
    for (const OutlineHeading &heading : mdoutline::headingsOf(&doc)) {
        const QTextBlock block = doc.findBlockByNumber(heading.blockNumber);
        QVERIFY(block.isValid());
        QCOMPARE(block.text(), heading.text);
    }
}

QTEST_MAIN(TestOutline)
#include "tst_outline.moc"
