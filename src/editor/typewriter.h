#ifndef TYPEWRITER_H
#define TYPEWRITER_H

/// \file
/// \brief Lógica pura del modo máquina de escribir: scroll que centra la línea del cursor y tramos a atenuar.

#include <QList>

/// Lógica pura del modo «máquina de escribir»: mantener la línea del cursor a
/// media altura del viewport. La integración (leer el cursor, mover el scroll)
/// vive en EditorStack; aquí solo el cálculo, para poder probarlo aislado.
namespace mdtypewriter {

/// Valor al que fijar la barra de scroll vertical para que el centro del cursor
/// quede a media altura del viewport. `cursorCenterY` es la Y del centro del
/// cursor en coordenadas del viewport (la que da `QTextEdit::cursorRect`, ya
/// relativa al scroll actual). El resultado se acota a [minScroll, maxScroll], así
/// que cerca del principio/final del documento el cursor no llega al centro (no
/// hay contenido con el que rellenar) y se queda lo más cerca posible.
int centeredScrollValue(int currentScroll, int cursorCenterY, int viewportHeight,
                        int minScroll, int maxScroll);

/// Un tramo de texto [start, start+length) por posición de carácter.
struct Range {
    int start;
    int length;
};

/// «Foco de línea»: dado un documento de `total` caracteres y el párrafo enfocado
/// [focusStart, focusEnd), devuelve los tramos a ATENUAR (todo lo demás): como
/// mucho dos, el de antes y el de después del párrafo. La integración convierte
/// cada tramo en una QTextEdit::ExtraSelection con color apagado. Los argumentos
/// se acotan a [0, total] y se ordena focusStart<=focusEnd, así que entradas
/// degeneradas (cursor al inicio/fin, documento vacío) no producen tramos
/// inválidos. Puro y aislado para poder probar los bordes sin GUI.
QList<Range> dimRanges(int total, int focusStart, int focusEnd);

} // namespace mdtypewriter

#endif // TYPEWRITER_H
