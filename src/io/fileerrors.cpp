/// \file
/// \brief Implementación de los avisos de error de archivo.

#include "fileerrors.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStorageInfo>

namespace mdfileerr {

namespace {
QString tr(const char *text)
{
    return QCoreApplication::translate("MainWindow", text);
}

// Primer ancestro que existe: si falta la carpeta entera (una unidad desconectada),
// el espacio libre y los permisos solo se pueden mirar ahí.
QString existingAncestor(const QString &dir)
{
    QDir d(dir);
    while (!d.exists() && !d.isRoot()) {
        if (!d.cdUp())
            break;
    }
    return d.absolutePath();
}
}  // namespace

QString hint(const QString &path, Op op)
{
    const QFileInfo info(path);
    const QString dir = QDir::toNativeSeparators(info.absolutePath());

    if (op == Op::Read) {
        if (!info.exists())
            return tr("El archivo ya no está ahí: puede que se haya movido, renombrado "
                      "o borrado, o que esté en una unidad que ya no está conectada.");
        if (info.isDir())
            return tr("Es una carpeta, no un archivo. Elige un archivo dentro de ella.");
        if (!info.isReadable())
            return tr("No tienes permiso para leerlo. Pide acceso a su propietario o "
                      "cambia sus permisos.");
        return tr("Comprueba que la unidad sigue conectada y que otro programa no lo "
                  "tiene bloqueado, y vuelve a intentarlo.");
    }

    if (!QFileInfo(info.absolutePath()).isDir())
        return tr("La carpeta «%1» ya no existe (puede que se haya borrado o que la "
                  "unidad se haya desconectado). Elige otra carpeta.").arg(dir);
    if (info.isDir())
        return tr("Ya hay una carpeta con ese nombre. Elige otro nombre.");
    if (info.exists() && !info.isWritable())
        return tr("El archivo es de solo lectura o pertenece a otro usuario. Guárdalo "
                  "con otro nombre o en otra carpeta, o cambia sus permisos.");
    if (!QFileInfo(info.absolutePath()).isWritable())
        return tr("No tienes permiso para escribir en la carpeta «%1». Elige otra "
                  "carpeta, por ejemplo una dentro de tu carpeta personal.").arg(dir);
    const QStorageInfo storage(existingAncestor(info.absolutePath()));
    if (storage.isValid() && storage.bytesAvailable() < qint64(1024) * 1024)
        return tr("El disco está lleno. Libera espacio o elige otra unidad y vuelve a "
                  "intentarlo.");
    if (storage.isValid() && storage.isReadOnly())
        return tr("La unidad es de solo lectura. Elige una carpeta en otra unidad.");
    return tr("Comprueba que la unidad sigue conectada y que otro programa no tiene el "
              "archivo abierto, y vuelve a intentarlo.");
}

void showError(QWidget *parent, const QString &title, const QString &what,
               const QString &path, const QString &systemError, Op op)
{
    QMessageBox box(QMessageBox::Warning, title, what, QMessageBox::Ok, parent);
    QString info = hint(path, op);
    if (!systemError.trimmed().isEmpty())
        info += QStringLiteral("\n\n") + tr("Motivo que da el sistema: %1").arg(systemError.trimmed());
    box.setInformativeText(info);
    box.exec();
}

}  // namespace mdfileerr
