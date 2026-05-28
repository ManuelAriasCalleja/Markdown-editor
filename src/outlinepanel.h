#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

#include <QDockWidget>
#include <QList>
#include <QString>

class QTextDocument;
class QTreeWidget;
class QHBoxLayout;

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
}

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

private:
    QTreeWidget *m_tree;
    QHBoxLayout *m_layout;  // contenedor: [relleno izquierdo][árbol]
};

#endif // OUTLINEPANEL_H
