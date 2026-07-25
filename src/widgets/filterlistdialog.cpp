/// \file
/// \brief Implementación del armazón común de los diálogos de filtro rápido.

#include "filterlistdialog.h"

#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>

FilterListDialog::FilterListDialog(const QString &title, const QString &filterPlaceholder,
                                   const QString &listName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(title);

    auto *layout = new QVBoxLayout(this);
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(filterPlaceholder);
    // El placeholder no lo lee un lector de pantalla; lo reusamos como nombre
    // accesible del campo (sin cadena nueva). La lista sí merece nombre propio.
    m_filter->setAccessibleName(filterPlaceholder);
    m_list = new QListWidget(this);
    m_list->setAccessibleName(listName);
    layout->addWidget(m_filter);
    layout->addWidget(m_list);

    connect(m_filter, &QLineEdit::textChanged, this,
            [this](const QString &text) { filterChanged(text); });
    connect(m_filter, &QLineEdit::returnPressed, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemActivated, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &QDialog::accept);

    m_filter->installEventFilter(this);  // reenvía las flechas a la lista
    m_filter->setFocus();
}

bool FilterListDialog::eventFilter(QObject *watched, QEvent *event)
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
