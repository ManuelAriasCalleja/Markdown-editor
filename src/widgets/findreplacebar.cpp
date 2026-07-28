/// \file
/// \brief Implementación de la barra de buscar/reemplazar (búsqueda con envoltura,
/// regex, reemplazar uno/todos).

#include "findreplacebar.h"

#include <QAction>
#include <QCheckBox>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QShortcut>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

#include "find.h"
#include "mathblocks.h"

namespace {

// ¿El rango [start, start+length) solapa alguna fórmula? Las fórmulas son grupos
// atómicos: reemplazar dentro de una corrompe el TeX (la serialización reinyecta
// el original) o parte el grupo, así que esos aciertos se saltan al reemplazar.
bool overlapsMath(const QList<QPair<int, int>> &groups, int start, int length)
{
    const int end = start + length;
    for (const auto &g : groups)
        if (start < g.second && g.first < end)
            return true;
    return false;
}

}  // namespace

FindReplaceBar::FindReplaceBar(QTextEdit *editor, QWidget *parent)
    : QToolBar(tr("Buscar"), parent), m_editor(editor)
{
    setObjectName(QStringLiteral("findBar"));  // para save/restoreState
    setMovable(false);
    buildUi();
    hide();  // oculta hasta que se pida (Ctrl+F / Ctrl+H)
}

void FindReplaceBar::buildUi()
{
    addWidget(new QLabel(tr("Buscar: "), this));
    m_findEdit = new QLineEdit(this);
    m_findEdit->setObjectName(QStringLiteral("findEdit"));
    m_findEdit->setAccessibleName(tr("Buscar"));  // sin esto, el campo no tiene nombre para el lector
    m_findEdit->setClearButtonEnabled(true);
    addWidget(m_findEdit);

    QAction *prev = addAction(tr("◀ Anterior"));
    connect(prev, &QAction::triggered, this, &FindReplaceBar::findPrev);
    QAction *next = addAction(tr("Siguiente ▶"));
    connect(next, &QAction::triggered, this, &FindReplaceBar::findNext);

    addSeparator();

    addWidget(new QLabel(tr("Reemplazar: "), this));
    m_replaceEdit = new QLineEdit(this);
    m_replaceEdit->setObjectName(QStringLiteral("replaceEdit"));
    m_replaceEdit->setAccessibleName(tr("Reemplazar"));
    m_replaceEdit->setClearButtonEnabled(true);
    addWidget(m_replaceEdit);

    QAction *repl = addAction(tr("Reemplazar"));
    connect(repl, &QAction::triggered, this, &FindReplaceBar::replaceOne);
    QAction *replAll = addAction(tr("Todo"));
    connect(replAll, &QAction::triggered, this, &FindReplaceBar::replaceAll);

    addSeparator();
    m_caseCheck = new QCheckBox(tr("May/min"), this);
    m_caseCheck->setToolTip(tr("Distinguir mayúsculas y minúsculas"));
    addWidget(m_caseCheck);

    m_wholeWordCheck = new QCheckBox(tr("Palabra completa"), this);
    m_wholeWordCheck->setToolTip(tr("Buscar solo palabras completas"));
    addWidget(m_wholeWordCheck);

    m_regexCheck = new QCheckBox(tr("Regex"), this);
    m_regexCheck->setToolTip(tr("Usar expresiones regulares"));
    addWidget(m_regexCheck);

    addSeparator();
    m_countLabel = new QLabel(this);
    m_countLabel->setAccessibleName(tr("Coincidencias"));
    addWidget(m_countLabel);

    QAction *close = addAction(tr("✕"));
    connect(close, &QAction::triggered, this, &FindReplaceBar::closeBar);

    // Enter busca / reemplaza; Esc cierra la barra.
    connect(m_findEdit, &QLineEdit::returnPressed, this, &FindReplaceBar::findNext);
    connect(m_replaceEdit, &QLineEdit::returnPressed, this, &FindReplaceBar::replaceOne);
    // Recalcular el contador y el resaltado al cambiar el término o las opciones.
    connect(m_findEdit, &QLineEdit::textChanged, this, &FindReplaceBar::updateMatches);
    connect(m_caseCheck, &QCheckBox::toggled, this, &FindReplaceBar::updateMatches);
    connect(m_wholeWordCheck, &QCheckBox::toggled, this, &FindReplaceBar::updateMatches);
    connect(m_regexCheck, &QCheckBox::toggled, this, &FindReplaceBar::updateMatches);

    auto *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WidgetWithChildrenShortcut);
    connect(esc, &QShortcut::activated, this, &FindReplaceBar::closeBar);
}

void FindReplaceBar::setEditor(QTextEdit *editor)
{
    m_editor = editor;
    // Si la barra está abierta, recuenta/re-resalta sobre el nuevo editor (cambio de
    // vista WYSIWYG↔fuente). Oculta: no hay resaltado que mantener.
    if (isVisible())
        updateMatches();
}

void FindReplaceBar::showFind()
{
    show();
    // Si hay texto seleccionado, lo usamos como término de búsqueda inicial.
    const QString sel = m_editor->textCursor().selectedText();
    if (!sel.isEmpty() && !sel.contains(QChar(QChar::ParagraphSeparator)))
        m_findEdit->setText(sel);
    m_findEdit->setFocus();
    m_findEdit->selectAll();
    updateMatches();  // refleja el contador y resalta al abrir con término
}

void FindReplaceBar::showReplace()
{
    showFind();
    m_replaceEdit->setFocus();
}

void FindReplaceBar::closeBar()
{
    hide();
    emit highlightMatches({});  // quita el resaltado de coincidencias
    m_countLabel->clear();
    m_editor->setFocus();
}

void FindReplaceBar::findAgain(bool backward)
{
    // Sin término no hay nada que repetir: abrir la barra es lo útil (y lo que
    // espera quien pulsa F3 al llegar al documento). Con término se busca aunque
    // la barra esté oculta, sin robarle el foco al editor: el resultado se ve en
    // el propio documento y en el mensaje de estado.
    if (m_findEdit->text().isEmpty()) {
        showFind();
        return;
    }
    if (backward)
        findPrev();
    else
        findNext();
}

QList<mdfind::Match> FindReplaceBar::currentMatches() const
{
    if (!m_editor || m_findEdit->text().isEmpty())
        return {};
    return mdfind::findAll(m_editor->toPlainText(), m_findEdit->text(),
                           m_regexCheck->isChecked(), m_caseCheck->isChecked(),
                           m_wholeWordCheck->isChecked());
}

void FindReplaceBar::updateMatches()
{
    const QList<mdfind::Match> matches = currentMatches();
    if (m_findEdit->text().isEmpty()) {
        m_countLabel->clear();
    } else if (matches.isEmpty()) {
        m_countLabel->setText(tr("Sin coincidencias"));
    } else {
        // Si el cursor está justo sobre una coincidencia (tras buscar), «N de M»;
        // si no (recién escrito el término), el total.
        const int ord = mdfind::matchOrdinal(matches, m_editor->textCursor().selectionStart());
        m_countLabel->setText(ord > 0 ? tr("%1 de %2").arg(ord).arg(matches.size())
                                      : tr("%n coincidencia(s)", nullptr, int(matches.size())));
    }
    emit highlightMatches(matches);
}

bool FindReplaceBar::buildRegex(QRegularExpression *re)
{
    QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
    if (!m_caseCheck->isChecked())
        opts |= QRegularExpression::CaseInsensitiveOption;
    re->setPattern(m_findEdit->text());
    re->setPatternOptions(opts);
    if (!re->isValid()) {
        emit statusMessage(tr("Expresión regular no válida: %1").arg(re->errorString()),
                           4000);
        return false;
    }
    return true;
}

bool FindReplaceBar::doFind(bool backward)
{
    const QString text = m_findEdit->text();
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;  // (ignorado en modo regex)
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;
    if (backward)
        flags |= QTextDocument::FindBackward;

    // Busca el término; si no hay más, da la vuelta (al principio o al final).
    // Genérico para reutilizar la misma lógica con texto literal o con regex.
    auto findWithWrap = [this, backward, flags](const auto &needle) -> bool {
        if (m_editor->find(needle, flags))
            return true;
        const QTextCursor original = m_editor->textCursor();
        QTextCursor c = original;
        c.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        m_editor->setTextCursor(c);
        if (m_editor->find(needle, flags))
            return true;
        m_editor->setTextCursor(original);  // sin resultado: no movemos al usuario
        return false;
    };

    bool found = false;
    if (m_regexCheck->isChecked()) {
        QRegularExpression re;
        if (!buildRegex(&re))
            return false;  // patrón inválido: ya se avisó
        found = findWithWrap(re);
    } else {
        found = findWithWrap(text);
    }
    if (found)
        return true;

    emit statusMessage(tr("No se encontró: %1").arg(text), 3000);
    return false;
}

void FindReplaceBar::findNext()
{
    doFind(false);
    updateMatches();  // refresca «N de M» según la nueva posición del cursor
}

void FindReplaceBar::findPrev()
{
    doFind(true);
    updateMatches();
}

bool FindReplaceBar::selectionMatches(const QTextCursor &c) const
{
    if (!c.hasSelection())
        return false;
    if (m_regexCheck->isChecked()) {
        QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
        if (!m_caseCheck->isChecked())
            opts |= QRegularExpression::CaseInsensitiveOption;
        const QRegularExpression re(m_findEdit->text(), opts);
        if (!re.isValid())
            return false;
        // Coincidencia solo si la regex casa la selección entera.
        const QRegularExpressionMatch m = re.match(c.selectedText());
        return m.hasMatch() && m.capturedStart() == 0
               && m.capturedLength() == c.selectedText().size();
    }
    const Qt::CaseSensitivity cs =
        m_caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    return c.selectedText().compare(m_findEdit->text(), cs) == 0;
}

void FindReplaceBar::replaceOne()
{
    QTextCursor c = m_editor->textCursor();
    // Si la selección actual ya es la coincidencia, la reemplazamos, salvo que
    // caiga dentro de una fórmula (grupo atómico): en ese caso no se toca.
    if (selectionMatches(c)
        && !overlapsMath(mdmath::mathGroupBounds(m_editor->document()),
                         c.selectionStart(), c.selectionEnd() - c.selectionStart()))
        c.insertText(m_replaceEdit->text());
    findNext();
}

void FindReplaceBar::replaceAll()
{
    if (m_findEdit->text().isEmpty())
        return;

    // Camino ÚNICO de coincidencias (mdfind, el mismo que cuenta y resalta). Si el
    // patrón regex es inválido, buildRegex avisa; findAll ya devolvería vacío.
    if (m_regexCheck->isChecked()) {
        QRegularExpression re;
        if (!buildRegex(&re))
            return;  // patrón inválido: ya se avisó
    }
    const QList<mdfind::Match> matches = currentMatches();
    if (matches.isEmpty()) {
        emit statusMessage(tr("%n reemplazo(s)", nullptr, 0), 3000);
        return;
    }

    QTextDocument *doc = m_editor->document();
    const QString replacement = m_replaceEdit->text();
    // Aciertos que caen dentro de una fórmula: se omiten (reemplazarlos corrompería
    // el TeX o partiría el grupo). Se calcula una vez sobre el documento actual.
    const QList<QPair<int, int>> mathGroups = mdmath::mathGroupBounds(doc);
    QTextCursor group(doc);
    group.beginEditBlock();  // un solo paso de deshacer para todos los reemplazos
    // De atrás hacia adelante: así cada reemplazo no invalida las posiciones de los
    // anteriores (que están antes en el documento).
    int replaced = 0;
    for (int i = matches.size() - 1; i >= 0; --i) {
        if (overlapsMath(mathGroups, matches.at(i).start, matches.at(i).length))
            continue;
        QTextCursor c(doc);
        c.setPosition(matches.at(i).start);
        c.setPosition(matches.at(i).start + matches.at(i).length, QTextCursor::KeepAnchor);
        c.insertText(replacement);
        ++replaced;
    }
    group.endEditBlock();

    emit statusMessage(tr("%n reemplazo(s)", nullptr, replaced), 3000);
    updateMatches();  // el documento cambió: recuenta y re-resalta
}
