#include "blockconstructs.h"

#include <QChar>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

// ---------------------------------------------------------------------------
// Transformaciones de texto puras
// ---------------------------------------------------------------------------

QString mdblock::addBlockquoteMarkers(const QString &markdown)
{
    const QStringList lines = markdown.split(QLatin1Char('\n'));
    QStringList result;
    result.reserve(lines.size());
    for (const QString &line : lines)
        result << QLatin1String("> ") + line;
    return result.join(QLatin1Char('\n'));
}

QString mdblock::removeBlockquoteMarkers(const QString &markdown)
{
    const QStringList lines = markdown.split(QLatin1Char('\n'));
    QStringList result;
    result.reserve(lines.size());
    for (QString line : lines) {
        if (line.startsWith(QLatin1String("> ")))
            line = line.mid(2);
        else if (line.startsWith(QLatin1Char('>')))
            line = line.mid(1);
        result << line;
    }
    return result.join(QLatin1Char('\n'));
}

QString mdblock::fenceCode(const QString &plainText)
{
    return QLatin1String("```\n") + plainText + QLatin1String("\n```");
}

// ---------------------------------------------------------------------------
// Plumbing del editor (común a todos los constructos)
// ---------------------------------------------------------------------------

namespace {

// Expande el cursor a bloques completos. Si `c` no es nulo, abarca además todos
// los bloques contiguos que pertenecen al mismo constructo (p. ej. todas las
// líneas de un bloque de código o de una cita de varias líneas).
void expandSelection(QTextCursor &cursor, const BlockConstruct *c)
{
    QTextDocument *doc = cursor.document();
    int first = doc->findBlock(cursor.selectionStart()).blockNumber();
    int last = doc->findBlock(cursor.selectionEnd()).blockNumber();

    if (c) {
        while (first > 0 &&
               c->contains(doc->findBlockByNumber(first - 1).blockFormat()))
            --first;
        while (last < doc->blockCount() - 1 &&
               c->contains(doc->findBlockByNumber(last + 1).blockFormat()))
            ++last;
    }

    const QTextBlock fb = doc->findBlockByNumber(first);
    const QTextBlock lb = doc->findBlockByNumber(last);
    cursor.setPosition(fb.position());
    cursor.setPosition(lb.position() + lb.length() - 1, QTextCursor::KeepAnchor);
}

// Sustituye la selección (de bloques completos) por `fragment`, reiniciando el
// formato de bloque para que la estructura nueva venga solo del fragmento.
void replaceSelectionWith(QTextEdit *editor, const QTextDocumentFragment &fragment)
{
    QTextCursor cursor = editor->textCursor();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.insertFragment(fragment);
    cursor.endEditBlock();
    editor->setFocus();
}

} // namespace

void BlockConstruct::toggle(QTextEdit *editor) const
{
    QTextCursor cursor = editor->textCursor();
    const bool removing = contains(cursor.blockFormat());

    // Al quitar, abarca todo el constructo; al poner, los bloques seleccionados.
    expandSelection(cursor, removing ? this : nullptr);
    editor->setTextCursor(cursor);

    replaceSelectionWith(editor, buildReplacement(cursor, removing));
}

// ---------------------------------------------------------------------------
// Cita / blockquote
// ---------------------------------------------------------------------------

namespace {

// Devuelve el Markdown de la selección (preservando el formato en línea).
QString selectionToMarkdown(const QTextCursor &cursor)
{
    QTextDocument tmp;
    QTextCursor tc(&tmp);
    tc.insertFragment(cursor.selection());
    return tmp.toMarkdown().trimmed();
}

} // namespace

bool Blockquote::contains(const QTextBlockFormat &bf) const
{
    return bf.intProperty(QTextFormat::BlockQuoteLevel) > 0;
}

QTextDocumentFragment Blockquote::buildReplacement(const QTextCursor &selection,
                                                   bool removing) const
{
    const QString md = selectionToMarkdown(selection);
    const QString out = removing ? mdblock::removeBlockquoteMarkers(md)
                                 : mdblock::addBlockquoteMarkers(md);
    return QTextDocumentFragment::fromMarkdown(out);
}

// ---------------------------------------------------------------------------
// Bloque de código
// ---------------------------------------------------------------------------

bool CodeBlock::contains(const QTextBlockFormat &bf) const
{
    return bf.hasProperty(QTextFormat::BlockCodeFence);
}

QTextDocumentFragment CodeBlock::buildReplacement(const QTextCursor &selection,
                                                  bool removing) const
{
    // El código es literal: trabajamos con texto plano (los saltos de bloque
    // vienen como separador de párrafo de Qt, U+2029).
    QString text = selection.selectedText();
    text.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));

    if (removing) {
        // Reinsertar como texto plano: no reinterpreta '*', '#', etc. como
        // Markdown y conserva las líneas tal cual.
        return QTextDocumentFragment::fromPlainText(text);
    }
    return QTextDocumentFragment::fromMarkdown(mdblock::fenceCode(text));
}
