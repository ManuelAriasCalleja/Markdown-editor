#include <QtTest>

#include "closedtabs.h"

// Pruebas de la pila de pestañas cerradas (closedtabs).
class TestClosedTabStack : public QObject
{
    Q_OBJECT
private slots:
    void pushThenPopIsLifo();
    void popEmptyReturnsEmpty();
    void ignoresEmptyPath();
    void deduplicatesMovingToTop();
    void respectsCap();
};

void TestClosedTabStack::pushThenPopIsLifo()
{
    QStringList s;
    closedtabs::push(s, QStringLiteral("/a.md"));
    closedtabs::push(s, QStringLiteral("/b.md"));
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/b.md"));  // el último cerrado, primero
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/a.md"));
    QVERIFY(s.isEmpty());
}

void TestClosedTabStack::popEmptyReturnsEmpty()
{
    QStringList s;
    QCOMPARE(closedtabs::pop(s), QString());
}

void TestClosedTabStack::ignoresEmptyPath()
{
    QStringList s;
    closedtabs::push(s, QString());  // un documento nuevo sin ruta no se recuerda
    QVERIFY(s.isEmpty());
}

void TestClosedTabStack::deduplicatesMovingToTop()
{
    QStringList s;
    closedtabs::push(s, QStringLiteral("/a.md"));
    closedtabs::push(s, QStringLiteral("/b.md"));
    closedtabs::push(s, QStringLiteral("/a.md"));  // reaparece: sube a la cima, sin duplicar
    QCOMPARE(s.size(), 2);
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/a.md"));
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/b.md"));
}

void TestClosedTabStack::respectsCap()
{
    QStringList s;
    for (int i = 0; i < 10; ++i)
        closedtabs::push(s, QStringLiteral("/f%1.md").arg(i), /*cap=*/3);
    QCOMPARE(s.size(), 3);
    // Solo las 3 más recientes; las antiguas se descartaron por el frente.
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/f9.md"));
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/f8.md"));
    QCOMPARE(closedtabs::pop(s), QStringLiteral("/f7.md"));
    QVERIFY(s.isEmpty());
}

QTEST_MAIN(TestClosedTabStack)
#include "tst_closedtabstack.moc"
