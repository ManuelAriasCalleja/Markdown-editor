#ifndef RICHPASTE_H
#define RICHPASTE_H

/// \file
/// \brief Conversión del HTML del portapapeles a Markdown («Pegar como Markdown»).

#include <QString>

/// Conversión de texto enriquecido (HTML del portapapeles) a Markdown, para
/// «Pegar como Markdown»: en vez de incrustar el formato del origen (con todo el
/// ruido de estilos de navegadores/procesadores de texto), se normaliza a la
/// sintaxis Markdown que el documento sí sabe round-tripear.
namespace mdrichpaste {

/// Convierte un fragmento de HTML a Markdown. Pasa por un QTextDocument auxiliar
/// (`setHtml`) y serializa con la ruta canónica del proyecto
/// (`mdtable::documentMarkdown`), de modo que el resultado es el mismo subconjunto
/// de Markdown que produce el editor. Devuelve el Markdown sin el salto de línea
/// final que añade `toMarkdown()`. Función «pura» (sin GUI, testeable aislada).
QString htmlToMarkdown(const QString &html);

}  // namespace mdrichpaste

#endif  // RICHPASTE_H
