#ifndef TABLESORT_H
#define TABLESORT_H

/// \file
/// \brief Lógica pura para ordenar las filas de una tabla por una columna.

#include <QList>
#include <QStringList>

namespace mdtablesort {

/// ¿La columna es numérica? True si algún valor parsea como número y **todos** los
/// no vacíos lo hacen (decimal con punto, locale C). Para autodetectar el modo.
bool looksNumeric(const QStringList &keys);

/// Orden (permutación de índices 0..n-1) que ordena `keys` de forma **estable**:
/// - `numeric`: compara como números; los no numéricos van juntos al final del
///   orden ascendente (y su posición se invierte en descendente).
/// - si no, alfabético sin distinguir mayúsculas (`localeAwareCompare`).
/// `ascending` invierte el sentido. Los empates conservan el orden original.
/// Función pura (sin GUI), probada aislada.
QList<int> sortedOrder(const QStringList &keys, bool numeric, bool ascending);

}  // namespace mdtablesort

#endif  // TABLESORT_H
