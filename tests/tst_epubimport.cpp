#include <QtTest>

#include <QBuffer>
#include <QByteArray>

#include <private/qzipwriter_p.h>

#include "epubimport.h"

// Pruebas de la importación de EPUB (mdimport): parseo puro de container.xml y del
// OPF (lomo), y la conversión completa de un .epub construido en memoria.
class TestEpubImport : public QObject
{
    Q_OBJECT
private slots:
    void opfPathFromContainer();
    void spineHrefsInOrderXhtmlOnly();
    void convertsEpubToMarkdownInSpineOrder();

private:
    static QByteArray container()
    {
        return "<?xml version=\"1.0\"?>\n"
               "<container version=\"1.0\" "
               "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
               "  <rootfiles>\n"
               "    <rootfile full-path=\"OEBPS/content.opf\" "
               "media-type=\"application/oebps-package+xml\"/>\n"
               "  </rootfiles>\n</container>\n";
    }
    static QByteArray opf()
    {
        return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
               "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" "
               "unique-identifier=\"id\">\n"
               "  <manifest>\n"
               "    <item id=\"c1\" href=\"ch1.xhtml\" media-type=\"application/xhtml+xml\"/>\n"
               "    <item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n"
               "    <item id=\"c2\" href=\"ch2.xhtml\" media-type=\"application/xhtml+xml\"/>\n"
               "  </manifest>\n"
               "  <spine>\n"
               "    <itemref idref=\"c1\"/>\n"
               "    <itemref idref=\"c2\"/>\n"
               "  </spine>\n</package>\n";
    }
    static QByteArray chapter(const QString &heading, const QString &strong)
    {
        const QString xhtml =
            QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                           "<html xmlns=\"http://www.w3.org/1999/xhtml\"><head><title>x"
                           "</title></head><body><h1>%1</h1><p>Texto <strong>%2</strong>."
                           "</p></body></html>")
                .arg(heading, strong);
        return xhtml.toUtf8();
    }
    // Construye un .epub mínimo en memoria.
    static QByteArray buildEpub()
    {
        QByteArray out;
        QBuffer buf(&out);
        buf.open(QIODevice::WriteOnly);
        QZipWriter zip(&buf);
        zip.setCompressionPolicy(QZipWriter::NeverCompress);
        zip.addFile(QStringLiteral("mimetype"), QByteArray("application/epub+zip"));
        zip.setCompressionPolicy(QZipWriter::AutoCompress);
        zip.addFile(QStringLiteral("META-INF/container.xml"), container());
        zip.addFile(QStringLiteral("OEBPS/content.opf"), opf());
        zip.addFile(QStringLiteral("OEBPS/ch1.xhtml"),
                    chapter(QStringLiteral("Capítulo uno"), QStringLiteral("uno")));
        zip.addFile(QStringLiteral("OEBPS/ch2.xhtml"),
                    chapter(QStringLiteral("Capítulo dos"), QStringLiteral("dos")));
        zip.addFile(QStringLiteral("OEBPS/style.css"), QByteArray("body{}"));
        zip.close();
        buf.close();
        return out;
    }
};

void TestEpubImport::opfPathFromContainer()
{
    QCOMPARE(mdimport::epubOpfPath(container()), QStringLiteral("OEBPS/content.opf"));
    QVERIFY(mdimport::epubOpfPath("<container/>").isEmpty());
}

void TestEpubImport::spineHrefsInOrderXhtmlOnly()
{
    // En orden de lomo (c1, c2), y el CSS del manifiesto queda fuera.
    const QStringList hrefs = mdimport::epubSpineHrefs(opf());
    QCOMPARE(hrefs, QStringList({QStringLiteral("ch1.xhtml"), QStringLiteral("ch2.xhtml")}));
}

void TestEpubImport::convertsEpubToMarkdownInSpineOrder()
{
    const QString md = mdimport::epubToMarkdown(buildEpub());
    QVERIFY2(md.contains(QStringLiteral("# Capítulo uno")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("# Capítulo dos")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("**uno**")), qPrintable(md));
    // Orden de lectura respetado.
    QVERIFY(md.indexOf(QStringLiteral("uno")) < md.indexOf(QStringLiteral("dos")));
    // Un .epub inválido (bytes basura) no revienta: devuelve vacío.
    QVERIFY(mdimport::epubToMarkdown(QByteArray("no soy un zip")).isEmpty());
}

QTEST_MAIN(TestEpubImport)
#include "tst_epubimport.moc"
