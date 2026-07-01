#ifndef FOCUSEDITOR_H
#define FOCUSEDITOR_H

/// \file
/// \brief QTextEdit con columna de lectura centrada (modo sin distracciones) y handler de pegado/soltado.

#include <QTextEdit>

#include <functional>

class QResizeEvent;
class QPaintEvent;
class QMimeData;

/// QTextEdit que puede centrar el texto en una columna de ancho máximo fijo,
/// dejando a los lados unos márgenes de un color oscuro y calmado (para el modo
/// sin distracciones). Con ancho 0 se comporta como un QTextEdit normal: el
/// texto ocupa todo el ancho disponible y los márgenes desaparecen.
///
/// El color del margen se consigue rellenando el fondo del marco (rol Window)
/// vía autoFillBackground; el fondo de la página (rol Base, en el viewport)
/// sigue al tema, así que la columna conserva el color claro/oscuro habitual.
class FocusEditor : public QTextEdit
{
public:
    using QTextEdit::QTextEdit;

    /// Ancho máximo de la columna de texto en píxeles; 0 = desactivado.
    void setReadingColumnWidth(int width);

    /// Alinea la columna al borde izquierdo (margen izquierdo 0, todo el sobrante
    /// a la derecha) en vez de centrarla. Se usa en el modo sin distracciones para
    /// que la columna quede pegada al panel de esquema. Por defecto, centrada.
    void setColumnLeftAligned(bool on);

    /// Intercepta lo que se pega o se suelta en el editor. Si el handler está
    /// fijado y devuelve true, el contenido se considera ya gestionado (por
    /// ejemplo, una imagen del portapapeles guardada a disco) y no se inserta tal
    /// cual. Devuelve false para dejar que el QTextEdit lo pegue normalmente.
    void setMimeInsertHandler(std::function<bool(const QMimeData *)> handler);

protected:
    // Pinta la barra lateral de las citas (y admoniciones) sobre el texto ya trazado.
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;
    // Reaplica el color de los márgenes cuando cambia la paleta (cambio de tema o
    // de luz cálida): así el surround sigue derivándose del tema vigente.
    void changeEvent(QEvent *event) override;

private:
    // Recalcula los márgenes laterales para centrar la columna en el viewport.
    void updateColumnMargins();
    // Fija (o quita) la paleta de la columna: parte del tema actual y solo tiñe
    // el rol Window (los márgenes) con un tono derivado del fondo de la página,
    // de modo que la columna y el texto conservan los colores del tema.
    void applyColumnPalette();

    std::function<bool(const QMimeData *)> m_mimeInsertHandler;
    int m_columnWidth = 0;
    bool m_columnLeftAligned = false;  // columna pegada a la izquierda
    bool m_applyingPalette = false;    // evita recursión en changeEvent
};

#endif // FOCUSEDITOR_H
