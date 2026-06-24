#include "recentfilesmanager.h"

#include "appsettings.h"

#include <QAction>
#include <QFileInfo>
#include <QMenu>

RecentFilesManager::RecentFilesManager(QMenu *menu, QObject *parent)
    : QObject(parent), m_menu(menu)
{
    if (m_menu)
        m_menu->setToolTipsVisible(true);  // muestra la ruta completa al pasar
    rebuildMenu();
}

void RecentFilesManager::addFile(const QString &path)
{
    if (path.isEmpty())
        return;
    const QString absolute = QFileInfo(path).absoluteFilePath();

    QStringList files = load();
    files.removeAll(absolute);   // si ya estaba, lo subimos al principio
    files.prepend(absolute);     // el más reciente, primero
    while (files.size() > MaxRecentFiles)
        files.removeLast();
    save(files);

    rebuildMenu();
}

void RecentFilesManager::removeFile(const QString &path)
{
    QStringList files = load();
    if (files.removeAll(QFileInfo(path).absoluteFilePath()) > 0) {
        save(files);
        rebuildMenu();
    }
}

QStringList RecentFilesManager::load() const
{
    return AppSettings::recentFiles();
}

void RecentFilesManager::save(const QStringList &files)
{
    AppSettings::setRecentFiles(files);
}

void RecentFilesManager::rebuildMenu()
{
    if (!m_menu)
        return;

    m_menu->clear();

    QStringList files = load();

    // Descarta los que ya no existen en disco y reescribe la lista si cambió.
    const qsizetype before = files.size();
    files.removeIf([](const QString &p) { return !QFileInfo::exists(p); });
    if (files.size() != before)
        save(files);

    m_menu->setEnabled(!files.isEmpty());

    for (const QString &path : files) {
        // El nombre del archivo en la entrada; la ruta completa en el tooltip.
        QAction *action = m_menu->addAction(QFileInfo(path).fileName());
        action->setToolTip(path);
        connect(action, &QAction::triggered, this,
                [this, path] { emit fileOpenRequested(path); });
    }

    if (!files.isEmpty()) {
        m_menu->addSeparator();
        QAction *clear = m_menu->addAction(tr("Borrar la lista"));
        connect(clear, &QAction::triggered, this, [this] {
            save(QStringList());
            rebuildMenu();
        });
    }
}
