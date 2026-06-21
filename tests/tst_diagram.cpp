#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "diagram.h"
#include "diagramdoc.h"
#include "tableedit.h"

// Pruebas de la clasificación pura de lenguajes de diagrama.
class TestDiagram : public QObject
{
    Q_OBJECT

private slots:
    void recognizesMermaid();
    void recognizesPlantUml();
    void ignoresOtherLanguages();
    void previewBlocksAreStrippedFromSerialization();
};

void TestDiagram::recognizesMermaid()
{
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("mermaid")), mddiagram::Kind::Mermaid);
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("  Mermaid ")), mddiagram::Kind::Mermaid);
    QVERIFY(mddiagram::isDiagramLanguage(QStringLiteral("MERMAID")));
}

void TestDiagram::recognizesPlantUml()
{
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("plantuml")), mddiagram::Kind::PlantUml);
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("puml")), mddiagram::Kind::PlantUml);
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("uml")), mddiagram::Kind::PlantUml);
}

void TestDiagram::ignoresOtherLanguages()
{
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("cpp")), mddiagram::Kind::None);
    QCOMPARE(mddiagram::kindForLanguage(QStringLiteral("python")), mddiagram::Kind::None);
    QCOMPARE(mddiagram::kindForLanguage(QString()), mddiagram::Kind::None);
    QVERIFY(!mddiagram::isDiagramLanguage(QStringLiteral("json")));
}

// Un bloque de previsualización (marcado) es presentación: no debe aparecer en
// la serialización canónica (documentMarkdown), que es por donde pasan guardado,
// vista de fuente e isModified.
void TestDiagram::previewBlocksAreStrippedFromSerialization()
{
    QTextDocument doc;
    doc.setPlainText(QStringLiteral("Antes\nDespués"));

    // Inserta un bloque de preview entre las dos líneas.
    QTextCursor c(&doc);
    c.movePosition(QTextCursor::Start);
    c.movePosition(QTextCursor::EndOfBlock);
    c.insertBlock();
    QTextBlockFormat bf;
    bf.setProperty(mddiagram::PreviewBlockProperty, true);
    c.setBlockFormat(bf);
    c.insertText(QStringLiteral("[imagen de diagrama]"));

    // El texto del preview está en el documento...
    QVERIFY(doc.toPlainText().contains(QStringLiteral("[imagen de diagrama]")));
    // ...pero NO en la serialización canónica.
    const QString md = mdtable::documentMarkdown(&doc);
    QVERIFY(!md.contains(QStringLiteral("[imagen de diagrama]")));
    QVERIFY(md.contains(QStringLiteral("Antes")));
    QVERIFY(md.contains(QStringLiteral("Después")));
}

QTEST_MAIN(TestDiagram)
#include "tst_diagram.moc"
