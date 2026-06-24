#ifndef URLDETECT_H
#define URLDETECT_H

/// \file
/// \brief Detección de URLs «pegables» para auto-enlazar al pegar sobre una selección.

#include <QString>

/// Detección de URLs «pegables», para auto-enlazar al pegar sobre una selección.
namespace mdurl {

/// ¿`text` es una sola URL con esquema (http/https/ftp/mailto) y sin espacios?
/// Se exige esquema a propósito: sin él (p. ej. «www.ejemplo.com») el Markdown lo
/// tomaría por enlace relativo. Función pura, para poder probarla aislada.
bool looksLikeUrl(const QString &text);

}  // namespace mdurl

#endif  // URLDETECT_H
