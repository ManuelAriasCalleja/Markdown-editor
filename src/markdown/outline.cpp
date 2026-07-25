/// \file
/// \brief Implementación de las funciones puras de `mdoutline`.

#include "outline.h"

#include <algorithm>

#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <QVector>

QList<OutlineHeading> mdoutline::headingsOf(const QTextDocument *doc)
{
    QList<OutlineHeading> headings;
    if (!doc)
        return headings;

    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        const int level = block.blockFormat().headingLevel();
        if (level > 0)
            headings.append({level, block.text(), block.blockNumber()});
    }
    return headings;
}

int mdoutline::shiftedLevel(int current, int delta)
{
    if (current < 1)
        return 0;  // no es un encabezado: sin cambio
    return std::clamp(current + delta, 1, 6);
}

QSet<int> mdoutline::visibleOrdinals(const QList<OutlineHeading> &headings,
                                     const QString &filter)
{
    QSet<int> visible;
    const QString f = filter.trimmed();
    if (f.isEmpty()) {
        for (int i = 0; i < headings.size(); ++i)
            visible.insert(i);
        return visible;
    }
    for (int j = 0; j < headings.size(); ++j) {
        if (!headings.at(j).text.contains(f, Qt::CaseInsensitive))
            continue;
        visible.insert(j);
        // Ancestros: hacia atrás, cada nivel estrictamente menor cuelga a este.
        int needLevel = headings.at(j).level;
        for (int i = j - 1; i >= 0 && needLevel > 1; --i) {
            if (headings.at(i).level < needLevel) {
                visible.insert(i);
                needLevel = headings.at(i).level;
            }
        }
    }
    return visible;
}

QString mdoutline::tableOfContentsMarkdown(const QList<OutlineHeading> &headings)
{
    QString md;
    QVector<int> stack;  // niveles de los ancestros vigentes (misma idea que rebuild)
    for (const OutlineHeading &h : headings) {
        while (!stack.isEmpty() && stack.last() >= h.level)
            stack.removeLast();
        const int depth = stack.size();  // 0 = raíz
        md += QString(qsizetype(depth) * 2, QLatin1Char(' '));
        md += QStringLiteral("- ") + h.text + QLatin1Char('\n');
        stack.append(h.level);
    }
    return md;
}

namespace {

// Encabezados ATX (`#`..`######` + espacio o fin de línea) a principio de línea,
// ignorando el interior de los bloques de código vallados (``` o ~~~). Replica la
// secuencia de headingsOf sobre el Markdown serializado para que los ordinales
// coincidan. Devuelve, por cada encabezado, su índice de línea y su nivel.
struct HeadingLine { int line; int level; };

QVector<HeadingLine> scanHeadings(const QStringList &lines)
{
    static const QRegularExpression fenceRe(QStringLiteral(R"(^\s{0,3}(`{3,}|~{3,}))"));
    static const QRegularExpression atxRe(QStringLiteral(R"(^(#{1,6})(?: |$))"));
    QVector<HeadingLine> out;
    bool inFence = false;
    QString fenceMarker;  // p. ej. "```": cierra con la misma valla
    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);
        const QRegularExpressionMatch fm = fenceRe.match(line);
        if (fm.hasMatch()) {
            const QString marker = fm.captured(1);
            if (!inFence) {
                inFence = true;
                fenceMarker = marker.left(1);  // ` o ~
            } else if (marker.startsWith(fenceMarker)) {
                inFence = false;
            }
            continue;
        }
        if (inFence)
            continue;
        const QRegularExpressionMatch hm = atxRe.match(line);
        if (hm.hasMatch())
            out.append({i, int(hm.capturedLength(1))});
    }
    return out;
}

}  // namespace

QString mdoutline::moveSection(const QString &markdown, int fromOrdinal,
                               int toOrdinal, bool placeAfter)
{
    if (fromOrdinal == toOrdinal && !placeAfter)
        return markdown;
    QStringList lines = markdown.split(QLatin1Char('\n'));
    const QVector<HeadingLine> hs = scanHeadings(lines);
    if (fromOrdinal < 0 || fromOrdinal >= hs.size()
        || toOrdinal < 0 || toOrdinal >= hs.size())
        return markdown;

    // Fin (exclusivo, en líneas) de la sección del ordinal k: el siguiente
    // encabezado de nivel igual o menor, o el final del documento.
    const auto sectionEnd = [&](int k) {
        const int lvl = hs.at(k).level;
        for (int m = k + 1; m < hs.size(); ++m)
            if (hs.at(m).level <= lvl)
                return hs.at(m).line;
        return int(lines.size());
    };

    const int sStart = hs.at(fromOrdinal).line;
    const int sEnd = sectionEnd(fromOrdinal);

    // No mover una sección dentro de sí misma.
    const int destLine = hs.at(toOrdinal).line;
    if (destLine >= sStart && destLine < sEnd)
        return markdown;

    int insertLine = placeAfter ? sectionEnd(toOrdinal) : destLine;

    const QStringList moved = lines.mid(sStart, sEnd - sStart);
    lines.erase(lines.begin() + sStart, lines.begin() + sEnd);
    if (insertLine >= sEnd)
        insertLine -= (sEnd - sStart);  // las líneas tras el hueco se desplazaron

    for (int j = 0; j < moved.size(); ++j)
        lines.insert(insertLine + j, moved.at(j));

    return lines.join(QLatin1Char('\n'));
}
