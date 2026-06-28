#include <QtTest>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QKeySequence>
#include <QSettings>
#include <QTextEdit>

#include "appsettings.h"
#include "distractionfreecontroller.h"
#include "focuseditor.h"
#include "mainwindow.h"
#include "editorstack.h"
#include "outlinepanel.h"
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
    void distractionFreeFollowsActiveTab();
    void keyboardCyclesTabs();
    void outlineFocusToggleShortcut();
    void focusModeHasShortcut();

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

// El modo sin distracciones es de la ventana, pero opera sobre el editor de la
// pestaña activa. Al cambiar o cerrar pestañas debe reapuntarse: si no, aplica la
// columna sobre el editor de la pestaña anterior (que queda a todo el ancho) y, si
// esa pestaña se cerró, deja un puntero colgante que crashea al entrar al modo.
void TestSplitView::distractionFreeFollowsActiveTab()
{
    MainWindow w;
    w.show();
    w.addTab();         // segunda pestaña (queda activa)
    w.closeTab(0);      // cierra la primera (con la que se construyó el controlador)
    // Procesa el deleteLater para liberar de verdad el EditorStack cerrado: así, si
    // el controlador siguiera apuntándolo, entrar al modo accedería a memoria libre.
    qApp->processEvents();
    qApp->sendPostedEvents(nullptr, QEvent::DeferredDelete);

    w.m_distraction->setActive(true);     // F11: no debe crashear
    QVERIFY(w.m_distraction->isActive());
    // La columna se aplicó al editor de la pestaña ACTIVA (sin marco al activarse).
    QCOMPARE(w.m_stack->editor()->frameShape(), QFrame::NoFrame);
    w.m_distraction->setActive(false);
    QCOMPARE(w.m_stack->editor()->frameShape(), QFrame::StyledPanel);
}

// Ctrl+AvPág/RePág y Ctrl+Tab rotan entre pestañas (con envoltura). Probamos la
// lógica de cycleTab, a la que despachan los atajos.
void TestSplitView::keyboardCyclesTabs()
{
    MainWindow w;
    w.show();
    w.addTab();
    w.addTab();
    QCOMPARE(w.m_tabs->count(), 3);

    w.m_tabs->setCurrentIndex(0);
    w.cycleTab(1);
    QCOMPARE(w.m_tabs->currentIndex(), 1);
    w.cycleTab(1);
    QCOMPARE(w.m_tabs->currentIndex(), 2);
    w.cycleTab(1);                              // envuelve al principio
    QCOMPARE(w.m_tabs->currentIndex(), 0);
    w.cycleTab(-1);                             // envuelve al final
    QCOMPARE(w.m_tabs->currentIndex(), 2);
}

// Ctrl+Shift+O alterna el foco esquema↔editor, mostrando el esquema si está oculto.
void TestSplitView::outlineFocusToggleShortcut()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Uno\n\nTexto\n\n# Dos\n"));
    w.activateWindow();
    QApplication::processEvents();
    w.m_outline->setVisible(false);

    w.toggleOutlineFocus();                     // muestra y enfoca el esquema
    QApplication::processEvents();
    QVERIFY(!w.m_outline->isHidden());          // se mostró (isHidden refleja el hide explícito)
    QTRY_VERIFY(w.m_outline->treeHasFocus());   // y recibió el foco

    w.toggleOutlineFocus();                     // devuelve el foco al editor
    QApplication::processEvents();
    QVERIFY(!w.m_outline->treeHasFocus());
}

// El «Modo foco» tiene atajo F12 y alterna (persistido en AppSettings).
void TestSplitView::focusModeHasShortcut()
{
    MainWindow w;
    w.show();
    QVERIFY(w.m_typewriterAction);
    QCOMPARE(w.m_typewriterAction->shortcut(), QKeySequence(Qt::Key_F12));
    QVERIFY(w.m_typewriterAction->isCheckable());

    const bool before = AppSettings::typewriterMode();
    w.m_typewriterAction->trigger();
    QCOMPARE(AppSettings::typewriterMode(), !before);
    w.m_typewriterAction->trigger();
    QCOMPARE(AppSettings::typewriterMode(), before);
}

QTEST_MAIN(TestSplitView)
#include "tst_splitview.moc"
