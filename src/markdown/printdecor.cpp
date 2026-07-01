/// \file
/// \brief Implementación de la decoración de impresión.

#include "printdecor.h"

namespace mdprintdecor {

QString pageNumberText(int page, int total)
{
    return QStringLiteral("%1 / %2").arg(page).arg(total);
}

}  // namespace mdprintdecor
