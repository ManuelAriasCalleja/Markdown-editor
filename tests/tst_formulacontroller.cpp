#include <QtTest>

#include <QApplication>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QSettings>
#include <QTextEdit>

#include "focuseditor.h"
#include "mainwindow.h"
#include "editorstack.h"
#include "formulacontroller.h"
#include "mathblocks.h"

// Pruebas de caracterización de la interacción con fórmulas renderizadas: la
// protección del teclado (handleMathKeyPress) y la extensión de selección antes
// de pegar (guardPasteAgainstMath). Es la lógica más sensible a regresiones (vive
// en el filtro de eventos del editor), así que se fija ANTES de extraerla a
// FormulaController y se vuelve a comprobar después. Es friend de MainWindow.
class TestFormula : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void guardExpandsSelectionInsideFormula();
    void backspaceAtRightEdgeDeletesGroup();
    void deleteAtLeftEdgeDeletesGroup();
    void typingRightAfterFormulaInsertsText();
    void printableInsideFormulaIsBlocked();
    void navigationKeysInsideFormulaPassThrough();
    void formulaUnderCursorIsAnnouncedOnce();

private:
    // Inserta la fórmula `tex` (inline) en el cursor del editor como runs de math,
    // igual que insertFormula pero sin diálogo. Devuelve los límites [start,end).
    static QPair<int, int> insertMath(QTextEdit *editor, const QString &tex);
};

void TestFormula::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestFormula::cleanup()
{
    QSettings().clear();
}

QPair<int, int> TestFormula::insertMath(QTextEdit *editor, const QString &tex)
{
    const QTextCharFormat base = mdmath::mathCharFormat(tex, /*block=*/false);
    QTextCursor cursor = editor->textCursor();
    const int start = cursor.position();
    for (const mdmath::MathRun &r : mdmath::renderTexAsRuns(tex, base))
        cursor.insertText(r.text, r.fmt);
    const int end = cursor.position();
    editor->setCurrentCharFormat(QTextCharFormat());
    return {start, end};
}

void TestFormula::guardExpandsSelectionInsideFormula()
{
    MainWindow w;
    const auto bounds = insertMath(w.m_stack->editor(), QStringLiteral("x^2"));
    QVERIFY(bounds.second - bounds.first >= 2);  // varios runs (base + superíndice)

    // Cursor estrictamente dentro del grupo, sin selección.
    QTextCursor c = w.m_stack->editor()->textCursor();
    c.setPosition(bounds.first + 1);
    w.m_stack->editor()->setTextCursor(c);

    w.m_stack->formula()->guardPasteAgainstMath();

    const QTextCursor sel = w.m_stack->editor()->textCursor();
    QCOMPARE(sel.selectionStart(), bounds.first);
    QCOMPARE(sel.selectionEnd(), bounds.second);
}

void TestFormula::backspaceAtRightEdgeDeletesGroup()
{
    MainWindow w;
    const auto bounds = insertMath(w.m_stack->editor(), QStringLiteral("x^2"));

    // Cursor en el borde derecho del grupo (Backspace debe borrarlo entero).
    QTextCursor c = w.m_stack->editor()->textCursor();
    c.setPosition(bounds.second);
    w.m_stack->editor()->setTextCursor(c);

    QKeyEvent backspace(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    QVERIFY(w.m_stack->formula()->handleMathKeyPress(&backspace));
    // El grupo entero desaparece: no queda ningún fragmento de math.
    QVERIFY(mdmath::mathGroupBounds(w.m_stack->editor()->document()).isEmpty());
}

void TestFormula::deleteAtLeftEdgeDeletesGroup()
{
    // Regresión: Delete con el cursor en el BORDE IZQUIERDO de la fórmula debe
    // borrar el grupo entero (antes, por un desfase en la detección, borraba solo
    // el primer carácter y la fórmula «resucitaba» al guardar).
    MainWindow w;
    QTextEdit *ed = w.m_stack->editor();
    QTextCursor c = ed->textCursor();
    c.insertText(QStringLiteral("ab "));
    const auto bounds = insertMath(ed, QStringLiteral("x^2"));

    c = ed->textCursor();
    c.setPosition(bounds.first);  // justo antes del primer carácter de la fórmula
    ed->setTextCursor(c);

    QKeyEvent del(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    QVERIFY(w.m_stack->formula()->handleMathKeyPress(&del));
    QVERIFY(mdmath::mathGroupBounds(ed->document()).isEmpty());
    QCOMPARE(ed->toPlainText(), QStringLiteral("ab "));
}

void TestFormula::typingRightAfterFormulaInsertsText()
{
    // Regresión: teclear justo DESPUÉS de una fórmula inline debe insertar el
    // carácter como texto normal (antes se «tragaba» la tecla sin insertar nada).
    MainWindow w;
    QTextEdit *ed = w.m_stack->editor();
    const auto bounds = insertMath(ed, QStringLiteral("x^2"));

    QTextCursor c = ed->textCursor();
    c.setPosition(bounds.second);  // borde derecho de la fórmula
    ed->setTextCursor(c);

    QKeyEvent typeZ(QEvent::KeyPress, Qt::Key_Z, Qt::NoModifier, QStringLiteral("z"));
    QVERIFY(w.m_stack->formula()->handleMathKeyPress(&typeZ));  // consumido
    QVERIFY(ed->toPlainText().endsWith(QLatin1Char('z')));
    // La 'z' es texto normal, no parte de la fórmula: el grupo de math no crece.
    const auto groups = mdmath::mathGroupBounds(ed->document());
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups.first().second, bounds.second);  // el grupo termina donde estaba
}

void TestFormula::printableInsideFormulaIsBlocked()
{
    MainWindow w;
    const auto bounds = insertMath(w.m_stack->editor(), QStringLiteral("x^2"));
    const QString before = w.m_stack->editor()->toPlainText();

    QTextCursor c = w.m_stack->editor()->textCursor();
    c.setPosition(bounds.first + 1);  // dentro del grupo
    w.m_stack->editor()->setTextCursor(c);

    QKeyEvent typeA(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier, QStringLiteral("a"));
    QVERIFY(w.m_stack->formula()->handleMathKeyPress(&typeA));          // consumido
    QCOMPARE(w.m_stack->editor()->toPlainText(), before);    // no se insertó nada
}

void TestFormula::navigationKeysInsideFormulaPassThrough()
{
    // No keyboard trap: con el cursor DENTRO de una fórmula, las teclas de
    // navegación y Tab/Esc no deben consumirse (handleMathKeyPress devuelve false),
    // para que el foco/cursor pueda moverse y salir. Solo se bloquean las teclas
    // imprimibles (probado arriba) y Backspace/Delete en los bordes.
    MainWindow w;
    const auto bounds = insertMath(w.m_stack->editor(), QStringLiteral("x^2"));

    QTextCursor c = w.m_stack->editor()->textCursor();
    c.setPosition(bounds.first + 1);  // dentro del grupo
    w.m_stack->editor()->setTextCursor(c);

    const struct { Qt::Key key; const char *name; } navKeys[] = {
        {Qt::Key_Left, "Left"},   {Qt::Key_Right, "Right"},
        {Qt::Key_Up, "Up"},       {Qt::Key_Down, "Down"},
        {Qt::Key_Home, "Home"},   {Qt::Key_End, "End"},
        {Qt::Key_Tab, "Tab"},     {Qt::Key_Escape, "Escape"},
    };
    for (const auto &k : navKeys) {
        QKeyEvent ev(QEvent::KeyPress, k.key, Qt::NoModifier);
        QVERIFY2(!w.m_stack->formula()->handleMathKeyPress(&ev),
                 QByteArray("la tecla ") + k.name + " no debe consumirse dentro de la fórmula");
    }
}

void TestFormula::formulaUnderCursorIsAnnouncedOnce()
{
    // Accesibilidad: al situar el cursor sobre una fórmula, EditorStack anuncia su
    // TeX (statusMessage → QAccessibleAnnouncementEvent) una sola vez por fórmula.
    MainWindow w;
    QTextEdit *ed = w.m_stack->editor();
    QTextCursor c = ed->textCursor();
    c.insertText(QStringLiteral("ab "));
    const auto bounds = insertMath(ed, QStringLiteral("x^2"));
    c = ed->textCursor();
    c.insertText(QStringLiteral(" cd"));

    // Sitúa el cursor fuera de la fórmula para partir de un estado conocido.
    c.setPosition(0);
    ed->setTextCursor(c);

    QSignalSpy spy(w.m_stack, &EditorStack::statusMessage);

    // Entrar en la fórmula la anuncia una vez, con su TeX.
    c.setPosition(bounds.first + 1);
    ed->setTextCursor(c);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.last().at(0).toString().contains(QStringLiteral("x^2")));

    // Moverse DENTRO del mismo grupo no vuelve a anunciar.
    c.setPosition(bounds.first + 2);
    ed->setTextCursor(c);
    QCOMPARE(spy.count(), 1);

    // Salir de la fórmula no anuncia nada (pero permite re-anunciar al volver).
    c.setPosition(0);
    ed->setTextCursor(c);
    QCOMPARE(spy.count(), 1);
    c.setPosition(bounds.first + 1);
    ed->setTextCursor(c);
    QCOMPARE(spy.count(), 2);
}

QTEST_MAIN(TestFormula)
#include "tst_formulacontroller.moc"
