#include "appsettings.h"

#include <QSettings>

namespace {
inline QString themeKeyKey()    { return QStringLiteral("theme"); }
inline QString darkThemeKey()   { return QStringLiteral("darkTheme"); }
inline QString warmLightKey()   { return QStringLiteral("warmLight"); }
inline QString zoomLevelKey()   { return QStringLiteral("zoomLevel"); }
inline QString geometryKey()    { return QStringLiteral("geometry"); }
inline QString windowStateKey() { return QStringLiteral("windowState"); }
inline QString recentFilesKey() { return QStringLiteral("recentFiles"); }
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

int AppSettings::zoomLevel()
{
    return QSettings().value(zoomLevelKey(), 0).toInt();
}

void AppSettings::setZoomLevel(int level)
{
    QSettings().setValue(zoomLevelKey(), level);
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

QStringList AppSettings::recentFiles()
{
    return QSettings().value(recentFilesKey()).toStringList();
}

void AppSettings::setRecentFiles(const QStringList &files)
{
    QSettings().setValue(recentFilesKey(), files);
}

QString AppSettings::lastFile()
{
    return QSettings().value(lastFileKey()).toString();
}

void AppSettings::setLastFile(const QString &path)
{
    QSettings().setValue(lastFileKey(), path);
}
