#include <QtTest>

#include "markdowntidy.h"

class TestMarkdownTidy : public QObject
{
    Q_OBJECT

private slots:
    void stripsTrailingWhitespace();
    void preservesHardBreak();
    void collapsesBlankLines();
    void normalizesHeadingSpacing();
    void normalizesBullets();
    void preservesThematicBreaks();
    void leavesCodeFenceContentUntouched();
    void trimsEdgesAndEndsWithSingleNewline();
};

void TestMarkdownTidy::stripsTrailingWhitespace()
{
    // Un solo espacio o tabuladores finales se quitan (2+ espacios = salto duro,
    // ver preservesHardBreak).
    QCOMPARE(mdtidy::tidy(QStringLiteral("hola \nadiós\t\n")),
             QStringLiteral("hola\nadiós\n"));
}

void TestMarkdownTidy::preservesHardBreak()
{
    // Dos espacios finales = salto duro: se conservan (exactamente dos).
    QCOMPARE(mdtidy::tidy(QStringLiteral("línea  \nsiguiente\n")),
             QStringLiteral("línea  \nsiguiente\n"));
    // Tres o más también se normalizan a dos.
    QCOMPARE(mdtidy::tidy(QStringLiteral("línea    \nx\n")),
             QStringLiteral("línea  \nx\n"));
}

void TestMarkdownTidy::collapsesBlankLines()
{
    QCOMPARE(mdtidy::tidy(QStringLiteral("a\n\n\n\nb\n")),
             QStringLiteral("a\n\nb\n"));
}

void TestMarkdownTidy::normalizesHeadingSpacing()
{
    QCOMPARE(mdtidy::tidy(QStringLiteral("##   Título\n")), QStringLiteral("## Título\n"));
    // Sin espacio tras los # no es encabezado: se deja igual.
    QCOMPARE(mdtidy::tidy(QStringLiteral("##Texto\n")), QStringLiteral("##Texto\n"));
}

void TestMarkdownTidy::normalizesBullets()
{
    QCOMPARE(mdtidy::tidy(QStringLiteral("* uno\n+ dos\n-   tres\n")),
             QStringLiteral("- uno\n- dos\n- tres\n"));
    // Conserva la sangría de las viñetas anidadas.
    QCOMPARE(mdtidy::tidy(QStringLiteral("- a\n  * b\n")),
             QStringLiteral("- a\n  - b\n"));
}

void TestMarkdownTidy::preservesThematicBreaks()
{
    QCOMPARE(mdtidy::tidy(QStringLiteral("a\n\n---\n\nb\n")),
             QStringLiteral("a\n\n---\n\nb\n"));
    QCOMPARE(mdtidy::tidy(QStringLiteral("* * *\n")), QStringLiteral("* * *\n"));
}

void TestMarkdownTidy::leavesCodeFenceContentUntouched()
{
    // Dentro del fence: espacios finales, viñetas y líneas en blanco intactas.
    const QString in = QStringLiteral("```\n* no es viñeta   \n\n\nx\n```\n");
    QCOMPARE(mdtidy::tidy(in), in);
}

void TestMarkdownTidy::trimsEdgesAndEndsWithSingleNewline()
{
    QCOMPARE(mdtidy::tidy(QStringLiteral("\n\n  \nhola\n\n\n")), QStringLiteral("hola\n"));
    QCOMPARE(mdtidy::tidy(QStringLiteral("sin salto final")), QStringLiteral("sin salto final\n"));
    QCOMPARE(mdtidy::tidy(QString()), QString());
}

QTEST_APPLESS_MAIN(TestMarkdownTidy)
#include "tst_markdowntidy.moc"
