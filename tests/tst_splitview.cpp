#include <QtTest>

#include <QApplication>
#include <QCoreApplication>
#include <QSettings>
#include <QTextEdit>

#include "focuseditor.h"
#include "mainwindow.h"
#include "editorstack.h"
#include "splitviewcontroller.h"
#include "tableedit.h"

// Pruebas de caracterización de la vista dividida y el modo fuente de
// MainWindow. Fijan el comportamiento observable (visibilidad de paneles, editor
// activo, contenido del documento y del panel de fuente, exclusividad de modos y
// volcado del fuente) ANTES de extraer la lógica a un colaborador, para que la
// refactorización pueda demostrar que no cambia la conducta. Las asertaciones
// son sobre estado observable, no sobre miembros concretos, para sobrevivir a la
// extracción. Es friend de MainWindow para disparar los toggles privados.
class TestSplitView : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void startsInWysiwyg();
    void sourceModeShowsOnlySource();
    void sourceModePopulatesFromDocument();
    void splitViewShowsBothPanels();
    void splitAndSourceAreExclusive();
    void commitSourceUpdatesDocument();
    void syncSourceFromDocumentRefreshesPanel();

private:
    // Markdown actual del documento WYSIWYG (serialización canónica, igual que
    // la que usa la propia ventana para volcar al panel de fuente).
    static QString docMarkdown(MainWindow &w);
};

void TestSplitView::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestSplitView::cleanup()
{
    QSettings().clear();
}

QString TestSplitView::docMarkdown(MainWindow &w)
{
    return mdtable::documentMarkdown(w.m_stack->editor()->document());
}

void TestSplitView::startsInWysiwyg()
{
    MainWindow w;
    w.show();
    QVERIFY(w.m_stack->editor()->isVisible());
    QVERIFY(!w.m_stack->split()->sourceEditor()->isVisible());
    QCOMPARE(w.m_stack->activeEditor(), static_cast<QTextEdit *>(w.m_stack->editor()));
}

void TestSplitView::sourceModeShowsOnlySource()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);
    QVERIFY(!w.m_stack->editor()->isVisible());
    QVERIFY(w.m_stack->split()->sourceEditor()->isVisible());
    QCOMPARE(w.m_stack->activeEditor(), static_cast<QTextEdit *>(w.m_stack->split()->sourceEditor()));

    w.m_stack->split()->toggleSourceMode(false);
    QVERIFY(w.m_stack->editor()->isVisible());
    QVERIFY(!w.m_stack->split()->sourceEditor()->isVisible());
    QCOMPARE(w.m_stack->activeEditor(), static_cast<QTextEdit *>(w.m_stack->editor()));
}

void TestSplitView::sourceModePopulatesFromDocument()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Hola\n\nMundo\n"));
    const QString expected = docMarkdown(w);

    w.m_stack->split()->toggleSourceMode(true);
    QCOMPARE(w.m_stack->split()->sourceEditor()->toPlainText(), expected);
}

void TestSplitView::splitViewShowsBothPanels()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("texto\n"));
    const QString expected = docMarkdown(w);

    w.m_stack->split()->toggleSplitView(true);
    QVERIFY(w.m_stack->editor()->isVisible());
    QVERIFY(w.m_stack->split()->sourceEditor()->isVisible());
    QCOMPARE(w.m_stack->split()->sourceEditor()->toPlainText(), expected);

    w.m_stack->split()->toggleSplitView(false);
    QVERIFY(w.m_stack->editor()->isVisible());
    QVERIFY(!w.m_stack->split()->sourceEditor()->isVisible());
}

void TestSplitView::splitAndSourceAreExclusive()
{
    MainWindow w;
    w.show();

    w.m_stack->split()->toggleSourceMode(true);
    w.m_stack->split()->toggleSplitView(true);          // entrar en dividido debe sacar de fuente
    QVERIFY(w.m_stack->split()->splitMode());
    QVERIFY(!w.m_stack->split()->sourceMode());

    w.m_stack->split()->toggleSourceMode(true);         // entrar en fuente debe sacar de dividido
    QVERIFY(w.m_stack->split()->sourceMode());
    QVERIFY(!w.m_stack->split()->splitMode());
}

void TestSplitView::commitSourceUpdatesDocument()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSourceMode(true);
    w.m_stack->split()->sourceEditor()->setPlainText(QStringLiteral("# Nuevo título\n"));

    w.m_stack->split()->commitSourceToDocument();
    QVERIFY(docMarkdown(w).contains(QStringLiteral("Nuevo título")));
}

void TestSplitView::syncSourceFromDocumentRefreshesPanel()
{
    MainWindow w;
    w.show();
    w.m_stack->split()->toggleSplitView(true);

    // Cambia el documento WYSIWYG y fuerza la sincronización hacia el fuente.
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Cambiado\n"));
    w.m_stack->split()->syncSourceFromDocument();
    QCOMPARE(w.m_stack->split()->sourceEditor()->toPlainText(), docMarkdown(w));
}

QTEST_MAIN(TestSplitView)
#include "tst_splitview.moc"
