#ifndef INSTALLCMD_H
#define INSTALLCMD_H

/// \file
/// \brief Órdenes de instalación de las herramientas externas, según el sistema.

#include <QString>

/// Qué orden enseñarle al usuario para instalar lo que falta, pura y sin GUI
/// (`tst_installcmd`). Un aviso de «falta X» solo sirve si dice cómo conseguirlo, y
/// «sudo apt install» en Fedora no sirve de nada: la orden tiene que ser la de SU
/// sistema. Antes cada aviso (Pandoc, PlantUML, diccionarios) lo decidía por su
/// cuenta, y unos distinguían distribuciones y otros daban `apt` a todo Linux.
namespace mdinstall {

/// Familias de sistema con un gestor de paquetes distinto.
enum class Platform { Debian, Fedora, Arch, Suse, Alpine, MacOS, Windows };

/// \brief Familia de `productType` (`QSysInfo::productType()`: "ubuntu", "fedora",
/// "macos", "windows"…). Lo desconocido cae en Debian, que es lo más extendido.
Platform platformFor(const QString &productType);

/// \brief Familia del sistema en ejecución.
Platform currentPlatform();

/// \brief `sudo apt install <package>`, `sudo dnf install <package>`… En macOS,
/// `brew install`; en Windows, `winget install`.
QString packageCommand(Platform platform, const QString &package);

/// \brief Orden para instalar el MOTOR de Hunspell con sus cabeceras, que es lo que
/// hace falta para compilar el programa con corrector (no para usarlo).
QString hunspellEngineCommand(Platform platform);

/// \brief Orden para instalar Pandoc (importación de DOCX, ODT, RTF…).
QString pandocCommand(Platform platform);

/// \brief Orden para instalar PlantUML (previsualización de diagramas).
QString plantUmlCommand(Platform platform);

}  // namespace mdinstall

#endif // INSTALLCMD_H
