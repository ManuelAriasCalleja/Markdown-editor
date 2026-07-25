#ifndef COMMANDPALETTEDIALOG_H
#define COMMANDPALETTEDIALOG_H

/// \file
/// \brief Diálogo de la paleta de comandos. La lógica de recolección y filtrado
/// está en `commands.h`.

#include "commands.h"
#include "filterlistdialog.h"

#include <QList>
#include <QString>


/// \brief Diálogo «paleta de comandos»: un campo de filtro sobre la lista de
/// acciones de los menús. Se filtra de forma difusa al teclear (los mejores
/// resultados suben); las flechas mueven la selección sin salir del filtro;
/// Intro o doble clic aceptan. Tras aceptar, la acción elegida se consulta con
/// selectedAction() (el llamante hace `action->trigger()`).
class CommandPaletteDialog : public FilterListDialog
{
    Q_OBJECT

public:
    explicit CommandPaletteDialog(QList<mdcommands::Command> commands, QWidget *parent = nullptr);

    /// \brief La acción seleccionada, o nullptr si no hay ninguna visible.
    QAction *selectedAction() const;

protected:
    void filterChanged(const QString &query) override;

private:
    QList<mdcommands::Command> m_commands;
};

#endif  // COMMANDPALETTEDIALOG_H
