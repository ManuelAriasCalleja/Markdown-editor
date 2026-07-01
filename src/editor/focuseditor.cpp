/// \file
/// \brief Implementación de FocusEditor: columna de lectura centrada y handler de pegado/soltado.

#include "focuseditor.h"

#include <QApplication>
#include <QEvent>
#include <QMimeData>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QResizeEvent>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "typography.h"

namespace {
// Barra lateral de las citas: ancho del trazo y hueco entre la barra y el texto.
constexpr int kQuoteBarWidth = 3;
constexpr int kQuoteBarGap = 8;
}  // namespace

void FocusEditor::paintEvent(QPaintEvent *event)
{
    QTextEdit::paintEvent(event);  // el texto primero; la barra va encima

    QTextDocument *doc = document();
    if (!doc)
        return;

    const QRect band = event->rect();
    const QColor textColor = viewport()->palette().color(QPalette::Text);
    QPainter painter(viewport());

    // Arrancar en el bloque que cae al principio de la zona a repintar (no desde el
    // inicio del documento) mantiene el coste proporcional a lo visible, no al total.
    for (QTextBlock b = cursorForPosition(QPoint(0, band.top())).block(); b.isValid();
         b = b.next()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (bf.intProperty(QTextFormat::BlockQuoteLevel) <= 0)
            continue;

        // Extensión vertical y borde izquierdo del texto en coordenadas de viewport:
        // cursorRect ya las da con el scroll y la columna centrada aplicados, así que
        // la barra sigue a la sangría de la cita (más a la derecha si está anidada) y
        // a la columna del modo sin distracciones sin cálculos extra.
        QTextCursor cStart(b);
        QTextCursor cEnd(b);
        cEnd.setPosition(b.position() + qMax(0, b.length() - 1));
        const QRect top = cursorRect(cStart);
        const QRect bottom = cursorRect(cEnd);

        if (bottom.bottom() < band.top())
            continue;  // por encima de la zona a repintar
        if (top.top() > band.bottom())
            break;     // por debajo: los bloques siguientes también

        const QColor bar = mdtypography::quoteBarColor(
            bf.background().style() != Qt::NoBrush ? bf.background().color() : QColor(),
            textColor);
        painter.fillRect(QRect(top.left() - kQuoteBarGap, top.top(), kQuoteBarWidth,
                               bottom.bottom() - top.top()),
                         bar);
    }
}

void FocusEditor::setMimeInsertHandler(std::function<bool(const QMimeData *)> handler)
{
    m_mimeInsertHandler = std::move(handler);
}

bool FocusEditor::canInsertFromMimeData(const QMimeData *source) const
{
    // Aceptar imágenes para que el pegado/soltado llegue a insertFromMimeData,
    // donde el handler las desvía a disco.
    return source->hasImage() || QTextEdit::canInsertFromMimeData(source);
}

void FocusEditor::insertFromMimeData(const QMimeData *source)
{
    if (m_mimeInsertHandler && m_mimeInsertHandler(source))
        return;
    QTextEdit::insertFromMimeData(source);
}

void FocusEditor::setReadingColumnWidth(int width)
{
    m_columnWidth = qMax(0, width);
    applyColumnPalette();
    updateColumnMargins();
}

void FocusEditor::applyColumnPalette()
{
    if (m_applyingPalette)
        return;
    m_applyingPalette = true;

    if (m_columnWidth > 0) {
        // Sin marco. Partimos de la paleta del tema vigente (qApp) para que la
        // columna (rol Base) y el texto (rol Text) conserven SUS colores —si en
        // su lugar partiéramos de una QPalette por defecto, en el tema claro la
        // página y el texto quedarían con colores ajenos al tema y el texto se
        // volvería ilegible—. Las franjas laterales (rol Window) van en negro
        // puro en todos los temas, para que todo lo que no sea texto desaparezca.
        setFrameShape(QFrame::NoFrame);
        setAutoFillBackground(true);
        QPalette p = qApp->palette();
        p.setColor(QPalette::Window, Qt::black);
        setPalette(p);
    } else {
        // Vuelve al aspecto normal: marco estándar y paleta heredada del tema.
        setFrameShape(QFrame::StyledPanel);
        setAutoFillBackground(false);
        setPalette(QPalette());
    }

    m_applyingPalette = false;
}

void FocusEditor::changeEvent(QEvent *event)
{
    QTextEdit::changeEvent(event);
    // Al cambiar el tema (o la luz cálida), qApp reparte un PaletteChange. Si la
    // columna está activa, recalculamos su surround a partir del nuevo tema.
    if (event->type() == QEvent::PaletteChange && m_columnWidth > 0
        && !m_applyingPalette)
        applyColumnPalette();
}

void FocusEditor::setColumnLeftAligned(bool on)
{
    if (m_columnLeftAligned == on)
        return;
    m_columnLeftAligned = on;
    updateColumnMargins();
}

void FocusEditor::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    updateColumnMargins();  // los márgenes dependen del ancho actual
}

void FocusEditor::updateColumnMargins()
{
    if (m_columnWidth <= 0) {
        setViewportMargins(0, 0, 0, 0);
        return;
    }
    // Espacio sobrante a repartir. Si la ventana es más estrecha que la columna,
    // no hay márgenes (texto al ancho).
    const int slack = qMax(0, width() - m_columnWidth);
    if (m_columnLeftAligned)
        setViewportMargins(0, 0, slack, 0);   // pegada a la izquierda
    else
        setViewportMargins(slack / 2, 0, slack / 2, 0);  // centrada
}
