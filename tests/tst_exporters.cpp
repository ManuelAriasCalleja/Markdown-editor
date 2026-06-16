#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QTextDocument>

#include "exporters.h"

// Pruebas de la exportación: el serializador LaTeX puro, la lectura del front
// matter y los XML de idioma del ODF. El empaquetado real del ODT (QZip) se
// comprueba con un smoke test aparte.
class TestExporters : public QObject
{
    Q_OBJECT

private slots:
    void languageLookupNormalizesCode();
    void frontMatterValueReadsKeys();
    void latexHasBabelAndTitle();
    void latexEscapesSpecialChars();
    void latexConstructs();
    void latexHeadingNotDoublyBold();
    void latexSanitizesHighUnicode();
    void odfStylesCarryLanguage();
    void odfMetaCarriesLanguageAndTitle();
    void odfManifestListsExtraFiles();
    void docxDocumentConstructs();
    void docxTitleParagraph();
    void docxStylesCarryLanguage();
    void docxNumberingHasBulletAndDecimal();
    void docxWriteProducesZipPackage();
};

void TestExporters::languageLookupNormalizesCode()
{
    QCOMPARE(mdexport::languageForCode(QStringLiteral("es-ES")).babel,
             QStringLiteral("spanish"));
    QCOMPARE(mdexport::languageForCode(QStringLiteral("de_DE")).babel,
             QStringLiteral("ngerman"));
    // Desconocido → inglés de recurso.
    QCOMPARE(mdexport::languageForCode(QStringLiteral("xx")).code, QStringLiteral("en"));
}

void TestExporters::frontMatterValueReadsKeys()
{
    const QString fm = QStringLiteral("---\nlang: fr\ntitle: \"Mi Título\"\n---\n");
    QCOMPARE(mdexport::frontMatterValue(fm, QStringLiteral("lang")), QStringLiteral("fr"));
    QCOMPARE(mdexport::frontMatterValue(fm, QStringLiteral("title")),
             QStringLiteral("Mi Título"));
    QVERIFY(mdexport::frontMatterValue(fm, QStringLiteral("author")).isEmpty());
}

void TestExporters::latexHasBabelAndTitle()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QStringLiteral("T"));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[spanish]{babel}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{document}")));
    QVERIFY(tex.contains(QStringLiteral("\\end{document}")));
    QVERIFY(tex.contains(QStringLiteral("\\title{T}")));
    QVERIFY(tex.contains(QStringLiteral("\\maketitle")));
}

void TestExporters::latexEscapesSpecialChars()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("100% de a & b, costo $5 #1\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("en")), QString());
    QVERIFY(tex.contains(QStringLiteral("100\\% de a \\& b")));
    QVERIFY(tex.contains(QStringLiteral("\\$5")));
    QVERIFY(tex.contains(QStringLiteral("\\#1")));
    // Sin título: no hay \maketitle.
    QVERIFY(!tex.contains(QStringLiteral("\\maketitle")));
}

void TestExporters::latexConstructs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "- uno\n- dos\n\n1. a\n2. b\n\n> cita\n\n`x` y **negrita**\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(tex.contains(QStringLiteral("\\begin{itemize}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{enumerate}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{quote}")));
    QVERIFY(tex.contains(QStringLiteral("\\texttt{x}")));
    QVERIFY(tex.contains(QStringLiteral("\\textbf{negrita}")));
}

void TestExporters::latexHeadingNotDoublyBold()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Encabezado\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(tex.contains(QStringLiteral("\\section{Encabezado}")));
    QVERIFY(!tex.contains(QStringLiteral("\\section{\\textbf{")));
}

void TestExporters::latexSanitizesHighUnicode()
{
    QTextDocument doc;
    // ✅ (U+2705) rompía pdflatex; un emoji arbitrario (🚀) debe omitirse; la raya
    // — y los acentos deben conservarse.
    doc.setMarkdown(QStringLiteral("✅ hecho — cañón 🚀\n\n```\ncode ✅ x\n```\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(!tex.contains(QChar(0x2705)));   // ✅ ya no aparece crudo
    QVERIFY(!tex.contains(QChar(0x1F680)));  // 🚀 (par subrogado) omitido
    QVERIFY(tex.contains(QStringLiteral("\\checkmark")));  // ✅ → símbolo LaTeX
    QVERIFY(tex.contains(QString::fromUtf8("cañón")));     // acentos intactos
    QVERIFY(tex.contains(QString::fromUtf8("—")));         // raya intacta
    // En verbatim no caben comandos: el ✅ se omite, no se convierte.
    QVERIFY(tex.contains(QStringLiteral("code  x")));
    // Preámbulo portable entre motores.
    QVERIFY(tex.contains(QStringLiteral("\\usepackage{iftex}")));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage{fontspec}")));
}

void TestExporters::odfStylesCarryLanguage()
{
    const QByteArray xml =
        mdexport::odfStylesXml(mdexport::languageForCode(QStringLiteral("fr")));
    QVERIFY(xml.contains("fo:language=\"fr\""));
    QVERIFY(xml.contains("fo:country=\"FR\""));
    QVERIFY(xml.contains("style:default-style"));
}

void TestExporters::odfMetaCarriesLanguageAndTitle()
{
    const QByteArray xml = mdexport::odfMetaXml(
        mdexport::languageForCode(QStringLiteral("it")), QStringLiteral("Mi Doc"));
    QVERIFY(xml.contains("<dc:language>it-IT</dc:language>"));
    QVERIFY(xml.contains("<dc:title>Mi Doc</dc:title>"));
}

void TestExporters::odfManifestListsExtraFiles()
{
    const QByteArray manifest =
        "<?xml version=\"1.0\"?>\n<manifest:manifest>\n"
        " <manifest:file-entry manifest:full-path=\"content.xml\"/>\n"
        "</manifest:manifest>\n";
    const QByteArray out = mdexport::odfManifestWithLanguageFiles(manifest);
    QVERIFY(out.contains("full-path=\"styles.xml\""));
    QVERIFY(out.contains("full-path=\"meta.xml\""));
    QVERIFY(out.contains("</manifest:manifest>"));
}

void TestExporters::docxDocumentConstructs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "# Título\n\n**negrita** *cursiva* `codigo`\n\n"
        "[enlace](https://example.com)\n\n"
        "- uno\n- dos\n\n> cita\n\n| a | b |\n|---|---|\n| 1 | 2 |\n"));
    const QString xml = mdexport::toDocxDocumentXml(&doc, QString(), nullptr);

    QVERIFY(xml.contains(QStringLiteral("<w:document")));
    QVERIFY(xml.contains(QStringLiteral("<w:pStyle w:val=\"Heading1\"/>")));
    QVERIFY(xml.contains(QStringLiteral("<w:b/>")));
    QVERIFY(xml.contains(QStringLiteral("<w:i/>")));
    QVERIFY(xml.contains(QStringLiteral("Courier New")));            // código monoespaciado
    QVERIFY(xml.contains(QStringLiteral("HYPERLINK")));              // enlace como campo
    QVERIFY(xml.contains(QStringLiteral("example.com")));
    QVERIFY(xml.contains(QStringLiteral("<w:numPr>")));             // lista con numeración real
    QVERIFY(xml.contains(QStringLiteral("<w:tbl>")));               // tabla
    QVERIFY(xml.contains(QStringLiteral("<w:sectPr>")));           // propiedades de sección
}

void TestExporters::docxTitleParagraph()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("cuerpo\n"));
    const QString xml = mdexport::toDocxDocumentXml(&doc, QStringLiteral("Mi Doc"), nullptr);
    QVERIFY(xml.contains(QStringLiteral("<w:pStyle w:val=\"Title\"/>")));
    QVERIFY(xml.contains(QStringLiteral("Mi Doc")));
}

void TestExporters::docxStylesCarryLanguage()
{
    const QByteArray xml =
        mdexport::docxStylesXml(mdexport::languageForCode(QStringLiteral("de")));
    QVERIFY(xml.contains("<w:lang w:val=\"de-DE\"/>"));
    QVERIFY(xml.contains("w:styleId=\"Heading1\""));
    QVERIFY(xml.contains("w:styleId=\"Title\""));
}

void TestExporters::docxNumberingHasBulletAndDecimal()
{
    const QByteArray xml = mdexport::docxNumberingXml();
    QVERIFY(xml.contains("w:numFmt w:val=\"bullet\""));
    QVERIFY(xml.contains("w:numFmt w:val=\"decimal\""));
    QVERIFY(xml.contains("w:numId=\"1\""));
    QVERIFY(xml.contains("w:numId=\"2\""));
}

void TestExporters::docxWriteProducesZipPackage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("salida.docx"));

    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo con **negrita**\n"));
    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("T"), &error),
             qPrintable(error));

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray head = f.read(2);
    QCOMPARE(head, QByteArray("PK"));  // firma de un paquete ZIP (OOXML)
    QVERIFY(f.size() > 0);
}

QTEST_MAIN(TestExporters)
#include "tst_exporters.moc"
