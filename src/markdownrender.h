#ifndef MARKDOWNRENDER_H
#define MARKDOWNRENDER_H

#include <QString>

class QTextEdit;
class QTextDocument;

// Pipeline único de carga de Markdown en el editor WYSIWYG. Centraliza la
// secuencia que antes estaba duplicada en `DocumentIo::load`,
// `DocumentIo::loadFromString` y `MainWindow::setBodyMarkdown`:
//
//   1. proteger el texto fuente de las extensiones (fórmulas, notas al pie) antes
//      de `setMarkdown`, para que md4c no malinterprete `$`, `[^id]:`, etc.;
//   2. `setMarkdown`;
//   3. aplicar las pasadas de render que estilizan el documento (fórmulas, notas
//      al pie, admoniciones).
//
// Añadir una extensión ligera nueva = tocar SOLO este módulo, no los tres sitios.
namespace mdrender {

// Aplica a `markdown` los protectores de las extensiones, en el orden correcto,
// y devuelve el texto listo para `setMarkdown`. Función pura.
QString protect(const QString &markdown);

// Aplica las pasadas de render (fórmulas, notas al pie, admoniciones) sobre un
// documento ya cargado con `setMarkdown(protect(...))`. Idempotente.
void renderPasses(QTextDocument *doc);

// Conveniencia: `editor->setMarkdown(protect(markdown))` + `renderPasses`.
void setMarkdownWithExtensions(QTextEdit *editor, const QString &markdown);

}  // namespace mdrender

#endif  // MARKDOWNRENDER_H
