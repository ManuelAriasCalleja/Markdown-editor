/// \file
/// \brief Implementación de FormulaController: diálogo de fórmula, inserción y protección con el teclado.

#include "formulacontroller.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QVBoxLayout>

#include "mathblocks.h"
#include "mathobject.h"

// Los textos visibles conservan el contexto de traducción "MainWindow"
// (QCoreApplication::translate) para no re-hogar las cadenas ya traducidas en los
// .ts al moverlas desde MainWindow. Ver la nota de i18n en CLAUDE.md.

FormulaController::FormulaController(QTextEdit *editor, QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_parent(parent)
    , m_mathObject(new MathObject(this))
{
    // Registra el pintor de fórmulas 2D en el documento del editor. El editor
    // conserva el mismo QTextDocument durante toda la sesión (setMarkdown lo
    // reusa), así que basta registrarlo una vez.
    MathObject::registerOn(m_editor->document(), m_mathObject);
}

bool FormulaController::askFormula(QString *tex, bool *block,
                                   const QString &initialTex, bool initialBlock)
{
    QDialog dlg(m_parent);
    dlg.setWindowTitle(QCoreApplication::translate("MainWindow", "Insertar fórmula"));
    auto *form = new QFormLayout;
    auto *edit = new QPlainTextEdit(&dlg);
    edit->setPlaceholderText(QCoreApplication::translate(
        "MainWindow", "Expresión TeX, p. ej. E = mc^2"));
    edit->setMinimumWidth(360);
    edit->setPlainText(initialTex);
    auto *combo = new QComboBox(&dlg);
    combo->addItem(QCoreApplication::translate("MainWindow", "En línea ($...$)"));
    combo->addItem(QCoreApplication::translate("MainWindow", "Bloque ($$...$$)"));
    combo->setCurrentIndex(initialBlock ? 1 : 0);
    // Vista previa en vivo: alimentamos un QTextEdit (no un QLabel) con los
    // runs que renderTexAsRuns produce, para que los super/subíndices de Qt
    // se vean tal cual aparecerán en el editor.
    auto *preview = new QTextEdit(&dlg);
    preview->setReadOnly(true);
    preview->setMinimumWidth(360);
    preview->setMaximumHeight(70);
    preview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    preview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    preview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // La vista previa es de solo lectura: fuera del recorrido del Tab (no tiene
    // sentido tabular a ella), pero sigue enfocable con clic por si se copia.
    preview->setFocusPolicy(Qt::ClickFocus);
    // Las fórmulas 2D del preview las pinta el mismo handler que el editor.
    MathObject::registerOn(preview->document(), m_mathObject);
    QFont previewFont = preview->font();
    previewFont.setPointSizeF(previewFont.pointSizeF() * 1.25);
    preview->setFont(previewFont);
    auto updatePreview = [edit, preview] {
        preview->clear();
        const QString tex = edit->toPlainText().trimmed();
        if (tex.isEmpty()) {
            preview->setPlainText(QStringLiteral("—"));
            return;
        }
        QTextCursor c(preview->document());
        // Mismo camino que la inserción real: 2D si la fórmula lo necesita
        // (objeto pintado), runs con super/subíndice si no.
        for (const mdmath::MathRun &r : mdmath::renderFormulaRuns(tex, /*block=*/false))
            c.insertText(r.text, r.fmt);
    };
    QObject::connect(edit, &QPlainTextEdit::textChanged, &dlg, updatePreview);
    updatePreview();
    form->addRow(QCoreApplication::translate("MainWindow", "TeX:"), edit);
    form->addRow(QCoreApplication::translate("MainWindow", "Tipo:"), combo);
    form->addRow(QCoreApplication::translate("MainWindow", "Vista previa:"), preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    auto *layout = new QVBoxLayout(&dlg);
    layout->addLayout(form);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    const QString text = edit->toPlainText().trimmed();
    if (text.isEmpty())
        return false;
    *tex = text;
    *block = combo->currentIndex() == 1;
    return true;
}

void FormulaController::insertFormula()
{
    QString tex;
    bool block = false;
    if (!askFormula(&tex, &block))
        return;

    // Inserta la fórmula: 2D (un carácter objeto) si lleva fracciones o grandes
    // operadores con límites; si no, runs con super/subíndice reales. En ambos
    // casos comparten el TeX en MathTex, lo que permite agruparlos al serializar
    // y al editar con doble clic.
    const QList<mdmath::MathRun> runs = mdmath::renderFormulaRuns(tex, block);

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    if (block && !cursor.atBlockStart())
        cursor.insertText(QStringLiteral("\n"));
    for (const mdmath::MathRun &r : runs)
        cursor.insertText(r.text, r.fmt);
    if (block)
        cursor.insertText(QStringLiteral("\n"), QTextCharFormat());
    cursor.endEditBlock();
    // Restaura el formato de inserción para que lo que el usuario teclee a
    // continuación no herede las propiedades de math.
    m_editor->setCurrentCharFormat(QTextCharFormat());
    m_editor->setFocus();
}

void FormulaController::guardPasteAgainstMath()
{
    QTextCursor cursor = m_editor->textCursor();
    const QList<QPair<int, int>> groups = mdmath::mathGroupBounds(m_editor->document());
    if (groups.isEmpty())
        return;

    int newStart = cursor.selectionStart();
    int newEnd   = cursor.selectionEnd();
    const bool hadSelection = cursor.hasSelection();

    if (!hadSelection) {
        // Sin selección: si el cursor está dentro de un grupo (no en su
        // borde), seleccionamos el grupo entero para que el pegado lo
        // reemplace en bloque en vez de incrustarse a mitad de fórmula.
        const int pos = cursor.position();
        for (const auto &g : groups) {
            if (pos > g.first && pos < g.second) {
                newStart = g.first;
                newEnd = g.second;
                break;
            }
        }
    } else {
        // Con selección: extiéndela a los grupos que solape.
        for (const auto &g : groups) {
            if (g.second <= newStart || g.first >= newEnd)
                continue;  // sin solape
            if (g.first < newStart) newStart = g.first;
            if (g.second > newEnd)  newEnd = g.second;
        }
    }

    if (newStart != cursor.selectionStart() || newEnd != cursor.selectionEnd()) {
        QTextCursor c = m_editor->textCursor();
        c.setPosition(newStart);
        c.setPosition(newEnd, QTextCursor::KeepAnchor);
        m_editor->setTextCursor(c);
    }
}

bool FormulaController::handleMathKeyPress(QKeyEvent *event)
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection())
        return false;

    QTextDocument *doc = m_editor->document();
    const int pos = cursor.position();

    // Rangos [inicio, fin) de cada fórmula, en índices de carácter — el mismo
    // primitivo canónico que usa la protección del pegado. Se trabaja con índices
    // de carácter directamente para NO depender de la semántica de
    // QTextCursor::charFormat() en los bordes (devuelve el formato del carácter
    // anterior, y en el inicio de bloque el del siguiente): esa ambigüedad
    // desfasaba en uno toda la detección de bordes.
    const QList<QPair<int, int>> groups = mdmath::mathGroupBounds(doc);
    auto groupContaining = [&groups](int charIndex) -> QPair<int, int> {
        for (const auto &g : groups)
            if (charIndex >= g.first && charIndex < g.second)
                return g;
        return {-1, -1};
    };

    // El carácter a la izquierda del cursor ocupa el índice pos-1; el de la
    // derecha, el índice pos.
    const QPair<int, int> groupBefore =
        pos > 0 ? groupContaining(pos - 1) : QPair<int, int>{-1, -1};
    const QPair<int, int> groupAfter = groupContaining(pos);
    const bool mathBefore = groupBefore.first >= 0;
    const bool mathAfter  = groupAfter.first >= 0;

    if (!mathBefore && !mathAfter)
        return false;  // sin contacto con math: pasa al editor

    // Cursor estrictamente dentro de una misma fórmula (entre dos de sus caracteres).
    const bool insideMath = mathBefore && mathAfter
        && groupBefore.first == groupAfter.first;

    auto removeRange = [&](int start, int end) {
        QTextCursor c(doc);
        c.setPosition(start);
        c.setPosition(end, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        m_editor->setCurrentCharFormat(QTextCharFormat());
    };

    // Backspace tocando el borde derecho (o dentro): borra el grupo entero.
    if (event->key() == Qt::Key_Backspace && mathBefore) {
        removeRange(groupBefore.first, groupBefore.second);
        return true;
    }
    // Delete tocando el borde izquierdo (o dentro): borra el grupo entero.
    if (event->key() == Qt::Key_Delete && mathAfter) {
        removeRange(groupAfter.first, groupAfter.second);
        return true;
    }

    // Tecla imprimible dentro de la fórmula: se ignora y se avisa.
    if (insideMath && !event->text().isEmpty() && event->text().at(0).isPrint()) {
        emit statusMessage(QCoreApplication::translate("MainWindow",
            "Doble clic en la fórmula para editarla (Ctrl+Shift+F para insertar otra)."),
            3000);
        return true;
    }

    // Tecla imprimible en el BORDE de una fórmula (no dentro): si dejáramos que
    // la insertara el editor, Qt heredaría el formato del carácter contiguo y el
    // texto nuevo quedaría marcado como parte de la fórmula (mismo color y, peor,
    // la serialización se lo tragaría, sin aparecer en el fuente). Lo insertamos
    // nosotros con un formato limpio para que sea texto normal. Se excluyen los
    // atajos (Ctrl/Alt/Meta), que no son escritura.
    const bool typing = !event->text().isEmpty() && event->text().at(0).isPrint()
        && !(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier));
    if (typing) {
        const QTextCharFormat clean;  // sin IsMath/MathTex ni super/subíndice
        cursor.insertText(event->text(), clean);
        m_editor->setTextCursor(cursor);
        m_editor->setCurrentCharFormat(clean);
        return true;
    }

    return false;
}

bool FormulaController::editFormulaAt(const QPoint &viewportPos)
{
    QTextDocument *doc = m_editor->document();
    const int pos = m_editor->cursorForPosition(viewportPos).position();

    // Busca la fórmula tocada por el clic entre los grupos [inicio, fin) reales
    // (índices de carácter): el carácter a la derecha del punto (índice pos) o el
    // de la izquierda (índice pos-1). Usar los grupos canónicos evita el desfase
    // de charFormat() en los bordes.
    const QList<QPair<int, int>> groups = mdmath::mathGroupBounds(doc);
    QPair<int, int> hit{-1, -1};
    for (const auto &g : groups) {
        if ((pos >= g.first && pos < g.second)
            || (pos - 1 >= g.first && pos - 1 < g.second)) {
            hit = g;
            break;
        }
    }
    if (hit.first < 0)
        return false;

    // Lee el TeX/estilo desde un punto interior del grupo (pos hit.first+1 está
    // dentro, así charFormat() da con seguridad el formato de la fórmula).
    QTextCursor probe(doc);
    probe.setPosition(hit.first + 1);
    const QTextCharFormat cf = probe.charFormat();
    const QString tex = cf.property(mdmath::MathTexProperty).toString();
    const bool block = cf.boolProperty(mdmath::MathBlockProperty);

    QString newTex;
    bool newBlock = block;
    if (!askFormula(&newTex, &newBlock, tex, block))
        return true;  // canceló pero atendimos el clic

    const QList<mdmath::MathRun> runs = mdmath::renderFormulaRuns(newTex, newBlock);
    QTextCursor c(doc);
    c.beginEditBlock();
    c.setPosition(hit.first);
    c.setPosition(hit.second, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    for (const mdmath::MathRun &r : runs)
        c.insertText(r.text, r.fmt);
    c.endEditBlock();
    m_editor->setCurrentCharFormat(QTextCharFormat());
    return true;
}
