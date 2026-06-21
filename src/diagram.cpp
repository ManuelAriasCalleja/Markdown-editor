#include "diagram.h"

namespace mddiagram {

Kind kindForLanguage(const QString &lang)
{
    const QString l = lang.trimmed().toLower();
    if (l == QLatin1String("mermaid"))
        return Kind::Mermaid;
    if (l == QLatin1String("plantuml") || l == QLatin1String("puml")
        || l == QLatin1String("uml"))
        return Kind::PlantUml;
    return Kind::None;
}

} // namespace mddiagram
