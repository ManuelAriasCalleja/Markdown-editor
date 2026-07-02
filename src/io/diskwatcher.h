#ifndef DISKWATCHER_H
#define DISKWATCHER_H

/// \file
/// \brief Vigila el archivo abierto en disco para detectar cambios externos.

#include <QByteArray>
#include <QObject>
#include <QString>

class QFileSystemWatcher;
class QTimer;

/// \brief Vigila el archivo abierto para detectar cambios hechos por otra herramienta
/// (git, otro editor…). Posee el QFileSystemWatcher, el temporizador de debounce y
/// la instantánea de bytes que distingue el propio guardado de un cambio externo.
///
/// No decide qué hacer ante el cambio (recargar, preguntar): solo lo señala. La
/// reacción —que toca los editores y puede mostrar un diálogo— vive en MainWindow.
class DiskWatcher : public QObject
{
    Q_OBJECT

public:
    explicit DiskWatcher(QObject *parent = nullptr);

    /// \brief Empieza a vigilar `path` (deja de vigilar el anterior) y toma la instantánea
    /// de sus bytes, de modo que un guardado/carga propios no se confundan luego con
    /// un cambio externo. path vacío = no vigilar nada. Conéctalo a
    /// DocumentIo::currentFileChanged (se emite también al guardar).
    void watch(const QString &path);
    /// \brief Fija la instantánea de referencia a estos bytes: tras decidir conservar los
    /// cambios locales ante un conflicto, para no volver a avisar del mismo cambio.
    void setSnapshot(const QByteArray &bytes) { m_snapshot = bytes; }
    /// \brief Suspende/reactiva la comprobación mientras hay un diálogo de conflicto
    /// abierto, para no apilar otro (su exec corre un bucle de eventos anidado). Al
    /// reactivar, procesa cualquier comprobación que llegó durante la suspensión.
    void setSuspended(bool suspended);
    /// \brief Fuerza una comprobación inmediata del disco: re-añade la vigilancia si
    /// el archivo reapareció y compara con la instantánea (emite externalChange/
    /// vanished si procede). La llama MainWindow al activar una pestaña que estuvo en
    /// segundo plano (cuyos avisos no se atendieron) y al recrear la vigilancia.
    void recheck() { checkChange(); }

signals:
    /// \brief El archivo vigilado cambió en disco respecto a la instantánea; `diskBytes` es
    /// su contenido actual.
    void externalChange(const QByteArray &diskBytes);
    /// \brief El archivo vigilado se eliminó o movió.
    void vanished();

private:
    void onWatchedFileChanged(const QString &path);
    void checkChange();
    void updateSnapshot();

    QString m_path;                         // archivo vigilado actualmente
    QFileSystemWatcher *m_watcher = nullptr;
    QTimer *m_debounce = nullptr;           // agrupa los eventos de un guardado atómico
    QByteArray m_snapshot;                  // últimos bytes en disco (cargados/guardados)
    bool m_suspended = false;               // hay un diálogo de conflicto abierto
    bool m_pendingCheck = false;            // llegó un cambio mientras estaba suspendido
};

#endif // DISKWATCHER_H
