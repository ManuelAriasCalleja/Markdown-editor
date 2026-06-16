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
