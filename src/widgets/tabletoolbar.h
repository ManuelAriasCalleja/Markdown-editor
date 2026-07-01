#ifndef TABLETOOLBAR_H
#define TABLETOOLBAR_H

/// \file
/// \brief Barra flotante que aparece sobre una tabla con las operaciones más
///        habituales (insertar/eliminar fila y columna, alinear columna).

#include <QWidget>

class QToolButton;
class QColor;

/// Widget con botones de icono que se superpone al viewport, sobre la tabla en la que
/// está el cursor. No hace lógica: emite `operationRequested`; quien lo posee (el
/// EditorStack) lo enruta a TableController. Los botones no roban el foco del editor.
class TableToolbar : public QWidget
{
    Q_OBJECT

public:
    /// Operaciones ofrecidas, en el orden en que se muestran.
    enum Op { RowInsert, RowDelete, ColInsert, ColDelete, AlignLeft, AlignCenter, AlignRight };

    explicit TableToolbar(QWidget *parent = nullptr);

    /// Repinta los iconos con el color/tamaño dados (para seguir al tema y al zoom).
    void applyIcons(const QColor &color, int px, qreal dpr);

signals:
    void operationRequested(TableToolbar::Op op);

private:
    QToolButton *m_buttons[7] = {};
};

#endif  // TABLETOOLBAR_H
