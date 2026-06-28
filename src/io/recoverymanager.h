#ifndef RECOVERYMANAGER_H
#define RECOVERYMANAGER_H

/// \file
/// \brief Borrador de recuperación ante fallos del documento Markdown.

#include <QDateTime>
#include <QList>
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
///
/// **Una instancia por pestaña.** Cada documento abierto tiene su propio
/// RecoveryManager con un *slot* único (`recovery-draft-<uuid>.md`), de modo que
/// con varias pestañas abiertas los borradores no se pisan: si la app cae, todos
/// sobreviven. `leftoverDrafts()` (estático) los enumera al arrancar para
/// ofrecer recuperar **todos** los documentos con cambios, no solo el último.
class RecoveryManager : public QObject
{
    Q_OBJECT

public:
    explicit RecoveryManager(QObject *parent = nullptr);

    /// \brief ¿Existe un borrador recuperable de esta instancia?
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

    /// \brief Un borrador huérfano encontrado al arrancar (de cualquier pestaña/sesión).
    struct Draft
    {
        QString body;          ///< cuerpo Markdown del documento
        QString originalPath;  ///< ruta del archivo asociado ("" = sin título)
        QDateTime timestamp;   ///< último guardado del borrador
        QString bodyFile;      ///< ruta del .md del borrador (para borrarlo tras recuperar)
        QString metaFile;      ///< ruta del .path del borrador
    };

    /// \brief Enumera todos los borradores presentes en el directorio de datos (de
    /// cualquier slot, incluido el formato antiguo `recovery-draft.md`), ordenados
    /// del más reciente al más antiguo. Para ofrecer la recuperación al arrancar.
    static QList<Draft> leftoverDrafts();
    /// \brief Borra los ficheros de un borrador (cuerpo + meta). No falla si faltan.
    static void removeDraft(const Draft &draft);

private:
    QString draftFilePath() const;  // <AppData>/recovery-draft-<slot>.md
    QString metaFilePath() const;   // <AppData>/recovery-draft-<slot>.path
    static QString baseDir();

    static QString readFile(const QString &path);
    static bool writeFile(const QString &path, const QString &text);

    QString m_slot;  // identificador único de esta instancia (uuid), para su fichero
};

#endif // RECOVERYMANAGER_H
