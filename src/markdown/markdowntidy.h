#ifndef MARKDOWNTIDY_H
#define MARKDOWNTIDY_H

#include <QString>

// Limpieza/normalización del texto Markdown (acción «Limpiar Markdown»). Puro y
// deliberadamente conservador: NO toca el interior de los bloques de código
// cercados (```/~~~), preserva los saltos de línea duros (2+ espacios al final) y
// los separadores temáticos / reglas (`---`, `***`, `* * *`). Sobre el resto:
//   - recorta los espacios finales,
//   - colapsa varias líneas en blanco seguidas en una sola,
//   - normaliza las viñetas a «- » y el espacio tras los `#` de los encabezados,
//   - quita las líneas en blanco al principio y al final y deja un único salto
//     de línea final.
namespace mdtidy {

QString tidy(const QString &markdown);

} // namespace mdtidy

#endif // MARKDOWNTIDY_H
