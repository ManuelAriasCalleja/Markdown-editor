#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

/// \file
/// \brief QTextObjectInterface que mide y pinta las imágenes del documento escaladas al zoom.

#include <QObject>
#include <QTextObjectInterface>

class QTextDocument;

/// \brief Handler de imágenes del editor que las dimensiona = tamaño nativo · factor
/// de zoom, sustituyendo al `QTextImageHandler` interno de Qt (que las pinta a tamaño
/// fijo en píxeles, sin seguir a la fuente del zoom).
///
/// Se registra para el tipo estándar `QTextFormat::ImageObject`, así que lo usan
/// TODAS las imágenes del documento —`![]()` de fichero, pegadas, y las previews de
/// diagramas—. Como el tamaño se calcula en el trazado (`intrinsicSize`), y NO se
/// escribe en el documento, escalar con el zoom **no** genera entradas de deshacer ni
/// ensucia «modificado»; además, el formato de imagen queda sin ancho, de modo que la
/// exportación (que clona el documento sin este handler) sale a tamaño nativo, no al
/// del zoom de pantalla. Mismo patrón que `MathObject` para las fórmulas 2D.
///
/// El factor lo fija `EditorStack::setContentScale` (desde el zoom de la ventana). Es
/// único por documento: lo posee el `EditorStack` y se registra en su editor WYSIWYG.
class ImageObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    using QObject::QObject;

    /// \brief Factor de escala (1.0 = 100 %). Al cambiarlo hay que forzar un
    /// re-maquetado del documento para que se remidan las imágenes (lo hace
    /// EditorStack::setContentScale con markContentsDirty).
    void setScale(qreal scale) { m_scale = scale > 0 ? scale : 1.0; }
    qreal scale() const { return m_scale; }

    /// \brief Tamaño de la imagen del fragmento = nativo · escala, acotado al ancho
    /// del documento (nunca más ancha que la vista).
    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override;
    /// \brief Pinta la imagen del fragmento dentro de `rect` (o un marcador si falta).
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override;

    /// \brief Registra `handler` para QTextFormat::ImageObject en el layout de `doc`,
    /// reemplazando al handler de imágenes interno de Qt (idempotente).
    static void registerOn(QTextDocument *doc, ImageObject *handler);

private:
    qreal m_scale = 1.0;
};

#endif // IMAGEOBJECT_H
