/// \file
/// \brief Implementación de la lógica pura del modo máquina de escribir (scroll centrado y tramos a atenuar).

#include "typewriter.h"

namespace mdtypewriter {

int centeredScrollValue(int currentScroll, int cursorCenterY, int viewportHeight,
                        int minScroll, int maxScroll)
{
    // Desplazamiento necesario para llevar el cursor desde su Y actual al centro.
    const int delta = cursorCenterY - viewportHeight / 2;
    int target = currentScroll + delta;
    if (target < minScroll)
        target = minScroll;
    if (target > maxScroll)
        target = maxScroll;
    return target;
}

QList<Range> dimRanges(int total, int focusStart, int focusEnd)
{
    if (total <= 0)
        return {};
    // Acota y ordena: nada de tramos negativos ni fuera del documento.
    focusStart = qBound(0, focusStart, total);
    focusEnd = qBound(0, focusEnd, total);
    if (focusEnd < focusStart)
        qSwap(focusStart, focusEnd);

    QList<Range> ranges;
    if (focusStart > 0)
        ranges.append({0, focusStart});               // antes del párrafo
    if (focusEnd < total)
        ranges.append({focusEnd, total - focusEnd});  // después del párrafo
    return ranges;
}

} // namespace mdtypewriter
