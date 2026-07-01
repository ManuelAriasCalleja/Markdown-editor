#include <QtTest>
#include <QShortcut>

#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QKeySequence>
#include <QLabel>
#include <QSettings>
#include <QTextBlock>
#include <QTextBlockFormat>
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
    void distractionFreeSurvivesTabSwitch();
    void keyboardCyclesTabs();
    void outlineFocusToggleShortcut();
    void focusModeHasShortcut();
    void noConflictingShortcuts();
    void lineSpacingAppliesToBlocks();
    void lineColumnIndicatorTracksCursor();

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

// El modo sin distracciones no se sale al cambiar de pestaña: se traslada al
// editor de la pestaña entrante (columna fuera del saliente, dentro del nuevo).
void TestSplitView::distractionFreeSurvivesTabSwitch()
{
    MainWindow w;
    w.show();
    EditorStack *first = w.m_stack;
    EditorStack *second = w.addTab();   // segunda pestaña, queda activa
    QVERIFY(second && first != second);

    w.m_distraction->setActive(true);   // entra al modo en la segunda
    QVERIFY(w.m_distraction->isActive());
    QCOMPARE(second->editor()->frameShape(), QFrame::NoFrame);  // columna aplicada

    w.m_tabs->setCurrentWidget(first);  // cambia de pestaña (dispara retarget)
    QVERIFY(w.m_distraction->isActive());                          // sigue en el modo
    QCOMPARE(first->editor()->frameShape(), QFrame::NoFrame);      // columna en la nueva
    QCOMPARE(second->editor()->frameShape(), QFrame::StyledPanel); // y fuera de la anterior

    w.m_distraction->setActive(false);
    QCOMPARE(first->editor()->frameShape(), QFrame::StyledPanel);
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

// F6 alterna el foco esquema↔editor, mostrando el esquema si está oculto. Se
// envía la TECLA real (no se llama a toggleOutlineFocus): así el evento pasa por
// el mapa de atajos de Qt y la prueba detecta colisiones (un atajo ambiguo emite
// activatedAmbiguously y NO dispara, que es justo el fallo que tuvo Ctrl+Shift+O).
void TestSplitView::outlineFocusToggleShortcut()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Uno\n\nTexto\n\n# Dos\n"));
    w.activateWindow();
    w.m_outline->setVisible(false);
    w.m_stack->editor()->setFocus();
    QApplication::processEvents();
    QVERIFY(!w.m_outline->treeHasFocus());

    QTest::keyClick(w.m_stack->editor(), Qt::Key_F6);   // muestra y enfoca el esquema
    QApplication::processEvents();
    QVERIFY(!w.m_outline->isHidden());          // se mostró (isHidden refleja el hide explícito)
    QTRY_VERIFY(w.m_outline->treeHasFocus());   // y recibió el foco (vía el atajo real)

    QTest::keyClick(&w, Qt::Key_F6);            // devuelve el foco al editor
    QApplication::processEvents();
    QTRY_VERIFY(!w.m_outline->treeHasFocus());
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

// Guardia sistemática: ningún atajo de teclado debe estar asignado a dos sitios.
// Todos los atajos de la app son de ventana (QAction o QShortcut hijos de
// MainWindow), así que un duplicado = colisión real (Qt lo volvería ambiguo y no
// dispararía). Esto caza el fallo de Ctrl+Shift+O sin tener que probar cada tecla.
void TestSplitView::noConflictingShortcuts()
{
    MainWindow w;  // construir la ventana crea todas las acciones y QShortcut
    QHash<QString, QStringList> byKey;  // clave portable: QKeySequence::toString()

    for (QAction *a : w.findChildren<QAction *>())
        for (const QKeySequence &k : a->shortcuts())
            if (!k.isEmpty())
                byKey[k.toString()] << (a->text().isEmpty() ? QStringLiteral("(acción)")
                                                            : a->text());
    for (QShortcut *s : w.findChildren<QShortcut *>())
        if (!s->key().isEmpty())
            byKey[s->key().toString()] << QStringLiteral("QShortcut");

    // Lista blanca: teclas duplicadas a propósito porque sus QShortcut están
    // separados por contexto (solo uno activo a la vez), no en conflicto real.
    // - Esc: la barra de búsqueda (activa solo cuando es visible) y el modo sin
    //   distracciones (habilitado solo en ese modo).
    const QStringList allowed = {QKeySequence(Qt::Key_Escape).toString()};

    QStringList conflicts;
    for (auto it = byKey.cbegin(); it != byKey.cend(); ++it)
        if (it.value().size() > 1 && !allowed.contains(it.key()))
            conflicts << QStringLiteral("%1 → %2").arg(it.key(), it.value().join(QStringLiteral(", ")));
    if (!conflicts.isEmpty())
        qWarning("Atajos en conflicto:\n%s", qPrintable(conflicts.join(QLatin1Char('\n'))));
    QVERIFY2(conflicts.isEmpty(), "Hay atajos de teclado duplicados (ver el aviso)");
}

// El interlineado se aplica como altura proporcional a todos los bloques del
// editor WYSIWYG (presentación pura, no afecta al Markdown).
void TestSplitView::lineSpacingAppliesToBlocks()
{
    MainWindow w;
    w.show();
    w.m_stack->editor()->setMarkdown(QStringLiteral("# Uno\n\nUn párrafo.\n\n# Dos\n"));

    w.m_stack->setLineSpacing(150);
    for (QTextBlock b = w.m_stack->editor()->document()->begin();
         b != w.m_stack->editor()->document()->end(); b = b.next()) {
        QCOMPARE(b.blockFormat().lineHeightType(),
                 int(QTextBlockFormat::ProportionalHeight));
        QCOMPARE(b.blockFormat().lineHeight(), 150.0);
    }

    w.m_stack->setLineSpacing(100);  // sencillo de nuevo
    QCOMPARE(w.m_stack->editor()->document()->firstBlock().blockFormat().lineHeight(),
             100.0);
}

// El indicador Ln/Col sigue al cursor del editor activo (vía la señal cursorMoved,
// refactor R4: el WYSIWYG no propagaba cursorPositionChanged a la ventana).
void TestSplitView::lineColumnIndicatorTracksCursor()
{
    MainWindow w;
    w.show();
    w.m_lineColLabel->setVisible(true);  // updateLineColumn no trabaja si está oculto
    w.m_stack->editor()->setMarkdown(QStringLiteral("uno\n\ndos cuatro\n"));
    QApplication::processEvents();

    QTextEdit *ed = w.m_stack->editor();
    QTextCursor c(ed->document());
    c.movePosition(QTextCursor::Start);
    ed->setTextCursor(c);  // dispara cursorPositionChanged → cursorMoved
    QApplication::processEvents();
    QVERIFY(w.m_lineColLabel->text().contains(QStringLiteral("Ln 1")));
    QVERIFY(w.m_lineColLabel->text().contains(QStringLiteral("Col 1")));

    c.movePosition(QTextCursor::End);  // último bloque, tras «dos cuatro»
    ed->setTextCursor(c);
    QApplication::processEvents();
    // Se compara contra el propio cursor para no depender del recuento exacto de bloques.
    QVERIFY(w.m_lineColLabel->text().contains(
        QStringLiteral("Ln %1").arg(c.blockNumber() + 1)));
    QVERIFY(w.m_lineColLabel->text().contains(
        QStringLiteral("Col %1").arg(c.positionInBlock() + 1)));
    QVERIFY(c.blockNumber() + 1 >= 2);  // de verdad bajó a la 2ª línea/bloque
}

QTEST_MAIN(TestSplitView)
#include "tst_splitview.moc"
