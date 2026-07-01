#include <QtTest>

#include <QCoreApplication>
#include <QSettings>
#include <QTextBlock>
#include <QTextCursor>

#include "editorstack.h"
#include "focuseditor.h"
#include "formatcontroller.h"
#include "mainwindow.h"

// Integración de promover/degradar encabezado (#11): sobre un documento real,
// FormatController cambia el nivel del bloque del cursor, acota en [1,6] y no
// toca los párrafos. Es friend de MainWindow para acceder al stack activo.
class TestHeadingShift : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanup();

    void promoteAndDemoteChangeLevel();
    void clampsAtLimits();
    void ignoresNonHeading();

private:
    static int levelAtCursor(MainWindow &w);
    static void placeCursorInBlock(MainWindow &w, int n);
};

void TestHeadingShift::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestHeadingShift::cleanup()
{
    QSettings().clear();
}

int TestHeadingShift::levelAtCursor(MainWindow &w)
{
    return w.m_stack->editor()->textCursor().blockFormat().headingLevel();
}

void TestHeadingShift::placeCursorInBlock(MainWindow &w, int n)
{
    QTextCursor c = w.m_stack->editor()->textCursor();
    c.movePosition(QTextCursor::Start);
    for (int i = 0; i < n; ++i)
        c.movePosition(QTextCursor::NextBlock);
    w.m_stack->editor()->setTextCursor(c);
}

void TestHeadingShift::promoteAndDemoteChangeLevel()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("## Sección\n\ntexto\n"));
    placeCursorInBlock(w, 0);
    QCOMPARE(levelAtCursor(w), 2);

    w.m_stack->format()->promoteHeading();  // H2 -> H1
    QCOMPARE(levelAtCursor(w), 1);

    w.m_stack->format()->demoteHeading();   // H1 -> H2
    QCOMPARE(levelAtCursor(w), 2);
    w.m_stack->format()->demoteHeading();   // H2 -> H3
    QCOMPARE(levelAtCursor(w), 3);
}

void TestHeadingShift::clampsAtLimits()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Título\n"));
    placeCursorInBlock(w, 0);
    QCOMPARE(levelAtCursor(w), 1);

    w.m_stack->format()->promoteHeading();  // ya en H1: sin cambio
    QCOMPARE(levelAtCursor(w), 1);

    for (int i = 0; i < 10; ++i)
        w.m_stack->format()->demoteHeading();  // baja hasta H6 y se queda ahí
    QCOMPARE(levelAtCursor(w), 6);
}

void TestHeadingShift::ignoresNonHeading()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Título\n\nun párrafo\n"));
    placeCursorInBlock(w, 1);  // el párrafo
    QCOMPARE(levelAtCursor(w), 0);

    w.m_stack->format()->promoteHeading();
    QCOMPARE(levelAtCursor(w), 0);  // sigue siendo párrafo
    w.m_stack->format()->demoteHeading();
    QCOMPARE(levelAtCursor(w), 0);
}

QTEST_MAIN(TestHeadingShift)
#include "tst_headingshift.moc"
