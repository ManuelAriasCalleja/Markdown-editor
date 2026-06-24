#include "snippets.h"

namespace mdsnippet {

namespace {
const QChar kSep(0x1F);  // unit separator: no aparece en texto normal
}

QStringList serialize(const QList<Snippet> &snippets)
{
    QStringList out;
    out.reserve(snippets.size());
    for (const Snippet &s : snippets) {
        if (s.name.trimmed().isEmpty())
            continue;  // un snippet sin nombre no es alcanzable: no se guarda
        out << s.name + kSep + s.body;
    }
    return out;
}

QList<Snippet> deserialize(const QStringList &stored)
{
    QList<Snippet> out;
    out.reserve(stored.size());
    for (const QString &entry : stored) {
        const int sep = entry.indexOf(kSep);
        Snippet s;
        if (sep < 0) {
            s.name = entry;  // formato viejo / sin cuerpo
        } else {
            s.name = entry.left(sep);
            s.body = entry.mid(sep + 1);
        }
        if (!s.name.trimmed().isEmpty())
            out.append(s);
    }
    return out;
}

} // namespace mdsnippet
