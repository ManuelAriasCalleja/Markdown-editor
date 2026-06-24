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
    void dimRangesMiddleParagraph();
    void dimRangesAtStart();
    void dimRangesAtEnd();
    void dimRangesWholeDocFocused();
    void dimRangesDegenerateInputs();
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

// «Foco de línea»: los tramos a atenuar son el complemento del párrafo enfocado.
void TestTypewriter::dimRangesMiddleParagraph()
{
    // Documento de 100; párrafo enfocado [40, 60): se atenúa [0,40) y [60,40).
    const QList<mdtypewriter::Range> r = mdtypewriter::dimRanges(100, 40, 60);
    QCOMPARE(r.size(), 2);
    QCOMPARE(r[0].start, 0);
    QCOMPARE(r[0].length, 40);
    QCOMPARE(r[1].start, 60);
    QCOMPARE(r[1].length, 40);
}

void TestTypewriter::dimRangesAtStart()
{
    // Párrafo al principio: solo hay tramo posterior.
    const QList<mdtypewriter::Range> r = mdtypewriter::dimRanges(100, 0, 30);
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].start, 30);
    QCOMPARE(r[0].length, 70);
}

void TestTypewriter::dimRangesAtEnd()
{
    // Párrafo al final: solo hay tramo anterior.
    const QList<mdtypewriter::Range> r = mdtypewriter::dimRanges(100, 70, 100);
    QCOMPARE(r.size(), 1);
    QCOMPARE(r[0].start, 0);
    QCOMPARE(r[0].length, 70);
}

void TestTypewriter::dimRangesWholeDocFocused()
{
    // Todo el documento es el párrafo: no se atenúa nada.
    QCOMPARE(mdtypewriter::dimRanges(100, 0, 100).size(), 0);
}

void TestTypewriter::dimRangesDegenerateInputs()
{
    QCOMPARE(mdtypewriter::dimRanges(0, 0, 0).size(), 0);    // documento vacío
    QCOMPARE(mdtypewriter::dimRanges(-5, 0, 0).size(), 0);   // total negativo
    // Fuera de rango y orden invertido: se acota/ordena, no produce basura.
    const QList<mdtypewriter::Range> r = mdtypewriter::dimRanges(50, 80, 20);
    for (const mdtypewriter::Range &x : r) {
        QVERIFY(x.start >= 0);
        QVERIFY(x.length > 0);
        QVERIFY(x.start + x.length <= 50);
    }
}

QTEST_APPLESS_MAIN(TestTypewriter)
#include "tst_typewriter.moc"
