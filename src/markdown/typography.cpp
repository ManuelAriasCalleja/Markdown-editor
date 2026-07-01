/// \file
/// \brief Implementación de la pasada de tipografía del documento.

#include "typography.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>

namespace mdtypography {
namespace {

// Gris con alpha bajo: el tinte compone sobre el fondo del tema (claro u oscuro) y
// sigue legible sin necesidad de reaplicarlo al cambiar de tema. El código va algo
// más marcado que la cita para leerse como «panel».
constexpr int kGray = 128;
constexpr int kCodeAlpha = 30;
constexpr int kQuoteAlpha = 18;

// Sangría (px) del panel de código respecto al margen del texto.
constexpr qreal kCodeSideMargin = 12;
// Relleno vertical (px) en los bordes superior/inferior del grupo de código.
constexpr qreal kCodePadding = 8;

bool isCodeBlock(const QTextBlock &b)
{
    return b.blockFormat().hasProperty(QTextFormat::BlockCodeFence);
}

bool inTable(const QTextBlock &b)
{
    QTextCursor c(b);
    return c.currentTable() != nullptr;
}

}  // namespace

qreal headingTopMargin(int level)
{
    switch (level) {
    case 1:
        return 16;
    case 2:
        return 14;
    case 3:
        return 12;
    case 4:
        return 10;
    case 5:
        return 9;
    default:
        return 8;  // nivel 6 (y cualquier otro)
    }
}

qreal headingBottomMargin(int level)
{
    return level <= 2 ? 6 : 4;
}

qreal paragraphBottomMargin()
{
    return 5;
}

QColor codeBackground()
{
    return {kGray, kGray, kGray, kCodeAlpha};
}

QColor quoteBackground()
{
    return {kGray, kGray, kGray, kQuoteAlpha};
}

QColor quoteBarColor(const QColor &blockBackground, const QColor &textColor)
{
    // Admonición: su fondo es el color de acento (con alpha bajo); una barra a todo
    // color completa el aspecto de «callout». Se reconoce por no ser un gris neutro
    // (el fondo inválido = sin fondo se trata como cita normal).
    const QColor &bg = blockBackground;
    if (bg.isValid() && !(bg.red() == bg.green() && bg.green() == bg.blue()))
        return {bg.red(), bg.green(), bg.blue()};  // acento, opaco

    // Cita normal: barra neutra derivada del color del texto, semitransparente.
    QColor c = textColor;
    c.setAlpha(110);
    return c;
}

void apply(QTextDocument *doc)
{
    if (!doc)
        return;

    // Presentación pura: los márgenes y fondos no los serializa toMarkdown(), así que
    // ni el round-trip ni «modificado» se ven afectados. Preservamos la marca de Qt,
    // como styleTables()/applyLineSpacing()/recolorLinks().
    const bool wasModified = doc->isModified();
    QTextCursor guard(doc);
    guard.beginEditBlock();

    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        // Las tablas se dejan intactas (los márgenes hincharían las celdas).
        if (inTable(b))
            continue;

        const QTextBlockFormat bf = b.blockFormat();
        QTextCursor bc(b);
        QTextBlockFormat merge;

        if (isCodeBlock(b)) {
            merge.setBackground(codeBackground());
            merge.setLeftMargin(kCodeSideMargin);
            merge.setRightMargin(kCodeSideMargin);
            // Relleno vertical solo en los bordes del grupo de líneas de código.
            if (!isCodeBlock(b.previous()))
                merge.setTopMargin(kCodePadding);
            if (!isCodeBlock(b.next()))
                merge.setBottomMargin(kCodePadding);
            bc.mergeBlockFormat(merge);
            continue;
        }

        if (bf.intProperty(QTextFormat::BlockQuoteLevel) > 0) {
            // Solo las citas normales: las admoniciones ya llevan su propio fondo
            // (mdadmonition), no las volvemos a tintar (se sumarían).
            if (bf.background().style() == Qt::NoBrush) {
                merge.setBackground(quoteBackground());
                bc.mergeBlockFormat(merge);
            }
            continue;
        }

        const int level = bf.headingLevel();
        if (level > 0) {
            merge.setTopMargin(headingTopMargin(level));
            merge.setBottomMargin(headingBottomMargin(level));
            bc.mergeBlockFormat(merge);
            continue;
        }

        // Párrafo de cuerpo. Los ítems de lista se dejan compactos (tienen su propio
        // ritmo); solo separamos los párrafos sueltos.
        if (!b.textList()) {
            merge.setBottomMargin(paragraphBottomMargin());
            bc.mergeBlockFormat(merge);
        }
    }

    guard.endEditBlock();
    doc->setModified(wasModified);
}

}  // namespace mdtypography
