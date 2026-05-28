#include <QtTest>

#include <QApplication>
#include <QColor>
#include <QCoreApplication>
#include <QPalette>
#include <QSettings>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>

#include "appsettings.h"
#include "codehighlighter.h"
#include "themecontroller.h"

// Pruebas de ThemeController: cambio de tema (señal/estado) y recoloreado de
// enlaces. Usa un almacén de QSettings de prueba para no tocar los ajustes
// reales del usuario (applyTheme persiste la elección).
class TestThemeController : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void appliesDarkAndEmitsSignal();
    void recolorsLinksToThemeColor();
    void warmLightTogglePersists();
    void warmLightOffLeavesBaseUntinted();

private:
    static QColor firstAnchorColor(QTextEdit &edit);
};

void TestThemeController::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestThemeController::cleanup()
{
    QSettings().clear();
}

QColor TestThemeController::firstAnchorColor(QTextEdit &edit)
{
    QTextDocument *doc = edit.document();
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.isValid() && frag.charFormat().isAnchor())
                return frag.charFormat().foreground().color();
        }
    }
    return QColor();
}

void TestThemeController::appliesDarkAndEmitsSignal()
{
    QTextEdit edit;
    CodeBlockHighlighter hl(edit.document());
    ThemeController theme(&edit, &hl);

    QSignalSpy spy(&theme, &ThemeController::themeChanged);
    QVERIFY(!theme.isDark());

    theme.applyTheme(mdtheme::ThemeId::Dark);
    QVERIFY(theme.isDark());
    QCOMPARE(theme.currentTheme(), mdtheme::ThemeId::Dark);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().value<mdtheme::ThemeId>(), mdtheme::ThemeId::Dark);
}

void TestThemeController::recolorsLinksToThemeColor()
{
    QTextEdit edit;
    CodeBlockHighlighter hl(edit.document());
    ThemeController theme(&edit, &hl);

    edit.setMarkdown(QStringLiteral("[enlace](https://example.com)"));
    theme.applyTheme(mdtheme::ThemeId::Dark);  // enlaces al color de enlace del tema

    QCOMPARE(firstAnchorColor(edit),
             mdtheme::specFor(mdtheme::ThemeId::Dark).link);
}

void TestThemeController::warmLightTogglePersists()
{
    QTextEdit edit;
    CodeBlockHighlighter hl(edit.document());
    ThemeController theme(&edit, &hl);

    QVERIFY(theme.isWarmLight());  // activada por defecto
    theme.setWarmLight(false);
    QVERIFY(!theme.isWarmLight());
    QVERIFY(!AppSettings::warmLight());  // se persiste

    theme.setWarmLight(true);
    QVERIFY(theme.isWarmLight());
    QVERIFY(AppSettings::warmLight());
}

void TestThemeController::warmLightOffLeavesBaseUntinted()
{
    QTextEdit edit;
    CodeBlockHighlighter hl(edit.document());
    ThemeController theme(&edit, &hl);

    // Con la luz cálida apagada el fondo oscuro es exacto, sea la hora que sea.
    theme.setWarmLight(false);
    theme.applyTheme(mdtheme::ThemeId::Dark);
    QCOMPARE(qApp->palette().color(QPalette::Base),
             mdtheme::specFor(mdtheme::ThemeId::Dark).base);
}

QTEST_MAIN(TestThemeController)
#include "tst_themecontroller.moc"
