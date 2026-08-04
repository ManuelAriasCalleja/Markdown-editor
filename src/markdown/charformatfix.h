#ifndef CHARFORMATFIX_H
#define CHARFORMATFIX_H

/// \file
/// \brief Repara el formato de carácter que deja mal el importador de Markdown de
///        Qt: encabezados que pierden su tamaño/negrita tras un span, y runs de
///        código con el tamaño de fuente clavado en absoluto.

class QTextDocument;

/// Arreglos del formato de carácter que `QTextDocument::setMarkdown` deja mal. No
/// son preferencias de estilo: son dos defectos del importador de Qt que se ven en
/// pantalla y se arrastran a las exportaciones. Se aplican al cargar (pasada de
/// `mdrender::renderPasses`), así que **no tocan el Markdown**: solo reponen el
/// formato que el propio Qt habría puesto. Idempotentes y sin GUI, con
/// `tst_charformatfix`.
namespace mdcharfix {

/// Repone en TODOS los fragmentos de un encabezado el tamaño (`FontSizeAdjustment`
/// = `4 - nivel`) y la negrita que le corresponden por su nivel.
///
/// El importador de Qt solo se los pone al PRIMER fragmento del bloque: en cuanto
/// aparece un span en línea (código, negrita, cursiva, un enlace), ese span y
/// **todo lo que va detrás** se quedan con el formato base del documento. Un
/// `## ni los \`module-info\` las vigilan` se veía con «module-info» en cuerpo de
/// texto y «las vigilan» ni grande ni en negrita, como si el encabezado se cortara
/// a media línea. Se deriva del nivel del bloque (no de otro fragmento) porque un
/// encabezado puede EMPEZAR por un span, en cuyo caso no queda ninguno bien.
void repairHeadingRuns(QTextDocument *doc);

/// Quita el tamaño de fuente ABSOLUTO que Qt clava en los runs de código (spans en
/// línea y bloques vallados), para que hereden el del documento.
///
/// `setMarkdown` les fija `FontPointSize` = el tamaño por defecto AL IMPORTAR. Como
/// el zoom del editor cambia la fuente del documento (`editor->setFont`), el código
/// se quedaba anclado al tamaño de carga mientras la prosa crecía o menguaba a su
/// alrededor; y dentro de un encabezado, clavado al cuerpo de texto. La familia
/// monospace y el `fontFixedPitch` —que son lo que reconocen el resaltado, el
/// corrector y la serialización a backticks— no se tocan. Alcanza a los tres
/// sabores: span en línea, bloque vallado y bloque indentado (Qt marca el span con
/// `fontFixedPitch`, pero los bloques solo con la familia y `BlockCodeLanguage`).
void unpinCodeFontSize(QTextDocument *doc);

/// Conveniencia: ambos arreglos, en orden.
void apply(QTextDocument *doc);

}  // namespace mdcharfix

#endif  // CHARFORMATFIX_H
