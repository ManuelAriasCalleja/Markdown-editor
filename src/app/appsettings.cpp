/// \file
/// \brief Implementación de la fachada AppSettings sobre QSettings (lectura/escritura tipada).

#include "appsettings.h"

#include <QSettings>
#include <QVariantMap>

namespace {
inline QString themeKeyKey()    { return QStringLiteral("theme"); }
inline QString darkThemeKey()   { return QStringLiteral("darkTheme"); }
inline QString warmLightKey()   { return QStringLiteral("warmLight"); }
inline QString followSystemThemeKey() { return QStringLiteral("followSystemTheme"); }
inline QString zoomLevelKey()   { return QStringLiteral("zoomLevel"); }
inline QString showWordCountKey() { return QStringLiteral("showWordCount"); }
inline QString showLineColumnKey() { return QStringLiteral("showLineColumn"); }
inline QString typewriterModeKey() { return QStringLiteral("typewriterMode"); }
inline QString currentLineHighlightKey() { return QStringLiteral("currentLineHighlight"); }
inline QString lineSpacingKey()    { return QStringLiteral("lineSpacing"); }
inline QString snippetsKey()    { return QStringLiteral("snippets"); }
inline QString spellCheckKey()  { return QStringLiteral("spellCheck"); }
inline QString spellLanguageKey() { return QStringLiteral("spellLanguage"); }
inline QString cursorPositionsKey() { return QStringLiteral("cursorPositions"); }
constexpr int kMaxCursorPositions = 200;  // cota del mapa de posiciones recordadas
inline QString geometryKey()    { return QStringLiteral("geometry"); }
inline QString windowStateKey() { return QStringLiteral("windowState"); }
inline QString splitterStateKey() { return QStringLiteral("splitterState"); }
inline QString recentFilesKey() { return QStringLiteral("recentFiles"); }
inline QString personalDictionaryKey() { return QStringLiteral("personalDictionary"); }
inline QString languageKey()    { return QStringLiteral("language"); }
inline QString lastFileKey()    { return QStringLiteral("lastFile"); }
} // namespace

QString AppSettings::themeKey()
{
    QSettings s;
    const QVariant v = s.value(themeKeyKey());
    if (v.isValid())
        return v.toString();
    // Migración del ajuste antiguo: booleano claro/oscuro -> clave de tema.
    return s.value(darkThemeKey(), false).toBool() ? QStringLiteral("dark")
                                                   : QStringLiteral("light");
}

void AppSettings::setThemeKey(const QString &key)
{
    QSettings().setValue(themeKeyKey(), key);
}

bool AppSettings::darkTheme()
{
    return QSettings().value(darkThemeKey(), false).toBool();
}

void AppSettings::setDarkTheme(bool dark)
{
    QSettings().setValue(darkThemeKey(), dark);
}

bool AppSettings::warmLight()
{
    return QSettings().value(warmLightKey(), true).toBool();  // activada por defecto
}

void AppSettings::setWarmLight(bool on)
{
    QSettings().setValue(warmLightKey(), on);
}

bool AppSettings::followSystemTheme()
{
    return QSettings().value(followSystemThemeKey(), false).toBool();  // desactivado por defecto
}

void AppSettings::setFollowSystemTheme(bool on)
{
    QSettings().setValue(followSystemThemeKey(), on);
}

int AppSettings::cursorPosition(const QString &path)
{
    if (path.isEmpty())
        return -1;
    return QSettings().value(cursorPositionsKey()).toMap().value(path, -1).toInt();
}

void AppSettings::setCursorPosition(const QString &path, int pos)
{
    if (path.isEmpty())
        return;
    QSettings s;
    QVariantMap m = s.value(cursorPositionsKey()).toMap();
    // Al rebasar la cota se vacía el mapa (cota simple, sin orden de uso): el
    // archivo recién cerrado se vuelve a sembrar para no perder su posición.
    if (m.size() >= kMaxCursorPositions && !m.contains(path))
        m.clear();
    m.insert(path, pos);
    s.setValue(cursorPositionsKey(), m);
}

int AppSettings::zoomLevel()
{
    return QSettings().value(zoomLevelKey(), 0).toInt();
}

void AppSettings::setZoomLevel(int level)
{
    QSettings().setValue(zoomLevelKey(), level);
}

bool AppSettings::showWordCount()
{
    return QSettings().value(showWordCountKey(), true).toBool();  // activado por defecto
}

void AppSettings::setShowWordCount(bool on)
{
    QSettings().setValue(showWordCountKey(), on);
}

bool AppSettings::showLineColumn()
{
    return QSettings().value(showLineColumnKey(), false).toBool();  // desactivado por defecto
}

void AppSettings::setShowLineColumn(bool on)
{
    QSettings().setValue(showLineColumnKey(), on);
}

bool AppSettings::typewriterMode()
{
    return QSettings().value(typewriterModeKey(), false).toBool();  // desactivado por defecto
}
void AppSettings::setTypewriterMode(bool on)
{
    QSettings().setValue(typewriterModeKey(), on);
}

bool AppSettings::currentLineHighlight()
{
    return QSettings().value(currentLineHighlightKey(), false).toBool();  // desactivado por defecto
}
void AppSettings::setCurrentLineHighlight(bool on)
{
    QSettings().setValue(currentLineHighlightKey(), on);
}

int AppSettings::lineSpacing()
{
    return QSettings().value(lineSpacingKey(), 100).toInt();  // sencillo por defecto
}
void AppSettings::setLineSpacing(int percent)
{
    QSettings().setValue(lineSpacingKey(), percent);
}

bool AppSettings::spellCheck()
{
    return QSettings().value(spellCheckKey(), true).toBool();  // activado por defecto
}

void AppSettings::setSpellCheck(bool on)
{
    QSettings().setValue(spellCheckKey(), on);
}

QString AppSettings::spellLanguage()
{
    return QSettings().value(spellLanguageKey()).toString();
}

void AppSettings::setSpellLanguage(const QString &code)
{
    QSettings().setValue(spellLanguageKey(), code);
}

QString AppSettings::language()
{
    return QSettings().value(languageKey()).toString();
}

void AppSettings::setLanguage(const QString &code)
{
    QSettings().setValue(languageKey(), code);
}

QByteArray AppSettings::windowGeometry()
{
    return QSettings().value(geometryKey()).toByteArray();
}

void AppSettings::setWindowGeometry(const QByteArray &geometry)
{
    QSettings().setValue(geometryKey(), geometry);
}

QByteArray AppSettings::windowState()
{
    return QSettings().value(windowStateKey()).toByteArray();
}

void AppSettings::setWindowState(const QByteArray &state)
{
    QSettings().setValue(windowStateKey(), state);
}

QByteArray AppSettings::splitterState()
{
    return QSettings().value(splitterStateKey()).toByteArray();
}

void AppSettings::setSplitterState(const QByteArray &state)
{
    QSettings().setValue(splitterStateKey(), state);
}

QStringList AppSettings::recentFiles()
{
    return QSettings().value(recentFilesKey()).toStringList();
}

void AppSettings::setRecentFiles(const QStringList &files)
{
    QSettings().setValue(recentFilesKey(), files);
}

QStringList AppSettings::personalDictionary()
{
    return QSettings().value(personalDictionaryKey()).toStringList();
}

void AppSettings::setPersonalDictionary(const QStringList &words)
{
    QSettings().setValue(personalDictionaryKey(), words);
}

QList<mdsnippet::Snippet> AppSettings::snippets()
{
    return mdsnippet::deserialize(QSettings().value(snippetsKey()).toStringList());
}
void AppSettings::setSnippets(const QList<mdsnippet::Snippet> &snippets)
{
    QSettings().setValue(snippetsKey(), mdsnippet::serialize(snippets));
}

QString AppSettings::lastFile()
{
    return QSettings().value(lastFileKey()).toString();
}

void AppSettings::setLastFile(const QString &path)
{
    QSettings().setValue(lastFileKey(), path);
}

QStringList AppSettings::openFiles()
{
    return QSettings().value(QStringLiteral("openFiles")).toStringList();
}

void AppSettings::setOpenFiles(const QStringList &paths)
{
    QSettings().setValue(QStringLiteral("openFiles"), paths);
}
