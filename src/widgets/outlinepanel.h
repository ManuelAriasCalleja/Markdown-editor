#ifndef OUTLINEPANEL_H
#define OUTLINEPANEL_H

/// \file
/// \brief Panel del índice (TOC). La lógica pura de encabezados está en `outline.h`.

#include "outline.h"

#include <QDockWidget>
#include <QList>
#include <QString>
#include <QTreeWidget>

class QTextDocument;
class QHBoxLayout;
class QDropEvent;
class QLineEdit;

/// \brief Árbol del índice con reordenación por arrastre. Captura el drop para mover la
/// sección en el documento (no deja que el QTreeWidget reordene sus ítems: la
/// ventana regenera el documento y reconstruye el árbol). Emite la petición con
/// los ordinales de origen y destino y si va antes o después.
class OutlineTree : public QTreeWidget
{
    Q_OBJECT
public:
    explicit OutlineTree(QWidget *parent = nullptr);

signals:
    /// \brief Petición de mover una sección desde `fromOrdinal` junto a `toOrdinal`
    /// (después si `placeAfter`, antes si no).
    void sectionMoveRequested(int fromOrdinal, int toOrdinal, bool placeAfter);

protected:
    void dropEvent(QDropEvent *event) override;
};

/// \brief Panel lateral acoplable con el índice (TOC) de encabezados del documento,
/// mostrados como un árbol que refleja su anidamiento (H1 ▸ H2 ▸ H3). Al activar
/// una entrada emite headingActivated() con el número de bloque, para que la
/// vista lleve el cursor allí. No conoce el editor: solo recibe el documento a
/// reconstruir y avisa por señal, de modo que no se acopla a la ventana.
class OutlinePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit OutlinePanel(QWidget *parent = nullptr);

    /// \brief Reconstruye el árbol a partir del documento. Un documento nulo o sin
    /// encabezados muestra un texto de relleno no navegable.
    void rebuild(const QTextDocument *doc);

    /// \brief Relleno (en píxeles) a la izquierda del árbol, pintado del color de las
    /// franjas del modo sin distracciones. Sirve para centrar el bloque
    /// esquema+columna en pantalla en ese modo (el dock se ensancha y el árbol
    /// queda pegado a la columna de texto). 0 = sin relleno (modo normal).
    void setLeftPadding(int px);

    /// \brief Pone el foco de teclado en el árbol del índice; si no hay entrada
    /// seleccionada, selecciona la primera navegable. Para saltar al esquema desde
    /// el editor con el teclado (las flechas recorren, Enter activa la entrada).
    void focusTree();

    /// \brief Indica si el árbol del índice tiene el foco de teclado (para alternar
    /// el foco esquema↔editor con el mismo atajo).
    bool treeHasFocus() const;

signals:
    /// \brief Emite el número de bloque del encabezado activado, para navegar hasta él.
    void headingActivated(int blockNumber);
    /// \brief Reenvía la petición del árbol de mover una sección (ver OutlineTree).
    void sectionMoveRequested(int fromOrdinal, int toOrdinal, bool placeAfter);

private:
    friend class TestOutlinePanel;

    // Aplica al árbol YA CONSTRUIDO el estado de vista: el plegado recordado
    // (m_collapsed) y, si hay filtro, oculta lo que no coincide (ni es ancestro).
    // Separa el modelo (rebuild) del estado de vista (R7), para que el plegado
    // sobreviva a las reconstrucciones (antes rebuild hacía expandAll incondicional).
    void applyViewState();

    OutlineTree *m_tree = nullptr;
    QLineEdit *m_filter = nullptr;
    QHBoxLayout *m_layout = nullptr;  // contenedor: [relleno izquierdo][columna]
    QList<OutlineHeading> m_headings;  // encabezados actuales (para filtrar sin reconstruir)
    QSet<QString> m_collapsed;  // texto de las ramas plegadas (persiste entre reconstrucciones)
    bool m_applyingState = false;  // guard: no registrar los cambios de plegado programáticos
};

#endif // OUTLINEPANEL_H
