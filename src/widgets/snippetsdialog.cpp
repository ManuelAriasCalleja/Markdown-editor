/// \file
/// \brief Implementación del diálogo de gestión de snippets.

#include "snippetsdialog.h"

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>

SnippetsDialog::SnippetsDialog(const QList<mdsnippet::Snippet> &initial, QWidget *parent)
    : QDialog(parent), m_snippets(initial)
{
    setWindowTitle(tr("Snippets"));

    m_list = new QListWidget(this);
    m_list->setAccessibleName(tr("Lista de snippets"));
    auto *addButton = new QPushButton(tr("Añadir"), this);
    m_removeButton = new QPushButton(tr("Eliminar"), this);

    auto *listColumn = new QVBoxLayout;
    listColumn->addWidget(m_list);
    auto *listButtons = new QHBoxLayout;
    listButtons->addWidget(addButton);
    listButtons->addWidget(m_removeButton);
    listColumn->addLayout(listButtons);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setAccessibleName(tr("Nombre del snippet"));
    m_bodyEdit = new QPlainTextEdit(this);
    m_bodyEdit->setAccessibleName(tr("Cuerpo del snippet (Markdown)"));
    m_bodyEdit->setPlaceholderText(tr("Texto Markdown que se insertará"));

    auto *editColumn = new QVBoxLayout;
    editColumn->addWidget(new QLabel(tr("Nombre:"), this));
    editColumn->addWidget(m_nameEdit);
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
    resize(640, 420);

    connect(m_list, &QListWidget::currentRowChanged, this,
            [this] { loadSelectionIntoEditors(); });
    connect(addButton, &QPushButton::clicked, this, &SnippetsDialog::addSnippet);
    connect(m_removeButton, &QPushButton::clicked, this, &SnippetsDialog::removeSelected);
    // Editar el nombre/cuerpo actualiza el modelo (y el rótulo de la lista) al vuelo.
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        const int i = currentIndex();
        if (m_loading || i < 0)
            return;
        m_snippets[i].name = text;
        if (QListWidgetItem *item = m_list->item(i))
            item->setText(text);
    });
    connect(m_bodyEdit, &QPlainTextEdit::textChanged, this, [this] {
        const int i = currentIndex();
        if (m_loading || i < 0)
            return;
        m_snippets[i].body = m_bodyEdit->toPlainText();
    });

    rebuildList();
    if (!m_snippets.isEmpty())
        m_list->setCurrentRow(0);
    else
        loadSelectionIntoEditors();  // estado vacío: editores deshabilitados
}

int SnippetsDialog::currentIndex() const
{
    return m_list->currentRow();
}

void SnippetsDialog::rebuildList()
{
    m_loading = true;
    m_list->clear();
    for (const mdsnippet::Snippet &s : m_snippets)
        m_list->addItem(s.name);
    m_loading = false;
}

void SnippetsDialog::loadSelectionIntoEditors()
{
    const int i = currentIndex();
    const bool has = (i >= 0 && i < m_snippets.size());
    m_loading = true;
    m_nameEdit->setText(has ? m_snippets[i].name : QString());
    m_bodyEdit->setPlainText(has ? m_snippets[i].body : QString());
    m_loading = false;
    m_nameEdit->setEnabled(has);
    m_bodyEdit->setEnabled(has);
    m_removeButton->setEnabled(has);
}

void SnippetsDialog::addSnippet()
{
    m_snippets.append({tr("Nuevo snippet"), QString()});
    m_list->addItem(m_snippets.last().name);
    m_list->setCurrentRow(m_snippets.size() - 1);
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
}

void SnippetsDialog::removeSelected()
{
    const int i = currentIndex();
    if (i < 0 || i >= m_snippets.size())
        return;
    m_snippets.removeAt(i);
    delete m_list->takeItem(i);
    // currentRowChanged ya recoloca la selección y refresca los editores.
}
