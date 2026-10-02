/// \file
/// \brief Implementación de mdtabdrag.

#include "tabdrag.h"

#include <QDataStream>
#include <QIODevice>

namespace mdtabdrag {

const QString kMimeType = QStringLiteral("application/x-md-editor-tab");

QMimeData *toMime(const Payload &payload)
{
    QByteArray bytes;
    QDataStream out(&bytes, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << payload.path << qint32(payload.cursor) << payload.source;
    auto *mime = new QMimeData;
    mime->setData(kMimeType, bytes);
    return mime;
}

bool fromMime(const QMimeData *mime, Payload &payload)
{
    if (!mime || !mime->hasFormat(kMimeType))
        return false;
    QDataStream in(mime->data(kMimeType));
    in.setVersion(QDataStream::Qt_6_0);
    Payload p;
    qint32 cursor = 0;
    in >> p.path >> cursor >> p.source;
    if (in.status() != QDataStream::Ok || !in.atEnd() || p.path.isEmpty() || cursor < -1)
        return false;
    p.cursor = cursor;
    payload = p;
    return true;
}

bool leftBar(const QPoint &pos, const QRect &barRect, int margin)
{
    return !barRect.adjusted(-margin, -margin, margin, margin).contains(pos);
}

bool accepts(const Payload &payload, const QString &self)
{
    return !payload.path.isEmpty() && payload.source != self;
}

}  // namespace mdtabdrag
