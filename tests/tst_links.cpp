#include <QtTest>

#include <QCoreApplication>
#include <QSettings>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>

#include "codehighlighter.h"
#include "themecontroller.h"

// Pruebas de integridad de los enlaces: para que Ctrl+clic pueda abrir un
// enlace, el documento debe conservar su `href` tras cargar el Markdown y tras
// recolorear los enlaces al cambiar de tema.
class TestLinks : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();
    void markdownLinkKeepsHref();
    void recolorPreservesHref();

private:
    static QString firstAnchorHref(const QTextEdit &edit);
};

void TestLinks::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestLinks::cleanup()
{
    QSettings().clear();
}

QString TestLinks::firstAnchorHref(const QTextEdit &edit)
{
    const QTextDocument *doc = edit.document();
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment f = it.fragment();
            if (f.isValid() && f.charFormat().isAnchor())
                return f.charFormat().anchorHref();
        }
    }
    return QString();
}

void TestLinks::markdownLinkKeepsHref()
{
    QTextEdit edit;
    edit.setMarkdown(
        QStringLiteral("Un [enlace de ejemplo](https://www.qt.io) y texto final."));
    QCOMPARE(firstAnchorHref(edit), QStringLiteral("https://www.qt.io"));
}

void TestLinks::recolorPreservesHref()
{
    QTextEdit edit;
    CodeBlockHighlighter hl(edit.document());
    ThemeController theme(&edit, &hl);

    edit.setMarkdown(QStringLiteral("[enlace](https://www.qt.io)"));
    theme.applyTheme(mdtheme::ThemeId::Dark);  // recolorea los enlaces al tema

    // El recoloreo solo cambia el color: el href debe seguir intacto.
    QCOMPARE(firstAnchorHref(edit), QStringLiteral("https://www.qt.io"));
}

QTEST_MAIN(TestLinks)
#include "tst_links.moc"
