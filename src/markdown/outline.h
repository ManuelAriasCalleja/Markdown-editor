#ifndef OUTLINE_H
#define OUTLINE_H

/// \file
/// \brief Extracción y reordenación puras de los encabezados de un documento
///        (`mdoutline`). Vivía en `outlinepanel.h`, que obligaba a arrastrar un
///        `QDockWidget` a quien solo quería el índice (la exportación a EPUB, por
///        ejemplo).

#include <QList>
#include <QSet>
#include <QString>

class QTextDocument;

/// \brief Un encabezado del documento, para el índice (TOC).
struct OutlineHeading {
    int level;        ///< nivel Markdown (1..6)
    QString text;     ///< texto del encabezado
    int blockNumber;  ///< bloque del documento donde está (para navegar hasta él)
};

/// \brief Extracción y reordenación puras (sin GUI) de los encabezados de un documento.
namespace mdoutline {
/// \brief Recorre los bloques del documento y recoge los que tienen headingLevel() > 0.
QList<OutlineHeading> headingsOf(const QTextDocument *doc);

/// \brief Ordinales (0..N-1, orden de documento) de los encabezados VISIBLES bajo
/// `filter`: los que contienen el texto (sin distinguir mayúsculas) y, para que el
/// árbol siga teniendo sentido, todos sus ancestros. Un `filter` vacío devuelve
/// todos. Pura (sin GUI), testeable.
QSet<int> visibleOrdinals(const QList<OutlineHeading> &headings, const QString &filter);

/// \brief Nuevo nivel al promover/degradar un encabezado: `current` (1..6) más
/// `delta` (−1 promover hacia H1, +1 degradar hacia H6), acotado a [1,6]. Si
/// `current` no es un encabezado (< 1), devuelve 0 (sin cambio). Pura, testeable.
int shiftedLevel(int current, int delta);

/// \brief Genera el Markdown de un índice (TOC) a partir de una lista de encabezados:
/// una lista con viñetas anidada por nivel. La profundidad se calcula como en el
/// árbol del panel (una pila de ancestros), de modo que los saltos de nivel
/// (p. ej. H1 seguido de H3) se compactan en vez de dejar sangrías vacías. Una
/// lista vacía produce una cadena vacía. Es pura (sin GUI) para poder probarla.
QString tableOfContentsMarkdown(const QList<OutlineHeading> &headings);

/// \brief Reordena secciones en el Markdown: mueve la sección del encabezado `fromOrdinal`
/// (su encabezado más todo su contenido y subsecciones, hasta el siguiente
/// encabezado de nivel igual o menor) junto a la del encabezado `toOrdinal`. Los
/// ordinales son la posición del encabezado en orden de documento (0..N-1, como en
/// el árbol). `placeAfter` la coloca tras la sección destino; si no, antes. No
/// cambia niveles (no reanida). Devuelve el Markdown sin cambios si los ordinales
/// son inválidos, iguales, o el destino cae dentro de la propia sección origen.
/// Detecta encabezados ATX a principio de línea ignorando los bloques de código
/// vallados (``` o ~~~), igual que headingsOf, para que los ordinales coincidan.
/// Es pura (sin GUI) para poder probarla.
QString moveSection(const QString &markdown, int fromOrdinal, int toOrdinal,
                    bool placeAfter);
}

#endif  // OUTLINE_H
