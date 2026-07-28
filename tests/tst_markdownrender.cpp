#include <QtTest>

#include <QImage>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextFragment>

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
    void protectAlwaysEndsWithNewline();
    void thematicBreakSandwichSurvivesLoad();
    void rendersMathFootnotesAndAdmonitions();
    void nullSafe();
    void imageMarkdownNeverLeavesAltEmpty();
    void imageMarkdownQuotesAwkwardDestinations();
    void imageWithAltSurvivesSetMarkdown();
};

void TestMarkdownRender::protectWrapsMathAndFootnotes()
{
    // El protector envuelve las fórmulas para que md4c no se coma `_`/`*`.
    const QString out = mdrender::protect(QStringLiteral("Una $x_i$ y [^1]: nota"));
    QVERIFY(out != QStringLiteral("Una $x_i$ y [^1]: nota"));  // algo se ha protegido
}

// El salto de línea final que añade `protect` es un RODEO a un fallo de Qt:
// `setMarkdown` mata el proceso en Windows (0xC0000005) con un documento que
// empieza y acaba por regla temática `---` sin salto final —el caso mínimo son
// seis caracteres, `---\n\n---`—, y basta un `\n` al final para esquivarlo.
// Lo cazó tst_roundtripfuzz (caso 281); ver el comentario en markdownrender.cpp.
void TestMarkdownRender::protectAlwaysEndsWithNewline()
{
    QVERIFY(mdrender::protect(QStringLiteral("---\n\n---")).endsWith(QLatin1Char('\n')));
    QVERIFY(mdrender::protect(QStringLiteral("texto")).endsWith(QLatin1Char('\n')));
    QVERIFY(mdrender::protect(QString()).endsWith(QLatin1Char('\n')));

    // No se acumulan saltos: lo que ya termina en `\n` se deja como está.
    QCOMPARE(mdrender::protect(QStringLiteral("texto\n")), QStringLiteral("texto\n"));
}

// El documento del caso 281 del fuzzer, reducido a lo que dispara el fallo. En
// Windows sin el rodeo esto no falla: mata el proceso, así que llegar al final
// del test ES la comprobación. En Linux/macOS pasa igual (el fallo es de
// Windows), pero el caso se queda aquí para que nadie retire el rodeo sin más.
void TestMarkdownRender::thematicBreakSandwichSurvivesLoad()
{
    for (const QString &md : {QStringLiteral("---\n\n---"),
                              QStringLiteral("---\n\nx\n\n---"),
                              QStringLiteral("---\n\n- [ ] tarea\n\n!bang `a&b`\n\n---")}) {
        QTextEdit edit;
        mdrender::setMarkdownWithExtensions(&edit, md);
        QVERIFY(edit.document() != nullptr);
    }
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

// `QTextDocument::setMarkdown` DESCARTA `![](ruta)`: con el rótulo vacío no
// inserta ni la imagen ni un hueco, así que la imagen desaparece al reabrir el
// documento. imageMarkdown rellena ese rótulo con el nombre del fichero.
void TestMarkdownRender::imageMarkdownNeverLeavesAltEmpty()
{
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("fotos/gato.png")),
             QStringLiteral("![gato](fotos/gato.png)"));
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("fotos/gato.png"), QString()),
             QStringLiteral("![gato](fotos/gato.png)"));
    // El rótulo del usuario manda, y sus corchetes se escapan (cerrarían el enlace).
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("x.png"), QStringLiteral("Mi [gato]")),
             QStringLiteral("![Mi \\[gato\\]](x.png)"));
    // Sin nombre de fichero aprovechable, una palabra cualquiera antes que nada.
    QVERIFY(!mdrender::imageAltFallback(QStringLiteral("https://ejemplo.com/")).isEmpty());
}

void TestMarkdownRender::imageMarkdownQuotesAwkwardDestinations()
{
    // Un espacio o un paréntesis cortan `![](...)`: el destino va entre `<...>`.
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("Mis fotos/gato.png")),
             QStringLiteral("![gato](<Mis fotos/gato.png>)"));
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("copia (2)/gato.png")),
             QStringLiteral("![gato](<copia (2)/gato.png>)"));
    // Y un `<`/`>` en la ruta rompería ese envoltorio: se codifica.
    QCOMPARE(mdrender::imageMarkdown(QStringLiteral("a<b/gato.png")),
             QStringLiteral("![gato](a%3Cb/gato.png)"));
}

// La razón de ser de todo lo anterior, comprobada de punta a punta: lo que produce
// imageMarkdown sobrevive al parseo, y el `![](...)` pelado no.
void TestMarkdownRender::imageWithAltSurvivesSetMarkdown()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString png = dir.filePath(QStringLiteral("gato.png"));
    QImage img(4, 4, QImage::Format_RGB32);
    img.fill(qRgb(10, 20, 30));
    QVERIFY(img.save(png, "PNG"));

    const auto imageCount = [](const QString &markdown) {
        QTextDocument doc;
        doc.setMarkdown(mdrender::protect(markdown), mdrender::kMarkdownFeatures);
        int n = 0;
        for (QTextBlock b = doc.begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); it != b.end(); ++it)
                if (it.fragment().charFormat().isImageFormat())
                    ++n;
        return n;
    };

    QCOMPARE(imageCount(mdrender::imageMarkdown(png)), 1);
    // El contraejemplo: sin rótulo, Qt no inserta nada (por eso existe la función).
    QCOMPARE(imageCount(QStringLiteral("![](%1)").arg(png)), 0);
}

QTEST_MAIN(TestMarkdownRender)
#include "tst_markdownrender.moc"
