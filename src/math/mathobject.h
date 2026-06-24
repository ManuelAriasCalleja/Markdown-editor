#ifndef MATHOBJECT_H
#define MATHOBJECT_H

#include <QObject>
#include <QTextObjectInterface>

class QTextDocument;

// QTextObjectInterface que pinta una fórmula 2D (mdmath::MathObjectType) en el
// documento. Una fórmula que necesita maquetación 2D vive como un carácter
// ObjectReplacementCharacter cuyo char-format lleva el TeX (MathTexProperty);
// Qt llama aquí para medirla (intrinsicSize) y dibujarla (drawObject),
// delegando en mdmath::measureFormula/paintFormula.
//
// El tamaño se deriva de la fuente por defecto del documento, así que la
// fórmula escala con el zoom de la interfaz (que ajusta esa fuente).
class MathObject : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)

public:
    using QObject::QObject;

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument,
                         const QTextFormat &format) override;
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                    int posInDocument, const QTextFormat &format) override;

    // Registra `handler` como responsable de MathObjectType en el layout de
    // `doc` (idempotente: registrar de nuevo solo reemplaza el handler).
    static void registerOn(QTextDocument *doc, MathObject *handler);
};

#endif // MATHOBJECT_H
