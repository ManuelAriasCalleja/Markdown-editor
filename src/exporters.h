#ifndef EXPORTERS_H
#define EXPORTERS_H

#include <QByteArray>
#include <QList>
#include <QString>

class QTextDocument;

// Exportación a ODF (.odt) y LaTeX (.tex), teniendo en cuenta el idioma del
// documento. Qt sabe escribir ODF pero no emite el idioma; LaTeX no lo soporta
// en absoluto. Aquí se cubren ambos huecos sin dependencias externas (el ODF se
// reempaqueta con el QZip privado de Qt).
//
// Las partes puras (serializador LaTeX y construcción de los XML de idioma del
// ODF) se prueban aisladas en tst_exporters; writeOdf integra todo con el zip.
namespace mdexport {

// Un idioma soportado por la exportación, con los nombres que cada formato pide.
struct Language {
    QString code;        // ISO 639-1: "es", "en"…
    QString name;        // nombre nativo para el selector: "Español", "English"…
    QString babel;       // paquete babel de LaTeX: "spanish", "ngerman"…
    QString odfLang;     // fo:language del ODF: "es"
    QString odfCountry;  // fo:country del ODF: "ES"
};

// Los idiomas ofrecidos en el diálogo de exportación (los mismos que la UI).
QList<Language> languages();

// Idioma para un código ISO (admite "es", "es-ES", "es_ES"); si no se reconoce,
// devuelve el inglés como recurso seguro.
Language languageForCode(const QString &code);

// Valor de una clave del front matter (`clave: valor` o `clave = valor`), sin
// comillas envolventes; "" si no está. Sirve para leer `lang`/`language`/`title`.
// Función pura.
QString frontMatterValue(const QString &frontMatter, const QString &key);

// Serializa el documento a un .tex completo (preámbulo + cuerpo), con babel del
// idioma dado y, si `title` no está vacío, \maketitle. Función pura.
QString toLatex(const QTextDocument *doc, const Language &language, const QString &title);

// Devuelve los XML `styles.xml` y `meta.xml` que fijan el idioma (y el título)
// del ODF. Puras y testeables por separado.
QByteArray odfStylesXml(const Language &language);
QByteArray odfMetaXml(const Language &language, const QString &title);
// Inserta en el manifest de un ODT las entradas de styles.xml y meta.xml.
QByteArray odfManifestWithLanguageFiles(const QByteArray &manifest);

// Escribe el documento como .odt en `path`, con el idioma incrustado. Devuelve
// false y rellena *error si falla. `title` puede ir vacío.
bool writeOdf(const QTextDocument *doc, const QString &path, const Language &language,
              const QString &title, QString *error);

// Clon del documento listo para HTML/PDF/ODF/impresión: conserva los
// fragmentos de fórmula con su `verticalAlignment` (Qt serializa el super/sub a
// CSS en HTML, al equivalente en ODF y los pinta directamente en PDF), pero
// limpia las propiedades custom de math que solo tienen sentido dentro del
// editor (`IsMath`, `MathTex`, `MathBlock`). El llamador es dueño del
// documento devuelto. LaTeX, en cambio, emite las fórmulas verbatim a partir
// del original con `toLatex`.
QTextDocument *cloneForExport(const QTextDocument *src);

} // namespace mdexport

#endif // EXPORTERS_H
