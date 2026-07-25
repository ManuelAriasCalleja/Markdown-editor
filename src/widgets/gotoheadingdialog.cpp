/// \file
/// \brief Implementación del diálogo «quick open» de encabezados.

#include "gotoheadingdialog.h"

#include <QListWidget>

#include "outline.h"

GoToHeadingDialog::GoToHeadingDialog(const QList<OutlineHeading> &headings, QWidget *parent)
    : FilterListDialog(tr("Ir a encabezado"), tr("Filtrar encabezados…"),
                       tr("Encabezados"), parent)
{
    for (const OutlineHeading &h : headings) {
        auto *item = new QListWidgetItem(
            QString(qsizetype(h.level - 1) * 2, QLatin1Char(' ')) + h.text, list());
        item->setData(Qt::UserRole, h.blockNumber);
    }
    if (list()->count() > 0)
        list()->setCurrentRow(0);
    resize(360, 420);
}

// Filtro por subcadena: oculta lo que no casa y selecciona el primer visible.
void GoToHeadingDialog::filterChanged(const QString &query)
{
    int firstVisible = -1;
    for (int i = 0; i < list()->count(); ++i) {
        const bool match = list()->item(i)->text().contains(query, Qt::CaseInsensitive);
        list()->item(i)->setHidden(!match);
        if (match && firstVisible < 0)
            firstVisible = i;
    }
    if (firstVisible >= 0)
        list()->setCurrentRow(firstVisible);
}

int GoToHeadingDialog::selectedBlockNumber() const
{
    QListWidgetItem *item = list()->currentItem();
    if (!item || item->isHidden())
        return -1;
    return item->data(Qt::UserRole).toInt();
}
