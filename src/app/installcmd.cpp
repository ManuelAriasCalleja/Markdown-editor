/// \file
/// \brief Implementación de las órdenes de instalación por sistema.

#include "installcmd.h"

#include <QSysInfo>

namespace mdinstall {

Platform platformFor(const QString &productType)
{
    const QString os = productType.toLower();
    if (os == QLatin1String("fedora") || os == QLatin1String("rhel")
        || os == QLatin1String("centos") || os == QLatin1String("rocky")
        || os == QLatin1String("almalinux"))
        return Platform::Fedora;
    if (os == QLatin1String("arch") || os == QLatin1String("manjaro")
        || os == QLatin1String("endeavouros"))
        return Platform::Arch;
    if (os.startsWith(QLatin1String("opensuse")) || os == QLatin1String("suse")
        || os == QLatin1String("sled") || os == QLatin1String("sles"))
        return Platform::Suse;
    if (os == QLatin1String("alpine"))
        return Platform::Alpine;
    if (os == QLatin1String("macos") || os == QLatin1String("osx"))
        return Platform::MacOS;
    if (os == QLatin1String("windows") || os == QLatin1String("winnt"))
        return Platform::Windows;
    return Platform::Debian;
}

Platform currentPlatform()
{
    return platformFor(QSysInfo::productType());
}

QString packageCommand(Platform platform, const QString &package)
{
    switch (platform) {
    case Platform::Fedora:
        return QStringLiteral("sudo dnf install ") + package;
    case Platform::Arch:
        return QStringLiteral("sudo pacman -S ") + package;
    case Platform::Suse:
        return QStringLiteral("sudo zypper install ") + package;
    case Platform::Alpine:
        return QStringLiteral("sudo apk add ") + package;
    case Platform::MacOS:
        return QStringLiteral("brew install ") + package;
    case Platform::Windows:
        return QStringLiteral("winget install ") + package;
    case Platform::Debian:
        break;
    }
    return QStringLiteral("sudo apt install ") + package;
}

QString hunspellEngineCommand(Platform platform)
{
    // El paquete que trae las CABECERAS: con solo la biblioteca (libhunspell-1.7-0),
    // que casi cualquier escritorio ya tiene, el programa compila sin corrector.
    switch (platform) {
    case Platform::Fedora:
    case Platform::Suse:
        return packageCommand(platform, QStringLiteral("hunspell-devel"));
    case Platform::Arch:
    case Platform::MacOS:
        return packageCommand(platform, QStringLiteral("hunspell"));
    case Platform::Alpine:
        return packageCommand(platform, QStringLiteral("hunspell-dev"));
    case Platform::Windows:
        // winget no lo tiene; es el mismo triplete que usa la release.
        return QStringLiteral("vcpkg install hunspell:x64-windows-static-md");
    case Platform::Debian:
        break;
    }
    return packageCommand(platform, QStringLiteral("libhunspell-dev"));
}

QString pandocCommand(Platform platform)
{
    if (platform == Platform::Windows)
        return QStringLiteral("winget install --id JohnMacFarlane.Pandoc");
    return packageCommand(platform, QStringLiteral("pandoc"));
}

QString plantUmlCommand(Platform platform)
{
    if (platform == Platform::Windows)
        return QStringLiteral("choco install plantuml");  // winget no lo tiene
    return packageCommand(platform, QStringLiteral("plantuml"));
}

}  // namespace mdinstall
