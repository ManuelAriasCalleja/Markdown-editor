#include "findreplacebar.h"

#include <QAction>
#include <QCheckBox>
#include <QKeySequence>
#include <QLabel>
#include <QLineEdit>
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

bool FindReplaceBar::doFind(bool backward)
{
    const QString text = m_findEdit->text();
    if (text.isEmpty())
        return false;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    if (m_editor->find(text, flags))
        return true;

    // Sin más coincidencias: damos la vuelta (al principio o al final).
    QTextCursor c = m_editor->textCursor();
    c.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
    m_editor->setTextCursor(c);
    if (m_editor->find(text, flags))
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

void FindReplaceBar::replaceOne()
{
    QTextCursor c = m_editor->textCursor();
    const Qt::CaseSensitivity cs =
        m_caseCheck->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    // Si la selección actual ya es la coincidencia, la reemplazamos.
    if (c.hasSelection() && c.selectedText().compare(m_findEdit->text(), cs) == 0)
        c.insertText(m_replaceEdit->text());
    findNext();
}

void FindReplaceBar::replaceAll()
{
    const QString find = m_findEdit->text();
    if (find.isEmpty())
        return;

    QTextDocument::FindFlags flags;
    if (m_caseCheck->isChecked())
        flags |= QTextDocument::FindCaseSensitively;

    QTextDocument *doc = m_editor->document();
    QTextCursor group(doc);
    group.beginEditBlock();  // un solo paso de deshacer para todos los reemplazos

    int count = 0;
    QTextCursor found = doc->find(find, 0, flags);
    while (!found.isNull()) {
        found.insertText(m_replaceEdit->text());
        found = doc->find(find, found, flags);
        ++count;
    }
    group.endEditBlock();

    emit statusMessage(tr("%n reemplazo(s)", nullptr, count), 3000);
}
