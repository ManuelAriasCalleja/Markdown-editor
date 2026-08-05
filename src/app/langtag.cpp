/// \file
/// \brief Implementación de la normalización de códigos de idioma.

#include "langtag.h"

#include <QLocale>

QString mdlang::canonicalTag(const QString &code)
{
    if (code.isEmpty())
        return {};

    QString norm = code;
    norm.replace(QLatin1Char('-'), QLatin1Char('_'));

    // Solo el chino se consulta a QLocale, y solo para leer su sistema de escritura.
    // Para los demás vale el prefijo de la cadena: pasar por QLocale un código que no
    // reconoce devuelve el locale «C», y de ahí saldría un idioma que nadie pidió.
    const QLocale loc(norm);
    if (loc.language() == QLocale::Chinese) {
        return loc.script() == QLocale::TraditionalHanScript ? QStringLiteral("zh_TW")
                                                             : QStringLiteral("zh_CN");
    }
    return norm.section(QLatin1Char('_'), 0, 0).toLower();
}
