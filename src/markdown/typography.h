#ifndef TYPOGRAPHY_H
#define TYPOGRAPHY_H

/// \file
/// \brief Pasada de tipografía del documento: ritmo de encabezados, espacio entre
///        párrafos, panel de bloque de código y cita tintada. Presentación pura (no
///        toca el Markdown ni el round-trip); se aplica al cargar, junto a
///        styleTables() y applyLineSpacing().

#include <QColor>

class QTextDocument;

namespace mdtypography {

/// Margen superior (px) del encabezado de nivel `level` (1..6). Más aire sobre los
/// encabezados grandes: da el «ritmo» vertical del que carece el render plano de Qt.
qreal headingTopMargin(int level);
/// Margen inferior (px) del encabezado de nivel `level` (1..6).
qreal headingBottomMargin(int level);
/// Margen inferior (px) de un párrafo de cuerpo (separación entre párrafos).
qreal paragraphBottomMargin();

/// Fondo translúcido del panel de bloque de código. Translúcido a propósito: compone
/// sobre cualquier tema (claro u oscuro) y sigue legible sin re-derivar el color al
/// cambiar de tema, así que basta aplicarlo al cargar (no en cada cambio de tema).
QColor codeBackground();
/// Fondo translúcido de una cita normal (más sutil que el del código).
QColor quoteBackground();

/// Color de la barra lateral de una cita, a partir del fondo del bloque y del color
/// del texto. Si el fondo es de acento (una admonición, no un gris neutro) devuelve
/// ese acento opaco (barra de «callout»); si es neutro o inválido (cita normal),
/// una barra derivada del texto, semitransparente. La pinta FocusEditor::paintEvent.
QColor quoteBarColor(const QColor &blockBackground, const QColor &textColor);

/// Aplica la tipografía a `doc` recorriendo sus bloques (encabezados, párrafos,
/// bloques de código y citas). Presentación pura: preserva isModified(); no toca las
/// tablas ni las citas que ya son admoniciones (ya tintadas por mdadmonition).
/// Idempotente: reaplicarla no cambia el resultado.
void apply(QTextDocument *doc);

}  // namespace mdtypography

#endif  // TYPOGRAPHY_H
