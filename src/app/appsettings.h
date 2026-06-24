#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QByteArray>
#include <QStringList>

#include "snippets.h"

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

// Seguir el tema claro/oscuro del sistema operativo: cuando está activo, el
// tema se deriva del esquema de color del SO (claro -> Claro, oscuro -> Oscuro)
// y cambia solo si el SO cambia. Desactivado por defecto. Ortogonal a la luz
// cálida nocturna.
bool followSystemTheme();
void setFollowSystemTheme(bool on);

// Luz cálida nocturna: tiñe el fondo del editor de tono ámbar según la hora
// (más cálido al anochecer/noche). Activada por defecto.
bool warmLight();
void setWarmLight(bool on);

// Nivel de zoom: desfase de tamaño de fuente (en puntos) respecto al base, que
// se aplica al editor y al resto de la interfaz. 0 = tamaño normal.
int zoomLevel();
void setZoomLevel(int level);

// Mostrar el contador de palabras/caracteres en la barra de estado. Activado
// por defecto.
bool showWordCount();
void setShowWordCount(bool on);

// Modo «máquina de escribir»: la línea del cursor se mantiene centrada en
// vertical mientras se escribe. Desactivado por defecto.
bool typewriterMode();
void setTypewriterMode(bool on);

// Corrector ortográfico activo (subrayado de erratas). Activado por defecto.
bool spellCheck();
void setSpellCheck(bool on);

// Idioma de corrección forzado (basename de diccionario, p. ej. "en_US"). Vacío
// = automático (se deduce del documento: front matter › idioma de la app ›
// locale del sistema).
QString spellLanguage();
void setSpellLanguage(const QString &code);

// Posición del cursor recordada por archivo (índice de carácter), para reabrir
// cada documento donde se dejó. cursorPosition devuelve -1 si no hay nada
// guardado. El mapa está acotado para no crecer sin límite.
int cursorPosition(const QString &path);
void setCursorPosition(const QString &path, int pos);

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

// Diccionario personal del corrector ortográfico: palabras que el usuario marcó
// como correctas («añadir al diccionario»), compartidas entre idiomas.
QStringList personalDictionary();
void setPersonalDictionary(const QStringList &words);

// Snippets de usuario (ver `mdsnippet`): fragmentos Markdown reutilizables que el
// usuario define y se insertan desde *Insertar → Snippet*.
QList<mdsnippet::Snippet> snippets();
void setSnippets(const QList<mdsnippet::Snippet> &snippets);

// Último documento abierto: ruta del archivo activo al cerrar, para reabrirlo
// al arrancar (vacío = no había archivo, se arranca en blanco).
QString lastFile();
void setLastFile(const QString &path);

// Rutas de los documentos abiertos (una pestaña cada uno) al cerrar, para
// reabrirlas en la próxima sesión. Vacío = sin sesión previa de pestañas.
QStringList openFiles();
void setOpenFiles(const QStringList &paths);

} // namespace AppSettings

#endif // APPSETTINGS_H
