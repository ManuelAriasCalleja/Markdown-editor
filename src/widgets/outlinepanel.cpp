/// \file
/// \brief Implementación del panel del índice y de las funciones puras de `mdoutline`.

#include "outlinepanel.h"

#include <algorithm>

#include <QDropEvent>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPalette>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <QToolButton>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

namespace {
// Rol de datos del ítem: ordinal del encabezado en orden de documento (para la
// reordenación). El número de bloque va en Qt::UserRole (para navegar).
constexpr int OrdinalRole = Qt::UserRole + 1;
}  // namespace


OutlineTree::OutlineTree(QWidget *parent)
    : QTreeWidget(parent)
{
    setHeaderHidden(true);
    setUniformRowHeights(true);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setDropIndicatorShown(true);
}

void OutlineTree::dropEvent(QDropEvent *event)
{
    // No dejamos que el árbol reordene sus ítems: traducimos el drop a un
    // movimiento de sección en el documento y la ventana reconstruye el árbol.
    event->setDropAction(Qt::IgnoreAction);

    QTreeWidgetItem *src = currentItem();
    if (!src) {
        event->ignore();
        return;
    }
    const QVariant srcOrd = src->data(0, OrdinalRole);
    if (!srcOrd.isValid()) {
        event->ignore();
        return;
    }

    const QPoint pos = event->position().toPoint();
    QTreeWidgetItem *target = itemAt(pos);
    int toOrdinal = -1;
    bool placeAfter = true;
    if (!target) {
        // Soltado en zona vacía: al final del documento, es decir, tras la
        // sección de la última entrada de nivel superior (placeAfter la lleva
        // hasta el final porque incluye sus subsecciones).
        if (topLevelItemCount() == 0) {
            event->ignore();
            return;
        }
        const QVariant lastOrd =
            topLevelItem(topLevelItemCount() - 1)->data(0, OrdinalRole);
        if (!lastOrd.isValid()) {
            event->ignore();
            return;
        }
        toOrdinal = lastOrd.toInt();
        placeAfter = true;
    } else {
        const QVariant tOrd = target->data(0, OrdinalRole);
        if (!tOrd.isValid()) {
            event->ignore();
            return;
        }
        toOrdinal = tOrd.toInt();
        switch (dropIndicatorPosition()) {
        case QAbstractItemView::AboveItem: placeAfter = false; break;
        case QAbstractItemView::BelowItem:
        case QAbstractItemView::OnItem:    placeAfter = true;  break;  // sin reanidar
        default:                           placeAfter = true;  break;
        }
    }
    event->accept();
    emit sectionMoveRequested(srcOrd.toInt(), toOrdinal, placeAfter);
}

void OutlinePanel::setLeftPadding(int px)
{
    m_layout->setContentsMargins(qMax(0, px), 0, 0, 0);
}

void OutlinePanel::focusTree()
{
    // Si no hay entrada seleccionada, elige la primera navegable (con número de
    // bloque): así las flechas tienen un punto de partida y Enter activa algo.
    if (!m_tree->currentItem()) {
        for (QTreeWidgetItemIterator it(m_tree); *it; ++it) {
            if ((*it)->data(0, Qt::UserRole).isValid()) {
                m_tree->setCurrentItem(*it);
                break;
            }
        }
    }
    m_tree->setFocus(Qt::TabFocusReason);
}

bool OutlinePanel::treeHasFocus() const
{
    return m_tree->hasFocus();
}

OutlinePanel::OutlinePanel(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QStringLiteral("outlinePanel"));  // para saveState/restoreState
    setWindowTitle(tr("Esquema"));

    m_tree = new OutlineTree(this);
    m_tree->setAccessibleName(tr("Esquema"));  // el dock ya se llama así; lo hereda el árbol al enfocarlo
    m_tree->setAccessibleDescription(
        tr("Encabezados del documento; actívalos para saltar a esa sección."));

    // El árbol vive en un contenedor con un relleno izquierdo opcional (ver
    // setLeftPadding), pintado en negro puro igual que las franjas del modo sin
    // distracciones (FocusEditor), para que el bloque esquema+columna se vea
    // centrado sobre fondo negro.
    auto *container = new QWidget(this);
    container->setAutoFillBackground(true);
    QPalette pal = container->palette();
    pal.setColor(QPalette::Window, Qt::black);  // igual que FocusEditor
    container->setPalette(pal);
    m_layout = new QHBoxLayout(container);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    // Columna: [fila de filtro][árbol]. El relleno izquierdo (setLeftPadding) lo
    // sigue aplicando m_layout como margen izquierdo de toda la columna.
    auto *column = new QVBoxLayout;
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    // Fila de filtro: campo de búsqueda + botones «Expandir/Plegar todo».
    auto *filterRow = new QHBoxLayout;
    filterRow->setContentsMargins(0, 0, 0, 0);
    filterRow->setSpacing(0);
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Filtrar encabezados…"));
    m_filter->setAccessibleName(m_filter->placeholderText());
    m_filter->setClearButtonEnabled(true);
    auto *expandBtn = new QToolButton(this);
    expandBtn->setText(QStringLiteral("⊞"));
    expandBtn->setToolTip(tr("Expandir todo"));
    auto *collapseBtn = new QToolButton(this);
    collapseBtn->setText(QStringLiteral("⊟"));
    collapseBtn->setToolTip(tr("Plegar todo"));
    filterRow->addWidget(m_filter);
    filterRow->addWidget(expandBtn);
    filterRow->addWidget(collapseBtn);
    column->addLayout(filterRow);
    column->addWidget(m_tree);
    m_layout->addLayout(column);
    setWidget(container);

    // Filtrar en vivo: recalcula visibilidad al teclear (reusa applyViewState).
    connect(m_filter, &QLineEdit::textChanged, this, [this] { applyViewState(); });
    // Expandir/plegar todo: fija el conjunto plegado y quita el filtro para que el
    // efecto se vea sobre el árbol completo.
    connect(expandBtn, &QToolButton::clicked, this, [this] {
        m_collapsed.clear();
        QSignalBlocker block(m_filter);
        m_filter->clear();
        applyViewState();
    });
    connect(collapseBtn, &QToolButton::clicked, this, [this] {
        m_collapsed.clear();
        for (const OutlineHeading &h : m_headings)
            m_collapsed.insert(h.text);
        QSignalBlocker block(m_filter);
        m_filter->clear();
        applyViewState();
    });

    // Un clic (o Enter con el teclado) en un encabezado lleva el cursor a su bloque
    // (los ítems de relleno no llevan número de bloque, así que no navegan).
    const auto activate = [this](QTreeWidgetItem *item) {
        const QVariant blockNumber = item->data(0, Qt::UserRole);
        if (blockNumber.isValid())
            emit headingActivated(blockNumber.toInt());
    };
    connect(m_tree, &QTreeWidget::itemClicked, this, activate);
    connect(m_tree, &QTreeWidget::itemActivated, this, activate);
    // Reenvía la petición de mover sección que origina el arrastre en el árbol.
    connect(m_tree, &OutlineTree::sectionMoveRequested,
            this, &OutlinePanel::sectionMoveRequested);

    // Recuerda qué ramas pliega/expande el usuario, para reaplicarlas tras cada
    // reconstrucción (R7). El guard evita registrar los cambios que hace el propio
    // applyViewState al reaplicar el estado.
    connect(m_tree, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem *item) {
        if (!m_applyingState)
            m_collapsed.insert(item->text(0));
    });
    connect(m_tree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem *item) {
        if (!m_applyingState)
            m_collapsed.remove(item->text(0));
    });
}

void OutlinePanel::rebuild(const QTextDocument *doc)
{
    // MODELO: se reconstruye el árbol a partir del documento. El ESTADO DE VISTA
    // (plegado, filtro) lo reaplica applyViewState al final, no aquí (R7).
    m_tree->clear();
    m_headings = mdoutline::headingsOf(doc);
    if (m_headings.isEmpty()) {
        // Relleno cuando no hay encabezados: ítem deshabilitado, no navegable.
        auto *placeholder = new QTreeWidgetItem(m_tree, {tr("Sin encabezados")});
        placeholder->setFlags(Qt::NoItemFlags);
        return;
    }

    // Construye el árbol respetando el anidamiento por nivel. La pila guarda el
    // ancestro vigente: al llegar un encabezado se descartan los de nivel igual
    // o mayor y el nuevo se cuelga del que quede (o de la raíz si no queda).
    // Tolera saltos de nivel (p. ej. H1 seguido de H3).
    struct Node { int level; QTreeWidgetItem *item; };
    QVector<Node> stack;
    int ordinal = 0;
    for (const OutlineHeading &h : m_headings) {
        while (!stack.isEmpty() && stack.last().level >= h.level)
            stack.removeLast();

        QTreeWidgetItem *item = stack.isEmpty()
            ? new QTreeWidgetItem(m_tree)
            : new QTreeWidgetItem(stack.last().item);
        item->setText(0, h.text);
        item->setData(0, Qt::UserRole, h.blockNumber);
        // Ordinal en orden de documento, para la reordenación por arrastre.
        item->setData(0, OrdinalRole, ordinal++);

        stack.append({h.level, item});
    }
    applyViewState();
}

void OutlinePanel::applyViewState()
{
    if (m_headings.isEmpty())
        return;
    // El guard evita que estos cambios programáticos de plegado se registren en
    // m_collapsed (que solo debe reflejar lo que hace el usuario).
    m_applyingState = true;
    const QString filter = m_filter ? m_filter->text().trimmed() : QString();
    if (filter.isEmpty()) {
        // Sin filtro: todo visible, con el plegado recordado por el usuario.
        for (QTreeWidgetItemIterator it(m_tree); *it; ++it) {
            (*it)->setHidden(false);
            (*it)->setExpanded(!m_collapsed.contains((*it)->text(0)));
        }
    } else {
        // Con filtro: solo las coincidencias y sus ancestros, todo expandido para
        // que las coincidencias se vean (se ignora el plegado mientras se filtra).
        const QSet<int> visible = mdoutline::visibleOrdinals(m_headings, filter);
        for (QTreeWidgetItemIterator it(m_tree); *it; ++it) {
            const bool v = visible.contains((*it)->data(0, OrdinalRole).toInt());
            (*it)->setHidden(!v);
            if (v)
                (*it)->setExpanded(true);
        }
    }
    m_applyingState = false;
}
