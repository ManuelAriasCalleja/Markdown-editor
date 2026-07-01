#ifndef HTMLIMPORT_H
#define HTMLIMPORT_H

/// \file
/// \brief Importación de páginas HTML: decodificación de los bytes a texto,
///        respetando el juego de caracteres declarado. La conversión a Markdown la
///        hace mdrichpaste::htmlToMarkdown sobre el texto ya decodificado.

#include <QString>

class QByteArray;

namespace mdimport {

/// Juego de caracteres declarado en la cabecera del HTML (`<meta charset=…>` o
/// `<meta http-equiv="Content-Type" content="…; charset=…">`), en minúsculas, o
/// cadena vacía si no se declara ninguno. Solo mira el principio del contenido (la
/// cabecera). Función pura (sin GUI, testeable aislada).
QString charsetOf(const QByteArray &bytes);

/// Decodifica los bytes de un HTML a texto. Prioridad: BOM › `<meta charset>` ›
/// UTF-8 por defecto. Los juegos que Qt no sabe decodificar (p. ej. windows-1252)
/// caen a UTF-8. Función pura.
QString decodeHtml(const QByteArray &bytes);

}  // namespace mdimport

#endif  // HTMLIMPORT_H
