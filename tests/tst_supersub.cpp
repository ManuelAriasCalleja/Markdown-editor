#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include "markdownrender.h"
#include "supsub.h"
#include "tableedit.h"

// Pruebas del super/subíndice de texto (mdsupsub): protección, y sobre todo el
// ROUND-TRIP fiel (carga + render + serialización) que es lo delicado de #16.
class TestSuperSub : public QObject
{
    Q_OBJECT
private slots:
    void protectWrapsSingleTildeNotDouble();
    void protectWrapsCaret();
    void roundTripSuperscript();
    void roundTripSubscript();
    void roundTripMixedWithStrikethrough();
    void rendersAsVerticalAlign();
    void leavesCodeSpanLiteral();

private:
    // Carga `md` como lo hace el editor (protect + setMarkdown + renderPasses) y lo
    // vuelve a serializar como al guardar (documentMarkdown).
    static QString roundtrip(const QString &md)
    {
        QTextDocument doc;
        doc.setMarkdown(mdrender::protect(md), mdrender::kMarkdownFeatures);
        mdrender::renderPasses(&doc);
        return mdtable::documentMarkdown(&doc).trimmed();
    }
};

void TestSuperSub::protectWrapsSingleTildeNotDouble()
{
    // La `~` simple se protege (deja de parecer tachado); la doble `~~` no se toca.
    const QString p = mdsupsub::protect(QStringLiteral("H~2~O y ~~tachado~~"));
    QVERIFY(!p.contains(QStringLiteral("~2~")));         // la subíndice quedó envuelta
    QVERIFY(p.contains(QStringLiteral("~~tachado~~")));  // el tachado, intacto
}

void TestSuperSub::protectWrapsCaret()
{
    const QString p = mdsupsub::protect(QStringLiteral("x^2^"));
    QVERIFY(!p.contains(QStringLiteral("^2^")));
}

void TestSuperSub::roundTripSuperscript()
{
    QCOMPARE(roundtrip(QStringLiteral("x^2^ + y^2^")), QStringLiteral("x^2^ + y^2^"));
}

void TestSuperSub::roundTripSubscript()
{
    QCOMPARE(roundtrip(QStringLiteral("H~2~O")), QStringLiteral("H~2~O"));
}

void TestSuperSub::roundTripMixedWithStrikethrough()
{
    // El subíndice y el tachado GFM conviven sin pisarse.
    const QString out = roundtrip(QStringLiteral("a~1~ y ~~fuera~~"));
    QVERIFY(out.contains(QStringLiteral("a~1~")));
    QVERIFY(out.contains(QStringLiteral("~~fuera~~")));
}

void TestSuperSub::rendersAsVerticalAlign()
{
    QTextDocument doc;
    doc.setMarkdown(mdrender::protect(QStringLiteral("x^2^")), mdrender::kMarkdownFeatures);
    mdrender::renderPasses(&doc);
    // Debe existir un fragmento con AlignSuperScript + SupSubProperty y texto "2",
    // y NO deben quedar los `^` ni centinelas.
    bool found = false;
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment f = it.fragment();
            if (f.isValid() && f.charFormat().boolProperty(mdsupsub::SupSubProperty)
                && f.charFormat().verticalAlignment() == QTextCharFormat::AlignSuperScript) {
                found = true;
                QCOMPARE(f.text(), QStringLiteral("2"));
            }
        }
    QVERIFY(found);
    QVERIFY(!doc.toPlainText().contains(QLatin1Char('^')));  // marcador oculto
}

void TestSuperSub::leavesCodeSpanLiteral()
{
    // Dentro de código en línea, `^x^` se queda literal (no se convierte en super).
    const QString out = roundtrip(QStringLiteral("usa `x^2^` aqui"));
    QVERIFY(out.contains(QStringLiteral("`x^2^`")));
}

QTEST_MAIN(TestSuperSub)
#include "tst_supersub.moc"
