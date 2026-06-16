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

    QAction *close = addAction(tr("✕"));
    connect(close, &QAction::triggered, this, &FindReplaceBar::closeBar);

    // Enter busca / reemplaza; Esc cierra la barra.
    connect(m_findEdit, &QLineEdit::returnPressed, this, &FindReplaceBar::findNext);
    connect(m_replaceEdit, &QLineEdit::returnPressed, this, &FindReplaceBar::replaceOne);

    auto *esc = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    esc->setContext(Qt::WidgetWithChildrenShortcut);
    connect(esc, &QShortcut::activated, this, &FindReplaceBar::closeBar);
}

void FindReplaceBar::setEditor(QTextEdit *editor)
{
    m_editor = editor;
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
}

void FindReplaceBar::showReplace()
{
    showFind();
    m_replaceEdit->setFocus();
}

void FindReplaceBar::closeBar()
{
    hide();
    m_editor->setFocus();
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
        QTextCursor c = m_editor->textCursor();
        c.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        m_editor->setTextCursor(c);
        return m_editor->find(needle, flags);
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
}

void FindReplaceBar::findPrev()
{
    doFind(true);
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
    // Si la selección actual ya es la coincidencia, la reemplazamos.
    if (selectionMatches(c))
        c.insertText(m_replaceEdit->text());
    findNext();
}

void FindReplaceBar::replaceAll()
{
    if (m_findEdit->text().isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;  // (ignorado en modo regex)
    if (m_wholeWordCheck->isChecked())
        flags |= QTextDocument::FindWholeWords;

    QRegularExpression re;
    const bool useRegex = m_regexCheck->isChecked();
    if (useRegex && !buildRegex(&re))
        return;  // patrón inválido: ya se avisó

    QTextDocument *doc = m_editor->document();
    const QString replacement = m_replaceEdit->text();
    QTextCursor group(doc);
    group.beginEditBlock();  // un solo paso de deshacer para todos los reemplazos

    int count = 0;
    auto nextMatch = [&](const QTextCursor &from) {
        return useRegex ? doc->find(re, from, flags)
                        : doc->find(m_findEdit->text(), from, flags);
    };
    QTextCursor found = nextMatch(QTextCursor(doc));
    while (!found.isNull()) {
        // Una coincidencia vacía (p. ej. regex "a*") no se reemplaza y se avanza un
        // carácter, para no entrar en bucle infinito.
        if (found.selectionStart() == found.selectionEnd()) {
            QTextCursor adv = found;
            if (adv.atEnd())
                break;
            adv.movePosition(QTextCursor::NextCharacter);
            found = nextMatch(adv);
            continue;
        }
        found.insertText(replacement);
        ++count;
        found = nextMatch(found);
    }
    group.endEditBlock();

    emit statusMessage(tr("%n reemplazo(s)", nullptr, count), 3000);
}
