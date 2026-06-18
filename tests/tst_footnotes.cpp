#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "footnotes.h"
#include "tableedit.h"

// Notas al pie: funciones puras (localizar referencias/definiciones, autonumerar)
// y la integración con el documento (render como superíndice + round-trip intacto
// + localizar la definición para navegar).
class TestFootnotes : public QObject
{
    Q_OBJECT
private slots:
    void findsReferences();
    void ignoresDefinitionLabelAsReference();
    void findsDefinitions();
    void nextIdSkipsExistingNumbers();
    void nextIdIgnoresNonNumeric();
    void renderMarksReferencesAsSuperscript();
    void renderSkipsCodeSpans();
    void roundTripPreservesFootnotes();
    void roundTripPreservesSingleWordDefinitions();
    void definitionBlockNumberFound();

private:
    // Reproduce la carga real: protege, parsea y renderiza.
    static void loadInto(QTextDocument &doc, const QString &markdown);
};

void TestFootnotes::loadInto(QTextDocument &doc, const QString &markdown)
{
    doc.setMarkdown(mdfootnote::protectFootnotes(markdown));
    mdfootnote::renderFootnotesInDocument(&doc);
}

void TestFootnotes::findsReferences()
{
    const auto refs = mdfootnote::references(QStringLiteral("Hola[^1] mundo[^nota]."));
    QCOMPARE(refs.size(), 2);
    QCOMPARE(refs.at(0).id, QStringLiteral("1"));
    QCOMPARE(refs.at(1).id, QStringLiteral("nota"));
    // El span cubre exactamente `[^1]`.
    QCOMPARE(refs.at(0).length, 4);
}

void TestFootnotes::ignoresDefinitionLabelAsReference()
{
    // `[^1]:` es el rótulo de una definición, no una referencia.
    const auto refs = mdfootnote::references(QStringLiteral("[^1]: definición"));
    QVERIFY(refs.isEmpty());
}

void TestFootnotes::findsDefinitions()
{
    const QString md = QStringLiteral("texto[^1]\n\n[^1]: una def\n  [^2]: con sangría\n");
    const auto defs = mdfootnote::definitions(md);
    QCOMPARE(defs.size(), 2);
    QCOMPARE(defs.at(0).id, QStringLiteral("1"));
    QCOMPARE(defs.at(1).id, QStringLiteral("2"));
}

void TestFootnotes::nextIdSkipsExistingNumbers()
{
    QCOMPARE(mdfootnote::nextId(QStringLiteral("a[^1] b[^3]\n[^3]: x")),
             QStringLiteral("4"));
}

void TestFootnotes::nextIdIgnoresNonNumeric()
{
    QCOMPARE(mdfootnote::nextId(QStringLiteral("solo[^nota] aquí")), QStringLiteral("1"));
    QCOMPARE(mdfootnote::nextId(QStringLiteral("sin notas")), QStringLiteral("1"));
}

void TestFootnotes::renderMarksReferencesAsSuperscript()
{
    QTextDocument doc;
    loadInto(doc, QStringLiteral("Texto con nota[^1] aquí.\n\n[^1]: La def.\n"));

    // El `[` de la referencia debe quedar como superíndice y con la propiedad.
    const QTextBlock first = doc.firstBlock();
    const int idx = first.text().indexOf(QStringLiteral("[^1]"));
    QVERIFY(idx >= 0);
    QTextCursor c(&doc);
    c.setPosition(first.position() + idx + 1);  // formato del carácter '^'
    const QTextCharFormat cf = c.charFormat();
    QCOMPARE(cf.verticalAlignment(), QTextCharFormat::AlignSuperScript);
    QVERIFY(cf.boolProperty(mdfootnote::IsFootnoteRefProperty));
    QCOMPARE(cf.property(mdfootnote::FootnoteIdProperty).toString(), QStringLiteral("1"));
}

void TestFootnotes::renderSkipsCodeSpans()
{
    QTextDocument doc;
    loadInto(doc, QStringLiteral("Código `[^1]` literal.\n"));
    // Dentro de código no se renderiza: el carácter sigue sin la propiedad.
    const QTextBlock first = doc.firstBlock();
    const int idx = first.text().indexOf(QStringLiteral("[^1]"));
    QVERIFY(idx >= 0);
    QTextCursor c(&doc);
    c.setPosition(first.position() + idx + 1);
    QVERIFY(!c.charFormat().boolProperty(mdfootnote::IsFootnoteRefProperty));
}

void TestFootnotes::roundTripPreservesFootnotes()
{
    const QString src = QStringLiteral(
        "Una afirmación[^1] y otra[^2].\n\n"
        "[^1]: Primera definición con espacios.\n\n"
        "[^2]: Segunda con *énfasis*.\n");
    QTextDocument doc;
    loadInto(doc, src);

    const QString md = mdtable::documentMarkdown(&doc);
    QVERIFY2(md.contains(QStringLiteral("[^1]")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[^2]")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[^1]: Primera definición con espacios.")),
             qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[^2]: Segunda con *énfasis*.")), qPrintable(md));
}

void TestFootnotes::roundTripPreservesSingleWordDefinitions()
{
    // Caso que md4c tomaría por definición de enlace de referencia sin la
    // protección (destino de una sola palabra): debe sobrevivir igualmente.
    const QString src = QStringLiteral(
        "Cita[^1] y abrev[^2].\n\n[^1]: Ibíd.\n\n[^2]: cfr.\n");
    QTextDocument doc;
    loadInto(doc, src);

    const QString md = mdtable::documentMarkdown(&doc);
    QVERIFY2(md.contains(QStringLiteral("Cita[^1]")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("abrev[^2]")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[^1]: Ibíd.")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[^2]: cfr.")), qPrintable(md));
    // Y no debe haberse colado el centinela de protección.
    QVERIFY2(!md.contains(QChar(0xF8FB)), "centinela filtrado al Markdown");
}

void TestFootnotes::definitionBlockNumberFound()
{
    QTextDocument doc;
    loadInto(doc, QStringLiteral("texto[^1]\n\n[^1]: La def.\n"));
    const int n = mdfootnote::definitionBlockNumber(&doc, QStringLiteral("1"));
    QVERIFY(n >= 0);
    QVERIFY(doc.findBlockByNumber(n).text().startsWith(QStringLiteral("[^1]:")));
    QCOMPARE(mdfootnote::definitionBlockNumber(&doc, QStringLiteral("9")), -1);
}

QTEST_MAIN(TestFootnotes)
#include "tst_footnotes.moc"
