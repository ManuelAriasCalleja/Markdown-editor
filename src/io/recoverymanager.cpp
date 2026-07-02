/// \file
/// \brief Implementación de RecoveryManager: persistencia del borrador en AppDataLocation.

#include "recoverymanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringConverter>
#include <QTextStream>
#include <QUuid>

RecoveryManager::RecoveryManager(QObject *parent)
    : QObject(parent)
    , m_slot(QUuid::createUuid().toString(QUuid::WithoutBraces))  // único por pestaña
{
}

QString RecoveryManager::baseDir()
{
    // AppDataLocation: ~/.local/share/md-editor en Linux. En tests, QStandardPaths
    // en modo de prueba lo redirige a un directorio temporal.
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString RecoveryManager::draftFilePath() const
{
    return baseDir() + QStringLiteral("/recovery-draft-") + m_slot + QStringLiteral(".md");
}

QString RecoveryManager::metaFilePath() const
{
    return baseDir() + QStringLiteral("/recovery-draft-") + m_slot + QStringLiteral(".path");
}

QList<RecoveryManager::Draft> RecoveryManager::leftoverDrafts()
{
    QList<Draft> drafts;
    const QString dir = baseDir();
    if (dir.isEmpty())
        return drafts;
    // recovery-draft-<uuid>.md (por pestaña) y el antiguo recovery-draft.md (de una
    // versión previa con borrador único); el patrón cubre ambos.
    const QStringList names = QDir(dir).entryList(
        {QStringLiteral("recovery-draft*.md")}, QDir::Files, QDir::Time);
    for (const QString &name : names) {
        Draft d;
        d.bodyFile = dir + QLatin1Char('/') + name;
        d.metaFile = d.bodyFile;
        d.metaFile.chop(3);  // ".md"
        d.metaFile += QStringLiteral(".path");
        d.body = readFile(d.bodyFile);
        QString path = readFile(d.metaFile);
        while (path.endsWith(QLatin1Char('\n')) || path.endsWith(QLatin1Char('\r')))
            path.chop(1);
        d.originalPath = path;
        d.timestamp = QFileInfo(d.bodyFile).lastModified();
        drafts.append(d);
    }
    return drafts;
}

void RecoveryManager::removeDraft(const Draft &draft)
{
    QFile::remove(draft.bodyFile);
    QFile::remove(draft.metaFile);
}

bool RecoveryManager::hasDraft() const
{
    return QFileInfo::exists(draftFilePath());
}

void RecoveryManager::saveDraft(const QString &originalPath, const QString &body)
{
    const QString dir = baseDir();
    if (dir.isEmpty())
        return;
    QDir().mkpath(dir);
    // El cuerpo primero: si fallara, no dejamos metadatos huérfanos. La meta se
    // escribe después para que hasDraft (que mira el cuerpo) sea consistente.
    if (writeFile(draftFilePath(), body))
        writeFile(metaFilePath(), originalPath);
}

void RecoveryManager::clearDraft()
{
    QFile::remove(draftFilePath());
    QFile::remove(metaFilePath());
}

QString RecoveryManager::draftBody() const
{
    return readFile(draftFilePath());
}

QString RecoveryManager::draftOriginalPath() const
{
    // La meta lleva la ruta en una sola línea; recortamos el salto final.
    QString path = readFile(metaFilePath());
    while (path.endsWith(QLatin1Char('\n')) || path.endsWith(QLatin1Char('\r')))
        path.chop(1);
    return path;
}

QDateTime RecoveryManager::draftTimestamp() const
{
    return QFileInfo(draftFilePath()).lastModified();
}

QString RecoveryManager::readFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    return in.readAll();
}

bool RecoveryManager::writeFile(const QString &path, const QString &text)
{
    // QSaveFile: escritura atómica (temporal + rename). Si el volcado falla a
    // medias (disco lleno, corte de energía), el borrador ANTERIOR se conserva
    // íntegro en vez de quedar truncado, y devolvemos false para no escribir la
    // meta sobre un cuerpo corrupto.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << text;
    out.flush();
    return out.status() == QTextStream::Ok && file.commit();
}
