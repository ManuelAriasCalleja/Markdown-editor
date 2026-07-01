/// \file
/// \brief Implementación de la parte pura de la importación con Pandoc.

#include "pandocimport.h"

#include <QStandardPaths>
#include <QSysInfo>

namespace mdimport {

bool pandocAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("pandoc")).isEmpty();
}

QStringList pandocArguments(const QString &inputPath)
{
    // Salida a stdout en Markdown GitHub (el dialecto del editor); el formato de
    // entrada lo deduce Pandoc de la extensión de `inputPath`.
    return {QStringLiteral("--to=gfm"), QStringLiteral("--wrap=none"), inputPath};
}

QString pandocFilePattern()
{
    // Los formatos de entrada más útiles que Pandoc admite y que no tienen ya un
    // importador nativo propio (HTML/EPUB lo tienen, pero se incluyen por comodidad).
    return QStringLiteral(
        "*.docx *.odt *.rtf *.tex *.rst *.org *.textile *.wiki *.epub *.html *.htm "
        "*.man *.docbook");
}

QString pandocInstallCommand()
{
    const QString kernel = QSysInfo::kernelType();  // "linux" / "darwin" / "winnt"
    if (kernel == QLatin1String("darwin"))
        return QStringLiteral("brew install pandoc");
    if (kernel == QLatin1String("winnt"))
        return QStringLiteral("choco install pandoc");
    return QStringLiteral("sudo apt install pandoc");
}

}  // namespace mdimport
