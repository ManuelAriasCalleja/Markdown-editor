#ifndef SYMBOLCATALOG_H
#define SYMBOLCATALOG_H

#include <QList>
#include <QString>
#include <QStringList>

// Catálogo de símbolos especiales (no habituales en el teclado) agrupados por
// categoría, para el diálogo *Insertar → Símbolos especiales*. Es datos puros
// (sin GUI) salvo que los nombres de categoría pasan por tr() para traducirse;
// se expone aparte para poder probarlo de forma aislada.
namespace mdsymbols {

struct Category {
    QString name;          // nombre visible (traducible)
    QStringList symbols;   // un elemento por símbolo (un punto de código cada uno)
};

// Las categorías con sus símbolos, en orden de presentación.
QList<Category> categories();

}  // namespace mdsymbols

#endif  // SYMBOLCATALOG_H
