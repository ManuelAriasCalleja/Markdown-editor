/// \file
/// \brief Implementación de la pila de pestañas cerradas.

#include "closedtabs.h"

namespace closedtabs {

void push(QStringList &stack, const QString &path, int cap)
{
    if (path.isEmpty())
        return;
    stack.removeAll(path);      // sin duplicados: la ruta reaparece en la cima
    stack.append(path);         // cima = final de la lista
    while (stack.size() > cap)  // descarta las más antiguas
        stack.removeFirst();
}

QString pop(QStringList &stack)
{
    return stack.isEmpty() ? QString() : stack.takeLast();
}

}  // namespace closedtabs
