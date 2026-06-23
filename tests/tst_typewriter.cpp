#include <QtTest>

#include "typewriter.h"

// Pruebas de la lógica pura del modo «máquina de escribir»: el cálculo del valor
// de scroll que lleva el cursor al centro vertical del viewport.
class TestTypewriter : public QObject
{
    Q_OBJECT

private slots:
    void cursorBelowCenterScrollsDown();
    void cursorAboveCenterScrollsUp();
    void cursorAtCenterDoesNotMove();
    void clampsAtTop();
    void clampsAtBottom();
};

void TestTypewriter::cursorBelowCenterScrollsDown()
{
    // Cursor a 300 con viewport de 400 (centro 200): hay que bajar 100 el scroll.
    QCOMPARE(mdtypewriter::centeredScrollValue(
                 /*currentScroll=*/0, /*cursorCenterY=*/300, /*viewportHeight=*/400,
                 /*minScroll=*/0, /*maxScroll=*/1000),
             100);
}

void TestTypewriter::cursorAboveCenterScrollsUp()
{
    // Cursor a 100 (por encima del centro 200): el scroll sube 100, de 500 a 400.
    QCOMPARE(mdtypewriter::centeredScrollValue(500, 100, 400, 0, 1000), 400);
}

void TestTypewriter::cursorAtCenterDoesNotMove()
{
    QCOMPARE(mdtypewriter::centeredScrollValue(250, 200, 400, 0, 1000), 250);
}

void TestTypewriter::clampsAtTop()
{
    // Cerca del principio: centrar exigiría scroll negativo → se acota al mínimo.
    QCOMPARE(mdtypewriter::centeredScrollValue(0, 50, 400, 0, 1000), 0);
}

void TestTypewriter::clampsAtBottom()
{
    // Cerca del final: centrar exigiría pasar del máximo → se acota al máximo.
    QCOMPARE(mdtypewriter::centeredScrollValue(1000, 350, 400, 0, 1000), 1000);
}

QTEST_APPLESS_MAIN(TestTypewriter)
#include "tst_typewriter.moc"
