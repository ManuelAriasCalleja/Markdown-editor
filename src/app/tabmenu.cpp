/// \file
/// \brief Implementación de la lógica pura del menú contextual de pestañas.

#include "tabmenu.h"

#include <QFileInfo>

namespace tabmenu {

FileInfo infoForPath(const QString &path)
{
    FileInfo info;
    if (path.isEmpty())
        return info;  // «Sin título»: hasFile = false, todo vacío.
    const QFileInfo fi(path);
    info.hasFile = true;
    info.fileName = fi.fileName();
    info.fullPath = fi.absoluteFilePath();
    info.containingFolder = fi.absolutePath();
    return info;
}

}  // namespace tabmenu
