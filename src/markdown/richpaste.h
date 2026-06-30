#ifndef RICHPASTE_H
#define RICHPASTE_H

/// \file
/// \brief Conversión del HTML del portapapeles a Markdown («Pegar como Markdown»).

#include <QString>

class QTextDocumentFragment;

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

/// Serializa un fragmento del documento (p. ej. la selección del editor) a
/// Markdown, para «Copiar como Markdown». Inserta el fragmento en un QTextDocument
/// auxiliar y lo pasa por la ruta canónica (`mdtable::documentMarkdown`), que
/// reinyecta la alineación de tablas y las fórmulas, conservando las propiedades de
/// carácter del fragmento (negrita/cursiva/enlaces, y los grupos de math). Devuelve
/// el Markdown sin el salto de línea final. Función «pura» (sin GUI).
QString fragmentToMarkdown(const QTextDocumentFragment &fragment);

}  // namespace mdrichpaste

#endif  // RICHPASTE_H
