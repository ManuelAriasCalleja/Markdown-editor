/// \file
/// \brief Implementación de los constructos de bloque (cita, bloque de código) y sus transformaciones de texto.

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

} // namespace

void BlockConstruct::toggle(QTextEdit *editor) const
{
    QTextCursor cursor = editor->textCursor();
    const bool removing = contains(cursor.blockFormat());

    // Al quitar, abarca todo el constructo; al poner, los bloques seleccionados.
    expandSelection(cursor, removing ? this : nullptr);
    editor->setTextCursor(cursor);

    const QTextDocumentFragment fragment = buildReplacement(cursor, removing);

    cursor.beginEditBlock();
    const int start = cursor.selectionStart();
    cursor.removeSelectedText();
    // Reinicia el formato del bloque vacío resultante: la estructura nueva debe
    // venir solo del fragmento / de blockFormat(), no arrastrar el formato viejo.
    cursor.setBlockFormat(QTextBlockFormat());
    cursor.setBlockCharFormat(QTextCharFormat());
    cursor.insertFragment(fragment);
    const int end = cursor.position();

    if (!removing) {
        // insertFragment fusiona el primer bloque del fragmento con el bloque
        // actual y, al hacerlo, descarta su formato de bloque (no el de
        // carácter): la primera línea perdería la valla/cita. Re-aplicamos el
        // formato canónico del constructo a TODOS los bloques insertados, lo que
        // además crea el bloque cuando el contenido estaba vacío (fragmento vacío
        // ⇒ el rango es el propio bloque, que así adquiere la valla/cita).
        QTextDocument *doc = cursor.document();
        const QTextBlockFormat bf = blockFormat();
        const QTextCharFormat bcf = blockCharFormat();
        const int firstNo = doc->findBlock(start).blockNumber();
        const int lastNo = doc->findBlock(end).blockNumber();
        for (int i = firstNo; i <= lastNo; ++i) {
            QTextCursor bc(doc->findBlockByNumber(i));
            bc.mergeBlockFormat(bf);
            if (bcf.propertyCount() > 0)
                bc.mergeBlockCharFormat(bcf);
        }
    }
    cursor.endEditBlock();
    editor->setTextCursor(cursor);
    editor->setFocus();
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
    if (!removing && md.isEmpty())
        return {};  // cita vacía: el formato lo pone toggle() vía blockFormat()
    const QString out = removing ? mdblock::removeBlockquoteMarkers(md)
                                 : mdblock::addBlockquoteMarkers(md);
    return QTextDocumentFragment::fromMarkdown(out);
}

QTextBlockFormat Blockquote::blockFormat() const
{
    // Capturamos el formato exacto que Qt asigna a una cita (propiedad
    // BlockQuoteLevel y sangrías), parseando una muestra: así el round-trip a
    // Markdown emite "> " sin tener que codificar las propiedades a mano.
    static const QTextBlockFormat fmt = [] {
        QTextDocument tmp;
        tmp.setMarkdown(QStringLiteral("> x"));
        return tmp.firstBlock().blockFormat();
    }();
    return fmt;
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
    if (text.isEmpty())
        return {};  // bloque vacío: el formato lo pone toggle() vía blockFormat()
    return QTextDocumentFragment::fromMarkdown(mdblock::fenceCode(text));
}

QTextBlockFormat CodeBlock::blockFormat() const
{
    static const QTextBlockFormat fmt = [] {
        QTextDocument tmp;
        tmp.setMarkdown(QStringLiteral("```\nx\n```"));
        return tmp.firstBlock().blockFormat();
    }();
    return fmt;
}

QTextCharFormat CodeBlock::blockCharFormat() const
{
    // Monoespaciado: para que el texto tecleado en un bloque de código vacío
    // salga ya con la fuente de paso fijo (lo toma de la misma muestra).
    static const QTextCharFormat fmt = [] {
        QTextDocument tmp;
        tmp.setMarkdown(QStringLiteral("```\nx\n```"));
        const QTextBlock b = tmp.firstBlock();
        auto it = b.begin();
        return it.atEnd() ? b.charFormat() : it.fragment().charFormat();
    }();
    return fmt;
}
