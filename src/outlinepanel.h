#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QDockWidget>
#include <QList>
#include <QString>
#include <QTreeWidget>

class QTextDocument;
class QHBoxLayout;
class QDropEvent;

// Un encabezado del documento, para el índice (TOC).
struct OutlineHeading {
    int level;        // nivel Markdown (1..6)
    QString text;     // texto del encabezado
    int blockNumber;  // bloque del documento donde está (para navegar hasta él)
};

// Extracción pura (sin GUI) de los encabezados de un documento: recorre sus
// bloques y recoge los que tienen headingLevel() > 0. Se expone aparte para
// poder probarla de forma aislada (igual que mdblock en blockconstructs).
namespace mdoutline {
QList<OutlineHeading> headingsOf(const QTextDocument *doc);

// Genera el Markdown de un índice (TOC) a partir de una lista de encabezados:
// una lista con viñetas anidada por nivel. La profundidad se calcula como en el
// árbol del panel (una pila de ancestros), de modo que los saltos de nivel
// (p. ej. H1 seguido de H3) se compactan en vez de dejar sangrías vacías. Una
// lista vacía produce una cadena vacía. Es pura (sin GUI) para poder probarla.
QString tableOfContentsMarkdown(const QList<OutlineHeading> &headings);

// Reordena secciones en el Markdown: mueve la sección del encabezado `fromOrdinal`
// (su encabezado más todo su contenido y subsecciones, hasta el siguiente
// encabezado de nivel igual o menor) junto a la del encabezado `toOrdinal`. Los
// ordinales son la posición del encabezado en orden de documento (0..N-1, como en
// el árbol). `placeAfter` la coloca tras la sección destino; si no, antes. No
// cambia niveles (no reanida). Devuelve el Markdown sin cambios si los ordinales
// son inválidos, iguales, o el destino cae dentro de la propia sección origen.
// Detecta encabezados ATX a principio de línea ignorando los bloques de código
// vallados (``` o ~~~), igual que headingsOf, para que los ordinales coincidan.
// Es pura (sin GUI) para poder probarla.
QString moveSection(const QString &markdown, int fromOrdinal, int toOrdinal,
                    bool placeAfter);
}

// Árbol del índice con reordenación por arrastre. Captura el drop para mover la
// sección en el documento (no deja que el QTreeWidget reordene sus ítems: la
// ventana regenera el documento y reconstruye el árbol). Emite la petición con
// los ordinales de origen y destino y si va antes o después.
class OutlineTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit OutlineTree(QWidget *parent = nullptr);

signals:
    void sectionMoveRequested(int fromOrdinal, int toOrdinal, bool placeAfter);

protected:
    void dropEvent(QDropEvent *event) override;
};

// Panel lateral acoplable con el índice (TOC) de encabezados del documento,
// mostrados como un árbol que refleja su anidamiento (H1 ▸ H2 ▸ H3). Al activar
// una entrada emite headingActivated() con el número de bloque, para que la
// vista lleve el cursor allí. No conoce el editor: solo recibe el documento a
// reconstruir y avisa por señal, de modo que no se acopla a la ventana.
class OutlinePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit OutlinePanel(QWidget *parent = nullptr);

    // Reconstruye el árbol a partir del documento. Un documento nulo o sin
    // encabezados muestra un texto de relleno no navegable.
    void rebuild(const QTextDocument *doc);

    // Relleno (en píxeles) a la izquierda del árbol, pintado del color de las
    // franjas del modo sin distracciones. Sirve para centrar el bloque
    // esquema+columna en pantalla en ese modo (el dock se ensancha y el árbol
    // queda pegado a la columna de texto). 0 = sin relleno (modo normal).
    void setLeftPadding(int px);

signals:
    void headingActivated(int blockNumber);
    // Reenvía la petición del árbol de mover una sección (ver OutlineTree).
    void sectionMoveRequested(int fromOrdinal, int toOrdinal, bool placeAfter);

private:
    OutlineTree *m_tree;
    QHBoxLayout *m_layout;  // contenedor: [relleno izquierdo][árbol]
};

#endif // OUTLINEPANEL_H
