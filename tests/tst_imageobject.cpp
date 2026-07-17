#include <QtTest>

#include <QImage>
#include <QTextDocument>
#include <QTextImageFormat>
#include <QUrl>
#include <QVariant>

#include "imageobject.h"

// Pruebas del handler de imágenes (ImageObject), que dimensiona las imágenes del
// documento = nativo · zoom en el trazado (sin editarlas, de ahí que escalar con el
// zoom no toque el historial de deshacer). Se comprueba la lógica de medición
// (intrinsicSize): escala, acotado al ancho de la vista, y el marcador cuando el
// recurso no existe. El registro que sustituye al handler interno de Qt y el dibujo
// se validan a nivel de aplicación (no aquí, que sería medir el pintor de Qt).
class TestImageObject : public QObject
{
    Q_OBJECT

private slots:
    void nativeSizeAtScaleOne();
    void scalesWithZoomFactor();
    void clampsToDocumentWidth();
    void missingResourceGetsPlaceholder();

private:
    // Documento con un recurso de imagen `name` de `w`×`h` px y un ancho de vista
    // holgado (sin acotar) salvo que el test lo cambie.
    static QTextImageFormat imageFmt(const QString &name);
    static void addImage(QTextDocument &doc, const QString &name, int w, int h);
};

QTextImageFormat TestImageObject::imageFmt(const QString &name)
{
    QTextImageFormat f;
    f.setName(name);
    return f;
}

void TestImageObject::addImage(QTextDocument &doc, const QString &name, int w, int h)
{
    QImage im(w, h, QImage::Format_ARGB32);
    im.fill(Qt::blue);
    doc.addResource(QTextDocument::ImageResource, QUrl(name), QVariant(im));
}

void TestImageObject::nativeSizeAtScaleOne()
{
    QTextDocument doc;
    doc.setTextWidth(2000);  // ancho holgado: sin acotar
    addImage(doc, QStringLiteral("mem://a"), 100, 80);

    ImageObject obj;  // escala por defecto = 1.0
    QCOMPARE(obj.intrinsicSize(&doc, 0, imageFmt(QStringLiteral("mem://a"))),
             QSizeF(100, 80));
}

void TestImageObject::scalesWithZoomFactor()
{
    QTextDocument doc;
    doc.setTextWidth(2000);
    addImage(doc, QStringLiteral("mem://a"), 100, 80);

    ImageObject obj;
    obj.setScale(2.0);
    QCOMPARE(obj.intrinsicSize(&doc, 0, imageFmt(QStringLiteral("mem://a"))),
             QSizeF(200, 160));

    obj.setScale(0.5);
    QCOMPARE(obj.intrinsicSize(&doc, 0, imageFmt(QStringLiteral("mem://a"))),
             QSizeF(50, 40));
}

void TestImageObject::clampsToDocumentWidth()
{
    QTextDocument doc;
    addImage(doc, QStringLiteral("mem://a"), 100, 80);
    doc.setTextWidth(150);  // estrecho: a escala 2.0 (w=200) hay que acotar

    ImageObject obj;
    obj.setScale(2.0);
    const QSizeF s = obj.intrinsicSize(&doc, 0, imageFmt(QStringLiteral("mem://a")));
    // Nunca más ancha que la vista (con un pequeño margen); se preserva la proporción.
    QVERIFY(s.width() <= 150);
    QVERIFY(s.width() > 100);  // sigue siendo mayor que el nativo (crecía a 200)
    QCOMPARE(s.width() / s.height(), 100.0 / 80.0);
}

void TestImageObject::missingResourceGetsPlaceholder()
{
    QTextDocument doc;
    doc.setTextWidth(2000);
    // No se añade el recurso: el nombre no resuelve.
    ImageObject obj;
    const QSizeF s = obj.intrinsicSize(&doc, 0, imageFmt(QStringLiteral("mem://ausente")));
    // Marcador visible (no colapsa a 0×0), para que se note que falta la imagen.
    QVERIFY(!s.isEmpty());
}

QTEST_MAIN(TestImageObject)
#include "tst_imageobject.moc"
