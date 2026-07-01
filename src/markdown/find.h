#ifndef FIND_H
#define FIND_H

/// \file
/// \brief Lógica pura de búsqueda: todas las coincidencias y su ordinal («N de M»).

#include <QList>
#include <QString>

namespace mdfind {

/// Una coincidencia: posición inicial y longitud (en caracteres del texto plano,
/// que en un QTextDocument se corresponden 1:1 con posiciones del documento).
struct Match
{
    int start = 0;
    int length = 0;
};

/// Todas las coincidencias de `needle` en `text`, en orden de aparición. Camino
/// ÚNICO de coincidencias (lo usan el contador, el resaltado y «Reemplazar todo»):
/// - `useRegex`: `needle` es un patrón; si no, se busca literal (se escapa).
/// - `caseSensitive`: distingue mayúsculas/minúsculas.
/// - `wholeWord`: solo palabras completas (bordes de palabra Unicode).
/// Las coincidencias vacías se ignoran (evita bucles). Patrón inválido → vacío.
/// Función pura (sin GUI), probada aislada.
QList<Match> findAll(const QString &text, const QString &needle, bool useRegex,
                     bool caseSensitive, bool wholeWord);

/// Ordinal (1-based) de la coincidencia que EMPIEZA en `pos`, o 0 si ninguna
/// empieza ahí (p. ej. el cursor no está sobre una coincidencia). Sirve para el
/// «N de M» tras mover el cursor a una coincidencia (su selectionStart == start).
int matchOrdinal(const QList<Match> &matches, int pos);

}  // namespace mdfind

#endif  // FIND_H
