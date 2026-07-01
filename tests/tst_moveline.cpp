#include <QtTest>

#include "moveline.h"

// Pruebas de los comandos de línea puros (mdmoveline).
class TestMoveLine : public QObject
{
    Q_OBJECT
private slots:
    void moveUpSwaps();
    void moveUpAtTopIsNoOp();
    void moveDownSwaps();
    void moveDownAtBottomIsNoOp();
    void duplicateInsertsCopyBelow();
    void removeLineDropsIt();
    void removeOnlyLineLeavesEmpty();
    void joinNextMergesTrimming();
    void joinAtBottomIsNoOp();
    void outOfRangeIsNoOp();
};

static QStringList L(std::initializer_list<const char *> xs)
{
    QStringList out;
    for (const char *x : xs)
        out << QString::fromUtf8(x);
    return out;
}

void TestMoveLine::moveUpSwaps()
{
    const auto r = mdmoveline::moveUp(L({"a", "b", "c"}), 1);
    QCOMPARE(r.lines, L({"b", "a", "c"}));
    QCOMPARE(r.line, 0);
}

void TestMoveLine::moveUpAtTopIsNoOp()
{
    const auto r = mdmoveline::moveUp(L({"a", "b"}), 0);
    QCOMPARE(r.lines, L({"a", "b"}));
    QCOMPARE(r.line, 0);
}

void TestMoveLine::moveDownSwaps()
{
    const auto r = mdmoveline::moveDown(L({"a", "b", "c"}), 1);
    QCOMPARE(r.lines, L({"a", "c", "b"}));
    QCOMPARE(r.line, 2);
}

void TestMoveLine::moveDownAtBottomIsNoOp()
{
    const auto r = mdmoveline::moveDown(L({"a", "b"}), 1);
    QCOMPARE(r.lines, L({"a", "b"}));
    QCOMPARE(r.line, 1);
}

void TestMoveLine::duplicateInsertsCopyBelow()
{
    const auto r = mdmoveline::duplicate(L({"a", "b"}), 0);
    QCOMPARE(r.lines, L({"a", "a", "b"}));
    QCOMPARE(r.line, 0);
}

void TestMoveLine::removeLineDropsIt()
{
    const auto r = mdmoveline::removeLine(L({"a", "b", "c"}), 1);
    QCOMPARE(r.lines, L({"a", "c"}));
    QCOMPARE(r.line, 1);  // el cursor pasa a la que ocupa su lugar
    const auto last = mdmoveline::removeLine(L({"a", "b"}), 1);
    QCOMPARE(last.line, 0);  // borrar la última sube el cursor
}

void TestMoveLine::removeOnlyLineLeavesEmpty()
{
    const auto r = mdmoveline::removeLine(L({"solo"}), 0);
    QCOMPARE(r.lines, QStringList{QString()});
    QCOMPARE(r.line, 0);
}

void TestMoveLine::joinNextMergesTrimming()
{
    const auto r = mdmoveline::joinNext(L({"hola  ", "   mundo"}), 0);
    QCOMPARE(r.lines, L({"hola mundo"}));
    QCOMPARE(r.line, 0);
    // Línea vacía + siguiente: sin espacio sobrante al principio.
    QCOMPARE(mdmoveline::joinNext(L({"", "x"}), 0).lines, L({"x"}));
}

void TestMoveLine::joinAtBottomIsNoOp()
{
    const auto r = mdmoveline::joinNext(L({"a", "b"}), 1);
    QCOMPARE(r.lines, L({"a", "b"}));
}

void TestMoveLine::outOfRangeIsNoOp()
{
    QCOMPARE(mdmoveline::moveUp(L({"a"}), 5).lines, L({"a"}));
    QCOMPARE(mdmoveline::duplicate(L({"a"}), -1).lines, L({"a"}));
    QCOMPARE(mdmoveline::removeLine(L({"a"}), 9).lines, L({"a"}));
}

QTEST_MAIN(TestMoveLine)
#include "tst_moveline.moc"
