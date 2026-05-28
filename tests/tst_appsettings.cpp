#include <QtTest>

#include <QCoreApplication>
#include <QSettings>

#include "appsettings.h"

// Pruebas de la fachada AppSettings: ida y vuelta de cada ajuste. Usa un
// almacén de QSettings propio (organización/aplicación de prueba) que se
// limpia, para no tocar los ajustes reales del usuario.
class TestAppSettings : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void darkThemeRoundTrips();
    void darkThemeDefaultsFalse();
    void themeKeyRoundTrips();
    void themeKeyDefaultsLight();
    void themeKeyMigratesFromLegacyDarkBool();
    void themeKeyPrefersNewKeyOverLegacy();
    void warmLightRoundTrips();
    void warmLightDefaultsTrue();
    void zoomLevelRoundTrips();
    void zoomLevelDefaultsZero();
    void languageRoundTrips();
    void languageDefaultsEmpty();
    void geometryRoundTrips();
    void windowStateRoundTrips();
    void recentFilesRoundTrips();
    void lastFileRoundTrips();
    void lastFileDefaultsEmpty();
};

void TestAppSettings::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestAppSettings::cleanup()
{
    QSettings().clear();  // cada prueba parte de cero
}

void TestAppSettings::darkThemeRoundTrips()
{
    AppSettings::setDarkTheme(true);
    QVERIFY(AppSettings::darkTheme());
    AppSettings::setDarkTheme(false);
    QVERIFY(!AppSettings::darkTheme());
}

void TestAppSettings::darkThemeDefaultsFalse()
{
    QVERIFY(!AppSettings::darkTheme());  // sin valor guardado
}

void TestAppSettings::themeKeyRoundTrips()
{
    AppSettings::setThemeKey(QStringLiteral("monokai"));
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("monokai"));
    AppSettings::setThemeKey(QStringLiteral("github-light"));
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("github-light"));
}

void TestAppSettings::themeKeyDefaultsLight()
{
    // Sin valor guardado y sin ajuste antiguo: tema claro.
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("light"));
}

void TestAppSettings::themeKeyMigratesFromLegacyDarkBool()
{
    // Usuario de una versión anterior: solo existe el booleano "darkTheme".
    AppSettings::setDarkTheme(true);
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("dark"));

    QSettings().clear();
    AppSettings::setDarkTheme(false);
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("light"));
}

void TestAppSettings::themeKeyPrefersNewKeyOverLegacy()
{
    // Si ya hay clave nueva, manda sobre el booleano antiguo.
    AppSettings::setDarkTheme(true);
    AppSettings::setThemeKey(QStringLiteral("github-dark"));
    QCOMPARE(AppSettings::themeKey(), QStringLiteral("github-dark"));
}

void TestAppSettings::warmLightRoundTrips()
{
    AppSettings::setWarmLight(false);
    QVERIFY(!AppSettings::warmLight());
    AppSettings::setWarmLight(true);
    QVERIFY(AppSettings::warmLight());
}

void TestAppSettings::warmLightDefaultsTrue()
{
    QVERIFY(AppSettings::warmLight());  // activada por defecto
}

void TestAppSettings::zoomLevelRoundTrips()
{
    AppSettings::setZoomLevel(3);
    QCOMPARE(AppSettings::zoomLevel(), 3);
    AppSettings::setZoomLevel(-2);
    QCOMPARE(AppSettings::zoomLevel(), -2);
}

void TestAppSettings::zoomLevelDefaultsZero()
{
    QCOMPARE(AppSettings::zoomLevel(), 0);  // sin valor guardado
}

void TestAppSettings::languageRoundTrips()
{
    AppSettings::setLanguage(QStringLiteral("en"));
    QCOMPARE(AppSettings::language(), QStringLiteral("en"));
    AppSettings::setLanguage(QString());  // "" = automático
    QVERIFY(AppSettings::language().isEmpty());
}

void TestAppSettings::languageDefaultsEmpty()
{
    QVERIFY(AppSettings::language().isEmpty());  // sin guardar = automático
}

void TestAppSettings::geometryRoundTrips()
{
    // Binario con un byte nulo en medio: comprueba que sobrevive intacto.
    const QByteArray g = QByteArray::fromHex("0102dead00beef7f");
    AppSettings::setWindowGeometry(g);
    QCOMPARE(AppSettings::windowGeometry(), g);
}

void TestAppSettings::windowStateRoundTrips()
{
    const QByteArray s = QByteArrayLiteral("estado-de-barras");
    AppSettings::setWindowState(s);
    QCOMPARE(AppSettings::windowState(), s);
}

void TestAppSettings::recentFilesRoundTrips()
{
    const QStringList files{QStringLiteral("/a/uno.md"),
                            QStringLiteral("/b/dos.md")};
    AppSettings::setRecentFiles(files);
    QCOMPARE(AppSettings::recentFiles(), files);
}

void TestAppSettings::lastFileRoundTrips()
{
    AppSettings::setLastFile(QStringLiteral("/home/u/doc.md"));
    QCOMPARE(AppSettings::lastFile(), QStringLiteral("/home/u/doc.md"));
    AppSettings::setLastFile(QString());  // "" = sin archivo (arranque en blanco)
    QVERIFY(AppSettings::lastFile().isEmpty());
}

void TestAppSettings::lastFileDefaultsEmpty()
{
    QVERIFY(AppSettings::lastFile().isEmpty());  // sin valor guardado
}

QTEST_GUILESS_MAIN(TestAppSettings)
#include "tst_appsettings.moc"
