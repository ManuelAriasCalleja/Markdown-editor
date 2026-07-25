/// \file
/// \brief Implementación del diálogo de la paleta de comandos.

#include "commandpalettedialog.h"

#include <QAction>
#include <QListWidget>
#include <QVariant>

#include <utility>


CommandPaletteDialog::CommandPaletteDialog(QList<mdcommands::Command> commands, QWidget *parent)
    : FilterListDialog(tr("Paleta de comandos"), tr("Filtrar acciones…"),
                       tr("Acciones"), parent)
    , m_commands(std::move(commands))
{
    filterChanged(QString());
    resize(480, 420);
}

// Filtro DIFUSO (no por subcadena): se rehace la lista con los mejores resultados
// arriba, así que se repuebla entera en vez de ocultar filas.
void CommandPaletteDialog::filterChanged(const QString &query)
{
    list()->clear();
    for (const mdcommands::Command &cmd : mdcommands::filterCommands(m_commands, query)) {
        QString label = cmd.path;
        if (!cmd.shortcut.isEmpty())
            label += QLatin1Char('\t') + cmd.shortcut;
        auto *item = new QListWidgetItem(label, list());
        item->setData(Qt::UserRole, QVariant::fromValue<QObject *>(cmd.action));
    }
    if (list()->count() > 0)
        list()->setCurrentRow(0);
}

QAction *CommandPaletteDialog::selectedAction() const
{
    QListWidgetItem *item = list()->currentItem();
    if (!item)
        return nullptr;
    return qobject_cast<QAction *>(item->data(Qt::UserRole).value<QObject *>());
}
