#include "focuseditor.h"

#include <QApplication>
#include <QEvent>
#include <QMimeData>
#include <QPalette>
#include <QResizeEvent>

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

void FocusEditor::setMarginColor(const QColor &color)
{
    if (m_marginColor == color)
        return;
    m_marginColor = color;
    if (m_columnWidth > 0)
        applyColumnPalette();
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
        // volvería ilegible—. Solo se tiñe el rol Window (las franjas laterales)
        // con un tono ligeramente más apagado que el de la página, en ambos
        // temas, para enmarcar sin tapar nada.
        setFrameShape(QFrame::NoFrame);
        setAutoFillBackground(true);
        QPalette p = qApp->palette();
        const QColor base = p.color(QPalette::Base);
        // Color curado del tema si MainWindow lo fijó; si no, uno derivado del
        // fondo (un punto más apagado que la página, en ambos temas).
        const QColor margin = m_marginColor.isValid()
            ? m_marginColor
            : (base.lightness() < 128 ? base.darker(135) : base.darker(110));
        p.setColor(QPalette::Window, margin);
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
