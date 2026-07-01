#include <QtTest>

#include <QTextBlock>
#include <QTextDocument>

#include "admonitions.h"
#include "markdownrender.h"
#include "tableedit.h"
#include "typography.h"

// Pruebas de la pasada de tipografía del documento (mdtypography): que dé ritmo a los
// encabezados, panel al código y tinte a las citas, sin tocar el round-trip ni el
// estado «modificado», y sin doble-tintar las admoniciones ni las tablas.
class TestTypography : public QObject
{
    Q_OBJECT
private slots:
    void headingMarginsDecreaseWithLevel();
    void tintsAreTranslucent();
    void quoteBarColorPicksAccentOrNeutral();
    void codeBlocksGetBackground();
    void quotesGetBackground();
    void admonitionsNotDoubleTinted();
    void headingsGetVerticalRhythm();
    void tablesLeftUntouched();
    void preservesModifiedFlag();
    void roundTripUnaffected();

private:
    // Carga `md` como al abrir (protect + setMarkdown + renderPasses) y aplica la
    // tipografía, como hace EditorStack tras cargar.
    static void load(QTextDocument &doc, const QString &md)
    {
        doc.setMarkdown(mdrender::protect(md), mdrender::kMarkdownFeatures);
        mdrender::renderPasses(&doc);
        mdtypography::apply(&doc);
    }

    // Primer bloque cuyo texto empieza por `prefix`.
    static QTextBlock blockStarting(QTextDocument &doc, const QString &prefix)
    {
        for (QTextBlock b = doc.begin(); b.isValid(); b = b.next())
            if (b.text().startsWith(prefix))
                return b;
        return {};
    }
};

void TestTypography::headingMarginsDecreaseWithLevel()
{
    // Más aire sobre un H1 que sobre un H3 (ritmo).
    QVERIFY(mdtypography::headingTopMargin(1) > mdtypography::headingTopMargin(3));
    QVERIFY(mdtypography::headingTopMargin(3) >= mdtypography::headingTopMargin(6));
    QVERIFY(mdtypography::headingTopMargin(6) > 0);
}

void TestTypography::tintsAreTranslucent()
{
    // El tinte debe ser translúcido para componer sobre claro y oscuro sin re-derivar.
    QVERIFY(mdtypography::codeBackground().alpha() > 0);
    QVERIFY(mdtypography::codeBackground().alpha() < 255);
    QVERIFY(mdtypography::quoteBackground().alpha() < mdtypography::codeBackground().alpha());
}

void TestTypography::quoteBarColorPicksAccentOrNeutral()
{
    const QColor textColor(20, 20, 20);
    // Fondo de acento (admonición NOTE, azulado con alpha): barra de acento OPACA.
    const QColor accentBar =
        mdtypography::quoteBarColor(QColor(47, 109, 222, 28), textColor);
    QCOMPARE(accentBar.red(), 47);
    QCOMPARE(accentBar.green(), 109);
    QCOMPARE(accentBar.blue(), 222);
    QCOMPARE(accentBar.alpha(), 255);
    // Fondo gris neutro (cita normal) o inválido: barra derivada del texto, translúcida.
    for (const QColor bg : {mdtypography::quoteBackground(), QColor()}) {
        const QColor neutral = mdtypography::quoteBarColor(bg, textColor);
        QCOMPARE(neutral.red(), textColor.red());
        QCOMPARE(neutral.green(), textColor.green());
        QCOMPARE(neutral.blue(), textColor.blue());
        QVERIFY(neutral.alpha() < 255);
    }
}

void TestTypography::codeBlocksGetBackground()
{
    QTextDocument doc;
    load(doc, QStringLiteral("Texto\n\n```\ncodigo\n```\n"));
    bool foundCode = false;
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().hasProperty(QTextFormat::BlockCodeFence)) {
            foundCode = true;
            QVERIFY2(b.blockFormat().background().style() != Qt::NoBrush,
                     "el bloque de código debería tener fondo");
        }
    }
    QVERIFY(foundCode);
}

void TestTypography::quotesGetBackground()
{
    QTextDocument doc;
    load(doc, QStringLiteral("> una cita normal\n"));
    const QTextBlock q = blockStarting(doc, QStringLiteral("una cita"));
    QVERIFY(q.isValid());
    QVERIFY(q.blockFormat().intProperty(QTextFormat::BlockQuoteLevel) > 0);
    QVERIFY2(q.blockFormat().background().style() != Qt::NoBrush,
             "la cita normal debería tener fondo tintado");
}

void TestTypography::admonitionsNotDoubleTinted()
{
    // Una admonición ya lleva su tinte de acento (mdadmonition); la tipografía NO debe
    // volver a tintarla (se sumarían). Comprobamos que su fondo sigue siendo el del
    // acento y no el gris de la cita normal.
    QTextDocument doc;
    load(doc, QStringLiteral("> [!NOTE]\n>\n> contenido\n"));
    const QTextBlock marker = blockStarting(doc, QStringLiteral("[!NOTE]"));
    QVERIFY(marker.isValid());
    const QColor bg = marker.blockFormat().background().color();
    // El acento de NOTE es azulado (mdadmonition), no un gris neutro.
    QVERIFY2(bg != mdtypography::quoteBackground(),
             "la admonición no debe llevar el tinte gris de cita normal");
    QVERIFY(bg.blue() > bg.red());  // tinte azulado del acento NOTE
}

void TestTypography::headingsGetVerticalRhythm()
{
    QTextDocument doc;
    load(doc, QStringLiteral("# Titulo\n\nParrafo\n"));
    const QTextBlock h = blockStarting(doc, QStringLiteral("Titulo"));
    QVERIFY(h.isValid());
    QCOMPARE(h.blockFormat().headingLevel(), 1);
    QVERIFY(h.blockFormat().topMargin() > 0);
    QVERIFY(h.blockFormat().bottomMargin() > 0);
}

void TestTypography::tablesLeftUntouched()
{
    // Los bloques dentro de una tabla no reciben márgenes de párrafo (hincharían la
    // celda): su margen inferior debe quedar en 0.
    QTextDocument doc;
    load(doc, QStringLiteral("| a | b |\n|---|---|\n| c | d |\n"));
    bool sawCell = false;
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        QTextCursor c(b);
        if (c.currentTable()) {
            sawCell = true;
            QCOMPARE(b.blockFormat().bottomMargin(), qreal(0));
        }
    }
    QVERIFY(sawCell);
}

void TestTypography::preservesModifiedFlag()
{
    QTextDocument doc;
    doc.setMarkdown(mdrender::protect(QStringLiteral("# Hola\n\nAdios\n")),
                    mdrender::kMarkdownFeatures);
    mdrender::renderPasses(&doc);
    doc.setModified(false);
    mdtypography::apply(&doc);
    QVERIFY2(!doc.isModified(), "la tipografía es presentación pura: no debe ensuciar");
}

void TestTypography::roundTripUnaffected()
{
    // Los márgenes y fondos no los serializa toMarkdown(): el Markdown debe salir igual
    // con y sin la pasada de tipografía.
    const QString md = QStringLiteral("# Titulo\n\nParrafo con texto\n\n> cita\n\n```\ncodigo\n```");
    QTextDocument a;
    a.setMarkdown(mdrender::protect(md), mdrender::kMarkdownFeatures);
    mdrender::renderPasses(&a);
    const QString sinTipografia = mdtable::documentMarkdown(&a).trimmed();

    QTextDocument b;
    load(b, md);
    const QString conTipografia = mdtable::documentMarkdown(&b).trimmed();

    QCOMPARE(conTipografia, sinTipografia);
}

QTEST_MAIN(TestTypography)
#include "tst_typography.moc"
