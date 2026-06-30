/// \file
/// \brief Implementación de la navegación pura por el documento.

#include "nav.h"

#include <algorithm>

namespace mdnav {

int clampLine(int line, int blockCount)
{
    const int maxLine = blockCount > 0 ? blockCount : 1;
    return std::clamp(line, 1, maxLine);
}

}  // namespace mdnav
