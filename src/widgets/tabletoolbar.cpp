/// \file
/// \brief Implementación de la fila de herramientas de tabla.

#include "tabletoolbar.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QToolButton>

#include "formaticons.h"

TableToolbar::TableToolbar(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_ShowWithoutActivating);
    // Es chrome, no documento: pinta su propio fondo (el de la ventana) para
    // separarse visualmente del editor, que usa el color Base del tema.
    setAutoFillBackground(true);
    setAccessibleName(tr("Herramientas de tabla"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    struct Item {
        Op op;
        const char *tip;
    };
    // El texto va por tr() en el contexto TableToolbar (se traduce con el resto).
    const Item items[7] = {
        {RowInsert, QT_TR_NOOP("Insertar fila debajo")},
        {RowDelete, QT_TR_NOOP("Eliminar fila")},
        {ColInsert, QT_TR_NOOP("Insertar columna a la derecha")},
        {ColDelete, QT_TR_NOOP("Eliminar columna")},
        {AlignLeft, QT_TR_NOOP("Alinear la columna a la izquierda")},
        {AlignCenter, QT_TR_NOOP("Centrar la columna")},
        {AlignRight, QT_TR_NOOP("Alinear la columna a la derecha")},
    };

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 2, 4, 2);
    layout->setSpacing(1);
    for (int i = 0; i < 7; ++i) {
        auto *button = new QToolButton(this);
        button->setFocusPolicy(Qt::NoFocus);
        button->setAutoRaise(false);
        button->setToolTip(tr(items[i].tip));
        const Op op = items[i].op;
        connect(button, &QToolButton::clicked, this, [this, op] { emit operationRequested(op); });
        layout->addWidget(button);
        m_buttons[i] = button;
        // Separadores entre los tres grupos (filas | columnas | alineación).
        if (op == RowDelete || op == ColDelete) {
            auto *sep = new QFrame(this);
            sep->setFrameShape(QFrame::VLine);
            sep->setFrameShadow(QFrame::Sunken);
            layout->addWidget(sep);
        }
    }
    layout->addStretch();  // los botones se quedan a la izquierda, no se estiran
}

void TableToolbar::applyIcons(const QColor &color, int px, qreal dpr)
{
    using K = formaticons::TableIconKind;
    static const K kinds[7] = {K::RowInsert, K::RowDelete,  K::ColInsert, K::ColDelete,
                               K::AlignLeft, K::AlignCenter, K::AlignRight};
    for (int i = 0; i < 7; ++i) {
        m_buttons[i]->setIcon(formaticons::makeTableIcon(kinds[i], color, px, dpr));
        m_buttons[i]->setIconSize(QSize(px, px));
    }
    // Ya no se dimensiona a sí misma: la coloca el layout del EditorStack, al que
    // basta con avisarle de que su tamaño preferido ha cambiado.
    updateGeometry();
}
