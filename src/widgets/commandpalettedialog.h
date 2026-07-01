#ifndef COMMANDPALETTEDIALOG_H
#define COMMANDPALETTEDIALOG_H

/// \file
/// \brief Paleta de comandos: búsqueda difusa sobre las acciones de los menús.

#include <QDialog>
#include <QList>
#include <QString>

class QAction;
class QEvent;
class QLineEdit;
class QListWidget;
class QMenuBar;

/// \brief Lógica pura de la paleta de comandos (recolección y filtrado difuso).
///
/// `collectCommands` es lo único que toca la GUI (recorre el `QMenuBar`); el
/// filtrado (`fuzzyMatch`/`filterCommands`) es puro y se prueba aislado en
/// `tst_commands`.
namespace mdcommands {

/// Una acción invocable del menú, con su ruta legible y su atajo.
struct Command
{
    QString path;      ///< Ruta de menús, p. ej. "Archivo › Exportar › A texto plano".
    QString shortcut;  ///< Atajo en texto nativo (puede estar vacío).
    QAction *action = nullptr;  ///< La acción a disparar (no se usa en los tests puros).
};

/// Recorre el menú (recursivo por submenús) y devuelve las acciones invocables:
/// las hojas visibles y habilitadas (salta separadores, contenedores y las
/// deshabilitadas), con su ruta de menús (sin el mnemónico `&`) y su atajo.
QList<Command> collectCommands(const QMenuBar *menuBar);

/// Coincidencia difusa: ¿aparecen los caracteres de `query` como subsecuencia
/// (sin distinguir mayúsculas) dentro de `text`? Una `query` vacía siempre casa.
/// Si `score` no es nulo, escribe una puntuación (mayor = mejor: bonifica los
/// tramos contiguos y las coincidencias a inicio de palabra).
bool fuzzyMatch(const QString &text, const QString &query, int *score = nullptr);

/// Filtra `commands` por `query` (difuso sobre la ruta) y los ordena por
/// puntuación descendente; los empates conservan el orden original (estable).
/// Una `query` vacía devuelve todos los comandos sin reordenar.
QList<Command> filterCommands(const QList<Command> &commands, const QString &query);

}  // namespace mdcommands

/// \brief Diálogo «paleta de comandos»: un campo de filtro sobre la lista de
/// acciones de los menús. Se filtra de forma difusa al teclear (los mejores
/// resultados suben); las flechas mueven la selección sin salir del filtro;
/// Intro o doble clic aceptan. Tras aceptar, la acción elegida se consulta con
/// selectedAction() (el llamante hace `action->trigger()`).
class CommandPaletteDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CommandPaletteDialog(QList<mdcommands::Command> commands, QWidget *parent = nullptr);

    /// \brief La acción seleccionada, o nullptr si no hay ninguna visible.
    QAction *selectedAction() const;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void repopulate(const QString &query);

    QList<mdcommands::Command> m_commands;
    QLineEdit *m_filter;
    QListWidget *m_list;
};

#endif  // COMMANDPALETTEDIALOG_H
