#include "outlinepanel.h"

#include <QDropEvent>
#include <QHBoxLayout>
#include <QPalette>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>
#include <QTreeWidgetItem>
#include <QVector>
#include <QWidget>

namespace {
// Rol de datos del ítem: ordinal del encabezado en orden de documento (para la
// reordenación). El número de bloque va en Qt::UserRole (para navegar).
constexpr int OrdinalRole = Qt::UserRole + 1;
}  // namespace

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

QString mdoutline::tableOfContentsMarkdown(const QList<OutlineHeading> &headings)
{
    QString md;
    QVector<int> stack;  // niveles de los ancestros vigentes (misma idea que rebuild)
    for (const OutlineHeading &h : headings) {
        while (!stack.isEmpty() && stack.last() >= h.level)
            stack.removeLast();
        const int depth = stack.size();  // 0 = raíz
        md += QString(qsizetype(depth) * 2, QLatin1Char(' '));
        md += QStringLiteral("- ") + h.text + QLatin1Char('\n');
        stack.append(h.level);
    }
    return md;
}

namespace {

// Encabezados ATX (`#`..`######` + espacio o fin de línea) a principio de línea,
// ignorando el interior de los bloques de código vallados (``` o ~~~). Replica la
// secuencia de headingsOf sobre el Markdown serializado para que los ordinales
// coincidan. Devuelve, por cada encabezado, su índice de línea y su nivel.
struct HeadingLine { int line; int level; };

QVector<HeadingLine> scanHeadings(const QStringList &lines)
{
    static const QRegularExpression fenceRe(QStringLiteral(R"(^\s{0,3}(`{3,}|~{3,}))"));
    static const QRegularExpression atxRe(QStringLiteral(R"(^(#{1,6})(?: |$))"));
    QVector<HeadingLine> out;
    bool inFence = false;
    QString fenceMarker;  // p. ej. "```": cierra con la misma valla
    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines.at(i);
        const QRegularExpressionMatch fm = fenceRe.match(line);
        if (fm.hasMatch()) {
            const QString marker = fm.captured(1);
            if (!inFence) {
                inFence = true;
                fenceMarker = marker.left(1);  // ` o ~
            } else if (marker.startsWith(fenceMarker)) {
                inFence = false;
            }
            continue;
        }
        if (inFence)
            continue;
        const QRegularExpressionMatch hm = atxRe.match(line);
        if (hm.hasMatch())
            out.append({i, int(hm.capturedLength(1))});
    }
    return out;
}

}  // namespace

QString mdoutline::moveSection(const QString &markdown, int fromOrdinal,
                               int toOrdinal, bool placeAfter)
{
    if (fromOrdinal == toOrdinal && !placeAfter)
        return markdown;
    QStringList lines = markdown.split(QLatin1Char('\n'));
    const QVector<HeadingLine> hs = scanHeadings(lines);
    if (fromOrdinal < 0 || fromOrdinal >= hs.size()
        || toOrdinal < 0 || toOrdinal >= hs.size())
        return markdown;

    // Fin (exclusivo, en líneas) de la sección del ordinal k: el siguiente
    // encabezado de nivel igual o menor, o el final del documento.
    const auto sectionEnd = [&](int k) {
        const int lvl = hs.at(k).level;
        for (int m = k + 1; m < hs.size(); ++m)
            if (hs.at(m).level <= lvl)
                return hs.at(m).line;
        return int(lines.size());
    };

    const int sStart = hs.at(fromOrdinal).line;
    const int sEnd = sectionEnd(fromOrdinal);

    // No mover una sección dentro de sí misma.
    const int destLine = hs.at(toOrdinal).line;
    if (destLine >= sStart && destLine < sEnd)
        return markdown;

    int insertLine = placeAfter ? sectionEnd(toOrdinal) : destLine;

    const QStringList moved = lines.mid(sStart, sEnd - sStart);
    lines.erase(lines.begin() + sStart, lines.begin() + sEnd);
    if (insertLine >= sEnd)
        insertLine -= (sEnd - sStart);  // las líneas tras el hueco se desplazaron

    for (int j = 0; j < moved.size(); ++j)
        lines.insert(insertLine + j, moved.at(j));

    return lines.join(QLatin1Char('\n'));
}

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
    m_layout->addWidget(m_tree);
    setWidget(container);

    // Un clic en un encabezado lleva el cursor a su bloque (los ítems de relleno
    // no llevan número de bloque, así que no navegan).
    connect(m_tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *item) {
        const QVariant blockNumber = item->data(0, Qt::UserRole);
        if (blockNumber.isValid())
            emit headingActivated(blockNumber.toInt());
    });
    // Reenvía la petición de mover sección que origina el arrastre en el árbol.
    connect(m_tree, &OutlineTree::sectionMoveRequested,
            this, &OutlinePanel::sectionMoveRequested);
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
    int ordinal = 0;
    for (const OutlineHeading &h : headings) {
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
    m_tree->expandAll();
}
