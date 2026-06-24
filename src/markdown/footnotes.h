#ifndef FOOTNOTES_H
#define FOOTNOTES_H

/// \file
/// \brief Notas al pie de Markdown: localización de referencias/definiciones y
///        estilo de superíndice de las referencias en el documento.

#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QTextFormat>

class QTextDocument;

/// Notas al pie de Markdown: referencias en el texto (`[^id]`) y definiciones a
/// nivel de bloque (`[^id]: texto`). Qt no entiende esta extensión, pero —y esto
/// es clave— tampoco la estorba: `setMarkdown`/`toMarkdown` dejan `[^id]` y
/// `[^id]:` como texto literal sin escaparlos ni convertirlos en enlaces, así que
/// el round-trip ya funciona sin protección ni centinelas (a diferencia de las
/// fórmulas). Por eso este módulo NO toca la serialización: solo
///   1) localiza referencias/definiciones (funciones puras, testeables), y
///   2) da estilo a las referencias como superíndice en el documento (un formato
///      de carácter que Markdown ignora al serializar, luego no rompe el
///      round-trip), marcándolas con propiedades para poder navegar a su
///      definición con un clic.
namespace mdfootnote {

/// Propiedad de carácter que marca una referencia `[^id]` ya renderizada.
constexpr int IsFootnoteRefProperty = QTextFormat::UserProperty + 10;
/// Propiedad de carácter con el identificador de la referencia renderizada.
constexpr int FootnoteIdProperty    = QTextFormat::UserProperty + 11;

/// Una referencia o definición localizada en un texto Markdown.
struct Footnote {
    int start;      ///< posición del `[` inicial
    int length;     ///< longitud del token (referencia: `[^id]`; definición: `[^id]:`)
    QString id;     ///< identificador entre `[^` y `]`
};

/// Referencias `[^id]` del texto (las que NO van seguidas de `:`, que serían el
/// rótulo de una definición). Posiciones relativas a `text`.
QList<Footnote> references(const QString &text);

/// Definiciones `[^id]:` al principio de línea (admite hasta 3 espacios de
/// sangría). Posiciones relativas a `text`.
QList<Footnote> definitions(const QString &text);

/// Protege las definiciones antes de setMarkdown(). Una línea `[^id]: destino`
/// con un destino de una sola palabra (p. ej. `[^1]: Ibíd.`) la tomaría md4c por
/// una *definición de enlace de referencia* (`[label]: url`) y se la comería,
/// rompiendo el round-trip; con espacios no pasa. Para evitarlo de forma uniforme,
/// sustituye el `:` que sigue al rótulo por un centinela de la PUA, que setMarkdown
/// deja pasar como texto. renderFootnotesInDocument() lo restaura a `:` en el
/// documento, así que el centinela nunca llega ni a la pantalla ni al disco.
QString protectFootnotes(const QString &markdown);

/// Siguiente identificador numérico libre como cadena ("1", "2"…): el mayor id
/// numérico existente + 1, o "1" si no hay ninguno. Ignora los ids no numéricos.
QString nextId(const QString &markdown);

/// Formato a fusionar (mergeCharFormat) sobre una referencia para renderizarla
/// como superíndice y marcarla con sus propiedades. Solo fija el superíndice y las
/// propiedades: al fusionarse conserva la fuente/!color del texto alrededor.
QTextCharFormat refFormat(const QString &id);

/// Recorre el documento y da estilo de superíndice a las referencias `[^id]` que
/// no estén dentro de código ni de una fórmula. Idempotente. Engancha tras
/// renderMathInDocument en la carga y en setBodyMarkdown.
void renderFootnotesInDocument(QTextDocument *doc);

/// Número de bloque cuya definición es `[^id]:`, o -1 si no existe. Para saltar de
/// una referencia a su definición.
int definitionBlockNumber(const QTextDocument *doc, const QString &id);

}  // namespace mdfootnote

#endif  // FOOTNOTES_H
