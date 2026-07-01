#include <QtTest>

#include "mark.h"

// Pruebas de la localización pura de marcas `==texto==` (mdmark::spansIn).
class TestMarkHighlight : public QObject
{
    Q_OBJECT
private slots:
    void findsSingleMark();
    void findsMultipleMarks();
    void ignoresUnpaired();
    void ignoresEmptyAndEqualsRuns();
    void spanIncludesDelimiters();
};

void TestMarkHighlight::findsSingleMark()
{
    const auto s = mdmark::spansIn(QStringLiteral("un ==texto== normal"));
    QCOMPARE(s.size(), 1);
    QCOMPARE(s.at(0).start, 3);
    QCOMPARE(s.at(0).length, 9);  // "==texto==" (2 + 5 + 2)
}

void TestMarkHighlight::findsMultipleMarks()
{
    const auto s = mdmark::spansIn(QStringLiteral("==a== y ==bb=="));
    QCOMPARE(s.size(), 2);
    QCOMPARE(s.at(1).length, 6);  // "==bb=="
}

void TestMarkHighlight::ignoresUnpaired()
{
    QVERIFY(mdmark::spansIn(QStringLiteral("solo == suelto")).isEmpty());
    QVERIFY(mdmark::spansIn(QStringLiteral("a == b (comparación)")).isEmpty());
}

void TestMarkHighlight::ignoresEmptyAndEqualsRuns()
{
    QVERIFY(mdmark::spansIn(QStringLiteral("====")).isEmpty());   // interior vacío
    QVERIFY(mdmark::spansIn(QStringLiteral("======")).isEmpty()); // solo signos igual
}

void TestMarkHighlight::spanIncludesDelimiters()
{
    const auto s = mdmark::spansIn(QStringLiteral("==x=="));
    QCOMPARE(s.size(), 1);
    QCOMPARE(s.at(0).start, 0);
    QCOMPARE(s.at(0).length, 5);
}

QTEST_MAIN(TestMarkHighlight)
#include "tst_markhighlight.moc"
