/// \file
/// \brief Implementación de ImageObject: medición y dibujo de las imágenes del documento escaladas al zoom.

#include "imageobject.h"

#include <QAbstractTextDocumentLayout>
#include <QColor>
#include <QImage>
#include <QMetaType>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QUrl>
#include <QVariant>

namespace {

// Margen (px) que se deja al acotar una imagen al ancho del documento, para que no
// pegue con el borde de la vista.
constexpr qreal kClampMargin = 8.0;
// Tamaño (nativo, px) del marcador cuando la imagen no se puede resolver.
constexpr qreal kMissingSize = 32.0;

// Imagen del recurso `name` del documento (o nula si no se resuelve). resource()
// carga a demanda y cachea; el recurso puede venir como QImage (previews de
// diagrama), QPixmap o los bytes del fichero (`![]()`).
QImage imageForFormat(QTextDocument *doc, const QString &name)
{
    if (!doc || name.isEmpty())
        return {};
    const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(name));
    switch (res.userType()) {
    case QMetaType::QImage:
        return res.value<QImage>();
    case QMetaType::QPixmap:
        return res.value<QPixmap>().toImage();
    case QMetaType::QByteArray: {
        QImage image;
        image.loadFromData(res.toByteArray());
        return image;
    }
    default:
        return {};
    }
}

}  // namespace

QSizeF ImageObject::intrinsicSize(QTextDocument *doc, int /*posInDocument*/,
                                  const QTextFormat &format)
{
    const QImage image = imageForFormat(doc, format.toImageFormat().name());
    if (image.isNull())
        return QSizeF(kMissingSize, kMissingSize) * m_scale;

    qreal w = image.width() * m_scale;
    qreal h = image.height() * m_scale;
    // Nunca más ancha que la vista (textWidth = ancho de maquetado del editor).
    const qreal maxW = doc->textWidth() - kClampMargin;
    if (maxW > 0 && w > maxW) {
        h *= maxW / w;
        w = maxW;
    }
    return QSizeF(w, h);
}

void ImageObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                             int /*posInDocument*/, const QTextFormat &format)
{
    const QImage image = imageForFormat(doc, format.toImageFormat().name());
    painter->save();
    if (image.isNull()) {
        // Recurso ausente: un recuadro tenue en vez de nada, para que se vea que hay
        // una imagen que no cargó (su ruta sigue en el Markdown).
        QColor line = painter->pen().color();
        line.setAlphaF(0.4);
        painter->setPen(QPen(line));
        painter->drawRect(rect.adjusted(0.5, 0.5, -0.5, -0.5));
    } else {
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter->drawImage(rect, image);
    }
    painter->restore();
}

void ImageObject::registerOn(QTextDocument *doc, ImageObject *handler)
{
    if (doc && doc->documentLayout())
        doc->documentLayout()->registerHandler(QTextFormat::ImageObject, handler);
}
