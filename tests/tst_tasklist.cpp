#include <QtTest>

#include <QTextBlock>
#include <QTextEdit>

#include "tasklist.h"
#include "tableedit.h"

// Casillas de tarea interactivas: el clic sobre la casilla alterna el estado y el
// round-trip a Markdown (`- [x]`/`- [ ]`) lo da Qt. Se usa un QTextEdit real
// porque la detección depende de la geometría (cursorRect/cursorForPosition).
class TestTaskList : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void clickOnCheckboxTogglesState();
    void clickOnTextDoesNotToggle();
    void clickOnNonTaskDoesNothing();

private:
    QTextEdit m_editor;
    // Punto del viewport sobre la casilla del bloque `blockIndex` (a la izquierda
    // del texto del ítem) y sobre su texto, respectivamente.
    QPoint checkboxPoint(int blockIndex) const;
    QPoint textPoint(int blockIndex) const;
    static QTextBlockFormat::MarkerType markerOf(const QTextEdit &e, int blockIndex);
};

void TestTaskList::initTestCase()
{
    m_editor.resize(600, 400);
    m_editor.show();
    QVERIFY(QTest::qWaitForWindowExposed(&m_editor));
}

QPoint TestTaskList::checkboxPoint(int blockIndex) const
{
    const QTextBlock block = m_editor.document()->findBlockByNumber(blockIndex);
    QTextCursor atStart(block);
    const QRect r = m_editor.cursorRect(atStart);
    // A la izquierda del texto del ítem, a su misma altura.
    return QPoint(r.left() - 8, r.center().y());
}

QPoint TestTaskList::textPoint(int blockIndex) const
{
    const QTextBlock block = m_editor.document()->findBlockByNumber(blockIndex);
    QTextCursor atStart(block);
    const QRect r = m_editor.cursorRect(atStart);
    return QPoint(r.left() + 5, r.center().y());
}

QTextBlockFormat::MarkerType TestTaskList::markerOf(const QTextEdit &e, int blockIndex)
{
    return e.document()->findBlockByNumber(blockIndex).blockFormat().marker();
}

void TestTaskList::clickOnCheckboxTogglesState()
{
    m_editor.setMarkdown(QStringLiteral("- [ ] uno\n- [x] dos\n"));
    QCOMPARE(markerOf(m_editor, 0), QTextBlockFormat::MarkerType::Unchecked);
    QCOMPARE(markerOf(m_editor, 1), QTextBlockFormat::MarkerType::Checked);

    // Marca el primero.
    QVERIFY(mdtask::toggleCheckboxAt(&m_editor, checkboxPoint(0)));
    QCOMPARE(markerOf(m_editor, 0), QTextBlockFormat::MarkerType::Checked);

    // Desmarca el segundo.
    QVERIFY(mdtask::toggleCheckboxAt(&m_editor, checkboxPoint(1)));
    QCOMPARE(markerOf(m_editor, 1), QTextBlockFormat::MarkerType::Unchecked);

    // Round-trip: la serialización canónica refleja el nuevo estado.
    const QString md = mdtable::documentMarkdown(m_editor.document());
    QVERIFY2(md.contains(QStringLiteral("- [x] uno")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("- [ ] dos")), qPrintable(md));
}

void TestTaskList::clickOnTextDoesNotToggle()
{
    m_editor.setMarkdown(QStringLiteral("- [ ] uno\n"));
    QVERIFY(!mdtask::toggleCheckboxAt(&m_editor, textPoint(0)));
    QCOMPARE(markerOf(m_editor, 0), QTextBlockFormat::MarkerType::Unchecked);
    QVERIFY(!mdtask::isCheckboxAt(&m_editor, textPoint(0)));
    QVERIFY(mdtask::isCheckboxAt(&m_editor, checkboxPoint(0)));
}

void TestTaskList::clickOnNonTaskDoesNothing()
{
    m_editor.setMarkdown(QStringLiteral("- una lista normal\ntexto suelto\n"));
    QVERIFY(!mdtask::toggleCheckboxAt(&m_editor, checkboxPoint(0)));
    QVERIFY(!mdtask::isCheckboxAt(&m_editor, checkboxPoint(0)));
}

QTEST_MAIN(TestTaskList)
#include "tst_tasklist.moc"
