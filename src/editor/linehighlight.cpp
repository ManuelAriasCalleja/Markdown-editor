/// \file
/// \brief Implementación del color de resaltado de la línea actual.

#include "linehighlight.h"

#include <algorithm>

namespace mdlinehighlight {

QColor currentLineColor(const QColor &base, const QColor &highlight, int weight)
{
    const int w = std::clamp(weight, 0, 100);
    const auto mix = [w](int b, int h) { return (b * (100 - w) + h * w) / 100; };
    return QColor(mix(base.red(), highlight.red()),
                  mix(base.green(), highlight.green()),
                  mix(base.blue(), highlight.blue()));
}

}  // namespace mdlinehighlight
