/// \file
/// \brief Implementación de `mdcommands`: recolección de las acciones del menú y
///        filtrado difuso.

#include "commands.h"

#include <algorithm>

#include <QAction>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>

namespace mdcommands {
namespace {

const QString kSep = QStringLiteral(" › ");

/// Elimina el mnemónico `&` de un texto de menú, conservando `&&` como `&`.
QString stripMnemonic(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        if (s.at(i) == QLatin1Char('&')) {
            if (i + 1 < s.size() && s.at(i + 1) == QLatin1Char('&')) {
                out += QLatin1Char('&');
                ++i;  // consume el segundo '&'
            }
            // un '&' suelto es el marcador de acelerador: se descarta.
        } else {
            out += s.at(i);
        }
    }
    return out;
}

void collectFrom(const QMenu *menu, const QString &prefix, QList<Command> &out)
{
    const QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        if (action->isSeparator() || !action->isVisible() || !action->isEnabled())
            continue;
        const QString text = stripMnemonic(action->text());
        if (QMenu *sub = action->menu()) {
            const QString childPrefix = prefix.isEmpty() ? text : prefix + kSep + text;
            collectFrom(sub, childPrefix, out);
        } else if (!text.isEmpty()) {
            Command cmd;
            cmd.path = prefix.isEmpty() ? text : prefix + kSep + text;
            cmd.shortcut = action->shortcut().toString(QKeySequence::NativeText);
            cmd.action = action;
            out.append(cmd);
        }
    }
}

}  // namespace

QList<Command> collectCommands(const QMenuBar *menuBar)
{
    QList<Command> out;
    if (!menuBar)
        return out;
    const QList<QAction *> topLevel = menuBar->actions();
    for (QAction *action : topLevel) {
        if (action->isSeparator() || !action->isVisible() || !action->isEnabled())
            continue;
        if (QMenu *sub = action->menu())
            collectFrom(sub, stripMnemonic(action->text()), out);
    }
    return out;
}

bool fuzzyMatch(const QString &text, const QString &query, int *score)
{
    if (query.isEmpty()) {
        if (score)
            *score = 0;
        return true;
    }
    int total = 0;
    int qi = 0;
    int consecutive = 0;
    for (int ti = 0; ti < text.size() && qi < query.size(); ++ti) {
        if (text.at(ti).toLower() == query.at(qi).toLower()) {
            int bonus = 1;
            if (ti == 0)
                bonus += 5;  // coincidencia al inicio absoluto (prefijo): la más fuerte
            else if (!text.at(ti - 1).isLetterOrNumber())
                bonus += 3;  // inicio de palabra (tras separador/no-alfanumérico)
            ++consecutive;
            bonus += consecutive;  // premia los tramos contiguos
            total += bonus;
            ++qi;
        } else {
            consecutive = 0;
        }
    }
    const bool matched = qi == query.size();
    if (score)
        *score = matched ? total : 0;
    return matched;
}

QList<Command> filterCommands(const QList<Command> &commands, const QString &query)
{
    const QString trimmed = query.trimmed();
    if (trimmed.isEmpty())
        return commands;

    struct Scored
    {
        int score;
        int index;
    };
    QList<Scored> scored;
    for (int i = 0; i < commands.size(); ++i) {
        int s = 0;
        if (fuzzyMatch(commands.at(i).path, trimmed, &s))
            scored.append({s, i});
    }
    // stable_sort por puntuación: los empates conservan el orden original (índice).
    std::stable_sort(scored.begin(), scored.end(),
                     [](const Scored &a, const Scored &b) { return a.score > b.score; });

    QList<Command> out;
    out.reserve(scored.size());
    for (const Scored &s : scored)
        out.append(commands.at(s.index));
    return out;
}

}  // namespace mdcommands
