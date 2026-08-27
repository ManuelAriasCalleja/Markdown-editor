#ifndef TABLETOOLBAR_H
#define TABLETOOLBAR_H

/// \file
/// \brief Fila de botones que aparece sobre el documento cuando el cursor está en una
///        tabla (insertar/eliminar fila y columna, alinear columna).

#include <QWidget>

class QToolButton;
class QColor;

/// Widget con botones de icono que se muestra como una FILA propia encima del editor
/// (no flotando sobre él). Flotaba pegada a la tabla, y ahí tapaba la última línea del
/// párrafo anterior: los iconos son opacos y el texto de debajo no se podía leer. Como
/// fila ocupa su propio espacio y no tapa nada; a cambio, el documento se desplaza al
/// aparecer. No hace lógica: emite `operationRequested`; quien lo posee (el
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
