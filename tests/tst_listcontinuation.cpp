#include <QtTest>

#include "listcontinuation.h"

// Pruebas de la lógica pura de continuación de listas (mdlist::analyze), que
// usa el editor de código fuente al pulsar Enter.
class TestListContinuation : public QObject
{
    Q_OBJECT

private slots:
    void notAListLine();
    void bulletContinues();
    void bulletVariants();
    void emptyBulletCloses();
    void numberedIncrements();
    void numberedParenDelimiter();
    void taskContinuesUnchecked();
    void checkedTaskContinuesUnchecked();
    void emptyTaskCloses();
    void indentationPreserved();
};

void TestListContinuation::notAListLine()
{
    QVERIFY(!mdlist::analyze(QStringLiteral("texto normal")).isItem);
    QVERIFY(!mdlist::analyze(QString()).isItem);
    // Un '-' sin espacio no es un ítem (p. ej. un guion suelto).
    QVERIFY(!mdlist::analyze(QStringLiteral("-sin espacio")).isItem);
}

void TestListContinuation::bulletContinues()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("- hola"));
    QVERIFY(c.isItem);
    QVERIFY(!c.empty);
    QCOMPARE(c.nextPrefix, QStringLiteral("- "));
}

void TestListContinuation::bulletVariants()
{
    QCOMPARE(mdlist::analyze(QStringLiteral("* a")).nextPrefix, QStringLiteral("* "));
    QCOMPARE(mdlist::analyze(QStringLiteral("+ a")).nextPrefix, QStringLiteral("+ "));
}

void TestListContinuation::emptyBulletCloses()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("- "));
    QVERIFY(c.isItem);
    QVERIFY(c.empty);
}

void TestListContinuation::numberedIncrements()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("3. cosa"));
    QVERIFY(c.isItem);
    QVERIFY(!c.empty);
    QCOMPARE(c.nextPrefix, QStringLiteral("4. "));
}

void TestListContinuation::numberedParenDelimiter()
{
    QCOMPARE(mdlist::analyze(QStringLiteral("1) uno")).nextPrefix,
             QStringLiteral("2) "));
}

void TestListContinuation::taskContinuesUnchecked()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("- [ ] tarea"));
    QVERIFY(c.isItem);
    QVERIFY(!c.empty);
    QCOMPARE(c.nextPrefix, QStringLiteral("- [ ] "));
}

void TestListContinuation::checkedTaskContinuesUnchecked()
{
    // Una tarea marcada continúa con una tarea SIN marcar.
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("- [x] hecha"));
    QVERIFY(c.isItem);
    QCOMPARE(c.nextPrefix, QStringLiteral("- [ ] "));
}

void TestListContinuation::emptyTaskCloses()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("- [ ] "));
    QVERIFY(c.isItem);
    QVERIFY(c.empty);
}

void TestListContinuation::indentationPreserved()
{
    const mdlist::Continuation c = mdlist::analyze(QStringLiteral("    - sub"));
    QCOMPARE(c.nextPrefix, QStringLiteral("    - "));
    const mdlist::Continuation t = mdlist::analyze(QStringLiteral("\t1. x"));
    QCOMPARE(t.nextPrefix, QStringLiteral("\t2. "));
}

QTEST_MAIN(TestListContinuation)
#include "tst_listcontinuation.moc"
