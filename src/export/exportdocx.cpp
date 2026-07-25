/// \file
/// \brief Serializador a DOCX (.docx, OOXML WordprocessingML): un ZIP de XML propio,
///        sin dependencias externas (Qt no sabe escribir DOCX).

#include "exporters.h"
#include "exportutil.h"
#include "mathblocks.h"

#include <QFile>
#include <QImage>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

#include <utility>
#include <private/qzipwriter_p.h>

namespace mdexport {

// --------------------------------------------------------------------------
// DOCX (OOXML WordprocessingML)
// --------------------------------------------------------------------------

namespace {

constexpr qint64 kEmuPerPx = 9525;       // 1 px = 9525 EMU
constexpr qint64 kMaxImageCx = 5731200;  // ancho útil A4 con márgenes de 1"
// El mismo ancho útil en twips (pgSz 11906 − 2×pgMar 1440), para repartirlo entre
// las columnas de una tabla.
constexpr int kUsableWidthTwips = 9026;

// Convierte un QTextDocument a OOXML: párrafos, encabezados, formato de carácter
// (negrita/cursiva/subrayado/tachado/código + super/subíndice de las fórmulas),
// enlaces, listas, citas, tablas, reglas e imágenes embebidas. El estado mutable
// (las imágenes y los enlaces registrados, con sus rId) vive aquí.
class DocxWriter
{
public:
    DocxWriter(const QTextDocument *doc, QList<DocxImage> *images,
               QList<DocxHyperlink> *hyperlinks)
        : m_doc(doc), m_images(images), m_hyperlinks(hyperlinks) {}

    QString document(const QString &title);

private:
    QString paragraph(const QString &pPr, const QString &runs) const;
    QString runsForBlock(const QTextBlock &block, bool ignoreBold);
    QString runProps(const QTextCharFormat &cf, bool ignoreBold, bool asLink) const;
    QString makeRun(const QString &text, const QString &rPr) const;
    QString imageRun(const QTextCharFormat &cf);
    QString linkRelationshipId(const QString &href);
    QString tableXml(QTextTable *table);
    QString nextRelationshipId() { return QStringLiteral("rId%1").arg(m_nextRelId++); }

    const QTextDocument *m_doc;
    QList<DocxImage> *m_images;            // nulo = omitir imágenes
    QList<DocxHyperlink> *m_hyperlinks;    // nulo = enlaces sin destino
    int m_docPrId = 1;                     // id de cada <wp:docPr> (debe ser único)
    int m_nextRelId = 3;                   // rId1 = styles.xml, rId2 = numbering.xml
};

QString DocxWriter::paragraph(const QString &pPr, const QString &runs) const
{
    QString o = QStringLiteral("<w:p>");
    if (!pPr.isEmpty())
        o += QStringLiteral("<w:pPr>") + pPr + QStringLiteral("</w:pPr>");
    o += runs + QStringLiteral("</w:p>\n");
    return o;
}

QString DocxWriter::makeRun(const QString &text, const QString &rPr) const
{
    // Los saltos suaves (\n, U+2028) dentro de un fragmento van como <w:br/>.
    static const QRegularExpression breaks(QStringLiteral("[\\n\\x{2028}]"));
    const QStringList parts = text.split(breaks);
    QString inner;
    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0)
            inner += QStringLiteral("<w:br/>");
        inner += QStringLiteral("<w:t xml:space=\"preserve\">") + xmlEsc(parts.at(i))
                 + QStringLiteral("</w:t>");
    }
    return QStringLiteral("<w:r>") + rPr + inner + QStringLiteral("</w:r>");
}

QString DocxWriter::runProps(const QTextCharFormat &cf, bool ignoreBold, bool asLink) const
{
    // El orden de los hijos de <w:rPr> lo fija el esquema (CT_RPr): rStyle, rFonts,
    // b, i, strike, color, u, vertAlign. Los consumidores tolerantes lo perdonan;
    // conviene no depender de ello.
    QString p;
    if (asLink)  // estilo de carácter «Hyperlink» (azul subrayado), como Word
        p += QStringLiteral("<w:rStyle w:val=\"Hyperlink\"/>");
    if (cf.fontFixedPitch())
        p += QStringLiteral("<w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\""
                            " w:cs=\"Courier New\"/>");
    if (!ignoreBold && cf.fontWeight() >= QFont::Bold)
        p += QStringLiteral("<w:b/>");
    if (cf.fontItalic())
        p += QStringLiteral("<w:i/>");
    if (cf.fontStrikeOut())
        p += QStringLiteral("<w:strike/>");
    if (cf.fontUnderline() && !asLink && !cf.isAnchor())
        p += QStringLiteral("<w:u w:val=\"single\"/>");
    const QTextCharFormat::VerticalAlignment va = cf.verticalAlignment();
    if (va == QTextCharFormat::AlignSuperScript)
        p += QStringLiteral("<w:vertAlign w:val=\"superscript\"/>");  // super/subíndice de fórmulas
    else if (va == QTextCharFormat::AlignSubScript)
        p += QStringLiteral("<w:vertAlign w:val=\"subscript\"/>");
    return p.isEmpty() ? QString()
                       : QStringLiteral("<w:rPr>") + p + QStringLiteral("</w:rPr>");
}

QString DocxWriter::imageRun(const QTextCharFormat &cf)
{
    if (!m_images)
        return QString();
    const QString name = cf.toImageFormat().name();
    if (name.isEmpty())
        return QString();

    // Siempre PNG: `[Content_Types].xml` solo declara esa extensión, así que meter
    // los bytes originales de un JPEG dejaría el paquete incoherente. El clon de
    // exportación trae la baseUrl, así que las rutas relativas se resuelven desde
    // disco; si no se obtiene la imagen, se omite.
    const ImageData image = imageData(name, m_doc, {});
    if (image.isNull())
        return QString();
    const QByteArray png = image.bytes;

    // El tamaño en EMU sale de los píxeles, así que hace falta decodificarla.
    const QImage img = QImage::fromData(png, "PNG");
    if (img.isNull())
        return QString();

    const int index = m_images->size() + 1;
    const QString rId = nextRelationshipId();
    m_images->append({QStringLiteral("media/image%1.png").arg(index), png, rId});

    qint64 cx = static_cast<qint64>(img.width()) * kEmuPerPx;
    qint64 cy = static_cast<qint64>(img.height()) * kEmuPerPx;
    if (cx > kMaxImageCx) {  // escala manteniendo proporción al ancho de página
        cy = cy * kMaxImageCx / cx;
        cx = kMaxImageCx;
    }
    const int id = m_docPrId++;
    return QStringLiteral(
        "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
        "<wp:extent cx=\"%1\" cy=\"%2\"/>"
        "<wp:docPr id=\"%3\" name=\"Picture %3\"/>"
        "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
        "<pic:pic><pic:nvPicPr><pic:cNvPr id=\"%3\" name=\"image%4.png\"/><pic:cNvPicPr/>"
        "</pic:nvPicPr><pic:blipFill><a:blip r:embed=\"%5\"/>"
        "<a:stretch><a:fillRect/></a:stretch></pic:blipFill>"
        "<pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%1\" cy=\"%2\"/></a:xfrm>"
        "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>"
        "</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>")
        .arg(QString::number(cx), QString::number(cy))
        .arg(id).arg(index).arg(rId);
}

// Un enlace en OOXML apunta a una RELACIÓN externa (`w:hyperlink r:id=…`), como
// escribe Word. La alternativa —el campo `w:fldSimple w:instr=" HYPERLINK … "`—
// la entiende Word, pero otros consumidores (Pandoc, y con él la reimportación de
// la propia app) descartan el campo ENTERO, texto visible incluido: el enlace y
// su rótulo desaparecían del documento. Los destinos repetidos comparten rId.
QString DocxWriter::linkRelationshipId(const QString &href)
{
    if (!m_hyperlinks)
        return QString();
    for (const DocxHyperlink &link : std::as_const(*m_hyperlinks)) {
        if (link.target == href)
            return link.relationshipId;
    }
    const QString rId = nextRelationshipId();
    m_hyperlinks->append({rId, href});
    return rId;
}

QString DocxWriter::runsForBlock(const QTextBlock &block, bool ignoreBold)
{
    QString out;
    for (auto it = block.begin(); it != block.end(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid())
            continue;
        const QTextCharFormat cf = frag.charFormat();
        if (cf.isImageFormat()) {
            out += imageRun(cf);
            continue;
        }
        const QString text = frag.text();
        if (text.isEmpty())
            continue;
        const bool asLink = cf.isAnchor() && !cf.anchorHref().isEmpty();
        const QString run = makeRun(text, runProps(cf, ignoreBold, asLink));
        const QString rId = asLink ? linkRelationshipId(cf.anchorHref()) : QString();
        if (!rId.isEmpty()) {
            out += QStringLiteral("<w:hyperlink r:id=\"%1\" w:history=\"1\">").arg(rId)
                   + run + QStringLiteral("</w:hyperlink>");
        } else {
            // Sin lista de enlaces (llamada sin recogerlos) el destino no cabe en
            // ninguna relación: se conserva el texto con su estilo, sin enlazar.
            out += run;
        }
    }
    return out;
}

QString DocxWriter::tableXml(QTextTable *table)
{
    static const QString borders = QStringLiteral(
        "<w:tblBorders>"
        "<w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "</w:tblBorders>");
    QString out = QStringLiteral("<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/>")
                  + borders + QStringLiteral("</w:tblPr><w:tblGrid>");
    // El ancho de cada columna es obligatorio en la práctica: con `<w:gridCol/>`
    // pelado hay consumidores (Pandoc, y con él la reimportación de la propia app)
    // que leen la tabla como si no tuviera columnas y PIERDEN todas las celdas.
    const int colWidth = kUsableWidthTwips / qMax(1, table->columns());
    for (int c = 0; c < table->columns(); ++c)
        out += QStringLiteral("<w:gridCol w:w=\"%1\"/>").arg(colWidth);
    out += QStringLiteral("</w:tblGrid>");
    for (int r = 0; r < table->rows(); ++r) {
        out += QStringLiteral("<w:tr>");
        // La fila 0 es siempre el encabezado en el modelo del editor (toda tabla
        // se serializa a Markdown con fila de encabezado). Marcarla hace que Word
        // la repita al partir la tabla entre páginas y que los importadores la
        // reconozcan, en vez de inventarse un encabezado vacío.
        if (r == 0)
            out += QStringLiteral("<w:trPr><w:tblHeader/></w:trPr>");
        for (int c = 0; c < table->columns(); ++c) {
            const QTextTableCell cell = table->cellAt(r, c);
            const Qt::Alignment a =
                cell.firstCursorPosition().blockFormat().alignment();
            QString jc;
            if (a & Qt::AlignHCenter)
                jc = QStringLiteral("<w:jc w:val=\"center\"/>");
            else if (a & Qt::AlignRight)
                jc = QStringLiteral("<w:jc w:val=\"right\"/>");
            QString runs;
            for (QTextFrame::iterator fit = cell.begin(); !fit.atEnd(); ++fit) {
                const QTextBlock b = fit.currentBlock();
                if (b.isValid())
                    runs += runsForBlock(b, false);
            }
            // Una celda = un párrafo (suficiente para tablas Markdown); siempre debe
            // contener al menos un <w:p>, que paragraph() garantiza.
            out += QStringLiteral("<w:tc><w:tcPr><w:tcW w:w=\"0\" w:type=\"auto\"/></w:tcPr>")
                   + paragraph(jc, runs) + QStringLiteral("</w:tc>");
        }
        out += QStringLiteral("</w:tr>");
    }
    out += QStringLiteral("</w:tbl>\n");
    return out;
}

QString DocxWriter::document(const QString &title)
{
    QString out = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:document"
        " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
        " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\""
        " xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""
        " xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n"
        "<w:body>\n");

    if (!title.isEmpty())
        out += paragraph(QStringLiteral("<w:pStyle w:val=\"Title\"/>"),
                         makeRun(title, QString()));

    QList<QTextTable *> doneTables;
    for (QTextBlock block = m_doc->begin(); block.isValid(); block = block.next()) {
        const QTextBlockFormat bf = block.blockFormat();

        // Tablas: se emiten una vez, al ver su primer bloque. Tras la tabla hace
        // falta un párrafo (OOXML no admite tabla pegada a otra ni al final).
        if (QTextTable *table = QTextCursor(block).currentTable()) {
            if (!doneTables.contains(table)) {
                doneTables.append(table);
                out += tableXml(table) + QStringLiteral("<w:p/>\n");
            }
            continue;
        }

        // Bloque de código → párrafo con sombreado y fuente monoespaciada.
        if (bf.hasProperty(QTextFormat::BlockCodeFence)) {
            out += paragraph(
                QStringLiteral("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"F2F2F2\"/>"),
                makeRun(block.text(), QStringLiteral(
                    "<w:rPr><w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\""
                    " w:cs=\"Courier New\"/></w:rPr>")));
            continue;
        }

        // Regla horizontal → párrafo con borde inferior.
        if (bf.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
            out += paragraph(QStringLiteral(
                "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"6\" w:space=\"1\""
                " w:color=\"auto\"/></w:pBdr>"), QString());
            continue;
        }

        // Listas (viñetas, numeradas y tareas) con numeración real (numbering.xml).
        if (QTextList *list = block.textList()) {
            const int depth = qMax(1, list->format().indent());
            const int ilvl = qMin(8, depth - 1);
            const bool ordered = list->format().style() == QTextListFormat::ListDecimal;
            const QString pPr =
                QStringLiteral("<w:pStyle w:val=\"ListParagraph\"/>"
                               "<w:numPr><w:ilvl w:val=\"%1\"/><w:numId w:val=\"%2\"/></w:numPr>")
                    .arg(ilvl).arg(ordered ? 2 : 1);
            // Las tareas no tienen casilla nativa: se prefija el carácter ☐/☒.
            QString marker;
            if (bf.marker() == QTextBlockFormat::MarkerType::Checked)
                marker = makeRun(QStringLiteral("☒ "), QString());
            else if (bf.marker() == QTextBlockFormat::MarkerType::Unchecked)
                marker = makeRun(QStringLiteral("☐ "), QString());
            out += paragraph(pPr, marker + runsForBlock(block, false));
            continue;
        }

        // Cita en bloque → sangría + borde izquierdo.
        if (bf.intProperty(QTextFormat::BlockQuoteLevel) > 0) {
            out += paragraph(QStringLiteral(
                "<w:ind w:left=\"720\"/>"
                "<w:pBdr><w:left w:val=\"single\" w:sz=\"18\" w:space=\"12\""
                " w:color=\"CCCCCC\"/></w:pBdr>"), runsForBlock(block, false));
            continue;
        }

        // Encabezado o párrafo normal, con alineación.
        const int level = bf.headingLevel();
        QString pPr;
        if (level >= 1 && level <= 6)
            pPr += QStringLiteral("<w:pStyle w:val=\"Heading%1\"/>").arg(level);
        const Qt::Alignment a = bf.alignment();
        if (a & Qt::AlignHCenter)
            pPr += QStringLiteral("<w:jc w:val=\"center\"/>");
        else if (a & Qt::AlignRight)
            pPr += QStringLiteral("<w:jc w:val=\"right\"/>");
        else if (a & Qt::AlignJustify)
            pPr += QStringLiteral("<w:jc w:val=\"both\"/>");
        out += paragraph(pPr, runsForBlock(block, /*ignoreBold=*/level >= 1));
    }

    out += QStringLiteral(
        "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
        "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\""
        " w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>\n"
        "</w:body>\n</w:document>\n");
    return out;
}

QByteArray docxContentTypesXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Default Extension=\"png\" ContentType=\"image/png\"/>"
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
        "<Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
        "<Override PartName=\"/word/numbering.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>"
        "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
        "</Types>\n");
}

QByteArray docxRootRelsXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>"
        "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" Target=\"docProps/core.xml\"/>"
        "</Relationships>\n");
}

QByteArray docxDocumentRelsXml(const QList<DocxImage> &images,
                               const QList<DocxHyperlink> &hyperlinks)
{
    QString out = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
        "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering\" Target=\"numbering.xml\"/>");
    for (const DocxImage &img : images)
        out += QStringLiteral(
            "<Relationship Id=\"%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%2\"/>")
            .arg(img.relationshipId, img.partName);
    // Los destinos de los enlaces son externos (URL, correo…): TargetMode obligado.
    for (const DocxHyperlink &link : hyperlinks)
        out += QStringLiteral(
            "<Relationship Id=\"%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/hyperlink\" Target=\"%2\" TargetMode=\"External\"/>")
            .arg(link.relationshipId, xmlEsc(link.target));
    out += QStringLiteral("</Relationships>\n");
    return out.toUtf8();
}

QByteArray docxCoreXml(const Language &language, const QString &title)
{
    QString meta;
    if (!title.isEmpty())
        meta += QStringLiteral("<dc:title>%1</dc:title>").arg(xmlEsc(title));
    meta += QStringLiteral("<dc:language>%1-%2</dc:language>")
                .arg(language.odfLang, language.odfCountry);
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<cp:coreProperties"
        " xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "%1</cp:coreProperties>\n").arg(meta);
    return xml.toUtf8();
}

}  // namespace

QString toDocxDocumentXml(const QTextDocument *doc, const QString &title,
                          QList<DocxImage> *images, QList<DocxHyperlink> *hyperlinks)
{
    DocxWriter writer(doc, images, hyperlinks);
    return writer.document(title);
}

QByteArray docxStylesXml(const Language &language)
{
    const QString lang = language.odfLang + QLatin1Char('-') + language.odfCountry;

    QString styles;
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\">"
        "<w:name w:val=\"Normal\"/></w:style>");
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:styleId=\"Title\"><w:name w:val=\"Title\"/>"
        "<w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/>"
        "<w:pPr><w:spacing w:after=\"300\"/><w:contextualSpacing/></w:pPr>"
        "<w:rPr><w:b/><w:sz w:val=\"56\"/><w:szCs w:val=\"56\"/></w:rPr></w:style>");
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:styleId=\"ListParagraph\">"
        "<w:name w:val=\"List Paragraph\"/><w:basedOn w:val=\"Normal\"/>"
        "<w:pPr><w:contextualSpacing/></w:pPr></w:style>");
    // Estilo de carácter de los enlaces, el que Word aplica y los importadores
    // reconocen por su nombre (mejor que pintar color y subrayado a mano en cada
    // run: así al reimportar no se cuela un subrayado suelto sobre el rótulo).
    styles += QStringLiteral(
        "<w:style w:type=\"character\" w:styleId=\"Hyperlink\">"
        "<w:name w:val=\"Hyperlink\"/>"
        "<w:rPr><w:color w:val=\"0563C1\"/><w:u w:val=\"single\"/></w:rPr></w:style>");

    const int sizes[6] = {32, 28, 26, 24, 23, 22};  // medio-puntos
    for (int n = 1; n <= 6; ++n)
        styles += QStringLiteral(
            "<w:style w:type=\"paragraph\" w:styleId=\"Heading%1\">"
            "<w:name w:val=\"heading %1\"/><w:basedOn w:val=\"Normal\"/>"
            "<w:next w:val=\"Normal\"/>"
            "<w:pPr><w:keepNext/><w:spacing w:before=\"240\" w:after=\"60\"/>"
            "<w:outlineLvl w:val=\"%2\"/></w:pPr>"
            "<w:rPr><w:b/><w:color w:val=\"2E74B5\"/><w:sz w:val=\"%3\"/>"
            "<w:szCs w:val=\"%3\"/></w:rPr></w:style>")
            .arg(n).arg(n - 1).arg(sizes[n - 1]);

    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:docDefaults><w:rPrDefault><w:rPr>"
        "<w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\"/>"
        "<w:sz w:val=\"22\"/><w:szCs w:val=\"22\"/><w:lang w:val=\"%1\"/>"
        "</w:rPr></w:rPrDefault>"
        "<w:pPrDefault><w:pPr><w:spacing w:after=\"160\" w:line=\"259\""
        " w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>"
        "%2</w:styles>\n").arg(lang, styles);
    return xml.toUtf8();
}

QByteArray docxNumberingXml()
{
    QString bullet, decimal;
    for (int i = 0; i < 9; ++i) {
        const int left = (i + 1) * 720;
        bullet += QStringLiteral(
            "<w:lvl w:ilvl=\"%1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/>"
            "<w:lvlText w:val=\"•\"/><w:lvlJc w:val=\"left\"/>"
            "<w:pPr><w:ind w:left=\"%2\" w:hanging=\"360\"/></w:pPr></w:lvl>")
            .arg(i).arg(left);
        // El lvlText decimal lleva «%N.» (con % literal): se concatena para que no
        // colisione con los marcadores de QString::arg.
        decimal += QStringLiteral("<w:lvl w:ilvl=\"") + QString::number(i)
            + QStringLiteral("\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/>"
                             "<w:lvlText w:val=\"%")
            + QString::number(i + 1) + QStringLiteral(".\"/><w:lvlJc w:val=\"left\"/>"
                                                      "<w:pPr><w:ind w:left=\"")
            + QString::number(left) + QStringLiteral("\" w:hanging=\"360\"/></w:pPr></w:lvl>");
    }
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/>%1</w:abstractNum>"
        "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"multilevel\"/>%2</w:abstractNum>"
        "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"
        "<w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>"
        "</w:numbering>\n").arg(bullet, decimal);
    return xml.toUtf8();
}

bool writeDocx(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error)
{
    QList<DocxImage> images;
    QList<DocxHyperlink> hyperlinks;
    const QByteArray documentXml =
        toDocxDocumentXml(doc, title, &images, &hyperlinks).toUtf8();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QZipWriter zip(&file);
    zip.setCompressionPolicy(QZipWriter::AutoCompress);
    zip.addFile(QStringLiteral("[Content_Types].xml"), docxContentTypesXml());
    zip.addFile(QStringLiteral("_rels/.rels"), docxRootRelsXml());
    zip.addFile(QStringLiteral("docProps/core.xml"), docxCoreXml(language, title));
    zip.addFile(QStringLiteral("word/document.xml"), documentXml);
    zip.addFile(QStringLiteral("word/styles.xml"), docxStylesXml(language));
    zip.addFile(QStringLiteral("word/numbering.xml"), docxNumberingXml());
    zip.addFile(QStringLiteral("word/_rels/document.xml.rels"),
                docxDocumentRelsXml(images, hyperlinks));
    for (const DocxImage &img : images)
        zip.addFile(QStringLiteral("word/") + img.partName, img.data);
    zip.close();
    file.close();

    if (zip.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete DOCX.");
        return false;
    }
    return true;
}

}  // namespace mdexport
