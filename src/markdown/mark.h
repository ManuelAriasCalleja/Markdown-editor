#ifndef MARK_H
#define MARK_H

/// \file
/// \brief Localización pura de marcas de resaltado `==texto==` en una cadena.

#include <QList>
#include <QString>

namespace mdmark {

/// Un tramo `==texto==`, con posición y longitud INCLUYENDO los delimitadores `==`.
struct Span
{
    int start = 0;
    int length = 0;
};

/// Encuentra los tramos `==texto==` (texto interior no vacío) en `text`, en orden.
/// Como `=` no es delimitador en Markdown (GFM/md4c), estas marcas viajan como
/// texto literal: la presentación (fondo) se aplica aparte, sin tocar el documento.
/// Función pura (sin GUI), probada aislada.
QList<Span> spansIn(const QString &text);

}  // namespace mdmark

#endif  // MARK_H
