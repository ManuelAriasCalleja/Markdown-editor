/// \file
/// \brief Empaquetado del EPUB 3: piezas XML (OPF, navegación, NCX), índice a partir de
///        los encabezados del documento e imágenes dentro del paquete.

#include "exporters.h"
#include "exportutil.h"
#include "outline.h"

#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QStringList>
#include <QTextDocument>
#include <QUuid>

#include <private/qzipwriter_p.h>

namespace mdexport {

QString epubContentXhtml(const QString &bodyInner, const QString &title,
                         const Language &language)
{
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<!DOCTYPE html>\n"
               "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"%1\" lang=\"%1\">\n"
               "<head>\n<meta charset=\"utf-8\"/>\n<title>%2</title>\n"
               "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n"
               "</head>\n<body>\n%3\n</body>\n</html>\n")
        .arg(language.bcp47(), title.toHtmlEscaped(), bodyInner);
}

QByteArray epubContainerXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<container version=\"1.0\" "
        "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "  <rootfiles>\n"
        "    <rootfile full-path=\"OEBPS/content.opf\" "
        "media-type=\"application/oebps-package+xml\"/>\n"
        "  </rootfiles>\n"
        "</container>\n");
}

QByteArray epubContentOpf(const Language &language, const QString &title,
                          const QStringList &imageHrefs, const QString &uuid,
                          const QString &modified)
{
    // El media-type debe describir el fichero DE VERDAD: declararlo todo como PNG
    // solo funcionaba porque se reencodeaba todo a PNG (ver writeEpub).
    QString images;
    int i = 0;
    for (const QString &href : imageHrefs)
        images += QStringLiteral(
                      "    <item id=\"img%1\" href=\"%2\" media-type=\"%3\"/>\n")
                      .arg(QString::number(++i), href,
                           imageMediaType(QFileInfo(href).suffix()));

    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" "
               "unique-identifier=\"bookid\">\n"
               "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
               "    <dc:identifier id=\"bookid\">urn:uuid:%1</dc:identifier>\n"
               "    <dc:title>%2</dc:title>\n"
               "    <dc:language>%3</dc:language>\n"
               "    <meta property=\"dcterms:modified\">%4</meta>\n"
               "  </metadata>\n"
               "  <manifest>\n"
               "    <item id=\"nav\" href=\"nav.xhtml\" "
               "media-type=\"application/xhtml+xml\" properties=\"nav\"/>\n"
               "    <item id=\"ncx\" href=\"toc.ncx\" "
               "media-type=\"application/x-dtbncx+xml\"/>\n"
               "    <item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n"
               "    <item id=\"content\" href=\"content.xhtml\" "
               "media-type=\"application/xhtml+xml\"/>\n"
               "%5"
               "  </manifest>\n"
               "  <spine toc=\"ncx\">\n"
               "    <itemref idref=\"content\"/>\n"
               "  </spine>\n"
               "</package>\n")
        .arg(uuid, title.toHtmlEscaped(), language.bcp47(), modified, images)
        .toUtf8();
}

QString epubAnchorHeadings(const QString &bodyInner)
{
    // `<hr` no cuela: el patrón exige un dígito 1..6 detrás de la h.
    static const QRegularExpression heading(QStringLiteral("<h([1-6])\\b"));
    QString out;
    qsizetype last = 0;
    int n = 0;
    QRegularExpressionMatchIterator it = heading.globalMatch(bodyInner);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        out += bodyInner.mid(last, m.capturedEnd() - last)
               + QStringLiteral(" id=\"sec%1\"").arg(++n);
        last = m.capturedEnd();
    }
    return n == 0 ? bodyInner : out + bodyInner.mid(last);
}

namespace {

// Índice anidado en `<ol>`/`<li>`, como pide la navegación de EPUB 3. Un XHTML mal
// formado invalida el libro entero, así que el anidamiento se lleva con una pila de
// niveles abiertos en vez de comparando con el anterior:
//   - un salto de varios niveles (de un h1 a un h3) abre UN solo nivel, porque un
//     `<ol>` sin `<li>` dentro no es válido y meter huecos vacíos es peor;
//   - un encabezado más alto que el primero (un h3 seguido de un h1) no puede
//     subir por encima de la raíz: se queda al mismo nivel.
QString epubNavList(const QList<EpubTocEntry> &entries)
{
    QString out;
    QList<int> open;  // nivel de cada <ol>/<li> abierto, de fuera hacia dentro
    for (const EpubTocEntry &e : entries) {
        const int level = qBound(1, e.level, 6);
        if (open.isEmpty()) {
            out += QStringLiteral("<ol>\n<li>");
            open.append(level);
        } else if (level > open.last()) {
            out += QStringLiteral("\n<ol>\n<li>");
            open.append(level);
        } else {
            out += QStringLiteral("</li>\n");
            while (open.size() > 1 && level < open.last()) {
                out += QStringLiteral("</ol>\n</li>\n");
                open.removeLast();
            }
            open.last() = level;
            out += QStringLiteral("<li>");
        }
        out += QStringLiteral("<a href=\"content.xhtml#%1\">%2</a>")
                   .arg(e.anchor, e.text.toHtmlEscaped());
    }
    if (open.isEmpty())
        return QString();
    out += QStringLiteral("</li>\n");
    while (open.size() > 1) {
        out += QStringLiteral("</ol>\n</li>\n");
        open.removeLast();
    }
    return out + QStringLiteral("</ol>\n");
}

}  // namespace

QByteArray epubNavXhtml(const Language &language, const QString &title,
                        const QList<EpubTocEntry> &entries)
{
    const QString t = title.toHtmlEscaped();
    // Sin encabezados, una única entrada al documento (mejor eso que un índice vacío).
    const QString list =
        entries.isEmpty()
            ? QStringLiteral("<ol><li><a href=\"content.xhtml\">%1</a></li></ol>\n").arg(t)
            : epubNavList(entries);
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE html>\n"
               "<html xmlns=\"http://www.w3.org/1999/xhtml\" "
               "xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"%1\" lang=\"%1\">\n"
               "<head><meta charset=\"utf-8\"/><title>%2</title></head>\n"
               "<body>\n<nav epub:type=\"toc\" id=\"toc\">\n<h1>%2</h1>\n"
               "%3</nav>\n"
               "</body>\n</html>\n")
        .arg(language.bcp47(), t, list)
        .toUtf8();
}

QByteArray epubTocNcx(const QString &title, const QString &uuid,
                      const QList<EpubTocEntry> &entries)
{
    const QString t = title.toHtmlEscaped();
    // El NCX es el índice legado (lectores EPUB 2): basta con la lista en plano,
    // sin anidar, y el orden de lectura en `playOrder`.
    QString points;
    int n = 0;
    for (const EpubTocEntry &e : entries)
        points += QStringLiteral(
                      "<navPoint id=\"np%1\" playOrder=\"%1\"><navLabel><text>%2</text>"
                      "</navLabel><content src=\"content.xhtml#%3\"/></navPoint>\n")
                      .arg(++n)
                      .arg(e.text.toHtmlEscaped(), e.anchor);
    if (points.isEmpty())
        points = QStringLiteral(
                     "<navPoint id=\"np1\" playOrder=\"1\"><navLabel><text>%1</text>"
                     "</navLabel><content src=\"content.xhtml\"/></navPoint>\n")
                     .arg(t);

    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
               "  <head><meta name=\"dtb:uid\" content=\"urn:uuid:%1\"/></head>\n"
               "  <docTitle><text>%2</text></docTitle>\n"
               "  <navMap>%3</navMap>\n"
               "</ncx>\n")
        .arg(uuid, t, points)
        .toUtf8();
}

QByteArray epubStyleCss()
{
    return QByteArrayLiteral(
        "body { font-family: serif; line-height: 1.5; margin: 1em; }\n"
        "h1, h2, h3, h4 { font-family: sans-serif; line-height: 1.2; }\n"
        "pre, code, tt { font-family: monospace; }\n"
        "pre { background: #f4f4f4; padding: 0.6em; overflow: auto; }\n"
        "blockquote { margin: 1em 0; padding: 0.2em 1em; "
        "border-left: 4px solid #ccc; }\n"
        "table { border-collapse: collapse; }\n"
        "td, th { border: 1px solid #999; padding: 0.3em 0.6em; }\n"
        "img { max-width: 100%; }\n"
        // Casillas de tarea: Qt marca los <li> con estas clases y deja la regla que
        // las pinta en el <style> de su <head>… que es justo lo que htmlBodyToXhtml
        // descarta al quedarse con el cuerpo. Sin esto, en el lector una tarea hecha
        // y una pendiente son dos viñetas idénticas. Se usa `::before` y no
        // `::marker` (que Qt emite) porque lo entienden muchos más lectores.
        "li.unchecked, li.checked { list-style-type: none; }\n"
        "li.unchecked::before { content: \"\\2610\\00a0\"; }\n"
        "li.checked::before { content: \"\\2612\\00a0\"; }\n");
}

bool writeEpub(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error)
{
    // Anclas en los encabezados: sin ellas el índice no puede saltar a los capítulos.
    QString body = epubAnchorHeadings(htmlBodyToXhtml(doc->toHtml()));

    // Localiza las imágenes referenciadas, las mete en el paquete y reescribe su
    // src. Se prefieren los BYTES ORIGINALES cuando el formato es de los que
    // entienden los lectores (png/jpg/gif/svg): reencodearlo todo a PNG conserva el
    // aspecto pero infla una foto JPEG y convierte un SVG vectorial en un mapa de
    // bits. Lo que no se pueda cargar (una URL externa) se deja tal cual.
    // Formatos que los lectores de EPUB 3 admiten de serie (webp no es de los
    // «core media types», así que no está: se rasteriza).
    static const QStringList readable = {QStringLiteral("png"), QStringLiteral("jpg"),
                                         QStringLiteral("jpeg"), QStringLiteral("gif"),
                                         QStringLiteral("svg")};
    QStringList imageHrefs;
    QList<QPair<QString, QByteArray>> imageFiles;
    QHash<QString, QString> remap;
    static const QRegularExpression imgRe(QStringLiteral("src=\"([^\"]+)\""));
    auto it = imgRe.globalMatch(body);
    while (it.hasNext()) {
        const QString src = it.next().captured(1);
        if (remap.contains(src))
            continue;

        const ImageData image = imageData(src, doc, readable);
        if (image.isNull())
            continue;  // remota o ilegible: se deja el src original
        const QString href = QStringLiteral("images/image%1.%2")
                                 .arg(QString::number(imageFiles.size() + 1),
                                      image.extension);
        remap.insert(src, href);
        imageFiles.append({href, image.bytes});
        imageHrefs.append(href);
    }
    for (auto i = remap.cbegin(); i != remap.cend(); ++i)
        body.replace(QStringLiteral("src=\"%1\"").arg(i.key()),
                     QStringLiteral("src=\"%1\"").arg(i.value()));

    // Índice a partir de los encabezados del documento: sin esto el libro llegaba
    // al lector con una sola entrada («el documento»), sin manera de saltar a un
    // capítulo. El orden coincide con el de las anclas que puso epubAnchorHeadings,
    // porque ambos recorren el documento de principio a fin.
    QList<EpubTocEntry> toc;
    for (const OutlineHeading &h : mdoutline::headingsOf(doc))
        toc.append({h.level, h.text, QStringLiteral("sec%1").arg(toc.size() + 1)});

    const QString safeTitle = title.isEmpty() ? QStringLiteral("Documento") : title;
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString modified =
        QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss'Z'"));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QZipWriter zip(&file);
    // `mimetype` debe ir primero y SIN comprimir (lo exige la especificación EPUB).
    zip.setCompressionPolicy(QZipWriter::NeverCompress);
    zip.addFile(QStringLiteral("mimetype"), QByteArrayLiteral("application/epub+zip"));
    zip.setCompressionPolicy(QZipWriter::AutoCompress);
    zip.addFile(QStringLiteral("META-INF/container.xml"), epubContainerXml());
    zip.addFile(QStringLiteral("OEBPS/content.opf"),
                epubContentOpf(language, safeTitle, imageHrefs, uuid, modified));
    zip.addFile(QStringLiteral("OEBPS/nav.xhtml"), epubNavXhtml(language, safeTitle, toc));
    zip.addFile(QStringLiteral("OEBPS/toc.ncx"), epubTocNcx(safeTitle, uuid, toc));
    zip.addFile(QStringLiteral("OEBPS/style.css"), epubStyleCss());
    zip.addFile(QStringLiteral("OEBPS/content.xhtml"),
                epubContentXhtml(body, safeTitle, language).toUtf8());
    for (const auto &im : imageFiles)
        zip.addFile(QStringLiteral("OEBPS/") + im.first, im.second);
    zip.close();
    file.close();

    if (zip.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete EPUB.");
        return false;
    }
    return true;
}

}  // namespace mdexport
