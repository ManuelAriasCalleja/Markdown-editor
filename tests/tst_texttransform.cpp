#include <QtTest>

#include "texttransform.h"

// Pruebas de las transformaciones de texto puras (mdtext) que aplica el menú
// Editar sobre la selección.
class TestTextTransform : public QObject
{
    Q_OBJECT

private slots:
    void upperAndLower();
    void capitalizeWords();
    void capitalizeLowersRestOfWord();
    void capitalizeHandlesAccents();
    void sortLinesAscending();
    void sortLinesDescending();
    void sortLinesIsStable();
    void sortSingleLineUnchanged();
    void smartTypographyDashesAndEllipsis();
    void smartTypographyQuotes();
};

void TestTextTransform::upperAndLower()
{
    QCOMPARE(mdtext::toUpper(QStringLiteral("Hola Mundo")), QStringLiteral("HOLA MUNDO"));
    QCOMPARE(mdtext::toLower(QStringLiteral("Hola Mundo")), QStringLiteral("hola mundo"));
}

void TestTextTransform::capitalizeWords()
{
    QCOMPARE(mdtext::capitalize(QStringLiteral("hola mundo feliz")),
             QStringLiteral("Hola Mundo Feliz"));
    // Los signos y espacios cuentan como frontera de palabra.
    QCOMPARE(mdtext::capitalize(QStringLiteral("uno-dos (tres)")),
             QStringLiteral("Uno-Dos (Tres)"));
}

void TestTextTransform::capitalizeLowersRestOfWord()
{
    QCOMPARE(mdtext::capitalize(QStringLiteral("hOLA MuNDo")),
             QStringLiteral("Hola Mundo"));
}

void TestTextTransform::capitalizeHandlesAccents()
{
    QCOMPARE(mdtext::capitalize(QStringLiteral("árbol ñandú")),
             QStringLiteral("Árbol Ñandú"));
}

void TestTextTransform::sortLinesAscending()
{
    QCOMPARE(mdtext::sortLines(QStringLiteral("banana\nmanzana\ncereza")),
             QStringLiteral("banana\ncereza\nmanzana"));
}

void TestTextTransform::sortLinesDescending()
{
    QCOMPARE(mdtext::sortLines(QStringLiteral("banana\nmanzana\ncereza"), false),
             QStringLiteral("manzana\ncereza\nbanana"));
}

void TestTextTransform::sortLinesIsStable()
{
    // Líneas iguales conservan su orden relativo (sufijo distinto tras el sort).
    const QString in = QStringLiteral("b\na\nb");
    QCOMPARE(mdtext::sortLines(in), QStringLiteral("a\nb\nb"));
}

void TestTextTransform::sortSingleLineUnchanged()
{
    QCOMPARE(mdtext::sortLines(QStringLiteral("una sola")), QStringLiteral("una sola"));
}

void TestTextTransform::smartTypographyDashesAndEllipsis()
{
    QCOMPARE(mdtext::smartTypography(QStringLiteral("a -- b")),
             QString::fromUtf8("a – b"));            // –
    QCOMPARE(mdtext::smartTypography(QStringLiteral("a --- b")),
             QString::fromUtf8("a — b"));            // —
    QCOMPARE(mdtext::smartTypography(QStringLiteral("espera...")),
             QString::fromUtf8("espera…"));          // …
}

void TestTextTransform::smartTypographyQuotes()
{
    QCOMPARE(mdtext::smartTypography(QStringLiteral("\"hola\"")),
             QString::fromUtf8("“hola”"));      // “hola”
    QCOMPARE(mdtext::smartTypography(QStringLiteral("don't")),
             QString::fromUtf8("don’t"));            // don’t
    QCOMPARE(mdtext::smartTypography(QStringLiteral("'hi'")),
             QString::fromUtf8("‘hi’"));        // ‘hi’
}

QTEST_MAIN(TestTextTransform)
#include "tst_texttransform.moc"
