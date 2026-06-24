#include <QtTest>

#include <QTextCursor>
#include <QTextDocument>

#include "autopair.h"

class TestAutoPair : public QObject
{
    Q_OBJECT

private slots:
    void autoClosesOnEmpty();
    void typeOverDoesNotDuplicate();
    void wrapsSelection();
    void doesNotAutoCloseBeforeWord();
    void backtickIsSymmetric();
    void ignoresNonPairChar();
    void closerWithoutTwinInsertsNormally();
};

// Aplica `typed` sobre el cursor `cur` y devuelve si se consumió.
static bool run(QTextCursor &cur, QChar typed)
{
    return mdautopair::apply(cur, typed, mdautopair::defaultPairs());
}

void TestAutoPair::autoClosesOnEmpty()
{
    QTextDocument doc;
    QTextCursor cur(&doc);
    QVERIFY(run(cur, QLatin1Char('(')));
    QCOMPARE(doc.toPlainText(), QStringLiteral("()"));
    QCOMPARE(cur.position(), 1);  // entre los dos
}

void TestAutoPair::typeOverDoesNotDuplicate()
{
    QTextDocument doc(QStringLiteral("()"));
    QTextCursor cur(&doc);
    cur.setPosition(1);  // entre paréntesis
    QVERIFY(run(cur, QLatin1Char(')')));
    QCOMPARE(doc.toPlainText(), QStringLiteral("()"));  // no se duplicó
    QCOMPARE(cur.position(), 2);  // saltó el cierre
}

void TestAutoPair::wrapsSelection()
{
    QTextDocument doc(QStringLiteral("word"));
    QTextCursor cur(&doc);
    cur.select(QTextCursor::Document);
    QVERIFY(run(cur, QLatin1Char('(')));
    QCOMPARE(doc.toPlainText(), QStringLiteral("(word)"));
    QCOMPARE(cur.selectedText(), QStringLiteral("word"));  // interior re-seleccionado
}

void TestAutoPair::doesNotAutoCloseBeforeWord()
{
    QTextDocument doc(QStringLiteral("word"));
    QTextCursor cur(&doc);
    cur.setPosition(0);  // justo antes de 'w'
    QVERIFY(!run(cur, QLatin1Char('(')));  // inserción normal
    QCOMPARE(doc.toPlainText(), QStringLiteral("word"));  // apply no tocó nada
}

void TestAutoPair::backtickIsSymmetric()
{
    QTextDocument doc;
    QTextCursor cur(&doc);
    QVERIFY(run(cur, QLatin1Char('`')));
    QCOMPARE(doc.toPlainText(), QStringLiteral("``"));
    QCOMPARE(cur.position(), 1);
    // Teclear otra comilla invertida delante de su gemela la salta.
    QVERIFY(run(cur, QLatin1Char('`')));
    QCOMPARE(doc.toPlainText(), QStringLiteral("``"));
    QCOMPARE(cur.position(), 2);
}

void TestAutoPair::ignoresNonPairChar()
{
    QTextDocument doc;
    QTextCursor cur(&doc);
    QVERIFY(!run(cur, QLatin1Char('a')));
    QVERIFY(doc.toPlainText().isEmpty());
}

void TestAutoPair::closerWithoutTwinInsertsNormally()
{
    // Teclear `)` sin un `)` delante: no se consume (inserción normal del editor).
    QTextDocument doc(QStringLiteral("ab"));
    QTextCursor cur(&doc);
    cur.setPosition(2);
    QVERIFY(!run(cur, QLatin1Char(')')));
}

QTEST_MAIN(TestAutoPair)
#include "tst_autopair.moc"
