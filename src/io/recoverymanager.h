#ifndef RECOVERYMANAGER_H
#define RECOVERYMANAGER_H

/// \file
/// \brief Borrador de recuperación ante fallos del documento Markdown.

#include <QDateTime>
#include <QObject>
#include <QString>

/// \brief Gestiona el borrador de recuperación ante fallos: un volcado del cuerpo
/// Markdown del documento que se reescribe periódicamente mientras hay cambios
/// sin guardar y se borra al guardar o al cerrar limpiamente. Si la aplicación
/// termina de forma anómala, el borrador sobrevive y puede ofrecerse al arrancar.
///
/// El borrador se guarda bajo el directorio de datos de la aplicación
/// (QStandardPaths::AppDataLocation): el cuerpo en un archivo y la ruta del
/// documento original asociado en otro contiguo, de modo que la recuperación es
/// autocontenida (no depende de QSettings). Una ruta original vacía indica un
/// documento sin título.
class RecoveryManager : public QObject
{
    Q_OBJECT

public:
    explicit RecoveryManager(QObject *parent = nullptr);

    /// \brief ¿Existe un borrador recuperable de una sesión anterior?
    bool hasDraft() const;

    /// \brief Escribe/actualiza el borrador con el cuerpo Markdown dado y la ruta del
    /// documento original asociado ("" = sin título).
    void saveDraft(const QString &originalPath, const QString &body);

    /// \brief Borra el borrador (no falla si no existe).
    void clearDraft();

    /// \brief Contenido del borrador (cuerpo Markdown), o "" si no hay.
    QString draftBody() const;
    /// \brief Ruta del documento original asociado, o "" (sin título / sin borrador).
    QString draftOriginalPath() const;
    /// \brief Momento del último guardado del borrador (para informar al recuperar).
    QDateTime draftTimestamp() const;

private:
    QString draftFilePath() const;  // <AppData>/recovery-draft.md
    QString metaFilePath() const;   // <AppData>/recovery-draft.path
    QString baseDir() const;

    static QString readFile(const QString &path);
    static bool writeFile(const QString &path, const QString &text);
};

#endif // RECOVERYMANAGER_H
