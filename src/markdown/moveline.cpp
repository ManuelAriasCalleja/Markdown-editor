/// \file
/// \brief Implementación de los comandos de línea puros (mdmoveline).

#include "moveline.h"

namespace mdmoveline {
namespace {

bool inRange(const QStringList &lines, int line)
{
    return line >= 0 && line < lines.size();
}

QString trimLeadingSpace(const QString &s)
{
    int k = 0;
    while (k < s.size() && (s.at(k) == QLatin1Char(' ') || s.at(k) == QLatin1Char('\t')))
        ++k;
    return s.mid(k);
}

QString trimTrailingSpace(const QString &s)
{
    int e = s.size();
    while (e > 0 && (s.at(e - 1) == QLatin1Char(' ') || s.at(e - 1) == QLatin1Char('\t')))
        --e;
    return s.left(e);
}

}  // namespace

Result moveUp(const QStringList &lines, int line)
{
    if (line <= 0 || !inRange(lines, line))
        return {lines, line};
    QStringList out = lines;
    out.swapItemsAt(line, line - 1);
    return {out, line - 1};
}

Result moveDown(const QStringList &lines, int line)
{
    if (line < 0 || line >= lines.size() - 1)
        return {lines, line};
    QStringList out = lines;
    out.swapItemsAt(line, line + 1);
    return {out, line + 1};
}

Result duplicate(const QStringList &lines, int line)
{
    if (!inRange(lines, line))
        return {lines, line};
    QStringList out = lines;
    out.insert(line + 1, lines.at(line));
    return {out, line};  // el cursor se queda en la línea original
}

Result removeLine(const QStringList &lines, int line)
{
    if (!inRange(lines, line))
        return {lines, line};
    QStringList out = lines;
    out.removeAt(line);
    if (out.isEmpty())
        return {QStringList{QString()}, 0};  // el editor siempre tiene al menos una línea
    return {out, qMin(line, int(out.size()) - 1)};
}

Result joinNext(const QStringList &lines, int line)
{
    if (line < 0 || line >= lines.size() - 1)
        return {lines, line};
    QStringList out = lines;
    const QString a = trimTrailingSpace(out.at(line));
    const QString b = trimLeadingSpace(out.at(line + 1));
    QString joined;
    if (a.isEmpty())
        joined = b;
    else if (b.isEmpty())
        joined = a;
    else
        joined = a + QLatin1Char(' ') + b;
    out[line] = joined;
    out.removeAt(line + 1);
    return {out, line};
}

}  // namespace mdmoveline
