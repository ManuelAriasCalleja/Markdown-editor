#include <QtTest>

#include <QImage>
#include <QSignalSpy>

#include "diagram.h"
#include "diagramrenderer.h"

// Pruebas del motor de render. Dependen de las herramientas externas
// (plantuml/mmdc); si no están, se saltan (QSKIP). El render es asíncrono, así
// que se espera la señal con QSignalSpy.
class TestDiagramRenderer : public QObject
{
    Q_OBJECT

private slots:
    void unavailableToolEmitsFailed();
    void rendersPlantUmlToImage();
    void cachesSecondRender();
};

void TestDiagramRenderer::unavailableToolEmitsFailed()
{
    // None nunca renderiza; lo usamos para no depender de herramientas. Para un
    // motor real ausente, isAvailable() es false y render() emite failed.
    if (DiagramRenderer::isAvailable(mddiagram::Kind::Mermaid))
        QSKIP("mmdc presente; no se puede probar el caso 'ausente'");
    DiagramRenderer r;
    QSignalSpy failed(&r, &DiagramRenderer::failed);
    r.render(mddiagram::Kind::Mermaid, QStringLiteral("graph TD; A-->B;"));
    QCOMPARE(failed.count(), 1);
}

void TestDiagramRenderer::rendersPlantUmlToImage()
{
    if (!DiagramRenderer::isAvailable(mddiagram::Kind::PlantUml))
        QSKIP("plantuml no instalado");
    DiagramRenderer r;
    QSignalSpy rendered(&r, &DiagramRenderer::rendered);
    QSignalSpy failed(&r, &DiagramRenderer::failed);
    r.render(mddiagram::Kind::PlantUml, QStringLiteral("Alice -> Bob: Hola"));
    QVERIFY(rendered.wait(15000));  // Java tarda en arrancar
    QCOMPARE(failed.count(), 0);
    QCOMPARE(rendered.count(), 1);
    const QImage img = rendered.takeFirst().at(2).value<QImage>();
    QVERIFY(!img.isNull());
    QVERIFY(img.width() > 0 && img.height() > 0);
}

void TestDiagramRenderer::cachesSecondRender()
{
    if (!DiagramRenderer::isAvailable(mddiagram::Kind::PlantUml))
        QSKIP("plantuml no instalado");
    DiagramRenderer r;
    const QString src = QStringLiteral("Bob -> Alice: Adiós");
    QSignalSpy rendered(&r, &DiagramRenderer::rendered);
    r.render(mddiagram::Kind::PlantUml, src);
    QVERIFY(rendered.wait(15000));
    // Segunda vez: debe salir de la caché de forma síncrona (sin proceso).
    rendered.clear();
    r.render(mddiagram::Kind::PlantUml, src);
    QCOMPARE(rendered.count(), 1);  // emitido ya, sin esperar
}

QTEST_MAIN(TestDiagramRenderer)
#include "tst_diagramrenderer.moc"
