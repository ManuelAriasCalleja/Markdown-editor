#ifndef LANGTAG_H
#define LANGTAG_H

/// \file
/// \brief Etiqueta canónica de idioma: el nombre con el que el programa busca los
///        recursos de un idioma (manual, traducción, tabla de exportación).

#include <QString>

/// Normalización de códigos de idioma, pura y sin GUI (`tst_langtag`). El programa
/// recibe códigos de sitios muy distintos —el ajuste del menú, el locale del
/// sistema, el `lang:` del front matter de un documento ajeno— escritos de todas
/// las maneras (`es`, `es-ES`, `es_ES`, `zh-Hans`), y todos tienen que acabar en el
/// mismo nombre para encontrar el mismo recurso.
namespace mdlang {

/// \brief Etiqueta canónica de `code`: `es-ES`/`es_ES`/`es` → `es`.
///
/// El chino es la excepción y la razón de que esto exista: simplificado y
/// tradicional son **dos recursos distintos** (manual y traducción propios), y
/// `code.left(2)` los confundía en un solo `zh`. Se distinguen por el **sistema de
/// escritura**, no por el territorio, porque el territorio se queda corto:
/// `zh_SG` (Singapur) es simplificado y `zh_HK` (Hong Kong) es tradicional, y ni uno
/// ni otro se llaman `CN` ni `TW`. `QLocale` deduce el script incluso cuando solo se
/// da el territorio, así que `zh`, `zh_CN`, `zh-Hans` y `zh_SG` → `zh_CN`, y
/// `zh-Hant`, `zh_TW`, `zh_HK` → `zh_TW`.
/// \param code código de idioma en cualquier forma; vacío devuelve vacío.
/// \return la etiqueta canónica (`es`, `pt`, `zh_CN`, `zh_TW`…).
QString canonicalTag(const QString &code);

}  // namespace mdlang

#endif // LANGTAG_H
