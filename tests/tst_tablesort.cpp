#include <QtTest>

#include "tablesort.h"

// Pruebas de la ordenación pura de filas (mdtablesort): autodetección numérica y
// permutación estable ascendente/descendente.
class TestTableSort : public QObject
{
    Q_OBJECT
private slots:
    void detectsNumericColumn();
    void detectsNonNumericColumn();
    void emptyCellsDontBreakNumeric();
    void sortsNumericAscending();
    void sortsNumericDescending();
    void sortsAlphabeticalCaseInsensitive();
    void stableOnTies();
    void nonNumbersGoLastInNumericAscending();
};

void TestTableSort::detectsNumericColumn()
{
    QVERIFY(mdtablesort::looksNumeric({"3", "10", "-2.5"}));
}

void TestTableSort::detectsNonNumericColumn()
{
    QVERIFY(!mdtablesort::looksNumeric({"3", "diez", "5"}));
    QVERIFY(!mdtablesort::looksNumeric({}));  // sin ningún número
}

void TestTableSort::emptyCellsDontBreakNumeric()
{
    QVERIFY(mdtablesort::looksNumeric({"3", "", "  ", "7"}));
}

void TestTableSort::sortsNumericAscending()
{
    // "10" debe ir DESPUÉS de "9" (numérico, no alfabético).
    const QStringList keys{"10", "9", "100", "2"};
    QCOMPARE(mdtablesort::sortedOrder(keys, true, true), (QList<int>{3, 1, 0, 2}));
}

void TestTableSort::sortsNumericDescending()
{
    const QStringList keys{"10", "9", "100", "2"};
    QCOMPARE(mdtablesort::sortedOrder(keys, true, false), (QList<int>{2, 0, 1, 3}));
}

void TestTableSort::sortsAlphabeticalCaseInsensitive()
{
    const QStringList keys{"banana", "Ana", "cereza"};
    QCOMPARE(mdtablesort::sortedOrder(keys, false, true), (QList<int>{1, 0, 2}));
}

void TestTableSort::stableOnTies()
{
    // Dos "5": conservan su orden original (índices 0 y 2).
    const QStringList keys{"5", "1", "5"};
    QCOMPARE(mdtablesort::sortedOrder(keys, true, true), (QList<int>{1, 0, 2}));
}

void TestTableSort::nonNumbersGoLastInNumericAscending()
{
    const QStringList keys{"2", "texto", "1"};
    QCOMPARE(mdtablesort::sortedOrder(keys, true, true), (QList<int>{2, 0, 1}));
}

QTEST_MAIN(TestTableSort)
#include "tst_tablesort.moc"
