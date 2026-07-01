#include <QtTest>

#include <QDir>
#include <QFile>
#include <QFont>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFragment>
#include <QXmlStreamReader>

#include "codehighlighter.h"
#include "exporters.h"
#include "mathblocks.h"
#include "themespec.h"

// Pruebas de la exportación: el serializador LaTeX puro, la lectura del front
// matter y los XML de idioma del ODF. El empaquetado real del ODT (QZip) se
// comprueba con un smoke test aparte.
class TestExporters : public QObject
{
    Q_OBJECT

private slots:
    void languageLookupNormalizesCode();
    void frontMatterValueReadsKeys();
    void pdfInfoReadsTitleAndAuthor();
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
    void htmlBodyToXhtmlExtractsAndSanitizes();
    void epubBuildersAreWellFormedXml();
    void epubContentXhtmlWrapsBody();
    void epubWriteProducesZipPackage();
    void twoDFormulaExpandsForHtmlAndLatex();
    void plainTextFlattensTwoDFormula();
    void codeHighlightingBakedIntoExport();
    void cloneNormalizesFontSizeAwayFromZoom();
    void cloneCodeInheritsBodyFontSize();
};

// ¿`xml` es XML bien formado? (para validar las piezas del EPUB).
static bool isWellFormedXml(const QByteArray &xml)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd())
        r.readNext();
    return !r.hasError();
}

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

void TestExporters::pdfInfoReadsTitleAndAuthor()
{
    const mdexport::PdfInfo info = mdexport::pdfDocumentInfo(
        QStringLiteral("---\ntitle: \"Mi Doc\"\nauthor: Ada Lovelace\n---\n"));
    QCOMPARE(info.title, QStringLiteral("Mi Doc"));
    QCOMPARE(info.creator, QStringLiteral("Ada Lovelace"));

    // Sin `author`, cae a `creator`.
    const mdexport::PdfInfo alt = mdexport::pdfDocumentInfo(
        QStringLiteral("+++\ntitle = T\ncreator = Babbage\n+++\n"));
    QCOMPARE(alt.creator, QStringLiteral("Babbage"));

    // Sin front matter, ambos vacíos.
    const mdexport::PdfInfo none = mdexport::pdfDocumentInfo(QString());
    QVERIFY(none.title.isEmpty() && none.creator.isEmpty());
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
    QVERIFY(!tex.contains(QChar(0x2705)));        // ✅ ya no aparece crudo
    QVERIFY(!tex.contains(QStringLiteral("🚀")));  // 🚀 omitido (es astral: QChar no lo
                                                  // representa, hay que comparar el QString)
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

void TestExporters::htmlBodyToXhtmlExtractsAndSanitizes()
{
    const QString html = QStringLiteral(
        "<!DOCTYPE HTML><html><head><style>p{}</style></head>"
        "<body style=\"x\"><p>Hola&nbsp;mundo</p><hr></body></html>");
    const QString body = mdexport::htmlBodyToXhtml(html);
    QVERIFY(body.contains(QStringLiteral("<p>Hola")));
    QVERIFY(!body.contains(QStringLiteral("&nbsp;")));   // entidad XML inválida saneada
    QVERIFY(body.contains(QStringLiteral("&#160;")));
    QVERIFY(!body.contains(QStringLiteral("<body")));    // solo el interior
    QVERIFY(body.contains(QStringLiteral("<hr/>")));     // elemento vacío cerrado
}

void TestExporters::epubBuildersAreWellFormedXml()
{
    const mdexport::Language es = mdexport::languageForCode(QStringLiteral("es"));
    QVERIFY(isWellFormedXml(mdexport::epubContainerXml()));
    const QByteArray opf = mdexport::epubContentOpf(
        es, QStringLiteral("Mi <libro>"), {QStringLiteral("images/image1.png")},
        QStringLiteral("abc-123"), QStringLiteral("2026-01-01T00:00:00Z"));
    QVERIFY(isWellFormedXml(opf));
    QVERIFY(opf.contains("<dc:language>es</dc:language>"));
    QVERIFY(opf.contains("urn:uuid:abc-123"));
    QVERIFY(opf.contains("image1.png"));
    QVERIFY(opf.contains("dcterms:modified"));
    QVERIFY(isWellFormedXml(mdexport::epubNavXhtml(es, QStringLiteral("T"))));
    QVERIFY(isWellFormedXml(mdexport::epubTocNcx(QStringLiteral("T"), QStringLiteral("abc-123"))));
}

void TestExporters::epubContentXhtmlWrapsBody()
{
    const mdexport::Language es = mdexport::languageForCode(QStringLiteral("es"));
    const QString xhtml = mdexport::epubContentXhtml(
        QStringLiteral("<p>cuerpo</p>"), QStringLiteral("Título"), es);
    QVERIFY(isWellFormedXml(xhtml.toUtf8()));
    QVERIFY(xhtml.contains(QStringLiteral("xml:lang=\"es\"")));
    QVERIFY(xhtml.contains(QStringLiteral("<p>cuerpo</p>")));
}

void TestExporters::epubWriteProducesZipPackage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("salida.epub"));

    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo con **negrita**\n"));
    QString error;
    QVERIFY2(mdexport::writeEpub(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("T"), &error),
             qPrintable(error));

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray all = f.readAll();
    QCOMPARE(all.left(2), QByteArray("PK"));             // firma ZIP
    QVERIFY(all.contains("application/epub+zip"));       // mimetype
    QVERIFY(all.contains("mimetype"));
}

// Una fórmula 2D vive en el documento como un carácter objeto que ningún writer
// sabe pintar. cloneForExport debe expandirla a runs inline para HTML/ODF/PDF/
// DOCX; LaTeX, que usa el documento original, la emite como `$$tex$$` desde la
// propiedad. Verificamos que no se cuela un U+FFFC (objeto sin pintar).
void TestExporters::twoDFormulaExpandsForHtmlAndLatex()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(
        QStringLiteral("Sea $$\\sum_{i=1}^n \\frac{x_i}{2}$$ fin\n")));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    const QString html = flat->toHtml();
    QVERIFY2(!html.contains(QChar(0xFFFC)),
             "el carácter objeto no debe llegar al HTML sin expandir");
    QVERIFY(html.contains(QChar(0x2211)));  // Σ del sumatorio expandido

    const QString latex = mdexport::toLatex(&doc, mdexport::Language{}, QString());
    QVERIFY(latex.contains(QStringLiteral("\\sum_{i=1}^n")));
    QVERIFY(latex.contains(QStringLiteral("\\frac{x_i}{2}")));
}

// exportPlainText() escribe doc->toPlainText() sobre el clon plano. cloneForExport
// debe expandir la fórmula 2D a runs para que su texto aparezca en el .txt, no el
// U+FFFC del carácter objeto sin pintar.
void TestExporters::plainTextFlattensTwoDFormula()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(
        QStringLiteral("Sea $$\\sum_{i=1}^n \\frac{x_i}{2}$$ fin\n")));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    const QString text = flat->toPlainText();
    QVERIFY2(!text.contains(QChar(0xFFFC)),
             "el carácter objeto no debe llegar al texto plano sin expandir");
    QVERIFY(text.contains(QChar(0x2211)));  // Σ del sumatorio expandido a runs
    QVERIFY(text.contains(QStringLiteral("Sea")) && text.contains(QStringLiteral("fin")));
}

// El resaltado de sintaxis (overlay del QSyntaxHighlighter) se hornea en el clon
// de exportación como color de carácter real, para que el código exporte con color
// (HTML/ODF/DOCX/EPUB/PDF). El clon directo no lo lleva.
void TestExporters::codeHighlightingBakedIntoExport()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("```cpp\nint main() { return 42; }\n```\n"));
    CodeBlockHighlighter hl(&doc);
    hl.setSyntaxColors(mdtheme::specFor(mdtheme::ThemeId::Light).syntax);
    hl.rehighlight();

    std::unique_ptr<QTextDocument> plain(doc.clone());
    QVERIFY2(!plain->toHtml().contains(QStringLiteral("color:#")),
             "el clon directo no lleva el color del resaltado (es overlay)");

    std::unique_ptr<QTextDocument> baked(mdexport::cloneForExport(&doc));
    // El azul de keyword del tema claro (#0000ff) del «int»/«return».
    QVERIFY2(baked->toHtml().contains(QStringLiteral("color:#0000ff"), Qt::CaseInsensitive),
             "el resaltado de código debe conservarse en la exportación");
}

// El zoom de interfaz agranda la fuente del editor (y con ella el defaultFont del
// documento). La exportación NO debe heredar ese tamaño de pantalla: cloneForExport
// normaliza el cuerpo a un tamaño estándar, aunque el origen venga inflado.
void TestExporters::cloneNormalizesFontSizeAwayFromZoom()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Título\n\nCuerpo del documento.\n"));
    QFont zoomed = doc.defaultFont();
    zoomed.setPointSizeF(30.0);  // simula un zoom fuerte
    doc.setDefaultFont(zoomed);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    // Independiente del tamaño de pantalla: normalizado a 11 pt, no 30.
    QCOMPARE(flat->defaultFont().pointSizeF(), 11.0);
    // La familia y demás atributos se conservan.
    QCOMPARE(flat->defaultFont().family(), doc.defaultFont().family());
    // Y el HTML resultante no lleva el tamaño inflado.
    QVERIFY(!flat->toHtml().contains(QStringLiteral("font-size:30")));
}

// Qt hornea un tamaño ABSOLUTO en los runs de código (inline y bloque); si no se
// neutraliza, sale con distinto tamaño que el cuerpo tras normalizar el defaultFont.
// cloneForExport quita ese tamaño absoluto para que el código herede el cuerpo.
void TestExporters::cloneCodeInheritsBodyFontSize()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "Texto con `codigo` en linea.\n\n```\nbloque\n```\n"));
    QFont zoomed = doc.defaultFont();
    zoomed.setPointSizeF(30.0);
    doc.setDefaultFont(zoomed);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    // Ningún fragmento conserva un tamaño de fuente absoluto: todos heredan el cuerpo.
    for (QTextBlock b = flat->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); it != b.end(); ++it)
            if (it.fragment().isValid())
                QVERIFY2(!it.fragment().charFormat().hasProperty(QTextFormat::FontPointSize),
                         qPrintable(QStringLiteral("fragmento con tamaño absoluto: '%1'")
                                        .arg(it.fragment().text())));
    // El HTML no emite el tamaño de pantalla en el código.
    QVERIFY(!flat->toHtml().contains(QStringLiteral("font-size:30")));
}

QTEST_MAIN(TestExporters)
#include "tst_exporters.moc"
