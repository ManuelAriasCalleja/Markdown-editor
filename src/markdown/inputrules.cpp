/// \file
/// \brief Implementación del reconocimiento de marcadores de entrada.

#include "inputrules.h"

#include <QRegularExpression>

namespace mdinputrules {

Rule ruleForPrefix(const QString &beforeCursor)
{
    // No se recorta: el marcador debe empezar en el inicio del bloque (sin sangría),
    // y `beforeCursor` es justo lo que hay antes del espacio a punto de teclearse.
    static const QRegularExpression heading(QStringLiteral("^(#{1,6})$"));
    const QRegularExpressionMatch h = heading.match(beforeCursor);
    if (h.hasMatch())
        return {Kind::Heading, int(h.captured(1).size())};

    if (beforeCursor == QLatin1String(">"))
        return {Kind::Quote, 0};

    if (beforeCursor == QLatin1String("-") || beforeCursor == QLatin1String("*")
        || beforeCursor == QLatin1String("+"))
        return {Kind::BulletList, 0};

    static const QRegularExpression numbered(QStringLiteral("^[0-9]+[.)]$"));
    if (numbered.match(beforeCursor).hasMatch())
        return {Kind::NumberedList, 0};

    return {Kind::None, 0};
}

}  // namespace mdinputrules
