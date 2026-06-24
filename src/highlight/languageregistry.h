#ifndef LANGUAGEREGISTRY_H
#define LANGUAGEREGISTRY_H

/// \file
/// \brief Registro declarativo de lenguajes (palabras clave y estilos de comentario) para el resaltado.

#include <QString>
#include <QStringList>

/// \brief Descripción de un lenguaje para el resaltado: sus palabras clave y qué
/// estilos de comentario admite. Es solo *datos*; la mecánica de resaltado vive
/// en CodeBlockHighlighter.
struct LangSpec
{
    QStringList keywords;
    bool slash = false;  // comentarios //
    bool hash = false;   // comentarios #
    bool block = false;  // comentarios /* */
};

/// \brief Registro de lenguajes: resuelve un alias (de la valla ```cpp, ```py, ...) a
/// su LangSpec mediante una tabla de búsqueda. Añadir un lenguaje es añadir una
/// entrada a la tabla, sin tocar el resaltador (Open/Closed).
namespace LanguageRegistry {

/// \brief Devuelve la especificación del lenguaje. Para alias desconocidos o vacíos
/// devuelve una spec genérica (sin palabras clave, pero con comentarios
/// //, # y /* */) para que algo se resalte igualmente.
LangSpec specFor(const QString &language);

} // namespace LanguageRegistry

#endif // LANGUAGEREGISTRY_H
