#include <QtTest>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextDocument>

#include "exporters.h"

#ifndef GOLDEN_DIR
#define GOLDEN_DIR "."
#endif

// Golden tests (snapshot) de los exportadores: fijan la salida EXACTA de los
// serializadores propios de la app para un documento canónico, y fallan ante
// cualquier cambio inesperado (red contra regresiones de salida que los tests de
// propiedades no ven). Se cubren solo los serializadores DETERMINISTAS y propios:
// LaTeX, el XML de DOCX, el XML de ODF, las piezas XML del EPUB (con uuid/fecha
// fijos) y el saneado HTML→XHTML. NO se fija el HTML de `QTextDocument::toHtml`:
// es de Qt y cambia entre versiones, así que un golden ahí avisaría de cambios de
// Qt, no de regresiones nuestras.
//
// Si un cambio de salida es INTENCIONADO (o tras subir de versión de Qt, que puede
// alterar el documento canónico), regenera los goldens con:
//   UPDATE_GOLDEN=1 ctest --test-dir build -R tst_goldenexport
// y revisa/commitea el diff de tests/golden/.
class TestGoldenExport : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void latex();
    void docxDocumentXml();
    void docxStyles();
    void docxNumbering();
    void odfStyles();
    void odfMeta();
    void epubContainer();
    void epubContentOpf();
    void epubNav();
    void epubTocNcx();
    void epubStyleCss();
    void epubContentXhtml();
    void htmlBodyToXhtml();

private:
    QTextDocument m_doc;
    mdexport::Language m_lang;

    void checkGolden(const QString &name, const QByteArray &content);
    void checkGolden(const QString &name, const QString &content)
    {
        checkGolden(name, content.toUtf8());
    }
};

void TestGoldenExport::initTestCase()
{
    // Documento canónico: cubre muchas construcciones a la vez. Fijo, así que la
    // salida es estable salvo que cambie un serializador (o el parseo Markdown de
    // Qt al subir de versión, caso en que se regeneran los goldens a propósito).
    m_doc.setMarkdown(QStringLiteral(
        "# Título principal\n\n"
        "Párrafo con **negrita**, *cursiva*, `código` y un [enlace](http://e.com).\n\n"
        "## Sección\n\n"
        "- uno\n- dos\n- tres\n\n"
        "1. primero\n2. segundo\n\n"
        "> una cita a dos\n> líneas\n\n"
        "| Col A | Col B |\n|:------|------:|\n| 1 | 2 |\n| 3 | 4 |\n\n"
        "```cpp\nint x = 0;\nreturn x;\n```\n\n"
        "Caracteres especiales: 100% de a & b, #1, $5 y _guion_.\n"));
    m_lang = mdexport::languageForCode(QStringLiteral("es"));
}

void TestGoldenExport::checkGolden(const QString &name, const QByteArray &content)
{
    const QString path = QStringLiteral("%1/%2").arg(QLatin1String(GOLDEN_DIR), name);

    // Modo regeneración: escribe el golden y no compara.
    if (!qEnvironmentVariableIsEmpty("UPDATE_GOLDEN")) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        QFile f(path);
        QVERIFY2(f.open(QIODevice::WriteOnly),
                 qPrintable(QStringLiteral("no se pudo escribir %1").arg(path)));
        f.write(content);
        return;
    }

    QFile f(path);
    QVERIFY2(f.exists(),
             qPrintable(QStringLiteral(
                 "falta el golden '%1'. Genéralo con UPDATE_GOLDEN=1.").arg(name)));
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray expected = f.readAll();
    f.close();

    if (content != expected) {
        QFAIL(qPrintable(QStringLiteral(
            "La salida difiere del golden '%1' (esperado %2 B, obtenido %3 B). Si el "
            "cambio es intencionado, regenera con UPDATE_GOLDEN=1 y revisa el diff.")
                             .arg(name)
                             .arg(expected.size())
                             .arg(content.size())));
    }
}

void TestGoldenExport::latex()
{
    checkGolden(QStringLiteral("export.tex"),
                mdexport::toLatex(&m_doc, m_lang, QStringLiteral("Documento de prueba")));
}

void TestGoldenExport::docxDocumentXml()
{
    checkGolden(QStringLiteral("docx-document.xml"),
                mdexport::toDocxDocumentXml(&m_doc, QStringLiteral("Documento de prueba")));
}

void TestGoldenExport::docxStyles()
{
    checkGolden(QStringLiteral("docx-styles.xml"), mdexport::docxStylesXml(m_lang));
}

void TestGoldenExport::docxNumbering()
{
    checkGolden(QStringLiteral("docx-numbering.xml"), mdexport::docxNumberingXml());
}

void TestGoldenExport::odfStyles()
{
    checkGolden(QStringLiteral("odf-styles.xml"), mdexport::odfStylesXml(m_lang));
}

void TestGoldenExport::odfMeta()
{
    checkGolden(QStringLiteral("odf-meta.xml"),
                mdexport::odfMetaXml(m_lang, QStringLiteral("Documento de prueba")));
}

void TestGoldenExport::epubContainer()
{
    checkGolden(QStringLiteral("epub-container.xml"), mdexport::epubContainerXml());
}

void TestGoldenExport::epubContentOpf()
{
    // uuid y fecha FIJOS: el OPF los toma por parámetro, así que el golden es estable.
    checkGolden(QStringLiteral("epub-content.opf"),
                mdexport::epubContentOpf(m_lang, QStringLiteral("Documento de prueba"),
                                         {QStringLiteral("images/image1.png")},
                                         QStringLiteral("00000000-0000-0000-0000-000000000000"),
                                         QStringLiteral("2024-01-01T00:00:00Z")));
}

void TestGoldenExport::epubNav()
{
    checkGolden(QStringLiteral("epub-nav.xhtml"),
                mdexport::epubNavXhtml(m_lang, QStringLiteral("Documento de prueba")));
}

void TestGoldenExport::epubTocNcx()
{
    checkGolden(QStringLiteral("epub-toc.ncx"),
                mdexport::epubTocNcx(QStringLiteral("Documento de prueba"),
                                     QStringLiteral("00000000-0000-0000-0000-000000000000")));
}

void TestGoldenExport::epubStyleCss()
{
    checkGolden(QStringLiteral("epub-style.css"), mdexport::epubStyleCss());
}

void TestGoldenExport::epubContentXhtml()
{
    // Entrada FIJA (no el toHtml de Qt): aísla el armado XHTML propio.
    checkGolden(QStringLiteral("epub-content.xhtml"),
                mdexport::epubContentXhtml(
                    QStringLiteral("<h1>Título</h1>\n<p>Cuerpo con <b>negrita</b>.</p>"),
                    QStringLiteral("Documento de prueba"), m_lang));
}

void TestGoldenExport::htmlBodyToXhtml()
{
    // Entrada FIJA imitando el HTML de Qt (con `&nbsp;` y un <br> sin cerrar, que
    // es lo que esta función sanea a XHTML).
    const QString html = QStringLiteral(
        "<html><head></head><body>\n<p>Hola&nbsp;mundo<br>segunda línea</p>\n"
        "</body></html>");
    checkGolden(QStringLiteral("html-body.xhtml"), mdexport::htmlBodyToXhtml(html));
}

QTEST_MAIN(TestGoldenExport)
#include "tst_goldenexport.moc"
