/// \file
/// \brief Implementación de la importación de EPUB (ZIP → OPF → capítulos XHTML).

#include "epubimport.h"

#include <QBuffer>
#include <QByteArray>
#include <QDir>
#include <QHash>
#include <QUrl>
#include <QXmlStreamReader>

#include <private/qzipreader_p.h>

#include "htmlimport.h"  // mdimport::decodeHtml
#include "richpaste.h"   // mdrichpaste::htmlToMarkdown

namespace mdimport {
namespace {

// ¿El item apunta a un documento XHTML? (por media-type o por extensión).
bool isXhtml(const QString &mediaType, const QString &href)
{
    if (mediaType.contains(QLatin1String("xhtml"), Qt::CaseInsensitive)
        || mediaType.contains(QLatin1String("html"), Qt::CaseInsensitive))
        return true;
    return href.endsWith(QLatin1String(".xhtml"), Qt::CaseInsensitive)
           || href.endsWith(QLatin1String(".html"), Qt::CaseInsensitive)
           || href.endsWith(QLatin1String(".htm"), Qt::CaseInsensitive);
}

// Une el directorio del OPF con un href relativo y normaliza («../»), en rutas de
// ZIP (siempre con «/»).
QString resolveAgainstOpf(const QString &opfDir, const QString &href)
{
    if (opfDir.isEmpty())
        return QDir::cleanPath(href);
    return QDir::cleanPath(opfDir + QLatin1Char('/') + href);
}

}  // namespace

QString epubOpfPath(const QByteArray &containerXml)
{
    QXmlStreamReader xml(containerXml);
    while (!xml.atEnd()) {
        if (xml.readNext() == QXmlStreamReader::StartElement
            && xml.name() == QLatin1String("rootfile")) {
            const QString path =
                xml.attributes().value(QLatin1String("full-path")).toString();
            if (!path.isEmpty())
                return path;
        }
    }
    return {};
}

QStringList epubSpineHrefs(const QByteArray &opfXml)
{
    QHash<QString, QString> href;   // id → href
    QHash<QString, QString> media;  // id → media-type
    QStringList spine;              // idrefs en orden de lectura

    QXmlStreamReader xml(opfXml);
    while (!xml.atEnd()) {
        if (xml.readNext() != QXmlStreamReader::StartElement)
            continue;
        const auto name = xml.name();
        if (name == QLatin1String("item")) {
            const QXmlStreamAttributes a = xml.attributes();
            const QString id = a.value(QLatin1String("id")).toString();
            if (!id.isEmpty()) {
                href.insert(id, a.value(QLatin1String("href")).toString());
                media.insert(id, a.value(QLatin1String("media-type")).toString());
            }
        } else if (name == QLatin1String("itemref")) {
            const QString idref = xml.attributes().value(QLatin1String("idref")).toString();
            if (!idref.isEmpty())
                spine.append(idref);
        }
    }

    QStringList out;
    for (const QString &id : std::as_const(spine)) {
        const QString h = href.value(id);
        if (h.isEmpty() || !isXhtml(media.value(id), h))
            continue;
        // Los href pueden venir escapados de porcentaje (espacios, acentos).
        out.append(QUrl::fromPercentEncoding(h.toUtf8()));
    }
    return out;
}

QString epubToMarkdown(const QByteArray &epubBytes)
{
    QBuffer buffer;
    buffer.setData(epubBytes);
    if (!buffer.open(QIODevice::ReadOnly))
        return {};

    QZipReader zip(&buffer);
    if (zip.status() != QZipReader::NoError)
        return {};

    const QByteArray container = zip.fileData(QStringLiteral("META-INF/container.xml"));
    if (container.isEmpty())
        return {};
    const QString opfPath = epubOpfPath(container);
    if (opfPath.isEmpty())
        return {};
    const QByteArray opf = zip.fileData(opfPath);
    if (opf.isEmpty())
        return {};

    const int slash = opfPath.lastIndexOf(QLatin1Char('/'));
    const QString opfDir = slash >= 0 ? opfPath.left(slash) : QString();

    QStringList parts;
    const QStringList hrefs = epubSpineHrefs(opf);
    for (const QString &href : hrefs) {
        const QByteArray data = zip.fileData(resolveAgainstOpf(opfDir, href));
        if (data.isEmpty())
            continue;
        const QString md = mdrichpaste::htmlToMarkdown(decodeHtml(data)).trimmed();
        if (!md.isEmpty())
            parts.append(md);
    }
    // Un capítulo por bloque, separados por una línea en blanco.
    return parts.join(QStringLiteral("\n\n"));
}

}  // namespace mdimport
