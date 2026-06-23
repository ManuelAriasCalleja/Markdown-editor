#ifndef TYPEWRITER_H
#define TYPEWRITER_H

// Lógica pura del modo «máquina de escribir»: mantener la línea del cursor a
// media altura del viewport. La integración (leer el cursor, mover el scroll)
// vive en EditorStack; aquí solo el cálculo, para poder probarlo aislado.
namespace mdtypewriter {

// Valor al que fijar la barra de scroll vertical para que el centro del cursor
// quede a media altura del viewport. `cursorCenterY` es la Y del centro del
// cursor en coordenadas del viewport (la que da `QTextEdit::cursorRect`, ya
// relativa al scroll actual). El resultado se acota a [minScroll, maxScroll], así
// que cerca del principio/final del documento el cursor no llega al centro (no
// hay contenido con el que rellenar) y se queda lo más cerca posible.
int centeredScrollValue(int currentScroll, int cursorCenterY, int viewportHeight,
                        int minScroll, int maxScroll);

} // namespace mdtypewriter

#endif // TYPEWRITER_H
