/// \file
/// \brief Implementación de la (de)serialización de las plantillas de usuario.

#include "usertemplate.h"

namespace mdusertemplate {

namespace {
const QChar kSep(0x1F);  // unit separator: no aparece en texto normal

// ¿`value` es una categoría válida del enum mdtemplate::Category?
bool isValidCategory(int value)
{
    for (const mdtemplate::Category c : mdtemplate::categoriesInOrder())
        if (static_cast<int>(c) == value)
            return true;
    return false;
}
}  // namespace

QStringList serialize(const QList<UserTemplate> &templates)
{
    QStringList out;
    out.reserve(templates.size());
    for (const UserTemplate &t : templates) {
        if (t.name.trimmed().isEmpty())
            continue;  // una plantilla sin nombre no es alcanzable: no se guarda
        out << t.name + kSep + QString::number(static_cast<int>(t.category)) + kSep + t.body;
    }
    return out;
}

QList<UserTemplate> deserialize(const QStringList &stored)
{
    QList<UserTemplate> out;
    out.reserve(stored.size());
    for (const QString &entry : stored) {
        UserTemplate t;
        const int sep1 = entry.indexOf(kSep);
        if (sep1 < 0) {
            t.name = entry;  // formato inesperado: todo es el nombre
        } else {
            t.name = entry.left(sep1);
            const int sep2 = entry.indexOf(kSep, sep1 + 1);
            if (sep2 < 0) {
                t.body = entry.mid(sep1 + 1);  // sin categoría: queda Personal
            } else {
                const int cat = entry.mid(sep1 + 1, sep2 - sep1 - 1).toInt();
                if (isValidCategory(cat))
                    t.category = static_cast<mdtemplate::Category>(cat);
                t.body = entry.mid(sep2 + 1);
            }
        }
        if (!t.name.trimmed().isEmpty())
            out.append(t);
    }
    return out;
}

}  // namespace mdusertemplate
