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
    int words = 0;           ///< palabras separadas por espacios + ideogramas (ver analyze)
    int chars = 0;           ///< todos los caracteres (incluidos los blancos)
    int charsNoSpaces = 0;   ///< sin espacios ni otros caracteres en blanco
    int paragraphs = 0;      ///< líneas con algún carácter no blanco
    int sentences = 0;       ///< grupos consecutivos de signos finales ( . ! ? … 。！？ )
    int cjkChars = 0;        ///< cuántas de las `words` son ideogramas/kana/hangul
    double readingMinutes = 0.0;  ///< ver analyze: las dos escrituras se leen a ritmos distintos
};

/// Analiza `text`. `wordsPerMinute` es la velocidad de lectura supuesta para el
/// tiempo estimado; si es <= 0 se usa 200 (media habitual de lectura adulta).
///
/// **Chino, japonés y coreano**: esas escrituras no separan las palabras con
/// espacios, así que partir por espacios contaba un párrafo entero como UNA palabra
/// y daba un tiempo de lectura absurdo. Cada ideograma, kana o sílaba hangul cuenta
/// como una palabra (la convención habitual, la misma de los procesadores de texto),
/// y se leen a `wordsPerMinute * 2` —el chino se lee a unos 300-500 caracteres por
/// minuto frente a las ~200 palabras del texto latino—, de modo que un documento
/// mezclado suma los dos ritmos. La puntuación de esas escrituras (`，。「」`) no
/// cuenta como palabra, igual que no cuenta la latina.
DocStats analyze(const QString &text, int wordsPerMinute = 200);

}  // namespace mdstats

#endif // DOCSTATS_H
