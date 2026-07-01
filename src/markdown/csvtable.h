#ifndef CSVTABLE_H
#define CSVTABLE_H

/// \file
/// \brief Detección de datos delimitados (TSV/CSV) del portapapeles y conversión a
///        una tabla Markdown.

#include <QList>
#include <QString>
#include <QStringList>

/// Conversión de texto delimitado (pegado de una hoja de cálculo o un CSV) a una
/// tabla Markdown. El HTML de las hojas de cálculo ya lo cubre «Pegar como
/// Markdown»; esto cubre el hueco del **texto plano** TSV/CSV. Funciones puras
/// (sin GUI), probadas en tst_csvtable.
namespace mdcsvtable {

/// Resultado del análisis: las filas (cada una, su lista de celdas ya recortadas y
/// rellenadas al mismo nº de columnas) y si el texto parece de verdad tabular.
struct Delimited {
    QList<QStringList> rows;
    bool ok = false;
};

/// Analiza `text` buscando datos delimitados. Delimitador: TAB si aparece (pegado
/// típico de hoja de cálculo), en su defecto la coma. Conservador con la coma
/// (para no convertir prosa como «uno, dos y tres» en tabla): exige ≥2 líneas y que
/// todas la contengan. Requiere ≥2 columnas. Rellena las filas cortas con celdas
/// vacías. `ok=false` si no parece tabular.
Delimited detectDelimited(const QString &text);

/// Serializa `rows` como una tabla Markdown (primera fila = cabecera). Escapa las
/// barras verticales de las celdas. Cadena vacía si no hay filas.
QString toMarkdownTable(const QList<QStringList> &rows);

}  // namespace mdcsvtable

#endif  // CSVTABLE_H
