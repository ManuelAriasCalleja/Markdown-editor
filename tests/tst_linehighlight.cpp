#include <QtTest>

#include "linehighlight.h"

// Pruebas del color de resaltado de la línea actual (mdlinehighlight): mezcla pura.
class TestLineHighlight : public QObject
{
    Q_OBJECT
private slots:
    void weightZeroIsBase();
    void weightFullIsHighlight();
    void midMixIsHalfway();
    void subtleWeightStaysCloseToBase();
    void weightClamped();
};

void TestLineHighlight::weightZeroIsBase()
{
    const QColor base(30, 40, 50);
    QCOMPARE(mdlinehighlight::currentLineColor(base, QColor(200, 100, 0), 0), base);
}

void TestLineHighlight::weightFullIsHighlight()
{
    const QColor hl(200, 100, 0);
    QCOMPARE(mdlinehighlight::currentLineColor(QColor(30, 40, 50), hl, 100), hl);
}

void TestLineHighlight::midMixIsHalfway()
{
    // 50 % entre negro y blanco → gris medio (127 por truncamiento entero).
    const QColor mid = mdlinehighlight::currentLineColor(QColor(0, 0, 0), QColor(255, 255, 255), 50);
    QCOMPARE(mid, QColor(127, 127, 127));
}

void TestLineHighlight::subtleWeightStaysCloseToBase()
{
    // Con el peso por defecto (sutil), cada canal se acerca a la selección pero
    // sigue dominado por el fondo.
    const QColor base(0, 0, 0);
    const QColor hl(100, 200, 50);
    const QColor c = mdlinehighlight::currentLineColor(base, hl);  // weight = 18
    QCOMPARE(c, QColor(18, 36, 9));  // (0*82 + h*18)/100 por canal
}

void TestLineHighlight::weightClamped()
{
    const QColor base(10, 20, 30);
    const QColor hl(240, 240, 240);
    QCOMPARE(mdlinehighlight::currentLineColor(base, hl, -5), base);   // < 0 → 0
    QCOMPARE(mdlinehighlight::currentLineColor(base, hl, 500), hl);    // > 100 → 100
}

QTEST_MAIN(TestLineHighlight)
#include "tst_linehighlight.moc"
