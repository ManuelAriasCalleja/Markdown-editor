#include <QtTest>

#include <QTextBlock>
#include <QTextEdit>

#include "admonitions.h"
#include "markdownrender.h"
#include "mathblocks.h"

// Pipeline único de carga: comprueba que aplica las tres pasadas de render
// (fórmulas, notas al pie, admoniciones) sobre el documento.
class TestMarkdownRender : public QObject
{
    Q_OBJECT
private slots:
    void protectWrapsMathAndFootnotes();
    void rendersMathFootnotesAndAdmonitions();
    void nullSafe();
};

void TestMarkdownRender::protectWrapsMathAndFootnotes()
{
    // El protector envuelve las fórmulas para que md4c no se coma `_`/`*`.
    const QString out = mdrender::protect(QStringLiteral("Una $x_i$ y [^1]: nota"));
    QVERIFY(out != QStringLiteral("Una $x_i$ y [^1]: nota"));  // algo se ha protegido
}

void TestMarkdownRender::rendersMathFootnotesAndAdmonitions()
{
    QTextEdit edit;
    mdrender::setMarkdownWithExtensions(
        &edit,
        QStringLiteral("Fórmula $x^2$ y nota[^1]\n\n"
                       "> [!NOTE]\n>\n> Aviso.\n\n[^1]: la definición\n"));
    QTextDocument *doc = edit.document();

    // Math: hay al menos un fragmento marcado como fórmula.
    bool hasMath = false;
    for (QTextBlock b = doc->begin(); b != doc->end() && !hasMath; b = b.next())
        for (auto it = b.begin(); it != b.end(); ++it)
            if (it.fragment().charFormat().boolProperty(mdmath::IsMathProperty)) {
                hasMath = true;
                break;
            }
    QVERIFY(hasMath);

    // Footnote: la referencia [^1] quedó en superíndice.
    bool hasSuper = false;
    for (QTextBlock b = doc->begin(); b != doc->end() && !hasSuper; b = b.next())
        for (auto it = b.begin(); it != b.end(); ++it)
            if (it.fragment().charFormat().verticalAlignment()
                == QTextCharFormat::AlignSuperScript) {
                hasSuper = true;
                break;
            }
    QVERIFY(hasSuper);

    // Admonición: el bloque del marcador tiene fondo (estilo de callout aplicado).
    bool hasCalloutBg = false;
    for (QTextBlock b = doc->begin(); b != doc->end(); b = b.next())
        if (!mdadmonition::markerKeyword(b.text()).isEmpty()
            && b.blockFormat().background().style() != Qt::NoBrush) {
            hasCalloutBg = true;
            break;
        }
    QVERIFY(hasCalloutBg);
}

void TestMarkdownRender::nullSafe()
{
    mdrender::setMarkdownWithExtensions(nullptr, QStringLiteral("x"));  // no crash
    mdrender::renderPasses(nullptr);
    QVERIFY(true);
}

QTEST_MAIN(TestMarkdownRender)
#include "tst_markdownrender.moc"
