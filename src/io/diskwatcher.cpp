/// \file
/// \brief Implementación de DiskWatcher: vigilancia con debounce e instantánea de bytes.

#include "diskwatcher.h"

#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QTimer>

DiskWatcher::DiskWatcher(QObject *parent)
    : QObject(parent)
{
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged,
            this, &DiskWatcher::onWatchedFileChanged);
    // El aviso se agrupa con un pequeño retardo para sortear los guardados atómicos
    // (escribir-temporal-y-renombrar), que disparan varios eventos y dejan el
    // archivo un instante inexistente.
    m_debounce = new QTimer(this);
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(300);
    connect(m_debounce, &QTimer::timeout, this, &DiskWatcher::checkChange);
}

void DiskWatcher::watch(const QString &path)
{
    // Deja de vigilar lo anterior y vigila el archivo actual (si lo hay en disco).
    const QStringList watched = m_watcher->files();
    if (!watched.isEmpty())
        m_watcher->removePaths(watched);
    m_path = path;
    if (!path.isEmpty() && QFileInfo::exists(path))
        m_watcher->addPath(path);
    // La instantánea se pone al día con lo que acabamos de cargar/guardar, de modo
    // que nuestro propio guardado no se confunda luego con un cambio externo.
    updateSnapshot();
}

void DiskWatcher::updateSnapshot()
{
    m_snapshot.clear();
    if (m_path.isEmpty())
        return;
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly))
        m_snapshot = file.readAll();
}

void DiskWatcher::onWatchedFileChanged(const QString &path)
{
    if (path == m_path)
        m_debounce->start();  // (re)arranca el debounce
}

void DiskWatcher::checkChange()
{
    if (m_suspended)
        return;  // ya hay un diálogo abierto: no apilar otro (exec corre anidado)
    if (m_path.isEmpty())
        return;

    // Un guardado atómico recrea el archivo y rompe la vigilancia: hay que volver a
    // añadirlo si volvió a existir.
    if (QFileInfo::exists(m_path) && !m_watcher->files().contains(m_path))
        m_watcher->addPath(m_path);

    if (!QFileInfo::exists(m_path)) {
        emit vanished();
        return;
    }

    QByteArray disk;
    QFile file(m_path);
    if (file.open(QIODevice::ReadOnly))
        disk = file.readAll();
    if (disk == m_snapshot)
        return;  // sin cambios reales (o fue nuestro propio guardado)

    emit externalChange(disk);
}
