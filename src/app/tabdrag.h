#ifndef TABDRAG_H
#define TABDRAG_H

/// \file
/// \brief Lógica pura del arrastre de una pestaña a otra ventana (instancia).

#include <QMimeData>
#include <QRect>
#include <QString>

/// Qué viaja en el arrastre de una pestaña entre instancias y cuándo empieza. El
/// `QDrag` y la apertura en la ventana destino viven en `MainWindow`; aquí solo lo
/// que se puede probar sin GUI real. Se prueba en tst_tabdrag.
namespace mdtabdrag {

/// Tipo MIME propio: no se ofrece `text/uri-list` a propósito (soltar la pestaña
/// sobre el explorador de archivos, o sobre el manejador de archivos del propio
/// editor, abriría el archivo sin que la pestaña de origen se cerrase).
extern const QString kMimeType;

/// Lo que lleva el arrastre.
struct Payload {
    QString path;    ///< documento (siempre guardado: el origen lo exige antes)
    int cursor = 0;  ///< posición del cursor en el editor WYSIWYG; -1 = desconocida (modo fuente)
    QString source;  ///< nombre de la instancia de origen (a quien avisar al adoptarlo)
};

/// Empaqueta `payload` en un `QMimeData` (pasa a ser propiedad de quien lo reciba).
QMimeData *toMime(const Payload &payload);

/// Lee un `Payload` de `mime`. Falso si no es de este tipo o está incompleto/corrupto.
bool fromMime(const QMimeData *mime, Payload &payload);

/// ¿Debe empezar el arrastre entre ventanas? Cierto cuando el ratón, con la pestaña
/// pulsada, se aleja de la barra más de `margin` píxeles: dentro de ella manda el
/// reordenado interno de `QTabBar`.
bool leftBar(const QPoint &pos, const QRect &barRect, int margin);

/// ¿Acepta esta instancia (`self`) un arrastre con ese `payload`? No si es el suyo
/// propio (reordenar es cosa de la barra) o si no hay nada que abrir.
bool accepts(const Payload &payload, const QString &self);

}  // namespace mdtabdrag

#endif  // TABDRAG_H
