#ifndef MATHLAYOUT_H
#define MATHLAYOUT_H

#include <QPointF>
#include <QSizeF>
#include <QString>

class QPainter;
class QFont;
class QColor;

// Motor de maquetación 2D de fórmulas TeX («Nivel 2»): convierte una expresión
// TeX en una caja con geometría (anchura, ascenso, descenso) y la pinta con un
// QPainter. A diferencia del render «Nivel 1» (mdmath::renderTexAsRuns, que
// aplana todo a runs de texto con super/subíndice de Qt), aquí las fracciones
// se apilan con barra real y los grandes operadores (`\sum`, `\int`, `\prod`…)
// llevan sus límites encima/debajo.
//
// No todas las fórmulas necesitan esto: `needsTwoDLayout` decide. Las que no,
// siguen por la ruta de runs (que además se exporta gratis a HTML/ODF/PDF). Las
// que sí, viven en el documento como un objeto inline (MathObjectType) que un
// QTextObjectInterface dibuja llamando a `measureFormula`/`paintFormula`.
//
// El módulo construye su árbol de cajas reutilizando la tabla de glifos de
// texparser (`mdmath::commandToUnicode`); el resto (super/subíndices, llaves,
// caracteres) lo maqueta él mismo.
namespace mdmath {

// ¿La fórmula contiene construcciones que requieren maquetación 2D? Cierto si
// hay un `\frac` o un gran operador (`\sum`/`\prod`/`\coprod`/`\int`/`\iint`/
// `\iiint`/`\oint`) seguido de un límite `_`/`^`.
bool needsTwoDLayout(const QString &tex);

// Tamaño que ocupa la fórmula `tex` maquetada en 2D con `baseFont` como tamaño
// de referencia (el resto se deriva: scripts más pequeños, operadores más
// grandes). La anchura/altura incluyen todos los niveles apilados.
QSizeF measureFormula(const QString &tex, const QFont &baseFont);

// Pinta la fórmula dentro de la caja cuyo extremo superior izquierdo es
// `topLeft` (su tamaño es el de `measureFormula`), con `baseFont` y `color`.
void paintFormula(QPainter *painter, const QPointF &topLeft, const QString &tex,
                  const QFont &baseFont, const QColor &color);

} // namespace mdmath

#endif // MATHLAYOUT_H
