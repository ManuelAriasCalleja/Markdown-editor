/// \file
/// \brief Implementación de la tokenización de palabras y la selección de diccionario.

#include "spellscan.h"

#include <QChar>
#include <QHash>

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

QString dictionaryPackage(const QString &lang)
{
    // es → hunspell-es; en_US → hunspell-en-us. Debian, Fedora y Arch usan la
    // misma convención de nombre (cambia el gestor, no el paquete).
    QString code = lang;
    code.replace(QLatin1Char('_'), QLatin1Char('-'));
    return QStringLiteral("hunspell-") + code.toLower();
}

QString dictionaryInstallCommand(const QString &lang, const QString &productType)
{
    const QString pkg = dictionaryPackage(lang);
    const QString os = productType.toLower();
    // Familias con gestor propio; el resto cae en apt, que es lo más extendido.
    if (os == QLatin1String("fedora") || os == QLatin1String("rhel")
        || os == QLatin1String("centos") || os == QLatin1String("rocky")
        || os == QLatin1String("almalinux"))
        return QStringLiteral("sudo dnf install ") + pkg;
    if (os == QLatin1String("arch") || os == QLatin1String("manjaro")
        || os == QLatin1String("endeavouros"))
        return QStringLiteral("sudo pacman -S ") + pkg;
    if (os.startsWith(QLatin1String("opensuse")) || os == QLatin1String("suse")
        || os == QLatin1String("sled") || os == QLatin1String("sles"))
        return QStringLiteral("sudo zypper install ") + pkg;
    if (os == QLatin1String("alpine"))
        return QStringLiteral("sudo apk add ") + pkg;
    // macOS y Windows no tienen repositorio de diccionarios: sin orden que dar.
    if (os == QLatin1String("macos") || os == QLatin1String("osx")
        || os == QLatin1String("windows") || os == QLatin1String("winnt"))
        return QString();
    return QStringLiteral("sudo apt install ") + pkg;
}

QPair<QString, QString> dictionaryUrls(const QString &lang)
{
    // Misma tabla que scripts/fetch-dictionaries.sh (ver el comentario del .h).
    static const QHash<QString, QString> kPaths = {
        {QStringLiteral("es_es"), QStringLiteral("es/es_ES")},
        {QStringLiteral("en_us"), QStringLiteral("en/en_US")},
        {QStringLiteral("de_de"), QStringLiteral("de/de_DE_frami")},
        {QStringLiteral("fr_fr"), QStringLiteral("fr_FR/dictionaries/fr")},
        {QStringLiteral("it_it"), QStringLiteral("it_IT/it_IT")},
        {QStringLiteral("pt_br"), QStringLiteral("pt_BR/pt_BR")},
        {QStringLiteral("pl_pl"), QStringLiteral("pl_PL/pl_PL")},
        {QStringLiteral("nl_nl"), QStringLiteral("nl_NL/nl_NL")},
        {QStringLiteral("ro_ro"), QStringLiteral("ro/ro_RO")},
    };
    // Un idioma sin región (es, de…) se resuelve a la variante que se empaqueta,
    // que es lo que el usuario espera al pedir «español» sin más.
    static const QHash<QString, QString> kBase = {
        {QStringLiteral("es"), QStringLiteral("es_es")},
        {QStringLiteral("en"), QStringLiteral("en_us")},
        {QStringLiteral("de"), QStringLiteral("de_de")},
        {QStringLiteral("fr"), QStringLiteral("fr_fr")},
        {QStringLiteral("it"), QStringLiteral("it_it")},
        {QStringLiteral("pt"), QStringLiteral("pt_br")},
        {QStringLiteral("pl"), QStringLiteral("pl_pl")},
        {QStringLiteral("nl"), QStringLiteral("nl_nl")},
        {QStringLiteral("ro"), QStringLiteral("ro_ro")},
    };

    QString key = lang.toLower();
    key.replace(QLatin1Char('-'), QLatin1Char('_'));
    if (!kPaths.contains(key))
        key = kBase.value(key.section(QLatin1Char('_'), 0, 0));
    const QString path = kPaths.value(key);
    if (path.isEmpty())
        return {};

    const QString base = QStringLiteral(
        "https://raw.githubusercontent.com/LibreOffice/dictionaries/master/");
    return {base + path + QStringLiteral(".aff"), base + path + QStringLiteral(".dic")};
}

} // namespace mdspell
