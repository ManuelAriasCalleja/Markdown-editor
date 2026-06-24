#include "texttransform.h"

#include <QStringList>

#include <algorithm>

QString mdtext::toUpper(const QString &s)
{
    return s.toUpper();
}

QString mdtext::toLower(const QString &s)
{
    return s.toLower();
}

QString mdtext::capitalize(const QString &s)
{
    QString out = s;
    bool startOfWord = true;
    for (int i = 0; i < out.size(); ++i) {
        const QChar c = out.at(i);
        if (c.isLetter()) {
            out[i] = startOfWord ? c.toUpper() : c.toLower();
            startOfWord = false;
        } else {
            startOfWord = true;  // cualquier no-letra reinicia la palabra
        }
    }
    return out;
}

QString mdtext::sortLines(const QString &s, bool ascending)
{
    QStringList lines = s.split(QLatin1Char('\n'));
    std::stable_sort(lines.begin(), lines.end(),
                     [ascending](const QString &a, const QString &b) {
                         const int cmp = QString::localeAwareCompare(a, b);
                         return ascending ? cmp < 0 : cmp > 0;
                     });
    return lines.join(QLatin1Char('\n'));
}

QString mdtext::smartTypography(const QString &s)
{
    // Guiones y puntos suspensivos (universales). El orden importa: `---` antes
    // que `--`, y como — y – son un solo carácter, no quedan `--` residuales.
    QString t = s;
    t.replace(QStringLiteral("..."), QString(QChar(0x2026)));   // …
    t.replace(QStringLiteral("---"), QString(QChar(0x2014)));   // —
    t.replace(QStringLiteral("--"), QString(QChar(0x2013)));    // –

    // Comillas rectas → tipográficas según el carácter anterior: tras un hueco o
    // un paréntesis/corchete de apertura se abre; en otro caso se cierra (y la
    // comilla simple de cierre hace además de apóstrofo: don't → don’t).
    QString out;
    out.reserve(t.size());
    for (int i = 0; i < t.size(); ++i) {
        const QChar ch = t.at(i);
        const QChar prev = i > 0 ? t.at(i - 1) : QChar();
        const bool atOpen = prev.isNull() || prev.isSpace()
            || prev == QLatin1Char('(') || prev == QLatin1Char('[')
            || prev == QLatin1Char('{');
        if (ch == QLatin1Char('"'))
            out += atOpen ? QChar(0x201C) : QChar(0x201D);   // “ ”
        else if (ch == QLatin1Char('\''))
            out += atOpen ? QChar(0x2018) : QChar(0x2019);   // ‘ ’
        else
            out += ch;
    }
    return out;
}
