#include "outlinepanel.h"

#include <QHBoxLayout>
#include <QPalette>
#include <QTextBlock>
#include <QTextDocument>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVector>
#include <QWidget>

QList<OutlineHeading> mdoutline::headingsOf(const QTextDocument *doc)
{
    QList<OutlineHeading> headings;
    if (!doc)
        return headings;

    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        const int level = block.blockFormat().headingLevel();
        if (level > 0)
            headings.append({level, block.text(), block.blockNumber()});
    }
    return headings;
}

void OutlinePanel::setLeftPadding(int px)
{
    m_layout->setContentsMargins(qMax(0, px), 0, 0, 0);
}

OutlinePanel::OutlinePanel(QWidget *parent)
    : QDockWidget(parent)
{
    setObjectName(QStringLiteral("outlinePanel"));  // para saveState/restoreState
    setWindowTitle(tr("Esquema"));

    m_tree = new QTreeWidget(this);
    m_tree->setHeaderHidden(true);
    m_tree->setUniformRowHeights(true);

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
    m_layout->addWidget(m_tree);
    setWidget(container);

    // Un clic en un encabezado lleva el cursor a su bloque (los ítems de relleno
    // no llevan número de bloque, así que no navegan).
    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const QVariant blockNumber = item->data(0, Qt::UserRole);
        if (blockNumber.isValid())
            emit headingActivated(blockNumber.toInt());
    });
}

void OutlinePanel::rebuild(const QTextDocument *doc)
{
    m_tree->clear();

    const QList<OutlineHeading> headings = mdoutline::headingsOf(doc);
    if (headings.isEmpty()) {
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
    for (const OutlineHeading &h : headings) {
        while (!stack.isEmpty() && stack.last().level >= h.level)
            stack.removeLast();

        QTreeWidgetItem *item = stack.isEmpty()
            ? new QTreeWidgetItem(m_tree)
            : new QTreeWidgetItem(stack.last().item);
        item->setText(0, h.text);
        item->setData(0, Qt::UserRole, h.blockNumber);

        stack.append({h.level, item});
    }
    m_tree->expandAll();
}
