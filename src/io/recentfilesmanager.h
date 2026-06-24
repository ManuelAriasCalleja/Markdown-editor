#ifndef RECENTFILESMANAGER_H
#define RECENTFILESMANAGER_H

/// \file
/// \brief Lista de archivos recientes: persistencia y poblado del menú.

#include <QObject>
#include <QString>
#include <QStringList>

class QMenu;

/// \brief Gestiona la lista de archivos abiertos recientemente: la persiste con
/// QSettings, puebla un QMenu (más reciente primero) y avisa por señal cuando el
/// usuario elige una entrada. Depura solo los archivos que ya no existen en disco.
class RecentFilesManager : public QObject
{
    Q_OBJECT

public:
    /// \brief Construye el gestor sobre `menu`, el submenú que se rellenará (propiedad del llamador).
    explicit RecentFilesManager(QMenu *menu, QObject *parent = nullptr);

    /// \brief Registra un archivo como el más reciente (no hace nada si la ruta es vacía).
    void addFile(const QString &path);
    /// \brief Quita un archivo de la lista (p. ej. si ya no se puede abrir).
    void removeFile(const QString &path);

signals:
    /// \brief El usuario eligió un archivo reciente del menú.
    void fileOpenRequested(const QString &path);

private:
    QStringList load() const;
    void save(const QStringList &files);
    void rebuildMenu();

    QMenu *m_menu;
    static constexpr int MaxRecentFiles = 10;
};

#endif // RECENTFILESMANAGER_H
