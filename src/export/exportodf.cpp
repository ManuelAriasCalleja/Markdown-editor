/// \file
/// \brief Serializador a ODF (.odt): Qt escribe el documento y aquí se reempaqueta el
///        zip para incrustarle el idioma, que Qt no emite.

#include "exporters.h"

#include <QFile>
#include <QTextDocument>
#include <QBuffer>
#include <QTextDocumentWriter>

#include <private/qzipreader_p.h>
#include <private/qzipwriter_p.h>

namespace mdexport {

// --------------------------------------------------------------------------
// ODF
// --------------------------------------------------------------------------

QByteArray odfStylesXml(const Language &language)
{
    // Un default-style de párrafo con el idioma: LibreOffice lo aplica a todo el
    // documento (corrector, separación silábica). Los idiomas asiático/complejo
    // se marcan "zxx" (ninguno) para no confundir al corrector.
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<office:document-styles"
        " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
        " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
        " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
        " office:version=\"1.2\">\n"
        " <office:styles>\n"
        "  <style:default-style style:family=\"paragraph\">\n"
        "   <style:text-properties fo:language=\"%1\" fo:country=\"%2\""
        " style:language-asian=\"zxx\" style:country-asian=\"none\""
        " style:language-complex=\"zxx\" style:country-complex=\"none\"/>\n"
        "  </style:default-style>\n"
        " </office:styles>\n"
        "</office:document-styles>\n")
        .arg(language.odfLang, language.odfCountry);
    return xml.toUtf8();
}

QByteArray odfMetaXml(const Language &language, const QString &title)
{
    QString meta;
    if (!title.isEmpty())
        meta += QStringLiteral("  <dc:title>%1</dc:title>\n").arg(title.toHtmlEscaped());
    meta += QStringLiteral("  <dc:language>%1-%2</dc:language>\n")
                .arg(language.odfLang, language.odfCountry);
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<office:document-meta"
        " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
        " xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
        " office:version=\"1.2\">\n"
        " <office:meta>\n%1"
        "  <meta:generator>md-editor</meta:generator>\n"
        " </office:meta>\n"
        "</office:document-meta>\n")
        .arg(meta);
    return xml.toUtf8();
}

QByteArray odfManifestWithLanguageFiles(const QByteArray &manifest)
{
    const QString entries = QStringLiteral(
        " <manifest:file-entry manifest:media-type=\"text/xml\""
        " manifest:full-path=\"styles.xml\"/>\n"
        " <manifest:file-entry manifest:media-type=\"text/xml\""
        " manifest:full-path=\"meta.xml\"/>\n");
    QString out = QString::fromUtf8(manifest);
    const int pos = out.indexOf(QStringLiteral("</manifest:manifest>"));
    if (pos >= 0)
        out.insert(pos, entries);
    return out.toUtf8();
}

bool writeOdf(const QTextDocument *doc, const QString &path, const Language &language,
              const QString &title, QString *error)
{
    // 1) Qt genera el ODT base (sin idioma) en memoria.
    QByteArray odt;
    {
        QBuffer buffer(&odt);
        buffer.open(QIODevice::WriteOnly);
        QTextDocumentWriter writer(&buffer, "ODF");
        if (!writer.write(doc)) {
            if (error)
                *error = QStringLiteral("QTextDocumentWriter falló al generar el ODF.");
            return false;
        }
    }

    // 2) Reempaqueta añadiendo styles.xml y meta.xml con el idioma, y la entrada
    //    correspondiente en el manifest. mimetype va primero y sin comprimir.
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QBuffer in(&odt);
    in.open(QIODevice::ReadOnly);
    QZipReader reader(&in);
    QZipWriter writer(&file);

    for (const QZipReader::FileInfo &fi : reader.fileInfoList()) {
        if (!fi.isFile)
            continue;
        QByteArray data = reader.fileData(fi.filePath);
        if (fi.filePath == QLatin1String("META-INF/manifest.xml"))
            data = odfManifestWithLanguageFiles(data);
        writer.setCompressionPolicy(fi.filePath == QLatin1String("mimetype")
                                        ? QZipWriter::NeverCompress
                                        : QZipWriter::AutoCompress);
        writer.addFile(fi.filePath, data);
    }
    writer.setCompressionPolicy(QZipWriter::AutoCompress);
    writer.addFile(QStringLiteral("styles.xml"), odfStylesXml(language));
    writer.addFile(QStringLiteral("meta.xml"), odfMetaXml(language, title));
    writer.close();
    file.close();

    if (writer.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete ODF.");
        return false;
    }
    return true;
}

}  // namespace mdexport
