#include "gotoheadingdialog.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

#include "outlinepanel.h"

GoToHeadingDialog::GoToHeadingDialog(const QList<OutlineHeading> &headings, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Ir a encabezado"));

    auto *layout = new QVBoxLayout(this);
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Filtrar encabezados…"));
    // El placeholder no lo lee un lector de pantalla; lo reusamos como nombre
    // accesible del campo (sin cadena nueva). La lista sí merece nombre propio.
    m_filter->setAccessibleName(m_filter->placeholderText());
    m_list = new QListWidget(this);
    m_list->setAccessibleName(tr("Encabezados"));
    layout->addWidget(m_filter);
    layout->addWidget(m_list);

    for (const OutlineHeading &h : headings) {
        auto *item = new QListWidgetItem(
            QString(qsizetype(h.level - 1) * 2, QLatin1Char(' ')) + h.text, m_list);
        item->setData(Qt::UserRole, h.blockNumber);
    }
    if (m_list->count() > 0)
        m_list->setCurrentRow(0);

    connect(m_filter, &QLineEdit::textChanged, this, [this](const QString &text) {
        int firstVisible = -1;
        for (int i = 0; i < m_list->count(); ++i) {
            const bool match = m_list->item(i)->text().contains(text, Qt::CaseInsensitive);
            m_list->item(i)->setHidden(!match);
            if (match && firstVisible < 0)
                firstVisible = i;
        }
        if (firstVisible >= 0)
            m_list->setCurrentRow(firstVisible);
    });
    connect(m_filter, &QLineEdit::returnPressed, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemActivated, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &QDialog::accept);

    m_filter->installEventFilter(this);  // reenvía las flechas a la lista
    m_filter->setFocus();
    resize(360, 420);
}

int GoToHeadingDialog::selectedBlockNumber() const
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item || item->isHidden())
        return -1;
    return item->data(Qt::UserRole).toInt();
}

bool GoToHeadingDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_filter && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        switch (ke->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            QApplication::sendEvent(m_list, ke);  // navegar la lista sin salir del filtro
            return true;
        default:
            break;
        }
    }
    return QDialog::eventFilter(watched, event);
}
