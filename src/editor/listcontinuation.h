#ifndef LISTCONTINUATION_H
#define LISTCONTINUATION_H

/// \file
/// \brief Lógica pura de la continuación inteligente de listas/tareas al pulsar Enter en la vista de fuente.

#include <QString>

/// Lógica pura (sin GUI) de la «continuación inteligente de listas» en el editor
/// de código fuente Markdown. Dada la línea donde está el cursor, decide si es un
/// ítem de lista/tarea y, si lo es, qué prefijo debe llevar el ítem siguiente.
/// Se prueba aislada en tst_listcontinuation; la integración con el editor vive
/// en MainWindow.
namespace mdlist {

/// \brief Resultado de analizar una línea: si es ítem, si está vacío y el prefijo del siguiente.
struct Continuation {
    bool isItem = false;   ///< la línea es un ítem de lista o tarea
    bool empty = false;    ///< el ítem no tiene contenido tras el marcador
    QString nextPrefix;    ///< sangría + marcador para el ítem siguiente
};

/// Analiza una línea de Markdown. Reconoce viñetas (`-`, `*`, `+`), listas
/// numeradas (`1.`/`1)`) y tareas (`- [ ]`/`- [x]`). En `nextPrefix` devuelve la
/// misma sangría y marcador, con el número incrementado y la casilla de tarea
/// siempre sin marcar.
Continuation analyze(const QString &line);

} // namespace mdlist

#endif // LISTCONTINUATION_H
