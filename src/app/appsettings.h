#ifndef APPSETTINGS_H
#define APPSETTINGS_H

/// \file
/// \brief Fachada tipada sobre QSettings: centraliza todas las claves de persistencia de la app.

#include <QByteArray>
#include <QStringList>

#include "snippets.h"
#include "usertemplate.h"

/// \brief Fachada sobre QSettings: centraliza las claves de persistencia y expone
/// accesores tipados.
///
/// El resto del código no toca QSettings directamente, de modo que las claves
/// viven en un único sitio.
namespace AppSettings {

/// \brief Tema activo: clave estable del tema (p. ej. "light", "dark", "monokai"; ver
/// mdtheme::ThemeSpec::key).
///
/// Si no hay valor guardado, migra desde el ajuste booleano antiguo
/// ("darkTheme": true -> "dark", false -> "light").
QString themeKey();
void setThemeKey(const QString &key);

/// \brief Obsoletos: tema claro/oscuro como booleano. Se conservan durante la
/// transición al sistema multi-tema; se retirarán cuando nadie los use.
bool darkTheme();
void setDarkTheme(bool dark);

/// \brief Seguir el tema claro/oscuro del sistema operativo: cuando está activo, el
/// tema se deriva del esquema de color del SO (claro -> Claro, oscuro -> Oscuro)
/// y cambia solo si el SO cambia. Desactivado por defecto. Ortogonal a la luz
/// cálida nocturna.
bool followSystemTheme();
void setFollowSystemTheme(bool on);

/// \brief Luz cálida nocturna: tiñe el fondo del editor de tono ámbar según la hora
/// (más cálido al anochecer/noche). Activada por defecto.
bool warmLight();
void setWarmLight(bool on);

/// \brief Nivel de zoom: desfase de tamaño de fuente (en puntos) respecto al base, que
/// se aplica al editor y al resto de la interfaz. 0 = tamaño normal.
int zoomLevel();
void setZoomLevel(int level);

/// \brief Mostrar el contador de palabras/caracteres en la barra de estado. Activado
/// por defecto.
bool showWordCount();
void setShowWordCount(bool on);

/// \brief Mostrar el indicador de línea y columna del cursor en la barra de estado.
/// Desactivado por defecto.
bool showLineColumn();
void setShowLineColumn(bool on);

/// \brief Modo «máquina de escribir»: la línea del cursor se mantiene centrada en
/// vertical mientras se escribe. Desactivado por defecto.
bool typewriterMode();
void setTypewriterMode(bool on);

/// \brief Previsualizar los bloques ```mermaid/```plantuml como imagen bajo el
/// bloque (ejecutando la herramienta externa). Activado por defecto; al apagarlo
/// esos bloques se quedan como código sin renderizar.
bool diagramPreview();
void setDiagramPreview(bool on);

/// \brief Resaltar la línea actual con un fondo sutil. Desactivado por defecto.
bool currentLineHighlight();
void setCurrentLineHighlight(bool on);

/// \brief Añadir el número de página en el pie al imprimir / exportar a PDF.
/// Activado por defecto.
bool printPageNumbers();
void setPrintPageNumbers(bool on);

/// \brief Interlineado del editor, en porcentaje de la altura de línea natural
/// (100 = sencillo, valor por defecto; 150 = 1,5 líneas; 200 = doble). Es
/// presentación pura: no se serializa al Markdown.
int lineSpacing();
void setLineSpacing(int percent);

/// \brief Corrector ortográfico activo (subrayado de erratas). Activado por defecto.
bool spellCheck();
void setSpellCheck(bool on);

/// \brief ¿Avisar cuando falta el diccionario del idioma del documento? Activado por
/// defecto; el propio aviso ofrece desactivarlo («No volver a avisar»).
bool spellMissingWarning();
void setSpellMissingWarning(bool on);

/// \brief Idioma de corrección forzado (basename de diccionario, p. ej. "en_US"). Vacío
/// = automático (se deduce del documento: front matter › idioma de la app ›
/// locale del sistema).
QString spellLanguage();
void setSpellLanguage(const QString &code);

/// \brief Posición del cursor recordada por archivo (índice de carácter), para reabrir
/// cada documento donde se dejó.
///
/// \return -1 si no hay nada guardado. El mapa está acotado para no crecer sin límite.
int cursorPosition(const QString &path);
void setCursorPosition(const QString &path, int pos);

/// \brief Idioma de la interfaz: código de locale (p. ej. "es", "en"). Cadena vacía =
/// usar el idioma del sistema.
QString language();
void setLanguage(const QString &code);

/// \brief Geometría de la ventana principal (QWidget::saveGeometry).
QByteArray windowGeometry();
void setWindowGeometry(const QByteArray &geometry);

/// \brief Estado de la ventana principal: barras y docks (QMainWindow::saveState).
QByteArray windowState();
void setWindowState(const QByteArray &state);

/// \brief Proporciones del divisor de la vista dividida (QSplitter::saveState).
QByteArray splitterState();
void setSplitterState(const QByteArray &state);

/// \brief Lista de archivos abiertos recientemente.
QStringList recentFiles();
void setRecentFiles(const QStringList &files);

/// \brief Diccionario personal del corrector ortográfico: palabras que el usuario marcó
/// como correctas («añadir al diccionario»), compartidas entre idiomas.
QStringList personalDictionary();
void setPersonalDictionary(const QStringList &words);

/// \brief Snippets de usuario (ver `mdsnippet`): fragmentos Markdown reutilizables que el
/// usuario define y se insertan desde *Insertar → Snippet*.
QList<mdsnippet::Snippet> snippets();
void setSnippets(const QList<mdsnippet::Snippet> &snippets);

/// \brief Plantillas de usuario (ver `mdusertemplate`): documentos completos que el
/// usuario guarda con *Archivo → Guardar como plantilla…* y reaparecen en *Nuevo
/// desde plantilla*, mezcladas con las de fábrica por categoría.
QList<mdusertemplate::UserTemplate> userTemplates();
void setUserTemplates(const QList<mdusertemplate::UserTemplate> &templates);

/// \brief ¿Se ha mostrado ya el documento de bienvenida? Se pone a true tras el
/// primer arranque para no repetirlo. Por defecto false (primer arranque).
bool welcomeShown();
void setWelcomeShown(bool shown);

/// \brief Último documento abierto: ruta del archivo activo al cerrar, para reabrirlo
/// al arrancar (vacío = no había archivo, se arranca en blanco).
QString lastFile();
void setLastFile(const QString &path);

/// \brief Rutas de los documentos abiertos (una pestaña cada uno) al cerrar, para
/// reabrirlas en la próxima sesión. Vacío = sin sesión previa de pestañas.
QStringList openFiles();
void setOpenFiles(const QStringList &paths);

} // namespace AppSettings

#endif // APPSETTINGS_H
