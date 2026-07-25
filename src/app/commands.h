#ifndef COMMANDS_H
#define COMMANDS_H

/// \file
/// \brief Recolección y filtrado difuso de las acciones de los menús
///        (`mdcommands`). Vivía en `commandpalettedialog.h`, que obligaba a
///        arrastrar el diálogo entero a quien solo quería la lista de comandos.

#include <QList>
#include <QString>

class QAction;
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

#endif  // COMMANDS_H
