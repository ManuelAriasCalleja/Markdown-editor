#ifndef DOCSTATS_H
#define DOCSTATS_H

/// \file
/// \brief Estadísticas de un documento (palabras, caracteres, párrafos, frases,
///        tiempo de lectura) sobre su texto plano.

#include <QString>

/// Estadísticas de un documento calculadas sobre su texto plano. Función pura
/// (sin GUI) para poder probarla aislada, igual que mdoutline o mdlist. La
/// alimenta MainWindow::updateWordCount (barra de estado) y el diálogo de
/// estadísticas, con el texto plano del editor activo o de la selección.
namespace mdstats {

/// Recuentos y métricas de un documento.
struct DocStats {
    int words = 0;           ///< tokens separados por espacios en blanco
    int chars = 0;           ///< todos los caracteres (incluidos los blancos)
    int charsNoSpaces = 0;   ///< sin espacios ni otros caracteres en blanco
    int paragraphs = 0;      ///< líneas con algún carácter no blanco
    int sentences = 0;       ///< grupos consecutivos de signos . ! ? … finales
    double readingMinutes = 0.0;  ///< words / wordsPerMinute
};

/// Analiza `text`. `wordsPerMinute` es la velocidad de lectura supuesta para el
/// tiempo estimado; si es <= 0 se usa 200 (media habitual de lectura adulta).
DocStats analyze(const QString &text, int wordsPerMinute = 200);

}  // namespace mdstats

#endif // DOCSTATS_H
