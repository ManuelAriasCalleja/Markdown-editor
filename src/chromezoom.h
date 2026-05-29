#ifndef CHROMEZOOM_H
#define CHROMEZOOM_H

#include <QtGlobal>  // qreal

class QMenu;

// Funciones puras de apoyo al zoom de la interfaz (escalado de fuentes de menús,
// barras, etc.). La orquestación —qué widgets escalar y en qué orden— vive en
// MainWindow (irreduciblemente atada a su conjunto de widgets); aquí solo lo
// comprobable de forma aislada.
namespace chromezoom {

// Tamaño de fuente (en puntos) resultante de aplicar el desfase de zoom `delta` a
// un tamaño `base`, con un mínimo de 1 punto. Devuelve <= 0 si `base` no es válido
// (la fuente no usa puntos), para que quien llame no escale ese widget.
qreal scaledPointSize(qreal base, int delta);

// Ancho mínimo (px) que un QMenu necesita para mostrar todas sus acciones —texto
// (sin el mnemonic `&`) + columnas de atajo, indicador de check y flecha de
// submenú— sin elidir, medido con la QFontMetrics actual del menú. Workaround para
// un bug de Qt 6.8 + gtk3 que cachea las anchuras de las QAction al construirlas y
// no las recalcula al re-escalar la fuente. Devuelve 0 si el menú no tiene ninguna
// acción con texto (no conviene fijarle un mínimo).
int menuMinimumWidth(const QMenu &menu);

}  // namespace chromezoom

#endif // CHROMEZOOM_H
