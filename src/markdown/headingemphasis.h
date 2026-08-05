#ifndef HEADINGEMPHASIS_H
#define HEADINGEMPHASIS_H

/// \file
/// \brief Conserva el énfasis (`**`, `*`, `~~`) dentro de los encabezados, que el
///        serializador de Qt no emite.

#include <QString>
#include <QTextFormat>

class QTextDocument;

/// Énfasis dentro de un encabezado: `## uno **negrita** y *cursiva* fin` volvía a
/// disco como `## uno negrita y cursiva fin`. `QTextDocument::toMarkdown()` **lee** el
/// énfasis de un encabezado al cargar pero no lo **escribe** al guardar (ni negrita,
/// ni cursiva, ni tachado; el código en línea y los enlaces sí los emite). Es pérdida
/// real de contenido, y solo ahí: en párrafos, citas, listas, tareas y celdas de tabla
/// Qt lo emite bien.
///
/// El arreglo tiene dos mitades porque la información se destruye entre una y otra:
/// - al **cargar**, `markExplicitBold` anota qué runs venían en negrita del fuente,
///   antes de que `mdcharfix::repairHeadingRuns` ponga la negrita ESTRUCTURAL en todo
///   el encabezado y ya no se distingan;
/// - al **guardar**, `replaceWithSentinels` + `restoreFromSentinels` reinyectan las
///   marcas, con la misma técnica de centinelas PUA que las fórmulas y el
///   super/subíndice (ver `mdtable::documentMarkdown`).
///
/// La cursiva y el tachado no necesitan anotarse: un encabezado no los lleva de suyo,
/// así que el propio formato del run los delata.
namespace mdheademph {

/// Marca de carga: el run venía en **negrita** en el Markdown fuente, no la hereda del
/// encabezado. La pone `markExplicitBold` y la lee `replaceWithSentinels`.
constexpr int ExplicitBoldProperty = QTextFormat::UserProperty + 40;

/// Carga: anota con `ExplicitBoldProperty` los runs de encabezado que están en negrita
/// por el `**` del fuente y no por ser encabezado.
///
/// Los distingue por el tamaño: Qt le pone `FontSizeAdjustment` (el paso del nivel, y
/// lo pone en los seis niveles, también el 0 del h4) al run estructural del encabezado,
/// pero no al span de énfasis. Hay que llamarla ANTES de `repairHeadingRuns`, que
/// repone ese ajuste —y la negrita— en TODOS los fragmentos y borra la pista; por eso
/// la llama esa misma función, y no el pipeline. Idempotente.
void markExplicitBold(QTextDocument *doc);

/// Serialización (sobre el clon): envuelve en centinelas PUA los runs con énfasis de
/// los encabezados. Los centinelas van en formato PLANO, para que no acaben dentro de
/// un span de código que Qt escaparía; los runs contiguos con el mismo énfasis se
/// fusionan (dos pares de marcas pegados, `**x****y**`, no serían énfasis para ningún
/// lector) y no se abarcan los espacios de los extremos (`*x *` tampoco lo sería).
void replaceWithSentinels(QTextDocument *doc);

/// Restaura en el markdown ya serializado los centinelas PUA a `**` / `*` / `~~`.
QString restoreFromSentinels(const QString &markdown);

}  // namespace mdheademph

#endif  // HEADINGEMPHASIS_H
