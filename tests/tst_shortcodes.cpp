#include <QtTest>

#include "shortcodes.h"

class TestShortcodes : public QObject
{
    Q_OBJECT
private slots:
    void expandsKnownNames();
    void caseSensitiveGreek();
    void unknownReturnsEmpty();
};

void TestShortcodes::expandsKnownNames()
{
    QCOMPARE(mdshortcode::expand(QStringLiteral("alpha")), QString::fromUtf8("α"));
    QCOMPARE(mdshortcode::expand(QStringLiteral("check")), QString::fromUtf8("✓"));
    QCOMPARE(mdshortcode::expand(QStringLiteral("to")), QString::fromUtf8("→"));
    QCOMPARE(mdshortcode::expand(QStringLiteral("euro")), QString::fromUtf8("€"));
}

void TestShortcodes::caseSensitiveGreek()
{
    QCOMPARE(mdshortcode::expand(QStringLiteral("omega")), QString::fromUtf8("ω"));
    QCOMPARE(mdshortcode::expand(QStringLiteral("Omega")), QString::fromUtf8("Ω"));
}

void TestShortcodes::unknownReturnsEmpty()
{
    QVERIFY(mdshortcode::expand(QStringLiteral("nope")).isEmpty());
    QVERIFY(mdshortcode::expand(QString()).isEmpty());
}

QTEST_MAIN(TestShortcodes)
#include "tst_shortcodes.moc"
