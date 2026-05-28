#ifndef FOCUSEDITOR_H
#define FOCUSEDITOR_H

#include <QColor>
#include <QTextEdit>

#include <functional>

class QResizeEvent;
class QMimeData;

// QTextEdit que puede centrar el texto en una columna de ancho máximo fijo,
// dejando a los lados unos márgenes de un color oscuro y calmado (para el modo
// sin distracciones). Con ancho 0 se comporta como un QTextEdit normal: el
// texto ocupa todo el ancho disponible y los márgenes desaparecen.
//
// El color del margen se consigue rellenando el fondo del marco (rol Window)
// vía autoFillBackground; el fondo de la página (rol Base, en el viewport)
// sigue al tema, así que la columna conserva el color claro/oscuro habitual.
class FocusEditor : public QTextEdit
{
public:
    using QTextEdit::QTextEdit;

    // Ancho máximo de la columna de texto en píxeles; 0 = desactivado.
    void setReadingColumnWidth(int width);

    // Alinea la columna al borde izquierdo (margen izquierdo 0, todo el sobrante
    // a la derecha) en vez de centrarla. Se usa en el modo sin distracciones para
    // que la columna quede pegada al panel de esquema. Por defecto, centrada.
    void setColumnLeftAligned(bool on);

    // Intercepta lo que se pega o se suelta en el editor. Si el handler está
    // fijado y devuelve true, el contenido se considera ya gestionado (por
    // ejemplo, una imagen del portapapeles guardada a disco) y no se inserta tal
    // cual. Devuelve false para dejar que el QTextEdit lo pegue normalmente.
    void setMimeInsertHandler(std::function<bool(const QMimeData *)> handler);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool canInsertFromMimeData(const QMimeData *source) const override;
    void insertFromMimeData(const QMimeData *source) override;

private:
    // Recalcula los márgenes laterales para centrar la columna en el viewport.
    void updateColumnMargins();

    std::function<bool(const QMimeData *)> m_mimeInsertHandler;
    int m_columnWidth = 0;
    bool m_columnLeftAligned = false;           // columna pegada a la izquierda
    QColor m_marginColor = QColor(30, 30, 30);  // fondo oscuro a los lados
};

#endif // FOCUSEDITOR_H
