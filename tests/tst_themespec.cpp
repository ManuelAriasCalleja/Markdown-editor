#include <QtTest>

#include <QSet>

#include "themespec.h"

using namespace mdtheme;

// Pruebas del catálogo de temas. La prueba estrella es la de contraste: cada
// tema debe garantizar legibilidad fondo/texto. Si alguien añade un tema con
// una combinación de bajo contraste (un gris medio sobre fondo oscuro, p. ej.),
// esta prueba falla y lo impide.
class TestThemeSpec : public QObject
{
    Q_OBJECT

private slots:
    void contrastRatioIsSymmetricAndBounded();
    void everyThemeHasReadableText();
    void everyThemeHasReadableUiElements();
    void catalogIsWellFormed();
    void keyRoundTrip();
    void unknownKeyFallsBack();
    void legacyDarkKeyMapsToDark();
};

void TestThemeSpec::contrastRatioIsSymmetricAndBounded()
{
    // Negro sobre blanco: el máximo de WCAG, 21:1.
    const double bw = contrastRatio(Qt::black, Qt::white);
    QVERIFY(qAbs(bw - 21.0) < 0.1);
    QCOMPARE(contrastRatio(Qt::white, Qt::black), bw);  // simétrico
    QCOMPARE(contrastRatio(Qt::white, Qt::white), 1.0); // mismo color: 1:1
}

void TestThemeSpec::everyThemeHasReadableText()
{
    // Texto de cuerpo: exigimos nivel AAA (>= 7:1), que es el objetivo de
    // legibilidad del proyecto, en sus dos fondos (documento y cromo).
    for (const ThemeSpec &t : allThemes()) {
        const QByteArray name = t.key.toUtf8();
        QVERIFY2(contrastRatio(t.text, t.base) >= 7.0, name + " text/base");
        QVERIFY2(contrastRatio(t.windowText, t.window) >= 7.0,
                 name + " windowText/window");
    }
}

void TestThemeSpec::everyThemeHasReadableUiElements()
{
    // Resto de elementos de interfaz: nivel AA (>= 4.5:1). Cada color se mide
    // contra el fondo sobre el que se dibuja realmente.
    for (const ThemeSpec &t : allThemes()) {
        const QByteArray name = t.key.toUtf8();
        QVERIFY2(contrastRatio(t.textSecondary, t.base) >= 4.5,
                 name + " textSecondary/base");
        QVERIFY2(contrastRatio(t.textSecondary, t.window) >= 4.5,
                 name + " textSecondary/window");
        QVERIFY2(contrastRatio(t.link, t.base) >= 4.5, name + " link/base");
        QVERIFY2(contrastRatio(t.buttonText, t.button) >= 4.5,
                 name + " buttonText/button");
        QVERIFY2(contrastRatio(t.highlightedText, t.highlight) >= 4.5,
                 name + " highlightedText/highlight");
        QVERIFY2(contrastRatio(t.tooltipText, t.tooltipBase) >= 4.5,
                 name + " tooltipText/tooltipBase");
    }
}

void TestThemeSpec::catalogIsWellFormed()
{
    const QList<ThemeSpec> &themes = allThemes();
    QCOMPARE(themes.size(), 6);

    QSet<QString> keys;
    for (const ThemeSpec &t : themes) {
        QVERIFY2(!t.key.isEmpty(), "clave vacía");
        QVERIFY2(!t.displayName.isEmpty(), "nombre vacío");
        QVERIFY2(!keys.contains(t.key), t.key.toUtf8());  // claves únicas
        keys.insert(t.key);
    }

    // Los temas oscuros declarados como tal y los claros como tal.
    QVERIFY(specFor(ThemeId::Light).isDark == false);
    QVERIFY(specFor(ThemeId::GitHubLight).isDark == false);
    QVERIFY(specFor(ThemeId::Dark).isDark);
    QVERIFY(specFor(ThemeId::GitHubDark).isDark);
    QVERIFY(specFor(ThemeId::Monokai).isDark);
    QVERIFY(specFor(ThemeId::HighContrast).isDark);
}

void TestThemeSpec::keyRoundTrip()
{
    for (const ThemeSpec &t : allThemes()) {
        QCOMPARE(keyForId(t.id), t.key);
        QCOMPARE(idFromKey(t.key, ThemeId::Light), t.id);
    }
}

void TestThemeSpec::unknownKeyFallsBack()
{
    QCOMPARE(idFromKey(QStringLiteral("no-existe"), ThemeId::Monokai),
             ThemeId::Monokai);
    QCOMPARE(idFromKey(QString(), ThemeId::Dark), ThemeId::Dark);
}

void TestThemeSpec::legacyDarkKeyMapsToDark()
{
    // La migración de AppSettings reutilizará estas claves: "dark"/"light".
    QCOMPARE(idFromKey(QStringLiteral("dark"), ThemeId::Light), ThemeId::Dark);
    QCOMPARE(idFromKey(QStringLiteral("light"), ThemeId::Dark), ThemeId::Light);
}

QTEST_APPLESS_MAIN(TestThemeSpec)
#include "tst_themespec.moc"
