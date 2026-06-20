#include "admonitions.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

namespace {

// Primera «línea» de un bloque: dentro de un QTextBlock los saltos de párrafo son
// separadores Unicode (U+2028/U+2029), no '\n'. Cortamos por el primero de ellos.
QString firstLineOf(const QString &text)
{
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char('\n') || c == QChar::LineSeparator
            || c == QChar::ParagraphSeparator)
            return text.left(i);
    }
    return text;
}

const QRegularExpression &markerRe()
{
    static const QRegularExpression re(
        QStringLiteral(R"(^\s*\[!(NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]\s*$)"),
        QRegularExpression::CaseInsensitiveOption);
    return re;
}

bool isBlockquote(const QTextBlockFormat &bf)
{
    return bf.intProperty(QTextFormat::BlockQuoteLevel) > 0;
}

}  // namespace

const QList<mdadmonition::Type> &mdadmonition::types()
{
    static const QList<Type> t = {
        {QStringLiteral("NOTE"), QColor(47, 109, 222)},       // azul
        {QStringLiteral("TIP"), QColor(34, 160, 80)},         // verde
        {QStringLiteral("IMPORTANT"), QColor(137, 87, 229)},  // violeta
        {QStringLiteral("WARNING"), QColor(191, 135, 0)},     // ámbar
        {QStringLiteral("CAUTION"), QColor(210, 60, 55)},     // rojo
    };
    return t;
}

QString mdadmonition::markerKeyword(const QString &blockText)
{
    const QRegularExpressionMatch m = markerRe().match(firstLineOf(blockText));
    return m.hasMatch() ? m.captured(1).toUpper() : QString();
}

QColor mdadmonition::accentFor(const QString &keyword)
{
    for (const Type &t : types())
        if (t.keyword == keyword)
            return t.accent;
    return QColor(47, 109, 222);
}

QString mdadmonition::skeleton(const QString &keyword)
{
    // Línea en blanco de cita entre el marcador y el contenido para que Qt los
    // mantenga en bloques separados (si no, los fundiría en un solo párrafo).
    return QStringLiteral("> [!%1]\n>\n> ").arg(keyword);
}

QString mdadmonition::unescapeMarkers(const QString &markdown)
{
    // Quita la barra que toMarkdown() antepone al `[` del marcador, solo cuando va
    // en una línea de cita (`>`), para no tocar texto que diga `\[!...]` a propósito.
    static const QRegularExpression re(
        QStringLiteral(R"((^|\n)([ \t]*(?:>[ \t]*)+)\\(\[!(?:NOTE|TIP|IMPORTANT|WARNING|CAUTION)\]))"),
        QRegularExpression::CaseInsensitiveOption);
    QString out = markdown;
    out.replace(re, QStringLiteral("\\1\\2\\3"));
    return out;
}

void mdadmonition::renderAdmonitionsInDocument(QTextDocument *doc)
{
    if (!doc)
        return;

    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        if (!isBlockquote(block.blockFormat()))
            continue;
        const QString kw = markerKeyword(block.text());
        if (kw.isEmpty())
            continue;

        const QColor accent = accentFor(kw);
        QColor tint = accent;
        tint.setAlpha(28);  // fondo translúcido: legible en temas claros y oscuros
        const int level = block.blockFormat().intProperty(QTextFormat::BlockQuoteLevel);

        // El callout abarca el bloque del marcador y las citas siguientes (su
        // contenido) hasta que se acaba la cita o empieza otro marcador.
        for (QTextBlock cur = block; cur != doc->end(); cur = cur.next()) {
            const bool isMarker = (cur == block);
            if (!isMarker) {
                if (!isBlockquote(cur.blockFormat())
                    || cur.blockFormat().intProperty(QTextFormat::BlockQuoteLevel) < level
                    || !markerKeyword(cur.text()).isEmpty())
                    break;
            }

            QTextCursor bc(cur);
            QTextBlockFormat bf;
            bf.setBackground(tint);
            bc.mergeBlockFormat(bf);  // solo añade el fondo; conserva la cita

            if (isMarker) {
                // Título (la línea del marcador) en color de acento. Solo color:
                // la negrita/cursiva sí se serializan a Markdown (`**…**`) y
                // romperían el marcador; el color no es expresable, así que se
                // descarta al guardar y no afecta al round-trip.
                const int lineLen = firstLineOf(cur.text()).size();
                QTextCursor tc(cur);
                tc.movePosition(QTextCursor::StartOfBlock);
                tc.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, lineLen);
                QTextCharFormat cf;
                cf.setForeground(accent);
                tc.mergeCharFormat(cf);
            }
        }
    }
}
