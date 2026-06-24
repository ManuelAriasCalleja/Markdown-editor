/// \file
/// \brief Implementación de MathObject: medición y dibujo del objeto de fórmula 2D del documento.

#include "mathobject.h"

#include <QAbstractTextDocumentLayout>
#include <QFont>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>

#include "mathblocks.h"
#include "mathlayout.h"

namespace {

// Fuente de referencia de una fórmula: la del documento (que el zoom ajusta).
// Las de bloque ($$) se componen algo más grandes, como el «display style».
QFont formulaFont(QTextDocument *doc, const QTextFormat &format)
{
    QFont f = doc ? doc->defaultFont() : QFont();
    if (format.property(mdmath::MathBlockProperty).toBool() && f.pointSizeF() > 0)
        f.setPointSizeF(f.pointSizeF() * 1.15);
    return f;
}

} // namespace

QSizeF MathObject::intrinsicSize(QTextDocument *doc, int /*posInDocument*/,
                                 const QTextFormat &format)
{
    const QString tex = format.property(mdmath::MathTexProperty).toString();
    return mdmath::measureFormula(tex, formulaFont(doc, format));
}

void MathObject::drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc,
                            int /*posInDocument*/, const QTextFormat &format)
{
    const QString tex = format.property(mdmath::MathTexProperty).toString();
    // Color: el del foreground del fragmento si el resaltado lo fijó (color de
    // math del tema); si no, el lápiz actual (color de texto del documento).
    QColor color = painter->pen().color();
    const QTextCharFormat cf = format.toCharFormat();
    if (cf.foreground().style() != Qt::NoBrush)
        color = cf.foreground().color();
    mdmath::paintFormula(painter, rect.topLeft(), tex, formulaFont(doc, format), color);
}

void MathObject::registerOn(QTextDocument *doc, MathObject *handler)
{
    if (doc && doc->documentLayout())
        doc->documentLayout()->registerHandler(mdmath::MathObjectType, handler);
}
