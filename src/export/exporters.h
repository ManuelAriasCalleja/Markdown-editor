#ifndef EXPORTERS_H
#define EXPORTERS_H

/// \file
/// \brief Serializadores puros de exportación a ODF, LaTeX, DOCX y EPUB (con el idioma del documento).

#include <QByteArray>
#include <QList>
#include <QString>

class QTextDocument;
class QPrinter;

/// \brief Exportación a ODF (.odt) y LaTeX (.tex), teniendo en cuenta el idioma del
/// documento. Qt sabe escribir ODF pero no emite el idioma; LaTeX no lo soporta
/// en absoluto. Aquí se cubren ambos huecos sin dependencias externas (el ODF se
/// reempaqueta con el QZip privado de Qt).
///
/// Las partes puras (serializador LaTeX y construcción de los XML de idioma del
/// ODF) se prueban aisladas en tst_exporters; writeOdf integra todo con el zip.
namespace mdexport {

/// \brief Un idioma soportado por la exportación, con los nombres que cada formato pide.
struct Language {
    QString code;        ///< ISO 639-1: "es", "en"…
    QString name;        ///< nombre nativo para el selector: "Español", "English"…
    QString babel;       ///< paquete babel de LaTeX: "spanish", "ngerman"…
    QString odfLang;     ///< fo:language del ODF: "es"
    QString odfCountry;  ///< fo:country del ODF: "ES"
};

/// \brief Los idiomas ofrecidos en el diálogo de exportación (los mismos que la UI).
QList<Language> languages();

/// \brief Idioma para un código ISO (admite "es", "es-ES", "es_ES"); si no se reconoce,
/// devuelve el inglés como recurso seguro.
Language languageForCode(const QString &code);

/// \brief Valor de una clave del front matter (`clave: valor` o `clave = valor`), sin
/// comillas envolventes; "" si no está. Sirve para leer `lang`/`language`/`title`.
/// Función pura.
QString frontMatterValue(const QString &frontMatter, const QString &key);

/// \brief Metadatos del PDF tomados del front matter: `title`, y `author` (o, en su
/// defecto, `creator`) como autor/creador. Cualquiera puede ir vacío.
struct PdfInfo {
    QString title;
    QString creator;
};

/// \brief Lee de `frontMatter` el título y el autor para incrustarlos en el PDF.
/// Función pura (los campos van a QPrinter::setDocName/setCreator).
PdfInfo pdfDocumentInfo(const QString &frontMatter);

/// \brief Serializa el documento a un .tex completo (preámbulo + cuerpo), con babel del
/// idioma dado y, si `title` no está vacío, `\maketitle`.
/// \param outputTexPath ruta del .tex de destino. Si se da, las imágenes que
/// pdflatex no sabe incluir (SVG, GIF, BMP…) se rasterizan a un PNG escrito en esa
/// misma carpeta y se referencian; si está vacía (p.ej. en tests), se referencian
/// tal cual. Salvo por ese efecto de escritura de PNGs, la función es pura.
QString toLatex(const QTextDocument *doc, const Language &language, const QString &title,
                const QString &outputTexPath = QString());

/// \brief Devuelve el XML `styles.xml` que fija el idioma del ODF. Pura y testeable.
QByteArray odfStylesXml(const Language &language);
/// \brief Devuelve el XML `meta.xml` que fija el idioma (y el título) del ODF. Pura.
QByteArray odfMetaXml(const Language &language, const QString &title);
/// \brief Inserta en el manifest de un ODT las entradas de styles.xml y meta.xml.
QByteArray odfManifestWithLanguageFiles(const QByteArray &manifest);

/// \brief Escribe el documento como .odt en `path`, con el idioma incrustado. Devuelve
/// false y rellena *error si falla. `title` puede ir vacío.
bool writeOdf(const QTextDocument *doc, const QString &path, const Language &language,
              const QString &title, QString *error);

// --- DOCX (.docx, OOXML WordprocessingML) ---
// Un .docx es un ZIP de XML; lo empaquetamos con el mismo QZip privado de Qt que
// el ODF, con un serializador OOXML propio (Qt no sabe escribir DOCX). Sin
// dependencias externas.

/// \brief Una imagen embebida que produce la serialización: su ruta dentro del paquete y
/// los bytes PNG.
struct DocxImage {
    QString partName;  ///< p. ej. "media/image1.png"
    QByteArray data;   ///< PNG
};

/// \brief Serializa el documento a `word/document.xml` (OOXML completo). Si `images` no es
/// nulo, las imágenes se reencodean a PNG y se registran ahí con su relación rId;
/// si es nulo, se omiten. `title`, si no está vacío, se emite como párrafo «Título».
QString toDocxDocumentXml(const QTextDocument *doc, const QString &title,
                          QList<DocxImage> *images = nullptr);

/// \brief XML de estilos (encabezados, código, cita, título) con el idioma del documento.
/// Puro y testeable aparte.
QByteArray docxStylesXml(const Language &language);
/// \brief XML de numeración (listas con viñetas y numeradas). Puro y testeable aparte.
QByteArray docxNumberingXml();

/// \brief Escribe el documento como .docx en `path`, con el idioma y el título
/// incrustados. Devuelve false y rellena *error si falla. `title` puede ir vacío.
bool writeDocx(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error);

// --- EPUB (.epub) ---
// Un .epub es un ZIP (con `mimetype` sin comprimir primero) de XHTML + paquete
// OPF + navegación. Lo empaquetamos con el mismo QZip privado de Qt, reutilizando
// el HTML de Qt como cuerpo (saneado a XHTML). Sin dependencias externas.

/// \brief Extrae el interior de `<body>` del HTML de Qt y lo deja apto para XHTML (arregla
/// `&nbsp;` y elementos vacíos sin cerrar). Función pura.
QString htmlBodyToXhtml(const QString &fullHtml);

/// \brief Documento XHTML completo (cabecera + cuerpo) para el capítulo del EPUB. Pura.
QString epubContentXhtml(const QString &bodyInner, const QString &title,
                         const Language &language);

/// \brief XML `META-INF/container.xml` del paquete EPUB. Pura.
QByteArray epubContainerXml();
/// \brief XML del paquete OPF del EPUB. `uuid` identifica el libro; `modified` es la marca
/// ISO-8601 UTC (dcterms:modified, obligatoria en EPUB 3). `imageHrefs` son las
/// rutas de las imágenes empaquetadas (p. ej. "images/image1.png"). Pura.
QByteArray epubContentOpf(const Language &language, const QString &title,
                          const QStringList &imageHrefs, const QString &uuid,
                          const QString &modified);
/// \brief XML de navegación `nav.xhtml` del EPUB. Pura.
QByteArray epubNavXhtml(const Language &language, const QString &title);
/// \brief XML `toc.ncx` (tabla de contenidos legada) del EPUB. Pura.
QByteArray epubTocNcx(const QString &title, const QString &uuid);
/// \brief Hoja de estilos CSS embebida en el EPUB. Pura.
QByteArray epubStyleCss();

/// \brief Escribe el documento como .epub en `path`, con el idioma y el título
/// incrustados. Devuelve false y rellena *error si falla. `title` puede ir vacío.
bool writeEpub(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error);

/// \brief Clon del documento listo para HTML/PDF/ODF/impresión: conserva los
/// fragmentos de fórmula con su `verticalAlignment` (Qt serializa el super/sub a
/// CSS en HTML, al equivalente en ODF y los pinta directamente en PDF), pero
/// limpia las propiedades custom de math que solo tienen sentido dentro del
/// editor (`IsMath`, `MathTex`, `MathBlock`). El llamador es dueño del
/// documento devuelto. LaTeX, en cambio, emite las fórmulas verbatim a partir
/// del original con `toLatex`.
QTextDocument *cloneForExport(const QTextDocument *src);

/// \brief Vuelca `doc` en `printer` paginando A MANO con QPainter/newPage(), para
/// poder añadir un pie con el número de página (`footerPageNumbers`). Reserva la
/// franja del pie sobre el área imprimible y pinta el contenido de cada página
/// recortado. Alternativa a `QTextDocument::print`, que no admite pies. MUTA el
/// pageSize de `doc` (pásale un clon, como hacen las rutas de exportación).
void paintPaginated(QPrinter *printer, QTextDocument *doc, bool footerPageNumbers);

} // namespace mdexport

#endif // EXPORTERS_H
