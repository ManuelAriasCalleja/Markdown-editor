#ifndef DIAGRAMDOC_H
#define DIAGRAMDOC_H

#include <QTextFormat>

class QTextDocument;

// Integración de los diagramas en el QTextDocument: las imágenes de
// previsualización (opción «imagen bajo el bloque») viven en bloques propios
// MARCADOS como presentación, para poder quitarlas antes de serializar. Así el
// round-trip Markdown no las ve (ni `isModified`, ni la vista de fuente, ni el
// guardado), porque todo pasa por `mdtable::documentMarkdown`, que llama aquí.
namespace mddiagram {

// Propiedades del QTextBlockFormat del bloque de previsualización:
//   PreviewBlockProperty  (bool)   — marca el bloque como preview (presentación).
//   PreviewSourceProperty (QString)— hash de la fuente que representa, para saber
//                                     si la imagen está al día sin re-renderizar.
constexpr int PreviewBlockProperty  = QTextFormat::UserProperty + 30;
constexpr int PreviewSourceProperty = QTextFormat::UserProperty + 31;

// Borra del documento todos los bloques de previsualización. Lo llama
// `documentMarkdown` sobre su clon antes de `toMarkdown`, así que las imágenes de
// diagrama nunca llegan al Markdown.
void removePreviewBlocks(QTextDocument *doc);

} // namespace mddiagram

#endif // DIAGRAMDOC_H
