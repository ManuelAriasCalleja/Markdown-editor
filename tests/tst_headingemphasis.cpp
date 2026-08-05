#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include "headingemphasis.h"
#include "markdownrender.h"
#include "tableedit.h"

// `QTextDocument::toMarkdown()` LEE el énfasis de un encabezado al cargar pero no lo
// ESCRIBE al guardar: `## uno **negrita** y *cursiva* fin` volvía a disco como
// `## uno negrita y cursiva fin`. Pérdida real de contenido, y solo en encabezados
// (en párrafos, citas, listas y celdas de tabla Qt lo emite bien).
class TestHeadingEmphasis : public QObject
{
    Q_OBJECT

private slots:
    void boldSurvivesRoundTrip();
    void italicSurvivesRoundTrip();
    void strikeSurvivesRoundTrip();
    void boldAndItalicNestTogether();
    void emphasisAtStartOfHeadingSurvives();
    void everyHeadingLevelSurvives();
    void adjacentRunsShareOnePairOfMarks();
    void plainHeadingGainsNoMarks();
    void codeSpanAndLinkAreNotDoubleMarked();
    void formulaInHeadingIsNotItalicised();
    void emphasisInsideALinkKeepsItsMarks();
    void bodyEmphasisIsUnaffected();
    void roundTripIsStable();
    void noSentinelLeaksIntoTheMarkdown();
    void trailingSpaceStaysOutsideTheMarks();
    void nullSafe();
};

namespace {

const auto kFeat = mdrender::kMarkdownFeatures;

// El camino real: pipeline de carga (protect + renderPasses) y serialización canónica.
QString roundTrip(const QString &md)
{
    QTextDocument doc;
    doc.setMarkdown(mdrender::protect(md), kFeat);
    mdrender::renderPasses(&doc);
    return mdtable::documentMarkdown(&doc).trimmed();
}

}  // namespace

// El caso que se reportó.
void TestHeadingEmphasis::boldSurvivesRoundTrip()
{
    QCOMPARE(roundTrip(QStringLiteral("## uno **negrita** fin\n")),
             QStringLiteral("## uno **negrita** fin"));
}

void TestHeadingEmphasis::italicSurvivesRoundTrip()
{
    QCOMPARE(roundTrip(QStringLiteral("## uno *cursiva* fin\n")),
             QStringLiteral("## uno *cursiva* fin"));
}

void TestHeadingEmphasis::strikeSurvivesRoundTrip()
{
    QCOMPARE(roundTrip(QStringLiteral("## uno ~~tachado~~ fin\n")),
             QStringLiteral("## uno ~~tachado~~ fin"));
}

// Negrita y cursiva sobre el mismo run: un solo par anidado, no dos sueltos.
void TestHeadingEmphasis::boldAndItalicNestTogether()
{
    QCOMPARE(roundTrip(QStringLiteral("## uno ***ambas*** fin\n")),
             QStringLiteral("## uno ***ambas*** fin"));
}

// Si el énfasis ABRE el encabezado, el run del `**` es el primero del bloque y no
// queda ningún fragmento con el formato estructural del que distinguirlo.
void TestHeadingEmphasis::emphasisAtStartOfHeadingSurvives()
{
    QCOMPARE(roundTrip(QStringLiteral("## **al principio** fin\n")),
             QStringLiteral("## **al principio** fin"));
}

void TestHeadingEmphasis::everyHeadingLevelSurvives()
{
    for (int level = 1; level <= 6; ++level) {
        const QString hashes(level, QLatin1Char('#'));
        const QString md = hashes + QStringLiteral(" a **b** c\n");
        QCOMPARE(roundTrip(md), md.trimmed());
    }
}

// Dos fragmentos contiguos con el mismo énfasis (aquí un enlace en negrita seguido de
// más negrita) tienen que compartir un par de marcas: `**x****y**` no es negrita para
// ningún lector de Markdown.
void TestHeadingEmphasis::adjacentRunsShareOnePairOfMarks()
{
    const QString out = roundTrip(
        QStringLiteral("## uno **[enlace](http://e.com) y mas** fin\n"));
    QVERIFY2(!out.contains(QStringLiteral("****")), qPrintable(out));
    QCOMPARE(out, QStringLiteral("## uno **[enlace](http://e.com) y mas** fin"));
}

// El encabezado va entero en negrita por ser encabezado: eso NO es un `**` del fuente
// y no debe aparecer al guardar. El h4 es el nivel delicado (su paso de tamaño es 0).
void TestHeadingEmphasis::plainHeadingGainsNoMarks()
{
    QCOMPARE(roundTrip(QStringLiteral("## solo texto\n")), QStringLiteral("## solo texto"));
    QCOMPARE(roundTrip(QStringLiteral("#### solo texto\n")), QStringLiteral("#### solo texto"));
}

// Código y enlaces sí los emite Qt: no hay que añadirles nada (y el código no puede
// acabar con las marcas DENTRO de los acentos graves).
void TestHeadingEmphasis::codeSpanAndLinkAreNotDoubleMarked()
{
    QCOMPARE(roundTrip(QStringLiteral("## uno `codigo` y [enlace](http://e.com)\n")),
             QStringLiteral("## uno `codigo` y [enlace](http://e.com)"));
}

// Los runs de una fórmula son cursiva: si el paso de los encabezados corriera antes
// que el de las fórmulas, saldrían envueltos en `*…*`.
void TestHeadingEmphasis::formulaInHeadingIsNotItalicised()
{
    const QString out = roundTrip(QStringLiteral("## uno $x^2$ fin\n"));
    QVERIFY2(!out.contains(QLatin1Char('*')), qPrintable(out));
    QCOMPARE(out, QStringLiteral("## uno $x^2$ fin"));
}

// Énfasis sobre PARTE de un enlace. Qt parte ese enlace en dos —`[a ](u)[b](u)`—, y lo
// hace también en un párrafo y ya antes de este arreglo: es cosa suya, no de aquí. Lo
// que sí se comprueba es que el énfasis sobrevive y que la salida es estable.
void TestHeadingEmphasis::emphasisInsideALinkKeepsItsMarks()
{
    const QString once = roundTrip(
        QStringLiteral("## Un [enlace **destacado**](https://ejemplo.com) dentro\n"));
    QVERIFY2(once.contains(QStringLiteral("**[destacado](https://ejemplo.com)**")),
             qPrintable(once));
    QCOMPARE(roundTrip(once + QLatin1Char('\n')), once);
}

// Fuera de los encabezados no se toca nada: ahí Qt ya emitía las marcas.
void TestHeadingEmphasis::bodyEmphasisIsUnaffected()
{
    QCOMPARE(roundTrip(QStringLiteral("Parrafo **negrita** y *cursiva* fin\n")),
             QStringLiteral("Parrafo **negrita** y *cursiva* fin"));
    QCOMPARE(roundTrip(QStringLiteral("> cita **negrita**\n")),
             QStringLiteral("> cita **negrita**"));
}

// Guardar dos veces no duplica las marcas (la marca de carga se conserva y los
// centinelas no dejan rastro en el documento).
void TestHeadingEmphasis::roundTripIsStable()
{
    const QString once = roundTrip(QStringLiteral("## uno **negrita** y *cursiva* fin\n"));
    QCOMPARE(roundTrip(once + QLatin1Char('\n')), once);
}

void TestHeadingEmphasis::noSentinelLeaksIntoTheMarkdown()
{
    const QString out = roundTrip(
        QStringLiteral("## **a** y *b* y ~~c~~\n\ntexto\n\n### **d**\n"));
    for (const QChar &ch : out)
        QVERIFY2(ch.unicode() < 0xE000 || ch.unicode() > 0xF8FF, qPrintable(out));
}

// Un run que acaba en espacio (lo produce, por ejemplo, pegar HTML) no puede llevar la
// marca de cierre tras el espacio: `*a *` no es cursiva para un lector de Markdown.
void TestHeadingEmphasis::trailingSpaceStaysOutsideTheMarks()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    QTextBlockFormat bf;
    bf.setHeadingLevel(2);
    c.setBlockFormat(bf);
    QTextCharFormat italic;
    italic.setFontItalic(true);
    c.insertText(QStringLiteral("cursiva "), italic);
    c.insertText(QStringLiteral("fin"), QTextCharFormat());

    QCOMPARE(mdtable::documentMarkdown(&doc).trimmed(),
             QStringLiteral("## *cursiva* fin"));
}

void TestHeadingEmphasis::nullSafe()
{
    mdheademph::markExplicitBold(nullptr);
    mdheademph::replaceWithSentinels(nullptr);
    QCOMPARE(mdheademph::restoreFromSentinels(QString()), QString());
}

QTEST_MAIN(TestHeadingEmphasis)
#include "tst_headingemphasis.moc"
