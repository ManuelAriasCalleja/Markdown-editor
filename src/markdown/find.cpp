/// \file
/// \brief Implementación de la búsqueda pura (todas las coincidencias + ordinal).

#include "find.h"

#include <QRegularExpression>

namespace mdfind {

QList<Match> findAll(const QString &text, const QString &needle, bool useRegex,
                     bool caseSensitive, bool wholeWord)
{
    QList<Match> out;
    if (needle.isEmpty())
        return out;

    QString pattern = useRegex ? needle : QRegularExpression::escape(needle);
    if (wholeWord)
        pattern = QStringLiteral("\\b(?:") + pattern + QStringLiteral(")\\b");

    QRegularExpression::PatternOptions opts = QRegularExpression::NoPatternOption;
    if (!caseSensitive)
        opts |= QRegularExpression::CaseInsensitiveOption;
    if (wholeWord)
        opts |= QRegularExpression::UseUnicodePropertiesOption;  // \b sensible a acentos

    const QRegularExpression re(pattern, opts);
    if (!re.isValid())
        return out;  // patrón inválido: sin coincidencias (la barra avisa por su cuenta)

    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const int len = m.capturedLength();
        if (len <= 0)
            continue;  // coincidencia vacía (p. ej. "a*"): se ignora, como en replaceAll
        out.append({int(m.capturedStart()), len});
    }
    return out;
}

int matchOrdinal(const QList<Match> &matches, int pos)
{
    for (int i = 0; i < matches.size(); ++i)
        if (matches.at(i).start == pos)
            return i + 1;
    return 0;
}

}  // namespace mdfind
