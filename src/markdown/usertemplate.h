#ifndef USERTEMPLATE_H
#define USERTEMPLATE_H

/// \file
/// \brief Modelo de las plantillas de usuario y su (de)serialización para QSettings.

#include <QList>
#include <QString>
#include <QStringList>

#include "doctemplates.h"  // mdtemplate::Category

/// Plantillas de usuario: como las de fábrica (`mdtemplate`) pero definidas por el
/// propio usuario («Guardar como plantilla…») y guardadas en QSettings. Aparecen en
/// el mismo menú «Nuevo desde plantilla», mezcladas con las de fábrica por categoría.
/// Módulo PURO: solo el modelo y su (de)serialización; el diálogo y el guardado del
/// documento actual viven aparte.
namespace mdusertemplate {

/// Una plantilla de usuario: rótulo, cuerpo Markdown y categoría (el submenú).
struct UserTemplate {
    QString name;
    QString body;
    mdtemplate::Category category = mdtemplate::Category::Personal;
};

/// Serializa a una QStringList (una entrada por plantilla) apta para QSettings:
/// nombre, categoría (entero) y cuerpo unidos por el separador de unidades U+001F,
/// que no aparece en texto normal y respeta los saltos de línea del cuerpo.
QStringList serialize(const QList<UserTemplate> &templates);

/// Inversa de `serialize`. Tolera entradas de formato incompleto; una categoría
/// fuera de rango cae a Personal; descarta las de nombre vacío o solo espacios.
QList<UserTemplate> deserialize(const QStringList &stored);

}  // namespace mdusertemplate

#endif  // USERTEMPLATE_H
