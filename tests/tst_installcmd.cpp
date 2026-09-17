#include <QtTest>

#include "installcmd.h"

// Pruebas de las órdenes de instalación: un aviso de «falta X» solo sirve si la
// orden que enseña funciona en el sistema del usuario.
class TestInstallCmd : public QObject
{
    Q_OBJECT

private slots:
    void recognisesDistributionFamilies();
    void packageCommandMatchesPackageManager();
    void hunspellEngineAsksForHeaders();
    void toolsHaveCommandOnEveryPlatform();
};

void TestInstallCmd::recognisesDistributionFamilies()
{
    using mdinstall::Platform;
    QCOMPARE(mdinstall::platformFor(QStringLiteral("ubuntu")), Platform::Debian);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("debian")), Platform::Debian);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("fedora")), Platform::Fedora);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("rocky")), Platform::Fedora);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("manjaro")), Platform::Arch);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("opensuse-tumbleweed")), Platform::Suse);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("alpine")), Platform::Alpine);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("macos")), Platform::MacOS);
    QCOMPARE(mdinstall::platformFor(QStringLiteral("windows")), Platform::Windows);
    // Desconocido: Debian, lo más extendido, en vez de ninguna orden.
    QCOMPARE(mdinstall::platformFor(QStringLiteral("loquesea")), Platform::Debian);
}

void TestInstallCmd::packageCommandMatchesPackageManager()
{
    using mdinstall::Platform;
    const QString pkg = QStringLiteral("x");
    QCOMPARE(mdinstall::packageCommand(Platform::Debian, pkg), QStringLiteral("sudo apt install x"));
    QCOMPARE(mdinstall::packageCommand(Platform::Fedora, pkg), QStringLiteral("sudo dnf install x"));
    QCOMPARE(mdinstall::packageCommand(Platform::Arch, pkg), QStringLiteral("sudo pacman -S x"));
    QCOMPARE(mdinstall::packageCommand(Platform::Suse, pkg), QStringLiteral("sudo zypper install x"));
    QCOMPARE(mdinstall::packageCommand(Platform::MacOS, pkg), QStringLiteral("brew install x"));
}

void TestInstallCmd::hunspellEngineAsksForHeaders()
{
    // Tener la biblioteca no basta para compilar con corrector: hace falta el
    // paquete de desarrollo. Mandar a instalar «hunspell» en Debian no arregla nada.
    using mdinstall::Platform;
    QCOMPARE(mdinstall::hunspellEngineCommand(Platform::Debian),
             QStringLiteral("sudo apt install libhunspell-dev"));
    QCOMPARE(mdinstall::hunspellEngineCommand(Platform::Fedora),
             QStringLiteral("sudo dnf install hunspell-devel"));
    QVERIFY(mdinstall::hunspellEngineCommand(Platform::Windows).contains(QStringLiteral("vcpkg")));
}

void TestInstallCmd::toolsHaveCommandOnEveryPlatform()
{
    using mdinstall::Platform;
    for (Platform p : {Platform::Debian, Platform::Fedora, Platform::Arch, Platform::Suse,
                       Platform::Alpine, Platform::MacOS, Platform::Windows}) {
        QVERIFY(mdinstall::pandocCommand(p).contains(QStringLiteral("andoc")));
        QVERIFY(mdinstall::plantUmlCommand(p).contains(QStringLiteral("plantuml")));
        QVERIFY(!mdinstall::hunspellEngineCommand(p).isEmpty());
    }
    QVERIFY(!mdinstall::pandocCommand(Platform::Fedora).contains(QStringLiteral("apt")));
}

QTEST_APPLESS_MAIN(TestInstallCmd)
#include "tst_installcmd.moc"
