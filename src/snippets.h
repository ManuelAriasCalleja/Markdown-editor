#ifndef SNIPPETS_H
#define SNIPPETS_H

#include <QList>
#include <QString>
#include <QStringList>

// Snippets de usuario: fragmentos de Markdown reutilizables que el usuario define
// e inserta por nombre (complementan las plantillas de documento, que son de
// archivo entero). Este módulo es PURO: solo el modelo y su (de)serialización
// para QSettings; la edición (diálogo) y la inserción (EditorStack) viven aparte.
namespace mdsnippet {

struct Snippet {
    QString name;   // rótulo que se ve en el menú
    QString body;   // cuerpo Markdown que se inserta
};

// Serializa la lista a una QStringList (una entrada por snippet) apta para
// QSettings: nombre y cuerpo unidos por el separador de unidades U+001F, que no
// aparece en texto normal y deja intactos los saltos de línea del cuerpo.
QStringList serialize(const QList<Snippet> &snippets);

// Inversa de `serialize`. Tolera entradas sin separador (todo es el nombre,
// cuerpo vacío) y descarta las de nombre vacío o solo espacios.
QList<Snippet> deserialize(const QStringList &stored);

} // namespace mdsnippet

#endif // SNIPPETS_H
