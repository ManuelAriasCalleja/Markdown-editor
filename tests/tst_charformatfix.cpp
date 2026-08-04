#include <QtTest>

#include <QFont>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include "charformatfix.h"
#include "markdownrender.h"
#include "tableedit.h"

// mdcharfix repara dos defectos del importador de Markdown de Qt:
//   1. un encabezado pierde tamaño y negrita en cuanto lleva un span en línea
//      (el span y TODO lo que va detrás se quedan con el formato del cuerpo);
//   2. los runs de código llevan el tamaño de fuente clavado en absoluto, así que
//      no siguen al zoom (que cambia la fuente por defecto del documento).
// Ninguno de los dos puede cambiar el Markdown resultante.
class TestCharFormatFix : public QObject
{
    Q_OBJECT

private slots:
    void headingRunAfterCodeSpanKeepsHeadingFormat();
    void codeSpanInHeadingIsSizedAsHeading();
    void headingStartingWithSpanIsRepaired();
    void everyHeadingLevelGetsItsStep();
    void boldItalicAndLinkInHeadingSurvive();
    void codeFollowsDocumentFontSize();
    void codeKeepsMonospaceIdentity();
    void bodyTextIsNotTouched();
    void roundTripIsUnchanged();
    void isIdempotent();
    void nullSafe();
    void appliedByRenderPipeline();
};

namespace {

const auto kFeat = mdrender::kMarkdownFeatures;

// Carga el Markdown en un documento con el tamaño de cuerpo dado, ya reparado.
void loadFixed(QTextDocument *doc, const QString &md, qreal bodyPointSize = 9.0)
{
    QFont f = doc->defaultFont();
    f.setPointSizeF(bodyPointSize);
    doc->setDefaultFont(f);
    doc->setMarkdown(md, kFeat);
    mdcharfix::apply(doc);
}

// El fragmento cuyo texto empieza por `prefix` (los fragmentos conservan los
// espacios que los separan, así que se busca por prefijo, no por igualdad).
QTextCharFormat formatOf(const QTextDocument &doc, const QString &prefix)
{
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (frag.isValid() && frag.text().trimmed().startsWith(prefix))
                return frag.charFormat();
        }
    }
    return {};
}

int adjustmentOf(const QTextCharFormat &cf)
{
    return cf.intProperty(QTextFormat::FontSizeAdjustment);
}

}  // namespace

// El caso que se veía en pantalla: `## … los `module-info` las vigilan` salía con
// el encabezado partido en tres tamaños distintos a partir del code span.
void TestCharFormatFix::headingRunAfterCodeSpanKeepsHeadingFormat()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("## ni los `module-info` las vigilan\n"), kFeat);

    // Sin reparar, Qt deja el resto del encabezado a cuerpo de texto y sin negrita.
    const QTextCharFormat before = formatOf(doc, QStringLiteral("las vigilan"));
    QCOMPARE(before.fontWeight(), int(QFont::Normal));
    QVERIFY(!before.hasProperty(QTextFormat::FontSizeAdjustment));

    mdcharfix::repairHeadingRuns(&doc);

    const QTextCharFormat after = formatOf(doc, QStringLiteral("las vigilan"));
    QCOMPARE(after.fontWeight(), int(QFont::Bold));
    QCOMPARE(adjustmentOf(after), 2);  // h2
}

void TestCharFormatFix::codeSpanInHeadingIsSizedAsHeading()
{
    QTextDocument doc;
    loadFixed(&doc, QStringLiteral("## ni los `module-info` las vigilan\n"));

    const QTextCharFormat code = formatOf(doc, QStringLiteral("module-info"));
    QVERIFY(code.fontFixedPitch());                                 // sigue siendo código
    QVERIFY(!code.hasProperty(QTextFormat::FontPointSize));         // sin tamaño clavado
    QCOMPARE(adjustmentOf(code), 2);                                // al tamaño del h2
    QCOMPARE(code.fontWeight(), int(QFont::Bold));
}

// Si el encabezado EMPIEZA por un span, no queda ningún fragmento bien formateado
// del que copiar: el formato hay que derivarlo del nivel del bloque.
void TestCharFormatFix::headingStartingWithSpanIsRepaired()
{
    QTextDocument doc;
    loadFixed(&doc, QStringLiteral("### `code` al principio\n"));

    for (const QString &prefix : {QStringLiteral("code"), QStringLiteral("al principio")}) {
        const QTextCharFormat cf = formatOf(doc, prefix);
        QCOMPARE(adjustmentOf(cf), 1);  // h3
        QCOMPARE(cf.fontWeight(), int(QFont::Bold));
    }
}

void TestCharFormatFix::everyHeadingLevelGetsItsStep()
{
    QTextDocument doc;
    loadFixed(&doc,
              QStringLiteral("# a `c` uno\n\n## b `c` dos\n\n### c `c` tres\n\n"
                             "#### d `c` cuatro\n\n##### e `c` cinco\n\n###### f `c` seis\n"));

    const QStringList tails = {QStringLiteral("uno"),    QStringLiteral("dos"),
                               QStringLiteral("tres"),   QStringLiteral("cuatro"),
                               QStringLiteral("cinco"),  QStringLiteral("seis")};
    for (int level = 1; level <= 6; ++level) {
        const QTextCharFormat cf = formatOf(doc, tails.at(level - 1));
        QCOMPARE(adjustmentOf(cf), 4 - level);
        QCOMPARE(cf.fontWeight(), int(QFont::Bold));
    }
}

// La reparación repone tamaño y negrita, pero no pisa lo demás: la cursiva de un
// span y la condición de enlace siguen ahí.
void TestCharFormatFix::boldItalicAndLinkInHeadingSurvive()
{
    QTextDocument doc;
    loadFixed(&doc, QStringLiteral("## uno *dos* y [enlace](http://ejemplo.org) fin\n"));

    const QTextCharFormat italic = formatOf(doc, QStringLiteral("dos"));
    QVERIFY(italic.fontItalic());
    QCOMPARE(adjustmentOf(italic), 2);

    const QTextCharFormat link = formatOf(doc, QStringLiteral("enlace"));
    QVERIFY(link.isAnchor());
    QCOMPARE(link.anchorHref(), QStringLiteral("http://ejemplo.org"));
    QCOMPARE(adjustmentOf(link), 2);
    QCOMPARE(link.fontWeight(), int(QFont::Bold));
}

// El zoom del editor cambia la fuente POR DEFECTO del documento; un tamaño absoluto
// en el run de código lo ignoraría y el código se quedaría anclado al de carga.
void TestCharFormatFix::codeFollowsDocumentFontSize()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("texto `code` fin\n\n```\nbloque\n```\n"), kFeat);

    // Qt clava el tamaño de importación tanto en el span como en el bloque vallado.
    QVERIFY(formatOf(doc, QStringLiteral("code")).hasProperty(QTextFormat::FontPointSize));
    QVERIFY(formatOf(doc, QStringLiteral("bloque")).hasProperty(QTextFormat::FontPointSize));

    mdcharfix::unpinCodeFontSize(&doc);

    QVERIFY(!formatOf(doc, QStringLiteral("code")).hasProperty(QTextFormat::FontPointSize));
    QVERIFY(!formatOf(doc, QStringLiteral("bloque")).hasProperty(QTextFormat::FontPointSize));
}

// Lo que reconoce al código (familia monospace + fixed pitch) es justo lo que miran
// el resaltado, el corrector y la serialización a backticks: no se toca.
void TestCharFormatFix::codeKeepsMonospaceIdentity()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("texto `code` fin\n"), kFeat);
    const QStringList families = formatOf(doc, QStringLiteral("code")).fontFamilies().toStringList();

    mdcharfix::apply(&doc);

    const QTextCharFormat cf = formatOf(doc, QStringLiteral("code"));
    QVERIFY(cf.fontFixedPitch());
    QCOMPARE(cf.fontFamilies().toStringList(), families);
}

void TestCharFormatFix::bodyTextIsNotTouched()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("prosa **negrita** y *cursiva*\n"), kFeat);
    const QTextCharFormat before = formatOf(doc, QStringLiteral("prosa"));

    mdcharfix::apply(&doc);

    const QTextCharFormat after = formatOf(doc, QStringLiteral("prosa"));
    QCOMPARE(after.fontWeight(), before.fontWeight());
    QVERIFY(!after.hasProperty(QTextFormat::FontSizeAdjustment));
    QCOMPARE(formatOf(doc, QStringLiteral("negrita")).fontWeight(), int(QFont::Bold));
    QVERIFY(formatOf(doc, QStringLiteral("cursiva")).fontItalic());
}

// Lo esencial: es una reparación de PRESENTACIÓN. El Markdown que se guarda no
// puede cambiar (ni ganar `**` en los encabezados, ni perder los backticks).
void TestCharFormatFix::roundTripIsUnchanged()
{
    const QString md = QStringLiteral(
        "# Uno `code` dos\n\n"
        "## ni los `module-info` las vigilan\n\n"
        "### `code` al principio\n\n"
        "Prosa con `code` y **negrita**.\n\n"
        "```cpp\nint main() {}\n```\n");

    QTextDocument plain;
    plain.setMarkdown(md, kFeat);
    const QString before = mdtable::documentMarkdown(&plain);

    QTextDocument fixed;
    fixed.setMarkdown(md, kFeat);
    mdcharfix::apply(&fixed);

    QCOMPARE(mdtable::documentMarkdown(&fixed), before);
    QVERIFY(before.contains(QStringLiteral("`module-info`")));  // sigue siendo código
}

void TestCharFormatFix::isIdempotent()
{
    QTextDocument doc;
    loadFixed(&doc, QStringLiteral("## ni los `module-info` las vigilan\n"));
    const QString once = mdtable::documentMarkdown(&doc);
    const int revision = doc.revision();

    mdcharfix::apply(&doc);

    // Sin nada que reparar no se toca el documento (no ensucia el «modificado»).
    QCOMPARE(doc.revision(), revision);
    QCOMPARE(mdtable::documentMarkdown(&doc), once);
}

void TestCharFormatFix::nullSafe()
{
    mdcharfix::apply(nullptr);
    mdcharfix::repairHeadingRuns(nullptr);
    mdcharfix::unpinCodeFontSize(nullptr);
}

// El arreglo llega por el pipeline de carga, que es por donde entra todo el
// Markdown del editor (DocumentIo::load, loadFromString y setBodyMarkdown).
void TestCharFormatFix::appliedByRenderPipeline()
{
    QTextDocument doc;
    doc.setMarkdown(mdrender::protect(QStringLiteral("## ni los `module-info` las vigilan\n")),
                    kFeat);
    mdrender::renderPasses(&doc);

    const QTextCharFormat tail = formatOf(doc, QStringLiteral("las vigilan"));
    QCOMPARE(tail.fontWeight(), int(QFont::Bold));
    QCOMPARE(adjustmentOf(tail), 2);
}

QTEST_MAIN(TestCharFormatFix)
#include "tst_charformatfix.moc"
