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

} // namespace mdtypewriter
