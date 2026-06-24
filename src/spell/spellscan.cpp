#include "spellscan.h"

#include <QChar>

namespace mdspell {

namespace {
bool isApostrophe(QChar c)
{
    return c == QLatin1Char('\'') || c == QChar(0x2019);  // ' y ’ tipográfico
}
bool isWordChar(QChar c)
{
    return c.isLetterOrNumber() || isApostrophe(c);
}
}  // namespace

QList<Word> tokenize(const QString &text)
{
    QList<Word> words;
    const int n = text.size();
    int i = 0;
    while (i < n) {
        if (!isWordChar(text.at(i))) {
            ++i;
            continue;
        }
        const int start = i;
        bool hasDigit = false;
        bool hasLetter = false;
        while (i < n && isWordChar(text.at(i))) {
            const QChar c = text.at(i);
            if (c.isNumber())
                hasDigit = true;
            else if (c.isLetter())
                hasLetter = true;
            ++i;
        }
        // Tokens con dígitos (h2o, v1) o sin letras (solo apóstrofos) no son
        // palabras de un idioma natural.
        if (hasDigit || !hasLetter)
            continue;
        int ws = start;
        int we = i;
        while (ws < we && isApostrophe(text.at(ws)))
            ++ws;
        while (we > ws && isApostrophe(text.at(we - 1)))
            --we;
        if (we > ws)
            words.append({ws, we - ws});
    }
    return words;
}

QString pickDictionary(const QString &lang, const QStringList &available)
{
    if (available.isEmpty())
        return QString();
    QString normalized = lang;
    normalized.replace(QLatin1Char('-'), QLatin1Char('_'));
    const QStringList parts = normalized.split(QLatin1Char('_'), Qt::SkipEmptyParts);
    if (parts.isEmpty())
        return QString();
    const QString base = parts.at(0).toLower();
    const QString region = parts.size() > 1 ? parts.at(1).toUpper() : QString();

    const auto findCi = [&available](const QString &want) -> QString {
        for (const QString &a : available)
            if (a.compare(want, Qt::CaseInsensitive) == 0)
                return a;
        return QString();
    };

    // 1) variante regional exacta (es_ES), 2) idioma base (es).
    if (!region.isEmpty()) {
        const QString hit = findCi(base + QLatin1Char('_') + region);
        if (!hit.isEmpty())
            return hit;
    }
    QString baseHit = findCi(base);  // no const: permite mover en el return
    if (!baseHit.isEmpty())
        return baseHit;

    // 3) cualquier variante regional del idioma (pt → pt_BR/pt_PT), determinista.
    QStringList variants;
    for (const QString &a : available)
        if (a.toLower().startsWith(base + QLatin1Char('_')))
            variants.append(a);
    variants.sort();
    return variants.isEmpty() ? QString() : variants.first();
}

} // namespace mdspell
