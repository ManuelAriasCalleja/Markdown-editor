#include "focuseditor.h"

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

    if (m_columnWidth > 0) {
        // Sin marco, y el fondo del marco (rol Window) en oscuro: así las
        // franjas laterales quedan oscuras mientras el viewport (rol Base)
        // mantiene el color de página del tema. Solo se fija el rol Window,
        // de modo que el resto de la paleta sigue resolviéndose del tema
        // global (la página no se "congela" al cambiar de tema o luz cálida).
        setFrameShape(QFrame::NoFrame);
        setAutoFillBackground(true);
        QPalette p;
        p.setColor(QPalette::Window, m_marginColor);
        setPalette(p);
    } else {
        // Vuelve al aspecto normal: marco estándar y paleta heredada del tema.
        setFrameShape(QFrame::StyledPanel);
        setAutoFillBackground(false);
        setPalette(QPalette());
    }

    updateColumnMargins();
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
