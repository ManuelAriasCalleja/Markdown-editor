#include <QtTest>

#include <QTextBlock>
#include <QTextDocument>

#include "outlinepanel.h"

// Pruebas de la extracción pura de encabezados (mdoutline::headingsOf) que
// alimenta el panel de esquema. Se construye el documento con setMarkdown, que
// es la misma ruta que usa el editor para fijar headingLevel() en los bloques.
class TestOutline : public QObject
{
    Q_OBJECT

private slots:
    void nullDocReturnsEmpty();
    void emptyDocHasNoHeadings();
    void plainTextHasNoHeadings();
    void extractsHeadingsWithLevels();
    void ignoresNonHeadingParagraphs();
    void blockNumberPointsToTheHeading();

    void tocEmptyForNoHeadings();
    void tocNestsByLevel();
    void tocCompactsLevelJumps();
    void tocRoundTripsThroughMarkdown();

    void moveSectionAfterSibling();
    void moveSectionBeforeSibling();
    void moveSectionCarriesSubsections();
    void moveSectionToEnd();
    void moveSectionIgnoresCodeFenceHashes();
    void moveSectionInvalidIsNoOp();
    void moveSectionIntoItselfIsNoOp();

    void shiftedLevelClampsAndIgnoresNonHeadings();
};

void TestOutline::nullDocReturnsEmpty()
{
    QVERIFY(mdoutline::headingsOf(nullptr).isEmpty());
}

void TestOutline::emptyDocHasNoHeadings()
{
    QTextDocument doc;
    QVERIFY(mdoutline::headingsOf(&doc).isEmpty());
}

void TestOutline::plainTextHasNoHeadings()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("solo texto\n\notro párrafo"));
    QVERIFY(mdoutline::headingsOf(&doc).isEmpty());
}

void TestOutline::extractsHeadingsWithLevels()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Uno\n\n## Dos\n\n### Tres"));

    const QList<OutlineHeading> h = mdoutline::headingsOf(&doc);
    QCOMPARE(h.size(), 3);
    QCOMPARE(h[0].level, 1);
    QCOMPARE(h[0].text, QStringLiteral("Uno"));
    QCOMPARE(h[1].level, 2);
    QCOMPARE(h[1].text, QStringLiteral("Dos"));
    QCOMPARE(h[2].level, 3);
    QCOMPARE(h[2].text, QStringLiteral("Tres"));
}

void TestOutline::ignoresNonHeadingParagraphs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Título\n\npárrafo normal\n\n## Sección"));

    const QList<OutlineHeading> h = mdoutline::headingsOf(&doc);
    QCOMPARE(h.size(), 2);
    QCOMPARE(h[0].text, QStringLiteral("Título"));
    QCOMPARE(h[1].text, QStringLiteral("Sección"));
}

void TestOutline::blockNumberPointsToTheHeading()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Alfa\n\npárrafo\n\n## Beta"));

    // El blockNumber guardado debe resolver al bloque correcto (es lo que usa la
    // navegación: findBlockByNumber para llevar el cursor allí).
    for (const OutlineHeading &heading : mdoutline::headingsOf(&doc)) {
        const QTextBlock block = doc.findBlockByNumber(heading.blockNumber);
        QVERIFY(block.isValid());
        QCOMPARE(block.text(), heading.text);
    }
}

void TestOutline::tocEmptyForNoHeadings()
{
    QVERIFY(mdoutline::tableOfContentsMarkdown({}).isEmpty());
}

void TestOutline::tocNestsByLevel()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Uno\n\n## Dos\n\n### Tres\n\n## Cuatro"));

    const QString toc = mdoutline::tableOfContentsMarkdown(mdoutline::headingsOf(&doc));
    QCOMPARE(toc, QStringLiteral("- Uno\n  - Dos\n    - Tres\n  - Cuatro\n"));
}

void TestOutline::tocCompactsLevelJumps()
{
    // Un salto de nivel (H1 → H3) no debe dejar una sangría de dos niveles: la
    // profundidad la marca la pila de ancestros, no el número de nivel.
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Uno\n\n### Tres"));

    const QString toc = mdoutline::tableOfContentsMarkdown(mdoutline::headingsOf(&doc));
    QCOMPARE(toc, QStringLiteral("- Uno\n  - Tres\n"));
}

void TestOutline::tocRoundTripsThroughMarkdown()
{
    // El Markdown generado debe releerse como una lista anidada con los mismos
    // textos (es lo que se inserta en el documento con fromMarkdown).
    QTextDocument src;
    src.setMarkdown(QStringLiteral("# Alfa\n\n## Beta\n\n## Gamma"));
    const QString toc = mdoutline::tableOfContentsMarkdown(mdoutline::headingsOf(&src));

    QTextDocument rendered;
    rendered.setMarkdown(toc);
    const QString plain = rendered.toPlainText();
    QVERIFY(plain.contains(QStringLiteral("Alfa")));
    QVERIFY(plain.contains(QStringLiteral("Beta")));
    QVERIFY(plain.contains(QStringLiteral("Gamma")));
    // Y no debe haber dejado ningún encabezado (es una lista, no títulos).
    QVERIFY(mdoutline::headingsOf(&rendered).isEmpty());
}

// --- moveSection (reordenación de secciones) -------------------------------

void TestOutline::moveSectionAfterSibling()
{
    // Tres secciones de nivel 1; mover la 0 (Alfa) tras la 1 (Beta).
    const QString md = QStringLiteral(
        "# Alfa\n\na\n\n# Beta\n\nb\n\n# Gamma\n\ng\n");
    const QString out = mdoutline::moveSection(md, 0, 1, /*placeAfter=*/true);
    const int alfa = out.indexOf(QStringLiteral("# Alfa"));
    const int beta = out.indexOf(QStringLiteral("# Beta"));
    const int gamma = out.indexOf(QStringLiteral("# Gamma"));
    QVERIFY(beta >= 0 && alfa >= 0 && gamma >= 0);
    QVERIFY(beta < alfa);          // Beta ahora antes que Alfa
    QVERIFY(alfa < gamma);         // Alfa antes que Gamma
    QVERIFY(out.contains(QStringLiteral("a\n")));  // el contenido viaja con su sección
}

void TestOutline::moveSectionBeforeSibling()
{
    const QString md = QStringLiteral("# Alfa\n\na\n\n# Beta\n\nb\n");
    const QString out = mdoutline::moveSection(md, 1, 0, /*placeAfter=*/false);
    QVERIFY(out.indexOf(QStringLiteral("# Beta")) < out.indexOf(QStringLiteral("# Alfa")));
}

void TestOutline::moveSectionCarriesSubsections()
{
    // La sección Alfa incluye su subsección Alfa.1; al mover Alfa tras Beta debe
    // arrastrar la subsección.
    const QString md = QStringLiteral(
        "# Alfa\n\na\n\n## Alfa.1\n\na1\n\n# Beta\n\nb\n");
    const QString out = mdoutline::moveSection(md, 0, 2, /*placeAfter=*/true);
    const int beta = out.indexOf(QStringLiteral("# Beta"));
    const int alfa = out.indexOf(QStringLiteral("# Alfa"));
    const int sub = out.indexOf(QStringLiteral("## Alfa.1"));
    QVERIFY(beta < alfa);          // Beta antes que Alfa
    QVERIFY(alfa < sub);           // la subsección sigue tras su sección
}

void TestOutline::moveSectionToEnd()
{
    const QString md = QStringLiteral("# Alfa\n\na\n\n# Beta\n\nb\n# Gamma\n\ng\n");
    // Mover Alfa tras la última sección (Gamma).
    const QString out = mdoutline::moveSection(md, 0, 2, /*placeAfter=*/true);
    QVERIFY(out.indexOf(QStringLiteral("# Alfa")) > out.indexOf(QStringLiteral("# Gamma")));
}

void TestOutline::moveSectionIgnoresCodeFenceHashes()
{
    // El `# no es título` dentro del bloque de código no cuenta como encabezado:
    // solo hay 2 secciones (ordinales 0 y 1).
    const QString md = QStringLiteral(
        "# Uno\n\n```\n# no es título\n```\n\n# Dos\n\nd\n");
    const QString out = mdoutline::moveSection(md, 0, 1, /*placeAfter=*/true);
    QVERIFY(out.indexOf(QStringLiteral("# Dos")) < out.indexOf(QStringLiteral("# Uno")));
    // El comentario del bloque de código viaja con la sección Uno, intacto.
    QVERIFY(out.contains(QStringLiteral("# no es título")));
}

void TestOutline::moveSectionInvalidIsNoOp()
{
    const QString md = QStringLiteral("# Alfa\n\n# Beta\n");
    QCOMPARE(mdoutline::moveSection(md, 5, 0, true), md);   // origen fuera de rango
    QCOMPARE(mdoutline::moveSection(md, 0, 9, false), md);  // destino fuera de rango
}

void TestOutline::moveSectionIntoItselfIsNoOp()
{
    // Mover una sección dentro de su propia subsección no debe hacer nada.
    const QString md = QStringLiteral("# Alfa\n\n## Alfa.1\n\n# Beta\n");
    QCOMPARE(mdoutline::moveSection(md, 0, 1, true), md);   // destino = subsección de origen
}

void TestOutline::shiftedLevelClampsAndIgnoresNonHeadings()
{
    // Promover (−1) hacia H1, degradar (+1) hacia H6.
    QCOMPARE(mdoutline::shiftedLevel(3, -1), 2);
    QCOMPARE(mdoutline::shiftedLevel(2, 1), 3);
    // Límites: H1 no sube más, H6 no baja más.
    QCOMPARE(mdoutline::shiftedLevel(1, -1), 1);
    QCOMPARE(mdoutline::shiftedLevel(6, 1), 6);
    // No es encabezado (0 = párrafo): sin cambio.
    QCOMPARE(mdoutline::shiftedLevel(0, -1), 0);
    QCOMPARE(mdoutline::shiftedLevel(0, 1), 0);
}

QTEST_MAIN(TestOutline)
#include "tst_outline.moc"
