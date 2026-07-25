#include <QtTest>

#include <memory>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QProcess>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextTable>
#include <QUrl>
#include <QXmlStreamReader>

#include <private/qzipreader_p.h>

#include "documentio.h"
#include "exporters.h"
#include "markdownrender.h"
#include "mathblocks.h"
#include "pandocimport.h"

#ifndef DOCX_FIXTURE_DIR
#define DOCX_FIXTURE_DIR "."
#endif

// Pruebas de ida y vuelta del formato DOCX, escritas para CAZAR ERRORES más que
// para documentar la salida (de eso ya se encargan el golden de tst_goldenexport
// y los smoke tests de tst_exporters, que solo miran si aparece tal o cual
// etiqueta).
//
// Dos mitades:
//
//   * EXPORTACIÓN — nuestro serializador OOXML. Un .docx roto no falla al
//     escribirse: falla al ABRIRLO en Word, que rechaza el paquete entero ante un
//     XML mal formado, una relación colgante o una parte sin tipo de contenido.
//     Aquí se valida el paquete como lo haría el consumidor: XML bien formado en
//     TODAS las partes, [Content_Types].xml que cubre todo, relaciones que
//     resuelven, y el texto que entra igual al que sale (escapado, caracteres de
//     control, fuera del BMP).
//
//   * IMPORTACIÓN — la conversión con Pandoc de .docx AJENOS, con los ficheros de
//     tests/fixtures/docx/ (ver su README). Se comprueba la cadena entera:
//     el .docx → Pandoc → Markdown → nuestro pipeline de carga (mdrender). Los
//     .docx de prueba llevan a propósito construcciones que la app nunca genera
//     (runs partidos a mitad de palabra, hipervínculos por relación, cambios
//     controlados, tabla anidada, nota al pie) porque son justo las que rompen a
//     un importador. Se omiten con QSKIP si Pandoc no está instalado.
class TestDocx : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    // --- Exportación ---
    void richDocumentIsWellFormedXml();
    void xmlMetacharactersSurviveEscaping();
    void controlCharactersDoNotCorruptXml();
    void nonBmpCharactersSurvive();
    void softLineBreaksBecomeBrElements();
    void packageIsSelfConsistent();
    void emptyDocumentProducesValidPackage();
    void imagesMatchTheirRelationships();
    void oversizedImageIsScaledKeepingAspect();
    void tablesAreAlwaysClosedByAParagraph();
    void deepListNestingClampsToNineLevels();
    void taskMarkersAreEmitted();
    void hyperlinkTargetsTravelInRelationships();
    void twoDFormulaLeavesNoObjectCharacter();

    // --- Importación (Pandoc + ficheros .docx de prueba) ---
    void fixtureFilesAreRealPackages();
    void importedMarkdownHasExpectedConstructs_data();
    void importedMarkdownHasExpectedConstructs();
    void emptyDocxImportsToNothing();
    void importedMarkdownLoadsIntoTheEditorDocument();
    void importedImagesReachTheDocument();
    void importedTitleArrivesAsFrontMatter();
    void importKnownLimitationsAreStillTheSame();
    void roundTripThroughPandocKeepsStructure();

private:
    QString m_fixtures;
    bool m_pandoc = false;

    /// Ejecuta Pandoc igual que MainWindow::importWithPandoc y devuelve el Markdown.
    /// `mediaDir` (opcional) es donde extraer las imágenes: SIEMPRE un directorio
    /// temporal, para no ensuciar el árbol fuente al lado de los .docx de prueba.
    QString importWithPandoc(const QString &fixture, const QString &mediaDir = QString());
    /// Ruta absoluta de un .docx de prueba.
    QString fixture(const QString &name) const { return m_fixtures + QLatin1Char('/') + name; }
};

// ---------------------------------------------------------------------------
// Utilidades
// ---------------------------------------------------------------------------

namespace {

const QString kNsW =
    QStringLiteral("http://schemas.openxmlformats.org/wordprocessingml/2006/main");
const QString kNsR =
    QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/relationships");

/// ¿XML bien formado? El motivo va a `why` para que el fallo sea legible.
bool wellFormed(const QByteArray &xml, QString *why = nullptr)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd())
        r.readNext();
    if (r.hasError() && why)
        *why = QStringLiteral("línea %1: %2")
                   .arg(QString::number(r.lineNumber()), r.errorString());
    return !r.hasError();
}

/// Todo el texto de los `<w:t>` concatenado, en orden: lo que un lector vería.
QString wordText(const QByteArray &xml)
{
    QString out;
    QXmlStreamReader r(xml);
    while (!r.atEnd()) {
        if (r.readNext() == QXmlStreamReader::StartElement
            && r.name() == QLatin1String("t") && r.namespaceUri() == kNsW)
            out += r.readElementText();
    }
    return out;
}

/// Valores del atributo `attr` (del espacio de nombres `ns`) en cada `element`.
QStringList attributeValues(const QByteArray &xml, const QString &element,
                            const QString &ns, const QString &attr)
{
    QStringList out;
    QXmlStreamReader r(xml);
    while (!r.atEnd()) {
        if (r.readNext() != QXmlStreamReader::StartElement)
            continue;
        if (r.name() != element)
            continue;
        const QStringView v = r.attributes().value(ns, attr);
        if (!v.isEmpty())
            out << v.toString();
    }
    return out;
}

/// Todas las partes de un paquete OOXML, por su ruta interna.
QHash<QString, QByteArray> readPackage(const QString &path)
{
    QHash<QString, QByteArray> parts;
    QZipReader zip(path);
    const QList<QZipReader::FileInfo> infos = zip.fileInfoList();
    for (const QZipReader::FileInfo &fi : infos) {
        if (fi.isFile)
            parts.insert(fi.filePath, zip.fileData(fi.filePath));
    }
    return parts;
}

/// Documento de referencia con TODAS las construcciones que el serializador
/// distingue: encabezados, marcas de carácter, enlace, listas, tareas, cita,
/// código, tabla, regla y alineaciones.
QString kitchenSinkMarkdown()
{
    return QStringLiteral(
        "# Encabezado 1\n\n"
        "## Encabezado 2\n\n"
        "Párrafo con **negrita**, *cursiva*, ~~tachado~~ y `código en línea`.\n\n"
        "Un [enlace](https://example.com/destino) en mitad del texto.\n\n"
        "- viñeta uno\n"
        "- viñeta dos\n"
        "  - viñeta anidada\n\n"
        "1. numerada uno\n"
        "2. numerada dos\n\n"
        "- [ ] tarea pendiente\n"
        "- [x] tarea hecha\n\n"
        "> Una cita en bloque.\n\n"
        "```cpp\n"
        "int main() { return 0; }\n"
        "```\n\n"
        "| Concepto | Importe |\n"
        "|:---------|--------:|\n"
        "| Licencias | 1.200 |\n"
        "| Soporte | 300 |\n\n"
        "---\n\n"
        "Último párrafo.\n");
}

/// Carga Markdown como lo hace el editor (mismo dialecto y mismas extensiones).
void loadLikeEditor(QTextDocument *doc, const QString &markdown)
{
    doc->setMarkdown(mdrender::protect(markdown), mdrender::kMarkdownFeatures);
    mdrender::renderPasses(doc);
}

/// Imagen sólida registrada como recurso del documento, lista para insertarla.
void addImageResource(QTextDocument *doc, const QString &name, int w, int h, QRgb color)
{
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(color);
    doc->addResource(QTextDocument::ImageResource, QUrl(name), QVariant(img));
}

}  // namespace

void TestDocx::initTestCase()
{
    m_fixtures = QStringLiteral(DOCX_FIXTURE_DIR);
    m_pandoc = mdimport::pandocAvailable();
}

QString TestDocx::importWithPandoc(const QString &name, const QString &mediaDir)
{
    QProcess pandoc;
    pandoc.start(QStringLiteral("pandoc"), mdimport::pandocArguments(fixture(name), mediaDir));
    if (!pandoc.waitForFinished(30000))
        return QString();
    if (pandoc.exitStatus() != QProcess::NormalExit || pandoc.exitCode() != 0) {
        qWarning("pandoc falló con %s: %s", qPrintable(name),
                 pandoc.readAllStandardError().constData());
        return QString();
    }
    // Mismo post-proceso que MainWindow: sin él, ni las imágenes ni las tablas que
    // GFM no expresa llegan al editor.
    return mdimport::htmlTablesToMarkdown(
        mdimport::repairImages(QString::fromUtf8(pandoc.readAllStandardOutput())));
}

// ===========================================================================
// EXPORTACIÓN
// ===========================================================================

// Un `word/document.xml` mal formado hace que Word rechace el fichero entero sin
// más explicación. Es la comprobación más barata y la que más errores caza: se
// hace sobre un documento con todas las construcciones a la vez.
void TestDocx::richDocumentIsWellFormedXml()
{
    QTextDocument doc;
    loadLikeEditor(&doc, kitchenSinkMarkdown());

    const QByteArray xml =
        mdexport::toDocxDocumentXml(&doc, QStringLiteral("Título del documento")).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));
}

// El texto del usuario no puede colarse crudo en el XML: `& < > " '` tienen que
// escaparse en TODOS los sitios donde se emite texto (párrafo, encabezado, celda
// de tabla, bloque de código, título del documento y destino del enlace) y volver
// a leerse idénticos.
void TestDocx::xmlMetacharactersSurviveEscaping()
{
    const QString peligro = QStringLiteral("R&D <tag> \"comillas\" 'simples'");

    QTextDocument doc;
    loadLikeEditor(&doc, QStringLiteral(
        "# %1\n\n"
        "Párrafo: %1\n\n"
        "| %1 | b |\n|---|---|\n| %1 | d |\n\n"
        "```\ncodigo: %1\n```\n\n"
        "[texto](https://example.com/?a=1&b=2&c=x)\n").arg(peligro));

    QList<mdexport::DocxHyperlink> links;
    const QByteArray xml =
        mdexport::toDocxDocumentXml(&doc, peligro, nullptr, &links).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));

    // Ni una sola aparición sin escapar de los metacaracteres: si `<` o `&`
    // llegasen crudos el XML ya habría fallado arriba, pero el `>` y las comillas
    // pueden pasar desapercibidos.
    const QString text = wordText(xml);
    QVERIFY2(text.contains(peligro), qPrintable(text.left(400)));
    // El título va como párrafo «Title»: aparece además de las 5 veces del cuerpo.
    QCOMPARE(text.count(peligro), 6);

    // El destino del enlace viaja íntegro (con su `&`) hasta la relación.
    QCOMPARE(links.size(), 1);
    QCOMPARE(links.first().target, QStringLiteral("https://example.com/?a=1&b=2&c=x"));
}

// Los caracteres de control C0 (salvo tabulador, salto de línea y retorno) NO son
// XML válido: si el documento trae uno —pegando texto de otra aplicación, o desde
// un .md con basura binaria— el .docx sale corrupto y Word lo rechaza entero.
// Deben desaparecer de la salida, no viajar crudos.
void TestDocx::controlCharactersDoNotCorruptXml()
{
    // Los C0 se construyen a mano: en el fuente irían crudos y cualquier editor
    // (o un guardado descuidado) los borraría sin que se notase.
    QString basura = QStringLiteral("antes");
    for (const char16_t bad : {u'\x0001', u'\x0002', u'\x0008', u'\x000B', u'\x000C', u'\x001F'})
        basura += QChar(bad);
    basura += QStringLiteral("después");

    QTextDocument doc;
    QTextCursor c(&doc);
    c.insertText(basura);
    c.insertBlock();
    c.insertText(QStringLiteral("con tabulador\tal medio"));  // el tabulador SÍ es válido
    c.insertBlock();
    // U+FFFE y U+FFFF tampoco son caracteres XML válidos.
    c.insertText(QStringLiteral("no permitidos: ") + QChar(0xFFFE) + QChar(0xFFFF));

    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString()).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));

    const QString text = wordText(xml);
    QVERIFY2(text.contains(QStringLiteral("antes")) && text.contains(QStringLiteral("después")),
             qPrintable(QStringLiteral("el texto legible debe conservarse: ") + text));
    for (const char16_t bad : {u'\x0001', u'\x0002', u'\x0008', u'\x000B', u'\x000C',
                               u'\x001F', u'\xFFFE', u'\xFFFF'})
        QVERIFY2(!text.contains(QChar(bad)),
                 qPrintable(QStringLiteral("U+%1 no debe llegar al XML")
                                .arg(QString::number(bad, 16))));
    QVERIFY(text.contains(QStringLiteral("con tabulador\tal medio")));
}

// Emoji y alfabetos matemáticos viven fuera del BMP (pares suplentes en UTF-16).
// Trocear mal la cadena al escapar los parte por la mitad y produce suplentes
// sueltos, que tampoco son XML válido.
void TestDocx::nonBmpCharactersSurvive()
{
    const QString texto = QStringLiteral("Prueba \U0001F9EA con \U0001D49E y \U0001F1EA\U0001F1F8 final");

    QTextDocument doc;
    QTextCursor(&doc).insertText(texto);

    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString()).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));
    QCOMPARE(wordText(xml), texto);
}

// Un salto de línea suave (Mayús+Intro en el editor: U+2028) vive DENTRO del
// párrafo, no lo parte: en OOXML es `<w:br/>`. Si se emitiese literal dentro de
// `<w:t>` se perdería el salto al abrir en Word.
void TestDocx::softLineBreaksBecomeBrElements()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    c.insertText(QStringLiteral("primera línea") + QChar(QChar::LineSeparator)
                 + QStringLiteral("segunda línea"));

    const QString xml = mdexport::toDocxDocumentXml(&doc, QString());
    QVERIFY2(xml.contains(QStringLiteral("<w:br/>")), qPrintable(xml));
    // Y las dos mitades siguen en el MISMO párrafo.
    QCOMPARE(xml.count(QStringLiteral("<w:p>")) + xml.count(QStringLiteral("<w:p/>")), 1);

    const QString text = wordText(xml.toUtf8());
    QVERIFY(text.contains(QStringLiteral("primera línea")));
    QVERIFY(text.contains(QStringLiteral("segunda línea")));
}

// El paquete completo, validado como lo haría el consumidor: partes obligatorias
// presentes, todo XML bien formado, [Content_Types].xml cubriendo cada parte,
// relaciones sin destinos colgantes y sin ids repetidos, y cada `r:id`/`r:embed`
// citado en el documento resuelto por una relación real.
void TestDocx::packageIsSelfConsistent()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("paquete.docx"));

    QTextDocument doc;
    loadLikeEditor(&doc, kitchenSinkMarkdown());
    addImageResource(&doc, QStringLiteral("uno.png"), 32, 24, qRgb(200, 30, 30));
    addImageResource(&doc, QStringLiteral("dos.png"), 16, 16, qRgb(30, 200, 30));
    QTextCursor c(&doc);
    c.movePosition(QTextCursor::End);
    c.insertBlock();
    c.insertImage(QStringLiteral("uno.png"));
    c.insertText(QStringLiteral(" entre imágenes "));
    c.insertImage(QStringLiteral("dos.png"));

    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("Paquete"), &error),
             qPrintable(error));

    const QHash<QString, QByteArray> parts = readPackage(path);
    QVERIFY2(!parts.isEmpty(), "el .docx no se pudo abrir como ZIP");

    for (const QString &required : {QStringLiteral("[Content_Types].xml"),
                                    QStringLiteral("_rels/.rels"),
                                    QStringLiteral("docProps/core.xml"),
                                    QStringLiteral("word/document.xml"),
                                    QStringLiteral("word/styles.xml"),
                                    QStringLiteral("word/numbering.xml"),
                                    QStringLiteral("word/_rels/document.xml.rels")}) {
        QVERIFY2(parts.contains(required), qPrintable(required));
    }

    // 1) Todas las partes XML, bien formadas.
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        if (!it.key().endsWith(QLatin1String(".xml")) && !it.key().endsWith(QLatin1String(".rels")))
            continue;
        QString why;
        QVERIFY2(wellFormed(it.value(), &why),
                 qPrintable(it.key() + QStringLiteral(": ") + why));
    }

    // 2) [Content_Types].xml declara un tipo para cada parte, por extensión
    //    (Default) o por nombre (Override). Una parte sin tipo invalida el paquete.
    const QByteArray types = parts.value(QStringLiteral("[Content_Types].xml"));
    QSet<QString> defaults, overrides;
    {
        QXmlStreamReader r(types);
        while (!r.atEnd()) {
            if (r.readNext() != QXmlStreamReader::StartElement)
                continue;
            if (r.name() == QLatin1String("Default"))
                defaults.insert(r.attributes().value(QLatin1String("Extension")).toString().toLower());
            else if (r.name() == QLatin1String("Override"))
                overrides.insert(r.attributes().value(QLatin1String("PartName")).toString());
        }
    }
    for (auto it = parts.cbegin(); it != parts.cend(); ++it) {
        if (it.key() == QLatin1String("[Content_Types].xml"))
            continue;
        const QString ext = QFileInfo(it.key()).suffix().toLower();
        const bool covered = overrides.contains(QLatin1Char('/') + it.key())
                             || defaults.contains(ext);
        QVERIFY2(covered, qPrintable(QStringLiteral("sin tipo de contenido: ") + it.key()));
    }

    // 3) Las relaciones apuntan a partes que existen y no repiten Id.
    for (const QString &relsPart : {QStringLiteral("_rels/.rels"),
                                    QStringLiteral("word/_rels/document.xml.rels")}) {
        const QString base = relsPart == QLatin1String("_rels/.rels")
                                 ? QString()
                                 : QStringLiteral("word/");
        QSet<QString> ids;
        QXmlStreamReader r(parts.value(relsPart));
        while (!r.atEnd()) {
            if (r.readNext() != QXmlStreamReader::StartElement
                || r.name() != QLatin1String("Relationship"))
                continue;
            const QString id = r.attributes().value(QLatin1String("Id")).toString();
            const QString target = r.attributes().value(QLatin1String("Target")).toString();
            const bool external =
                r.attributes().value(QLatin1String("TargetMode")) == QLatin1String("External");
            QVERIFY2(!ids.contains(id), qPrintable(QStringLiteral("Id repetido: ") + id));
            ids.insert(id);
            if (external || target.startsWith(QLatin1Char('/')))
                continue;
            QVERIFY2(parts.contains(base + target),
                     qPrintable(QStringLiteral("relación colgante: ") + base + target));
        }
    }

    // 4) Cada rId citado en el documento tiene relación (una imagen con un rId
    //    inventado deja el .docx irreparable a ojos de Word).
    const QByteArray document = parts.value(QStringLiteral("word/document.xml"));
    QSet<QString> declared;
    {
        QXmlStreamReader r(parts.value(QStringLiteral("word/_rels/document.xml.rels")));
        while (!r.atEnd()) {
            if (r.readNext() == QXmlStreamReader::StartElement
                && r.name() == QLatin1String("Relationship"))
                declared.insert(r.attributes().value(QLatin1String("Id")).toString());
        }
    }
    int referenced = 0;
    QXmlStreamReader r(document);
    while (!r.atEnd()) {
        if (r.readNext() != QXmlStreamReader::StartElement)
            continue;
        for (const QXmlStreamAttribute &a : r.attributes()) {
            if (a.namespaceUri() != kNsR)
                continue;
            ++referenced;
            QVERIFY2(declared.contains(a.value().toString()),
                     qPrintable(QStringLiteral("rId sin relación: ") + a.value().toString()));
        }
    }
    // Dos imágenes y un enlace: si no se citó ningún rId, la comprobación de
    // arriba habría pasado en vacío.
    QCOMPARE(referenced, 3);
    QVERIFY(parts.contains(QStringLiteral("word/media/image1.png")));
    QVERIFY(parts.contains(QStringLiteral("word/media/image2.png")));
}

// El documento recién creado (vacío) también debe producir un .docx abrible: el
// cuerpo no puede quedarse sin ningún `<w:p>`, que OOXML no admite.
void TestDocx::emptyDocumentProducesValidPackage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("vacio.docx"));

    QTextDocument doc;
    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path, mdexport::languageForCode(QStringLiteral("es")),
                                 QString(), &error),
             qPrintable(error));

    const QHash<QString, QByteArray> parts = readPackage(path);
    const QByteArray document = parts.value(QStringLiteral("word/document.xml"));
    QString why;
    QVERIFY2(wellFormed(document, &why), qPrintable(why));
    QVERIFY(document.contains("<w:p"));
    QVERIFY(document.contains("<w:sectPr>"));
}

// Cada `<a:blip r:embed>` debe casar con una relación de imagen y con una parte
// PNG real, y los `wp:docPr`/`pic:cNvPr` llevan ids únicos (Word rechaza el
// duplicado). Se cruzan las tres listas.
void TestDocx::imagesMatchTheirRelationships()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    for (int i = 1; i <= 3; ++i) {
        const QString name = QStringLiteral("img%1.png").arg(i);
        addImageResource(&doc, name, 8 * i, 6 * i, qRgb(10 * i, 20 * i, 30 * i));
        c.insertText(QStringLiteral("imagen %1: ").arg(i));
        c.insertImage(name);
        c.insertBlock();
    }

    QList<mdexport::DocxImage> images;
    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString(), &images).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));
    QCOMPARE(images.size(), 3);

    // Los bytes registrados son PNG legibles (no un QByteArray vacío).
    for (const mdexport::DocxImage &img : images) {
        QImage decoded;
        QVERIFY2(decoded.loadFromData(img.data, "PNG"), qPrintable(img.partName));
        QVERIFY(!decoded.isNull());
    }

    const QStringList embeds =
        attributeValues(xml, QStringLiteral("blip"), kNsR, QStringLiteral("embed"));
    QCOMPARE(embeds.size(), 3);
    QCOMPARE(QSet<QString>(embeds.begin(), embeds.end()).size(), 3);  // sin repetir

    const QStringList docPrIds =
        attributeValues(xml, QStringLiteral("docPr"), QString(), QStringLiteral("id"));
    QCOMPARE(docPrIds.size(), 3);
    QCOMPARE(QSet<QString>(docPrIds.begin(), docPrIds.end()).size(), 3);
    for (const QString &id : docPrIds)
        QVERIFY2(id.toInt() > 0, qPrintable(id));  // 0 no es un id válido en OOXML
}

// Una imagen más ancha que la caja de texto se escala al ancho útil de la página
// MANTENIENDO la proporción; si no, en Word sale recortada o deformada.
void TestDocx::oversizedImageIsScaledKeepingAspect()
{
    QTextDocument doc;
    addImageResource(&doc, QStringLiteral("ancha.png"), 4000, 1000, qRgb(0, 0, 0));
    QTextCursor c(&doc);
    c.insertImage(QStringLiteral("ancha.png"));

    QList<mdexport::DocxImage> images;
    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString(), &images).toUtf8();

    const QStringList cx = attributeValues(xml, QStringLiteral("extent"), QString(),
                                           QStringLiteral("cx"));
    const QStringList cy = attributeValues(xml, QStringLiteral("extent"), QString(),
                                           QStringLiteral("cy"));
    QCOMPARE(cx.size(), 1);
    QCOMPARE(cy.size(), 1);
    QCOMPARE(cx.first().toLongLong(), 5731200LL);  // ancho útil A4 con márgenes de 1"
    // 4000x1000 → proporción 4:1, conservada tras el escalado.
    QCOMPARE(cx.first().toLongLong() / cy.first().toLongLong(), 4LL);
    QVERIFY(cy.first().toLongLong() > 0);
}

// OOXML no admite una tabla pegada a otra ni una tabla como último hijo del
// cuerpo: siempre debe quedar un `<w:p>` de cierre. Se comprueba sobre la
// SECUENCIA real de hijos de `<w:body>`, no con un `contains`.
void TestDocx::tablesAreAlwaysClosedByAParagraph()
{
    QTextDocument doc;
    QTextCursor c(&doc);
    c.insertTable(2, 2);
    c.movePosition(QTextCursor::End);
    c.insertTable(2, 3);  // dos tablas seguidas, y una de ellas cierra el documento

    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString()).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));

    QStringList bodyChildren;
    QXmlStreamReader r(xml);
    int depth = 0;
    while (!r.atEnd()) {
        const QXmlStreamReader::TokenType t = r.readNext();
        if (t == QXmlStreamReader::StartElement) {
            ++depth;
            if (depth == 3 && r.namespaceUri() == kNsW)  // document > body > hijo
                bodyChildren << r.name().toString();
        } else if (t == QXmlStreamReader::EndElement) {
            --depth;
        }
    }

    QVERIFY2(bodyChildren.contains(QStringLiteral("tbl")), qPrintable(bodyChildren.join(u',')));
    for (int i = 0; i < bodyChildren.size(); ++i) {
        if (bodyChildren.at(i) != QLatin1String("tbl"))
            continue;
        QVERIFY2(i + 1 < bodyChildren.size() && bodyChildren.at(i + 1) == QLatin1String("p"),
                 qPrintable(QStringLiteral("tabla sin párrafo detrás: ")
                            + bodyChildren.join(u',')));
    }
}

// `numbering.xml` define nueve niveles (ilvl 0..8). Una lista más profunda no
// puede emitir un ilvl fuera de rango: Word ignora el párrafo o rompe la
// numeración. También el numId debe ser uno de los dos definidos.
void TestDocx::deepListNestingClampsToNineLevels()
{
    QString md;
    for (int level = 0; level < 14; ++level)
        md += QString(qsizetype(level) * 2, QLatin1Char(' '))
              + QStringLiteral("- nivel %1\n").arg(level);

    QTextDocument doc;
    loadLikeEditor(&doc, md);

    const QByteArray xml = mdexport::toDocxDocumentXml(&doc, QString()).toUtf8();
    const QStringList ilvl =
        attributeValues(xml, QStringLiteral("ilvl"), kNsW, QStringLiteral("val"));
    QVERIFY2(!ilvl.isEmpty(), "no se emitió ninguna lista");
    for (const QString &v : ilvl) {
        const int n = v.toInt();
        QVERIFY2(n >= 0 && n <= 8, qPrintable(v));
    }
    const QStringList numId =
        attributeValues(xml, QStringLiteral("numId"), kNsW, QStringLiteral("val"));
    for (const QString &v : numId)
        QVERIFY2(v == QLatin1String("1") || v == QLatin1String("2"), qPrintable(v));
}

// Las casillas de tarea no tienen equivalente nativo: se prefijan como ☐/☒. Si se
// perdieran, la lista exportada no distinguiría lo hecho de lo pendiente.
void TestDocx::taskMarkersAreEmitted()
{
    QTextDocument doc;
    loadLikeEditor(&doc, QStringLiteral("- [ ] pendiente\n- [x] hecha\n"));

    const QString text = wordText(mdexport::toDocxDocumentXml(&doc, QString()).toUtf8());
    QVERIFY2(text.contains(QStringLiteral("☐ pendiente")), qPrintable(text));
    QVERIFY2(text.contains(QStringLiteral("☒ hecha")), qPrintable(text));
}

// Los enlaces van por RELACIÓN (`w:hyperlink r:id`), no por campo
// (`w:fldSimple w:instr=" HYPERLINK … "`): el campo lo entiende Word, pero otros
// consumidores lo descartan entero —rótulo incluido— y el enlace desaparece del
// documento. Se comprueban las tres cosas que pueden salir mal: que no vuelva el
// campo, que los destinos repetidos compartan una sola relación, y que un destino
// con metacaracteres llegue intacto al .rels.
void TestDocx::hyperlinkTargetsTravelInRelationships()
{
    const QString raro = QStringLiteral("https://example.com/a\"b&c<d");

    QTextDocument doc;
    QTextCursor c(&doc);
    QTextCharFormat uno;
    uno.setAnchor(true);
    uno.setAnchorHref(QStringLiteral("https://example.com/uno"));
    QTextCharFormat otro;
    otro.setAnchor(true);
    otro.setAnchorHref(raro);
    c.insertText(QStringLiteral("primero"), uno);
    c.insertText(QStringLiteral(" texto normal "), QTextCharFormat());
    c.insertText(QStringLiteral("repetido"), uno);   // mismo destino: una relación
    c.insertText(QStringLiteral(" y "), QTextCharFormat());
    c.insertText(QStringLiteral("raro"), otro);

    QList<mdexport::DocxHyperlink> links;
    const QByteArray xml =
        mdexport::toDocxDocumentXml(&doc, QString(), nullptr, &links).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));

    QVERIFY2(!xml.contains("fldSimple"),
             "el campo HYPERLINK pierde el enlace en los consumidores que no lo entienden");
    QCOMPARE(links.size(), 2);  // los dos usos del mismo destino comparten relación
    QCOMPARE(links.at(0).target, QStringLiteral("https://example.com/uno"));
    QCOMPARE(links.at(1).target, raro);

    const QStringList used =
        attributeValues(xml, QStringLiteral("hyperlink"), kNsR, QStringLiteral("id"));
    QCOMPARE(used.size(), 3);  // tres rótulos enlazados...
    QCOMPARE(QSet<QString>(used.begin(), used.end()).size(), 2);  // ...con dos rId

    // Y el destino con metacaracteres sobrevive al viaje por el .rels del paquete.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("enlaces.docx"));
    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path, mdexport::languageForCode(QStringLiteral("es")),
                                 QString(), &error),
             qPrintable(error));
    const QByteArray rels =
        readPackage(path).value(QStringLiteral("word/_rels/document.xml.rels"));
    QVERIFY2(wellFormed(rels, &why), qPrintable(why));
    const QStringList targets =
        attributeValues(rels, QStringLiteral("Relationship"), QString(),
                        QStringLiteral("Target"));
    QVERIFY2(targets.contains(raro), qPrintable(targets.join(QLatin1Char(' '))));
}

// Una fórmula 2D es un carácter objeto que ningún writer sabe pintar; la ruta de
// exportación a DOCX pasa por cloneForExport, que lo expande a runs. Si se colase
// sin expandir, en Word aparecería un cuadrado vacío en vez de la fórmula.
void TestDocx::twoDFormulaLeavesNoObjectCharacter()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(
        QStringLiteral("Sea $$\\sum_{i=1}^n \\frac{x_i}{2}$$ fin\n")));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    const QByteArray xml = mdexport::toDocxDocumentXml(flat.get(), QString()).toUtf8();
    QString why;
    QVERIFY2(wellFormed(xml, &why), qPrintable(why));

    const QString text = wordText(xml);
    QVERIFY2(!text.contains(QChar(0xFFFC)),
             "el carácter objeto no debe llegar al DOCX sin expandir");
    QVERIFY(text.contains(QChar(0x2211)));  // Σ del sumatorio
    QVERIFY(text.contains(QStringLiteral("Sea")) && text.contains(QStringLiteral("fin")));
    // Los subíndices de la fórmula viajan como vertAlign, no como texto plano.
    QVERIFY(QString::fromUtf8(xml).contains(QStringLiteral("<w:vertAlign w:val=\"subscript\"/>")));
}

// ===========================================================================
// IMPORTACIÓN (Pandoc + ficheros .docx de prueba)
// ===========================================================================

// Los .docx de tests/fixtures/docx/ tienen que estar en el árbol y ser paquetes
// ZIP de verdad (se regeneran con scripts/make-docx-fixtures.py).
void TestDocx::fixtureFilesAreRealPackages()
{
    for (const QString &name : {QStringLiteral("pandoc-basico.docx"),
                                QStringLiteral("word-tipico.docx"),
                                QStringLiteral("word-raro.docx"),
                                QStringLiteral("vacio.docx")}) {
        const QString path = fixture(name);
        QVERIFY2(QFile::exists(path), qPrintable(path));
        const QHash<QString, QByteArray> parts = readPackage(path);
        QVERIFY2(parts.contains(QStringLiteral("word/document.xml")), qPrintable(name));
        QString why;
        QVERIFY2(wellFormed(parts.value(QStringLiteral("word/document.xml")), &why),
                 qPrintable(name + QStringLiteral(": ") + why));
    }
}

void TestDocx::importedMarkdownHasExpectedConstructs_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<QStringList>("forbidden");

    // Un .docx «de otra herramienta»: la estructura canónica debe volver entera.
    QTest::newRow("pandoc-basico")
        << QStringLiteral("pandoc-basico.docx")
        << QStringList{QStringLiteral("# Documento de otra herramienta"),
                       QStringLiteral("## Listas"),
                       QStringLiteral("**negrita**"),
                       QStringLiteral("*cursiva*"),
                       QStringLiteral("`código en línea`"),
                       QStringLiteral("- primer elemento"),
                       QStringLiteral("> Una cita en bloque."),
                       QStringLiteral("| Licencias"),
                       QStringLiteral("[enlace](https://example.com/destino)")}
        << QStringList{};

    // Salida típica de Word: estilos por nombre, runs partidos a mitad de palabra,
    // hipervínculo por relación y numeración de numbering.xml.
    QTest::newRow("word-tipico")
        << QStringLiteral("word-tipico.docx")
        << QStringList{QStringLiteral("# Primera sección"),
                       QStringLiteral("## Listas"),
                       // Los cuatro runs de Word recompuestos en una palabra.
                       QStringLiteral("La palabra Negociación va partida"),
                       QStringLiteral("**negrita**"),
                       QStringLiteral("*cursiva*"),
                       QStringLiteral("~~tachado~~"),
                       QStringLiteral("`codigo()`"),
                       QStringLiteral("[el sitio](https://example.com/sitio)"),
                       QStringLiteral("- Primer punto"),
                       QStringLiteral("> Una cita en bloque de Word."),
                       QStringLiteral("int main() {"),
                       QStringLiteral("| Licencias")}
        // Ni etiquetas OOXML crudas ni la palabra sin recomponer.
        << QStringList{QStringLiteral("<w:"), QStringLiteral("Nego ")};

    // Casos adversarios: lo borrado con control de cambios NO debe importarse.
    QTest::newRow("word-raro")
        << QStringLiteral("word-raro.docx")
        << QStringList{QStringLiteral("# Casos raros"),
                       QStringLiteral("&"),
                       QStringLiteral("\U0001F9EA"),
                       QStringLiteral("Texto INSERTADO"),
                       QStringLiteral("segunda línea del mismo párrafo"),
                       QStringLiteral("El texto de la nota al pie"),
                       QStringLiteral("Celda anidada")}
        << QStringList{QStringLiteral("ESTOSEBORRO"),  // texto borrado (w:delText)
                       QStringLiteral("<w:")};
}

void TestDocx::importedMarkdownHasExpectedConstructs()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    QFETCH(QString, file);
    QFETCH(QStringList, expected);
    QFETCH(QStringList, forbidden);

    const QString md = importWithPandoc(file);
    QVERIFY2(!md.trimmed().isEmpty(), qPrintable(file));
    for (const QString &needle : expected)
        QVERIFY2(md.contains(needle), qPrintable(needle + QStringLiteral(" | ") + md));
    for (const QString &needle : forbidden)
        QVERIFY2(!md.contains(needle), qPrintable(needle + QStringLiteral(" | ") + md));
}

// Un .docx válido pero sin contenido no debe abrir una pestaña con basura: la
// salida vacía es justo la condición que MainWindow avisa como «el archivo no
// produjo ningún contenido».
void TestDocx::emptyDocxImportsToNothing()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    QVERIFY(importWithPandoc(QStringLiteral("vacio.docx")).trimmed().isEmpty());
}

// La importación no acaba en Pandoc: el Markdown resultante lo carga NUESTRO
// pipeline. Aquí se comprueba lo que de verdad ve el usuario en el editor.
void TestDocx::importedMarkdownLoadsIntoTheEditorDocument()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    const QString md = importWithPandoc(QStringLiteral("word-tipico.docx"));
    QVERIFY(!md.trimmed().isEmpty());

    QTextDocument doc;
    loadLikeEditor(&doc, md);

    int h1 = 0, h2 = 0, listBlocks = 0;
    bool link = false;
    QTextTable *table = nullptr;
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        const int level = b.blockFormat().headingLevel();
        if (level == 1)
            ++h1;
        else if (level == 2)
            ++h2;
        if (b.textList())
            ++listBlocks;
        if (!table)
            table = QTextCursor(b).currentTable();
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (cf.isAnchor()
                && cf.anchorHref() == QLatin1String("https://example.com/sitio"))
                link = true;
        }
    }

    QCOMPARE(h1, 1);                 // «Primera sección»
    QVERIFY2(h2 >= 2, "los encabezados de nivel 2 deben conservarse");
    QVERIFY2(listBlocks >= 5, "las listas de viñetas y numeradas deben conservarse");
    QVERIFY2(link, "el hipervínculo de Word debe llegar al documento como enlace");
    QVERIFY2(table, "la tabla debe llegar al documento como tabla");
    QCOMPARE(table->columns(), 2);
    QCOMPARE(table->rows(), 3);
    QVERIFY(doc.toPlainText().contains(QStringLiteral("Negociación")));
    QVERIFY(doc.toPlainText().contains(QStringLiteral("int main()")));
}

// Las imágenes incrustadas en el .docx tienen que llegar al editor: extraídas a
// disco (`--extract-media`), referenciadas como Markdown —no como el `<img>` que
// emite Pandoc, que el editor mostraría como texto— y con sus píxeles cargados.
// La prueba recorre la cadena entera, que es donde estaba la fuga: el `.docx` solo
// llevaba una ruta interna (`media/image1.png`) que no existe fuera del paquete.
void TestDocx::importedImagesReachTheDocument()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString mediaDir = dir.filePath(QStringLiteral("word-tipico-media"));

    const QString md = importWithPandoc(QStringLiteral("word-tipico.docx"), mediaDir);
    QVERIFY(!md.trimmed().isEmpty());
    QVERIFY2(!md.contains(QStringLiteral("<img")), qPrintable(md));

    // 1) El fichero está en disco y es una imagen legible.
    const QString extracted = mediaDir + QStringLiteral("/media/image1.png");
    QVERIFY2(QFile::exists(extracted), qPrintable(extracted));
    QImage onDisk;
    QVERIFY(onDisk.load(extracted));
    QCOMPARE(onDisk.size(), QSize(24, 24));  // la del fixture

    // 2) El Markdown la referencia por su ruta absoluta, con alternativa no vacía
    //    (Qt descarta las imágenes con `![]`, ver mdimport::repairImages).
    QVERIFY2(md.contains(QStringLiteral("![image1](") + extracted + QLatin1Char(')')),
             qPrintable(md));

    // 3) Y en el documento del editor hay una imagen de verdad, con sus píxeles
    //    (sin baseUrl: el documento importado aún no tiene ubicación en disco).
    QTextDocument doc;
    loadLikeEditor(&doc, md);
    QVERIFY(doc.baseUrl().isEmpty());
    int images = 0;
    for (QTextBlock b = doc.begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextCharFormat cf = it.fragment().charFormat();
            if (!cf.isImageFormat())
                continue;
            ++images;
            const QImage loaded = qvariant_cast<QImage>(doc.resource(
                QTextDocument::ImageResource, QUrl(cf.toImageFormat().name())));
            QVERIFY2(!loaded.isNull(), qPrintable(cf.toImageFormat().name()));
            QCOMPARE(loaded.size(), QSize(24, 24));
        }
    }
    QCOMPARE(images, 1);
}

// El título del documento (el estilo «Title» de Word) no es texto del cuerpo: Pandoc
// lo lee como metadato y, sin `--standalone`, lo tiraba. Ahora llega como front
// matter YAML, que es lo que el editor conserva verbatim y de donde la exportación
// lee `title`.
void TestDocx::importedTitleArrivesAsFrontMatter()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    const QString md = importWithPandoc(QStringLiteral("word-tipico.docx"));
    QVERIFY2(md.startsWith(QStringLiteral("---\n")), qPrintable(md.left(80)));
    QVERIFY2(md.contains(QStringLiteral("title: Informe de pruebas")), qPrintable(md.left(80)));

    // Y al cargarlo, el front matter se separa del cuerpo: ni se renderiza como
    // regla horizontal ni el título se cuela como texto del documento.
    QTextEdit edit;
    DocumentIo io(&edit);
    io.loadFromString(md);
    QCOMPARE(mdexport::frontMatterValue(io.frontMatter(), QStringLiteral("title")),
             QStringLiteral("Informe de pruebas"));
    QVERIFY(!edit.document()->toPlainText().contains(QStringLiteral("Informe de pruebas")));

    // Un documento sin metadatos no gana ningún bloque de front matter...
    const QString basico = importWithPandoc(QStringLiteral("pandoc-basico.docx"));
    QVERIFY2(!basico.startsWith(QStringLiteral("---")), qPrintable(basico.left(80)));
    // ...y uno vacío sigue sin producir nada (el aviso de MainWindow depende de eso).
    QVERIFY(importWithPandoc(QStringLiteral("vacio.docx")).trimmed().isEmpty());
}

// LIMITACIÓN CONOCIDA, fijada aquí a propósito para que se note si cambia (al subir
// de versión de Pandoc o al tocar los argumentos): las tablas ANIDADAS —una tabla
// dentro de la celda de otra— no las sabe expresar ningún Markdown, así que Pandoc
// emite HTML. Se aplanan al importarlas (ver mdimport::htmlTablesToMarkdown): el
// contenido se conserva, la anidación no.
void TestDocx::importKnownLimitationsAreStillTheSame()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite la importación real");

    const QString raro = importWithPandoc(QStringLiteral("word-raro.docx"));
    QVERIFY2(!raro.contains(QStringLiteral("<table")), qPrintable(raro));
    QVERIFY(raro.contains(QStringLiteral("Celda externa")));
    QVERIFY(raro.contains(QStringLiteral("Celda anidada")));
}

// La prueba más dura de la exportación: que un consumidor AJENO (Pandoc) sepa
// leer el .docx que escribimos y recupere la estructura. Un XML que valide puede
// aun así estar mal (estilos que Word no reconoce, numeración sin definir…);
// esto lo caza.
void TestDocx::roundTripThroughPandocKeepsStructure()
{
    if (!m_pandoc)
        QSKIP("Pandoc no está instalado: se omite el round-trip real");

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("round-trip.docx"));

    QTextDocument doc;
    loadLikeEditor(&doc, kitchenSinkMarkdown());
    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("Ida y vuelta"), &error),
             qPrintable(error));

    QProcess pandoc;
    pandoc.start(QStringLiteral("pandoc"), mdimport::pandocArguments(path));
    QVERIFY(pandoc.waitForFinished(30000));
    QVERIFY2(pandoc.exitStatus() == QProcess::NormalExit && pandoc.exitCode() == 0,
             pandoc.readAllStandardError().constData());
    const QString md = QString::fromUtf8(pandoc.readAllStandardOutput());
    QVERIFY2(!md.trimmed().isEmpty(), "Pandoc no supo leer nuestro .docx");

    // Estructura: encabezados por nivel, marcas de carácter, enlace, listas,
    // tareas, cita y tabla con sus celdas.
    for (const QString &needle : {QStringLiteral("# Encabezado 1"),
                                  QStringLiteral("## Encabezado 2"),
                                  QStringLiteral("**negrita**"),
                                  QStringLiteral("*cursiva*"),
                                  QStringLiteral("~~tachado~~"),
                                  QStringLiteral("https://example.com/destino"),
                                  QStringLiteral("- viñeta uno"),
                                  QStringLiteral("viñeta anidada"),
                                  QStringLiteral("Una cita en bloque."),
                                  QStringLiteral("Licencias"),
                                  QStringLiteral("1.200"),
                                  QStringLiteral("int main() { return 0; }"),
                                  QStringLiteral("Último párrafo.")}) {
        QVERIFY2(md.contains(needle), qPrintable(needle + QStringLiteral(" | ") + md));
    }

    // Y la lista numerada vuelve numerada (no como viñetas): la numeración real
    // depende de que numbering.xml case con el numId que emite el documento.
    QVERIFY2(md.contains(QStringLiteral("numerada uno"))
                 && md.contains(QRegularExpression(
                        QStringLiteral("^\\s*1\\.\\s+numerada uno"),
                        QRegularExpression::MultilineOption)),
             qPrintable(md));

    // Las tareas: el marcador ☐/☒ que emitimos (OOXML no tiene casilla nativa) se
    // reconoce al reimportar, ya sea tal cual o reconvertido a `- [ ]`/`- [x]`.
    QVERIFY2(md.contains(QStringLiteral("☐ tarea pendiente"))
                 || md.contains(QStringLiteral("- [ ] tarea pendiente")),
             qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("☒ tarea hecha"))
                 || md.contains(QStringLiteral("- [x] tarea hecha")),
             qPrintable(md));

    // La tabla vuelve con su fila de encabezado en su sitio (no una vacía delante):
    // es lo que aporta el <w:tblHeader/> de la primera fila.
    QVERIFY2(md.contains(QRegularExpression(
                 QStringLiteral("^\\|\\s*Concepto\\s*\\|"),
                 QRegularExpression::MultilineOption)),
             qPrintable(md));
}

QTEST_MAIN(TestDocx)
#include "tst_docx.moc"
