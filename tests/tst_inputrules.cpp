#include <QtTest>

#include <QCoreApplication>
#include <QSettings>
#include <QTextBlock>
#include <QTextCursor>

#include "editorstack.h"
#include "focuseditor.h"
#include "inputrules.h"
#include "mainwindow.h"

// Pruebas de las reglas de entrada: el reconocimiento puro del marcador y la
// transformación real del bloque al teclear espacio (integración vía MainWindow).
class TestInputRules : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    // Puro.
    void recognizesHeadings();
    void recognizesQuoteAndLists();
    void rejectsNonMarkers();

    // Integración.
    void spaceTurnsHashIntoHeading();
    void spaceTurnsDashIntoBulletList();
    void doesNotTriggerMidLine();

private:
    // Coloca el cursor del editor WYSIWYG al final del texto del bloque 0.
    static void typeInEmptyDoc(MainWindow &w, const QString &text);
};

void TestInputRules::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestInputRules::cleanup()
{
    QSettings().clear();
}

void TestInputRules::typeInEmptyDoc(MainWindow &w, const QString &text)
{
    QTextCursor c = w.m_stack->editor()->textCursor();
    c.movePosition(QTextCursor::Start);
    c.insertText(text);
    w.m_stack->editor()->setTextCursor(c);
}

void TestInputRules::recognizesHeadings()
{
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("#")).kind, mdinputrules::Kind::Heading);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("#")).level, 1);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("###")).level, 3);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("######")).level, 6);
    // 7 almohadillas ya no es encabezado válido.
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("#######")).kind, mdinputrules::Kind::None);
}

void TestInputRules::recognizesQuoteAndLists()
{
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral(">")).kind, mdinputrules::Kind::Quote);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("-")).kind, mdinputrules::Kind::BulletList);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("*")).kind, mdinputrules::Kind::BulletList);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("1.")).kind, mdinputrules::Kind::NumberedList);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("42)")).kind, mdinputrules::Kind::NumberedList);
}

void TestInputRules::rejectsNonMarkers()
{
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("texto")).kind, mdinputrules::Kind::None);
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("a #")).kind, mdinputrules::Kind::None);  // no al inicio
    QCOMPARE(mdinputrules::ruleForPrefix(QStringLiteral("--")).kind, mdinputrules::Kind::None);
    QCOMPARE(mdinputrules::ruleForPrefix(QString()).kind, mdinputrules::Kind::None);
}

void TestInputRules::spaceTurnsHashIntoHeading()
{
    MainWindow w;
    w.show();
    typeInEmptyDoc(w, QStringLiteral("##"));  // cursor tras las dos almohadillas
    QVERIFY(w.applyInputRule());              // simula el espacio
    const QTextBlock b = w.m_stack->editor()->document()->firstBlock();
    QCOMPARE(b.blockFormat().headingLevel(), 2);
    QVERIFY(b.text().isEmpty());              // el marcador `##` desaparece
}

void TestInputRules::spaceTurnsDashIntoBulletList()
{
    MainWindow w;
    w.show();
    typeInEmptyDoc(w, QStringLiteral("-"));
    QVERIFY(w.applyInputRule());
    QVERIFY(w.m_stack->editor()->textCursor().currentList() != nullptr);
}

void TestInputRules::doesNotTriggerMidLine()
{
    MainWindow w;
    w.show();
    typeInEmptyDoc(w, QStringLiteral("hola #"));  // `#` no está al inicio del bloque
    QVERIFY(!w.applyInputRule());
    QCOMPARE(w.m_stack->editor()->document()->firstBlock().blockFormat().headingLevel(), 0);
}

QTEST_MAIN(TestInputRules)
#include "tst_inputrules.moc"
