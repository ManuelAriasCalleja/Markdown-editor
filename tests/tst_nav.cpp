#include <QtTest>

#include "nav.h"

// Pruebas de la navegación pura por el documento (mdnav).
class TestNav : public QObject
{
    Q_OBJECT
private slots:
    void clampLineWithinRange();
    void clampLineBelowOne();
    void clampLineBeyondEnd();
    void clampLineEmptyDocument();
};

void TestNav::clampLineWithinRange()
{
    QCOMPARE(mdnav::clampLine(1, 10), 1);
    QCOMPARE(mdnav::clampLine(5, 10), 5);
    QCOMPARE(mdnav::clampLine(10, 10), 10);
}

void TestNav::clampLineBelowOne()
{
    QCOMPARE(mdnav::clampLine(0, 10), 1);
    QCOMPARE(mdnav::clampLine(-3, 10), 1);
}

void TestNav::clampLineBeyondEnd()
{
    QCOMPARE(mdnav::clampLine(11, 10), 10);
    QCOMPARE(mdnav::clampLine(9999, 10), 10);
}

void TestNav::clampLineEmptyDocument()
{
    // Un QTextDocument nunca tiene 0 bloques, pero la función es robusta igualmente.
    QCOMPARE(mdnav::clampLine(1, 0), 1);
    QCOMPARE(mdnav::clampLine(5, 0), 1);
    QCOMPARE(mdnav::clampLine(0, -2), 1);
}

QTEST_MAIN(TestNav)
#include "tst_nav.moc"
