#ifndef AUTOPAIR_H
#define AUTOPAIR_H

/// \file
/// \brief Lógica pura del auto-emparejado de paréntesis, corchetes, llaves y comillas invertidas.

#include <QChar>
#include <QList>

class QTextCursor;

/// Auto-emparejado: al teclear un carácter de apertura, inserta también su cierre
/// (o envuelve la selección). Lógica pura sobre QTextCursor (testeable headless);
/// la integración con el teclado vive en MainWindow.
namespace mdautopair {

/// \brief Par de caracteres de apertura y cierre que se auto-emparejan.
struct Pair {
    QChar open;
    QChar close;
};

/// Pares que se auto-emparejan: paréntesis, corchetes, llaves y comillas
/// invertidas. Se omiten `*`/`_` a propósito: chocarían con las listas, los
/// identificadores `snake_case` y la multiplicación; el énfasis se aplica con la
/// barra de formato.
const QList<Pair> &defaultPairs();

/// Procesa el tecleo de `typed` con el cursor `cursor` (que puede traer
/// selección). Si `typed` participa en algún par de `pairs`, ejecuta la acción y
/// devuelve true (el llamador NO debe insertar el carácter); si no, devuelve false
/// (inserción normal). Acciones:
///   - con selección y apertura: envuelve la selección (open+sel+close) y la deja
///     re-seleccionada por dentro;
///   - sin selección y apertura: auto-cierra (open+close, cursor en medio), salvo
///     que lo siguiente sea una letra/dígito (evita «()palabra»);
///   - un cierre tecleado justo delante de su gemelo: lo salta sin duplicar.
/// \param cursor cursor del editor (puede traer selección); se modifica in situ.
/// \param typed carácter recién tecleado.
/// \param pairs pares activos de auto-emparejado.
/// \return true si la acción gestionó el carácter (no insertarlo); false si no.
bool apply(QTextCursor &cursor, QChar typed, const QList<Pair> &pairs);

} // namespace mdautopair

#endif // AUTOPAIR_H
