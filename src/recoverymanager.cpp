#include "recoverymanager.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QStringConverter>
#include <QTextStream>

RecoveryManager::RecoveryManager(QObject *parent)
    : QObject(parent)
{
}

QString RecoveryManager::baseDir() const
{
    // AppDataLocation: ~/.local/share/md-editor en Linux. En tests, QStandardPaths
    // en modo de prueba lo redirige a un directorio temporal.
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString RecoveryManager::draftFilePath() const
{
    return baseDir() + QStringLiteral("/recovery-draft.md");
}

QString RecoveryManager::metaFilePath() const
{
    return baseDir() + QStringLiteral("/recovery-draft.path");
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
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << text;
    return true;
}
