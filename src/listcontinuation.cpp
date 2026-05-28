#include "listcontinuation.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>

namespace mdlist {

Continuation analyze(const QString &line)
{
    // (1) sangría · (2) viñeta | (3) número (4) delimitador · espacios ·
    // (5) casilla de tarea opcional · (6) contenido.
    static const QRegularExpression re(
        QStringLiteral(R"(^([ \t]*)(?:([-*+])|([0-9]+)([.)]))[ \t]+(\[[ xX]\][ \t]+)?(.*)$)"));
    const QRegularExpressionMatch m = re.match(line);
    if (!m.hasMatch())
        return {};

    const QString indent = m.captured(1);
    const QString bullet = m.captured(2);
    const QString number = m.captured(3);
    const QString delim = m.captured(4);
    const bool isTask = !m.captured(5).isEmpty();
    const QString content = m.captured(6);

    QString marker;
    if (!number.isEmpty())
        marker = QString::number(number.toInt() + 1) + delim + QLatin1Char(' ');
    else
        marker = bullet + QLatin1Char(' ');
    if (isTask)  // la tarea nueva siempre nace sin marcar
        marker += QStringLiteral("[ ] ");

    Continuation c;
    c.isItem = true;
    c.empty = content.trimmed().isEmpty();
    c.nextPrefix = indent + marker;
    return c;
}

} // namespace mdlist
