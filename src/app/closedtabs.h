#ifndef CLOSEDTABS_H
#define CLOSEDTABS_H

/// \file
/// \brief Pila (LIFO) acotada de rutas de pestañas cerradas, para «reabrir pestaña».

#include <QString>
#include <QStringList>

/// Pila de las últimas pestañas cerradas (por ruta en disco), para reabrirlas en
/// orden inverso al cierre. Estado puro (una QStringList que vive en MainWindow);
/// funciones sin GUI, probadas en tst_closedtabstack.
namespace closedtabs {

/// Empuja `path` a la cima de `stack`. No hace nada si `path` está vacío (solo se
/// recuerdan documentos con archivo en disco). Evita duplicados —si la ruta ya
/// estaba, la mueve a la cima— y limita el tamaño a `cap` descartando las más
/// antiguas. Función pura.
void push(QStringList &stack, const QString &path, int cap = 20);

/// Saca y devuelve la ruta de la cima de `stack`; cadena vacía si está vacía.
/// Función pura.
QString pop(QStringList &stack);

}  // namespace closedtabs

#endif  // CLOSEDTABS_H
