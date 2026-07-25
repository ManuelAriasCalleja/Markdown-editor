#ifndef MARKDOWNRENDER_H
#define MARKDOWNRENDER_H

/// \file
/// \brief Pipeline único de carga de Markdown: proteger → `setMarkdown` →
///        pasadas de render de las extensiones ligeras.

#include <QString>
#include <QTextDocument>

class QTextEdit;

/// Pipeline único de carga de Markdown en el editor WYSIWYG. Centraliza la
/// secuencia que antes estaba duplicada en `DocumentIo::load`,
/// `DocumentIo::loadFromString` y `MainWindow::setBodyMarkdown`:
///
///   1. proteger el texto fuente de las extensiones (fórmulas, notas al pie) antes
///      de `setMarkdown`, para que md4c no malinterprete `$`, `[^id]:`, etc.;
///   2. `setMarkdown`;
///   3. aplicar las pasadas de render que estilizan el documento (fórmulas, notas
///      al pie, admoniciones).
///
/// Añadir una extensión ligera nueva = tocar SOLO este módulo, no los tres sitios.
namespace mdrender {

/// Dialecto Markdown único del editor, en carga (`setMarkdown`) y guardado
/// (`toMarkdown`, en `mdtable::documentMarkdown`). GitHub + **NoHTML**: sin NoHTML,
/// Qt trata `<algo>` como HTML en línea y se **traga ese texto y el de alrededor**
/// al cargar (pérdida de datos). Con NoHTML los `<...>` son texto literal —lo
/// correcto en un editor WYSIWYG que no ejecuta HTML— y el round-trip converge.
inline constexpr QTextDocument::MarkdownFeatures kMarkdownFeatures =
    QTextDocument::MarkdownFeatures(QTextDocument::MarkdownDialectGitHub
                                    | QTextDocument::MarkdownNoHTML);

/// Aplica a `markdown` los protectores de las extensiones, en el orden correcto,
/// y devuelve el texto listo para `setMarkdown`. Función pura.
QString protect(const QString &markdown);

/// Aplica las pasadas de render (fórmulas, notas al pie, admoniciones) sobre un
/// documento ya cargado con `setMarkdown(protect(...))`. Idempotente.
void renderPasses(QTextDocument *doc);

/// Conveniencia: `editor->setMarkdown(protect(markdown))` + `renderPasses`.
void setMarkdownWithExtensions(QTextEdit *editor, const QString &markdown);

/// Texto alternativo de reserva para una imagen que no trae ninguno: el nombre del
/// fichero del destino (sin carpeta ni extensión), que al menos dice qué falta si
/// la ruta se rompe. Admite el destino envuelto en `<...>`. Nunca devuelve "".
/// Función pura.
QString imageAltFallback(const QString &destination);

/// Sintaxis Markdown de una imagen (`![alt](destino)`) lista para insertar en el
/// editor. Dos reglas que no son opcionales, las dos porque si no la imagen SE
/// PIERDE, y por eso viven aquí, junto al resto del contrato de `setMarkdown`:
///   - el texto alternativo no puede quedar vacío: `QTextDocument::setMarkdown`
///     **descarta** `![](ruta)` sin insertar nada —ni la imagen ni un hueco—, así
///     que desaparecería al volver a abrir el documento. Vacío → `imageAltFallback`;
///   - un espacio o un paréntesis en la ruta corta el enlace en seco: para esos,
///     CommonMark admite envolver el destino en `<...>` (y Qt lo entiende).
/// Función pura.
QString imageMarkdown(const QString &destination, const QString &alt = QString());

}  // namespace mdrender

#endif  // MARKDOWNRENDER_H
