#ifndef TEXTTRANSFORM_H
#define TEXTTRANSFORM_H

#include <QString>

// Transformaciones de texto sobre la selección del editor. Funciones puras (sin
// GUI) para poder probarlas aisladas, igual que mdoutline o mdstats. La
// integración (qué texto y dónde reinsertarlo) vive en MainWindow.
namespace mdtext {

QString toUpper(const QString &s);
QString toLower(const QString &s);

// Pone en mayúscula la primera letra de cada palabra y el resto en minúscula
// (title case). La frontera de palabra es cualquier carácter no alfabético.
QString capitalize(const QString &s);

// Ordena las líneas (separadas por '\n') de forma sensible al locale. `ascending`
// false invierte el orden. El orden es estable (las líneas iguales conservan su
// posición relativa).
QString sortLines(const QString &s, bool ascending = true);

// Tipografía «inteligente»: `---`→— (raya), `--`→– (semirraya), `...`→…
// (puntos suspensivos) y comillas rectas `"`/`'` → tipográficas (“ ” / ‘ ’),
// abriendo o cerrando según el carácter anterior. No toca otros caracteres.
QString smartTypography(const QString &s);

}  // namespace mdtext

#endif // TEXTTRANSFORM_H
