/// \file
/// \brief Implementación del motor de corrección: carga de diccionarios y consulta a Hunspell.

#include "spellchecker.h"

#include "spellscan.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QSet>
#include <QStringConverter>

#ifdef HAVE_HUNSPELL
#include <hunspell/hunspell.hxx>
#endif

namespace {

// Rutas donde buscar los .aff/.dic. Incluye las del sistema (Linux) y, para
// empaquetados portables, un directorio junto al binario; las que no existan se
// ignoran solas, así que no hace falta código por plataforma.
QStringList searchPaths()
{
    QStringList paths;
    const QString appDir = QCoreApplication::applicationDirPath();
    paths << appDir + QStringLiteral("/dictionaries")               // junto al .exe (Windows) / binario
          << appDir + QStringLiteral("/../Resources/dictionaries")  // dentro del .app (macOS)
          << appDir + QStringLiteral("/../share/hunspell")          // <prefix>/share/hunspell (instalación)
          << QStringLiteral("/usr/share/hunspell")                  // diccionarios del sistema (Linux)
          << QStringLiteral("/usr/share/myspell/dicts")
          << QStringLiteral("/usr/share/myspell");
    return paths;
}

// Basenames de diccionario disponibles (un .dic con su .aff al lado) → el
// directorio donde está cada uno. El primero gana si hay duplicados.
QHash<QString, QString> availableDictionaries()
{
    QHash<QString, QString> result;
    for (const QString &dirPath : searchPaths()) {
        QDir dir(dirPath);
        if (!dir.exists())
            continue;
        for (const QFileInfo &fi : dir.entryInfoList({QStringLiteral("*.dic")}, QDir::Files)) {
            const QString base = fi.completeBaseName();
            if (!result.contains(base)
                && QFileInfo::exists(dir.filePath(base + QStringLiteral(".aff"))))
                result.insert(base, dirPath);
        }
    }
    return result;
}

#ifdef HAVE_HUNSPELL
QStringConverter::Encoding encodingFromName(const QString &name)
{
    const QString u = name.toUpper();
    if (u.contains(QLatin1String("8859-1")) || u.contains(QLatin1String("LATIN1")))
        return QStringConverter::Latin1;
    return QStringConverter::Utf8;  // lo habitual en diccionarios modernos
}
#endif

}  // namespace

struct SpellChecker::Impl {
    QString language;
    QSet<QString> ignored;
    QSet<QString> personal;
#ifdef HAVE_HUNSPELL
    std::unique_ptr<Hunspell> hunspell;
    QStringConverter::Encoding encoding = QStringConverter::Utf8;

    QByteArray encode(const QString &word) const
    {
        QStringEncoder enc(encoding);
        return QByteArray(enc.encode(word));
    }
    QString decode(const std::string &s) const
    {
        QStringDecoder dec(encoding);
        return dec.decode(QByteArray::fromStdString(s));
    }
#endif
};

SpellChecker::SpellChecker() : d(std::make_unique<Impl>()) {}
SpellChecker::~SpellChecker() = default;

bool SpellChecker::setLanguage(const QString &lang)
{
#ifdef HAVE_HUNSPELL
    const QHash<QString, QString> dicts = availableDictionaries();
    const QString pick = mdspell::pickDictionary(lang, dicts.keys());
    if (pick.isEmpty()) {
        d->hunspell.reset();
        d->language.clear();
        return false;
    }
    if (d->hunspell && d->language == pick)
        return true;  // ya cargado: no recargar (carga perezosa)

    const QDir dir(dicts.value(pick));
    const QString aff = dir.filePath(pick + QStringLiteral(".aff"));
    const QString dic = dir.filePath(pick + QStringLiteral(".dic"));
    d->hunspell = std::make_unique<Hunspell>(aff.toLocal8Bit().constData(),
                                             dic.toLocal8Bit().constData());
    d->encoding = encodingFromName(QString::fromLatin1(d->hunspell->get_dic_encoding()));
    d->language = pick;
    // Repuebla el diccionario en memoria con las palabras personales.
    for (const QString &w : std::as_const(d->personal))
        d->hunspell->add(d->encode(w).toStdString());
    return true;
#else
    Q_UNUSED(lang);
    return false;
#endif
}

QString SpellChecker::language() const
{
    return d->language;
}

bool SpellChecker::isAvailable() const
{
#ifdef HAVE_HUNSPELL
    return d->hunspell != nullptr;
#else
    return false;
#endif
}

bool SpellChecker::isCorrect(const QString &word) const
{
    if (word.isEmpty() || d->ignored.contains(word) || d->personal.contains(word))
        return true;
#ifdef HAVE_HUNSPELL
    if (!d->hunspell)
        return true;
    return d->hunspell->spell(d->encode(word).toStdString());
#else
    return true;  // sin motor no marcamos nada
#endif
}

QStringList SpellChecker::suggestions(const QString &word) const
{
    QStringList out;
#ifdef HAVE_HUNSPELL
    if (d->hunspell) {
        for (const std::string &s : d->hunspell->suggest(d->encode(word).toStdString()))
            out << d->decode(s);
    }
#else
    Q_UNUSED(word);
#endif
    return out;
}

void SpellChecker::ignoreWord(const QString &word)
{
    if (!word.isEmpty())
        d->ignored.insert(word);
}

void SpellChecker::addToPersonal(const QString &word)
{
    if (word.isEmpty())
        return;
    d->personal.insert(word);
#ifdef HAVE_HUNSPELL
    if (d->hunspell)
        d->hunspell->add(d->encode(word).toStdString());
#endif
}

QStringList SpellChecker::personalWords() const
{
    return QStringList(d->personal.values());
}

void SpellChecker::setPersonalWords(const QStringList &words)
{
    d->personal = QSet<QString>(words.cbegin(), words.cend());
#ifdef HAVE_HUNSPELL
    if (d->hunspell)
        for (const QString &w : words)
            d->hunspell->add(d->encode(w).toStdString());
#endif
}

QStringList SpellChecker::availableLanguages()
{
    QStringList langs = availableDictionaries().keys();
    langs.sort();
    return langs;
}
