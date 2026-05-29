#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QByteArray>
#include <QStringList>

// Fachada sobre QSettings: centraliza las claves de persistencia y expone
// accesores tipados. El resto del código no toca QSettings directamente, de
// modo que las claves viven en un único sitio.
namespace AppSettings {

// Tema activo: clave estable del tema (p. ej. "light", "dark", "monokai"; ver
// mdtheme::ThemeSpec::key). Si no hay valor guardado, migra desde el ajuste
// booleano antiguo ("darkTheme": true -> "dark", false -> "light").
QString themeKey();
void setThemeKey(const QString &key);

// Obsoletos: tema claro/oscuro como booleano. Se conservan durante la
// transición al sistema multi-tema; se retirarán cuando nadie los use.
bool darkTheme();
void setDarkTheme(bool dark);

// Luz cálida nocturna: tiñe el fondo del editor de tono ámbar según la hora
// (más cálido al anochecer/noche). Activada por defecto.
bool warmLight();
void setWarmLight(bool on);

// Nivel de zoom: desfase de tamaño de fuente (en puntos) respecto al base, que
// se aplica al editor y al resto de la interfaz. 0 = tamaño normal.
int zoomLevel();
void setZoomLevel(int level);

// Idioma de la interfaz: código de locale (p. ej. "es", "en"). Cadena vacía =
// usar el idioma del sistema.
QString language();
void setLanguage(const QString &code);

QByteArray windowGeometry();
void setWindowGeometry(const QByteArray &geometry);

QByteArray windowState();
void setWindowState(const QByteArray &state);

// Proporciones del divisor de la vista dividida (QSplitter::saveState).
QByteArray splitterState();
void setSplitterState(const QByteArray &state);

QStringList recentFiles();
void setRecentFiles(const QStringList &files);

// Último documento abierto: ruta del archivo activo al cerrar, para reabrirlo
// al arrancar (vacío = no había archivo, se arranca en blanco).
QString lastFile();
void setLastFile(const QString &path);

} // namespace AppSettings

#endif // APPSETTINGS_H
