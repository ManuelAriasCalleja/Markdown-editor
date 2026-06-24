/// \file
/// \brief Implementación de la detección de URLs «pegables».

#include "urldetect.h"

#include <QRegularExpression>

bool mdurl::looksLikeUrl(const QString &text)
{
    const QString t = text.trimmed();
    if (t.isEmpty())
        return false;
    // `\S+` ya excluye cualquier espacio interno; se ancla a toda la cadena.
    static const QRegularExpression re(
        QStringLiteral(R"(^(?:https?|ftp)://\S+$|^mailto:\S+@\S+$)"),
        QRegularExpression::CaseInsensitiveOption);
    return re.match(t).hasMatch();
}
