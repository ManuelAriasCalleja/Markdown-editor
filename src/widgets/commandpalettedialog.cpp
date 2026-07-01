/// \file
/// \brief Implementación de la paleta de comandos (recolección, filtro difuso y diálogo).

#include "commandpalettedialog.h"

#include <QAction>
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QVariant>

#include <algorithm>

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

CommandPaletteDialog::CommandPaletteDialog(QList<mdcommands::Command> commands, QWidget *parent)
    : QDialog(parent), m_commands(std::move(commands))
{
    setWindowTitle(tr("Paleta de comandos"));

    auto *layout = new QVBoxLayout(this);
    m_filter = new QLineEdit(this);
    m_filter->setPlaceholderText(tr("Filtrar acciones…"));
    // El placeholder no lo lee un lector de pantalla; lo reusamos como nombre
    // accesible del campo (sin cadena nueva). La lista sí merece nombre propio.
    m_filter->setAccessibleName(m_filter->placeholderText());
    m_list = new QListWidget(this);
    m_list->setAccessibleName(tr("Acciones"));
    layout->addWidget(m_filter);
    layout->addWidget(m_list);

    repopulate(QString());

    connect(m_filter, &QLineEdit::textChanged, this,
            [this](const QString &text) { repopulate(text); });
    connect(m_filter, &QLineEdit::returnPressed, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemActivated, this, &QDialog::accept);
    connect(m_list, &QListWidget::itemDoubleClicked, this, &QDialog::accept);

    m_filter->installEventFilter(this);  // reenvía las flechas a la lista
    m_filter->setFocus();
    resize(480, 420);
}

void CommandPaletteDialog::repopulate(const QString &query)
{
    m_list->clear();
    const QList<mdcommands::Command> filtered = mdcommands::filterCommands(m_commands, query);
    for (const mdcommands::Command &cmd : filtered) {
        QString label = cmd.path;
        if (!cmd.shortcut.isEmpty())
            label += QLatin1Char('\t') + cmd.shortcut;
        auto *item = new QListWidgetItem(label, m_list);
        item->setData(Qt::UserRole, QVariant::fromValue<QObject *>(cmd.action));
    }
    if (m_list->count() > 0)
        m_list->setCurrentRow(0);
}

QAction *CommandPaletteDialog::selectedAction() const
{
    QListWidgetItem *item = m_list->currentItem();
    if (!item)
        return nullptr;
    return qobject_cast<QAction *>(item->data(Qt::UserRole).value<QObject *>());
}

bool CommandPaletteDialog::eventFilter(QObject *watched, QEvent *event)
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
