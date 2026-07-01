/// \file
/// \brief Implementación del diálogo de gestión de plantillas de usuario.

#include "usertemplatesdialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "doctemplates.h"

UserTemplatesDialog::UserTemplatesDialog(const QList<mdusertemplate::UserTemplate> &initial,
                                         QWidget *parent)
    : QDialog(parent), m_templates(initial)
{
    setWindowTitle(tr("Plantillas de usuario"));

    m_list = new QListWidget(this);
    m_list->setAccessibleName(tr("Lista de plantillas"));
    auto *addButton = new QPushButton(tr("Añadir"), this);
    m_removeButton = new QPushButton(tr("Eliminar"), this);

    auto *listColumn = new QVBoxLayout;
    listColumn->addWidget(m_list);
    auto *listButtons = new QHBoxLayout;
    listButtons->addWidget(addButton);
    listButtons->addWidget(m_removeButton);
    listColumn->addLayout(listButtons);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setAccessibleName(tr("Nombre de la plantilla"));
    m_categoryCombo = new QComboBox(this);
    m_categoryCombo->setAccessibleName(tr("Categoría de la plantilla"));
    for (const mdtemplate::Category c : mdtemplate::categoriesInOrder())
        m_categoryCombo->addItem(mdtemplate::categoryName(c), static_cast<int>(c));
    m_bodyEdit = new QPlainTextEdit(this);
    m_bodyEdit->setAccessibleName(tr("Cuerpo de la plantilla (Markdown)"));
    m_bodyEdit->setPlaceholderText(tr("Texto Markdown de la plantilla"));

    auto *editColumn = new QVBoxLayout;
    editColumn->addWidget(new QLabel(tr("Nombre:"), this));
    editColumn->addWidget(m_nameEdit);
    editColumn->addWidget(new QLabel(tr("Categoría:"), this));
    editColumn->addWidget(m_categoryCombo);
    editColumn->addWidget(new QLabel(tr("Cuerpo (Markdown):"), this));
    editColumn->addWidget(m_bodyEdit, 1);

    auto *columns = new QHBoxLayout;
    columns->addLayout(listColumn, 1);
    columns->addLayout(editColumn, 2);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto *root = new QVBoxLayout(this);
    root->addLayout(columns, 1);
    root->addWidget(buttons);
    resize(640, 440);

    connect(m_list, &QListWidget::currentRowChanged, this,
            [this] { loadSelectionIntoEditors(); });
    connect(addButton, &QPushButton::clicked, this, &UserTemplatesDialog::addTemplate);
    connect(m_removeButton, &QPushButton::clicked, this, &UserTemplatesDialog::removeSelected);
    // Editar nombre/categoría/cuerpo actualiza el modelo (y el rótulo) al vuelo.
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        const int i = currentIndex();
        if (m_loading || i < 0)
            return;
        m_templates[i].name = text;
        if (QListWidgetItem *item = m_list->item(i))
            item->setText(labelFor(m_templates[i]));
    });
    connect(m_categoryCombo, &QComboBox::currentIndexChanged, this, [this](int) {
        const int i = currentIndex();
        if (m_loading || i < 0)
            return;
        m_templates[i].category =
            static_cast<mdtemplate::Category>(m_categoryCombo->currentData().toInt());
        if (QListWidgetItem *item = m_list->item(i))
            item->setText(labelFor(m_templates[i]));
    });
    connect(m_bodyEdit, &QPlainTextEdit::textChanged, this, [this] {
        const int i = currentIndex();
        if (m_loading || i < 0)
            return;
        m_templates[i].body = m_bodyEdit->toPlainText();
    });

    rebuildList();
    if (!m_templates.isEmpty())
        m_list->setCurrentRow(0);
    else
        loadSelectionIntoEditors();  // estado vacío: editores deshabilitados
}

int UserTemplatesDialog::currentIndex() const
{
    return m_list->currentRow();
}

QString UserTemplatesDialog::labelFor(const mdusertemplate::UserTemplate &t) const
{
    return t.name + QStringLiteral(" — ") + mdtemplate::categoryName(t.category);
}

void UserTemplatesDialog::rebuildList()
{
    m_loading = true;
    m_list->clear();
    for (const mdusertemplate::UserTemplate &t : m_templates)
        m_list->addItem(labelFor(t));
    m_loading = false;
}

void UserTemplatesDialog::loadSelectionIntoEditors()
{
    const int i = currentIndex();
    const bool has = (i >= 0 && i < m_templates.size());
    m_loading = true;
    m_nameEdit->setText(has ? m_templates[i].name : QString());
    m_categoryCombo->setCurrentIndex(
        has ? qMax(0, m_categoryCombo->findData(static_cast<int>(m_templates[i].category))) : 0);
    m_bodyEdit->setPlainText(has ? m_templates[i].body : QString());
    m_loading = false;
    m_nameEdit->setEnabled(has);
    m_categoryCombo->setEnabled(has);
    m_bodyEdit->setEnabled(has);
    m_removeButton->setEnabled(has);
}

void UserTemplatesDialog::addTemplate()
{
    m_templates.append({tr("Nueva plantilla"), QString(), mdtemplate::Category::Personal});
    m_list->addItem(labelFor(m_templates.last()));
    m_list->setCurrentRow(m_templates.size() - 1);
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void UserTemplatesDialog::startNewFromBody(const QString &body)
{
    m_templates.append({tr("Nueva plantilla"), body, mdtemplate::Category::Personal});
    m_list->addItem(labelFor(m_templates.last()));
    m_list->setCurrentRow(m_templates.size() - 1);
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void UserTemplatesDialog::removeSelected()
{
    const int i = currentIndex();
    if (i < 0 || i >= m_templates.size())
        return;
    m_templates.removeAt(i);
    delete m_list->takeItem(i);
    // currentRowChanged recoloca la selección y refresca los editores.
}
