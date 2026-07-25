/// \file
/// \brief Implementación de las piezas compartidas por los serializadores.

#include "exportutil.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QImage>
#include <QPixmap>
#include <QTextDocument>
#include <QUrl>
#include <QVariant>

namespace mdexport {

QString localFileFor(const QString &name, const QUrl &baseUrl)
{
    QUrl u(name);
    if (u.isRelative())
        u = baseUrl.resolved(u);
    if (u.isLocalFile()) {
        const QString p = u.toLocalFile();
        if (QFileInfo::exists(p))
            return p;
    }
    if (QFileInfo::exists(name))  // por si era ya una ruta de sistema de ficheros
        return name;
    return QString();
}

QString imageMediaType(const QString &extension)
{
    static const QHash<QString, QString> types = {
        {QStringLiteral("png"), QStringLiteral("image/png")},
        {QStringLiteral("jpg"), QStringLiteral("image/jpeg")},
        {QStringLiteral("jpeg"), QStringLiteral("image/jpeg")},
        {QStringLiteral("gif"), QStringLiteral("image/gif")},
        {QStringLiteral("webp"), QStringLiteral("image/webp")},
        {QStringLiteral("svg"), QStringLiteral("image/svg+xml")},
        {QStringLiteral("bmp"), QStringLiteral("image/bmp")},
        {QStringLiteral("tif"), QStringLiteral("image/tiff")},
        {QStringLiteral("tiff"), QStringLiteral("image/tiff")},
    };
    return types.value(extension.toLower(), QStringLiteral("image/png"));
}

ImageData imageData(const QString &name, const QTextDocument *doc,
                    const QStringList &keepFormats)
{
    if (name.isEmpty() || !doc)
        return {};

    const QString ext = QFileInfo(name).suffix().toLower();
    if (keepFormats.contains(ext)) {
        const QString local = localFileFor(name, doc->baseUrl());
        QFile in(local);
        if (!local.isEmpty() && in.open(QIODevice::ReadOnly)) {
            const QByteArray bytes = in.readAll();
            if (!bytes.isEmpty())
                return {bytes, ext, imageMediaType(ext)};
        }
    }

    // Rasterizado: el documento resuelve la ruta relativa y decodifica el formato
    // (incluido el SVG, con el mismo plugin que lo pinta en el editor).
    const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(name));
    QImage img = qvariant_cast<QImage>(res);
    if (img.isNull()) {
        const QPixmap pm = qvariant_cast<QPixmap>(res);
        if (!pm.isNull())
            img = pm.toImage();
    }
    if (img.isNull())
        return {};

    QByteArray png;
    QBuffer buf(&png);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    if (png.isEmpty())
        return {};
    return {png, QStringLiteral("png"), QStringLiteral("image/png")};
}

QString xmlEsc(const QString &s)
{
    QString o;
    o.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        const QChar c = s.at(i);
        const char16_t u = c.unicode();
        switch (u) {
        case u'&':  o += QStringLiteral("&amp;");  continue;
        case u'<':  o += QStringLiteral("&lt;");   continue;
        case u'>':  o += QStringLiteral("&gt;");   continue;
        case u'"':  o += QStringLiteral("&quot;"); continue;
        case u'\'': o += QStringLiteral("&apos;"); continue;
        default: break;
        }
        if ((u < 0x20 && u != u'\t' && u != u'\n' && u != u'\r') || u == 0xFFFE || u == 0xFFFF)
            continue;
        if (c.isHighSurrogate()) {  // un par válido viaja entero; suelto, fuera
            if (i + 1 < s.size() && s.at(i + 1).isLowSurrogate()) {
                o += c;
                o += s.at(++i);
            }
            continue;
        }
        if (c.isLowSurrogate())  // suplente bajo huérfano
            continue;
        o += c;
    }
    return o;
}

}  // namespace mdexport
