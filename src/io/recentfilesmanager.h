#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>

class QMenu;

// Gestiona la lista de archivos abiertos recientemente: la persiste con
// QSettings, puebla un QMenu (más reciente primero) y avisa por señal cuando el
// usuario elige una entrada. Depura solo los archivos que ya no existen en disco.
class RecentFilesManager : public QObject
{
    Q_OBJECT

public:
    // `menu` es el submenú que se rellenará (propiedad del llamador).
    explicit RecentFilesManager(QMenu *menu, QObject *parent = nullptr);

    // Registra un archivo como el más reciente (no hace nada si la ruta es vacía).
    void addFile(const QString &path);
    // Quita un archivo de la lista (p. ej. si ya no se puede abrir).
    void removeFile(const QString &path);

signals:
    void fileOpenRequested(const QString &path);

private:
    QStringList load() const;
    void save(const QStringList &files);
    void rebuildMenu();

    QMenu *m_menu;
    static constexpr int MaxRecentFiles = 10;
};

#endif // RECENTFILESMANAGER_H
