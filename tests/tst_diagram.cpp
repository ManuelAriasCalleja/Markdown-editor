#include <QtTest>

#include "diagram.h"

// Pruebas de la clasificación pura de lenguajes de diagrama.
class TestDiagram : public QObject
{
    Q_OBJECT

private slots:
    void recognizesMermaid();
    void recognizesPlantUml();
    void ignoresOtherLanguages();
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

QTEST_MAIN(TestDiagram)
#include "tst_diagram.moc"
