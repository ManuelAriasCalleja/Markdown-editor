#include <QtTest>

#include <QFont>
#include <QImage>
#include <QPainter>

#include "mathlayout.h"

// Pruebas del motor de maquetación 2D (mdmath::mathlayout). Como la geometría
// exacta depende de la fuente del sistema, las aserciones son sobre relaciones
// invariantes (una fracción es más alta que su contenido en línea; los límites
// de un sumatorio aumentan la altura) y sobre que pintar no rompe.
class TestMathLayout : public QObject
{
    Q_OBJECT

private:
    QFont baseFont() const
    {
        QFont f;
        f.setPointSizeF(12.0);
        return f;
    }

private slots:
    void needsTwoDDetectsFracAndLimitedBigOps();
    void needsTwoDDetectsSqrtAndMatrix();
    void needsTwoDIgnoresPlainAndBareBigOps();
    void fracIsTallerThanInline();
    void bigOpLimitsAddHeight();
    void sqrtIsTallerThanRadicand();
    void matrixGrowsWithRowsAndCols();
    void measurePositiveForVariety();
    void paintDoesNotCrash();
};

void TestMathLayout::needsTwoDDetectsFracAndLimitedBigOps()
{
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\frac{a}{b}")));
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("x + \\frac{1}{2}")));
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\sum_{i=1}^n a_i")));
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\int_0^1 f")));
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\prod _{k} k")));  // con espacio
}

void TestMathLayout::needsTwoDDetectsSqrtAndMatrix()
{
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\sqrt{2}")));
    QVERIFY(mdmath::needsTwoDLayout(QStringLiteral("\\sqrt[3]{x}")));
    QVERIFY(mdmath::needsTwoDLayout(
        QStringLiteral("\\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix}")));
}

void TestMathLayout::needsTwoDIgnoresPlainAndBareBigOps()
{
    QVERIFY(!mdmath::needsTwoDLayout(QStringLiteral("x^2 + y_1")));
    QVERIFY(!mdmath::needsTwoDLayout(QStringLiteral("\\alpha + \\beta")));
    // Un \sum sin límites no necesita apilado 2D.
    QVERIFY(!mdmath::needsTwoDLayout(QStringLiteral("\\sum a_i")));
}

void TestMathLayout::fracIsTallerThanInline()
{
    const QFont f = baseFont();
    const QSizeF frac = mdmath::measureFormula(QStringLiteral("\\frac{a}{b}"), f);
    const QSizeF flat = mdmath::measureFormula(QStringLiteral("ab"), f);
    QVERIFY2(frac.height() > flat.height() * 1.5,
             "una fracción apilada debe ser bastante más alta que su contenido en línea");
}

void TestMathLayout::bigOpLimitsAddHeight()
{
    const QFont f = baseFont();
    const QSizeF withLimits = mdmath::measureFormula(QStringLiteral("\\sum_{i=1}^n"), f);
    const QSizeF bare = mdmath::measureFormula(QStringLiteral("\\sum"), f);
    QVERIFY2(withLimits.height() > bare.height(),
             "los límites encima/debajo deben aumentar la altura del operador");
}

void TestMathLayout::sqrtIsTallerThanRadicand()
{
    const QFont f = baseFont();
    // El radical añade vínculo encima, así que `\sqrt{x}` es más alto que `x`.
    const QSizeF root = mdmath::measureFormula(QStringLiteral("\\sqrt{x}"), f);
    const QSizeF rad = mdmath::measureFormula(QStringLiteral("x"), f);
    QVERIFY(root.height() > rad.height());
    QVERIFY(root.width() > rad.width());  // el signo radical ocupa ancho
}

void TestMathLayout::matrixGrowsWithRowsAndCols()
{
    const QFont f = baseFont();
    const QSizeF m1 = mdmath::measureFormula(
        QStringLiteral("\\begin{pmatrix} a \\\\ b \\end{pmatrix}"), f);
    const QSizeF m2 = mdmath::measureFormula(
        QStringLiteral("\\begin{pmatrix} a & b \\\\ c & d \\end{pmatrix}"), f);
    QVERIFY(m2.width() > m1.width());    // más columnas → más ancho
    QVERIFY(m2.height() > 0 && m1.height() > 0);
}

void TestMathLayout::measurePositiveForVariety()
{
    const QFont f = baseFont();
    const QStringList cases = {
        QStringLiteral("\\frac{\\sum_{i=1}^n x_i}{n}"),
        QStringLiteral("\\int_a^b \\frac{1}{x} dx"),
        QStringLiteral("\\frac{a}{\\frac{b}{c}}"),  // anidada
        QStringLiteral("e^{i\\pi} + 1"),
        QStringLiteral("x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}"),  // raíz en fracción
        QStringLiteral("\\sqrt[3]{\\frac{a}{b}}"),                   // raíz con índice
        QStringLiteral("\\begin{bmatrix} 1 & 0 \\\\ 0 & 1 \\end{bmatrix}"),
    };
    for (const QString &tex : cases) {
        const QSizeF s = mdmath::measureFormula(tex, f);
        QVERIFY2(s.width() > 0 && s.height() > 0, qPrintable(tex));
    }
}

void TestMathLayout::paintDoesNotCrash()
{
    const QFont f = baseFont();
    const QString tex = QStringLiteral("\\frac{\\sum_{i=1}^n x_i^2}{n}");
    const QSizeF s = mdmath::measureFormula(tex, f);
    QImage img(qCeil(s.width()) + 4, qCeil(s.height()) + 4, QImage::Format_ARGB32);
    img.fill(Qt::white);
    QPainter p(&img);
    mdmath::paintFormula(&p, QPointF(0, 0), tex, f, Qt::black);
    p.end();
    // Algo se pintó (hay píxeles no blancos).
    bool drewSomething = false;
    for (int y = 0; y < img.height() && !drewSomething; ++y)
        for (int x = 0; x < img.width(); ++x)
            if (img.pixelColor(x, y) != QColor(Qt::white)) { drewSomething = true; break; }
    QVERIFY(drewSomething);
}

QTEST_MAIN(TestMathLayout)
#include "tst_mathlayout.moc"
