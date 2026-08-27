#ifndef EXPORTERS_H
#define EXPORTERS_H

/// \file
/// \brief Serializadores puros de exportación a ODF, LaTeX, DOCX y EPUB (con el idioma del documento).

#include <QByteArray>
#include <QList>
#include <QString>

class QPainter;
class QRectF;
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
    QString code;        ///< etiqueta canónica (`mdlang::canonicalTag`): "es", "en", "zh_CN"…
    QString name;        ///< nombre nativo para el selector: "Español", "English"…
    QString babel;       ///< opciones de babel en LaTeX: "spanish", "ngerman"…
    QString odfLang;     ///< fo:language del ODF: "es"
    QString odfCountry;  ///< fo:country del ODF: "ES"

    /// \brief El código como etiqueta de idioma BCP 47 ("zh_CN" → "zh-CN").
    ///
    /// `code` es la etiqueta CANÓNICA del programa, que separa con `_` porque así se
    /// llaman los recursos (traducciones, manual). XML no admite esa forma: el guion
    /// bajo no es un separador válido de subetiqueta, así que un `xml:lang="zh_CN"`
    /// invalida el EPUB entero para epubcheck y deja al lector sin saber el idioma
    /// (separación silábica y lectura en voz alta incluidas). Los nueve idiomas cuyo
    /// código no lleva región salen igual que antes.
    QString bcp47() const { return QString(code).replace(QLatin1Char('_'), QLatin1Char('-')); }
};

/// \brief Los idiomas ofrecidos en el diálogo de exportación (los mismos que la UI).
QList<Language> languages();

/// \brief Idioma para un código ISO (admite "es", "es-ES", "es_ES"); si no se reconoce,
/// devuelve el inglés como recurso seguro.
Language languageForCode(const QString &code);

/// \brief Escrituras CJK que aparecen en un documento.
///
/// Hace falta saberlo antes de imprimir o exportar a PDF: ahí el texto se pinta con
/// las fuentes del sistema, y lo que ninguna sepa dibujar **no sale**. Y no sale como
/// una caja vacía, que al menos se vería: desaparece, igual que desaparecía del .tex
/// antes de A1. Comprobado exportando en un sistema sin ninguna fuente CJK: del PDF
/// se cae el encabezado, la prosa, las celdas de la tabla y el comentario del bloque
/// de código, y el resto del documento sale como si tal cosa.
struct CjkScripts {
    bool han = false;     ///< ideogramas (chino, y los kanji del japonés)
    bool kana = false;    ///< hiragana y katakana (japonés)
    bool hangul = false;  ///< jamo y sílabas hangul (coreano)

    bool any() const { return han || kana || hangul; }
};

/// \brief Qué escrituras CJK usa el documento (ninguna, en la inmensa mayoría de los
/// casos). Pura: solo mira el texto, no consulta las fuentes instaladas.
CjkScripts cjkScriptsIn(const QTextDocument *doc);
/// \brief Igual, sobre un texto suelto: el título de la exportación no está en el
/// documento y puede traer ideogramas él solo.
CjkScripts cjkScriptsIn(const QString &text);

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

/// \brief Lo que `toLatex` no ha podido trasladar tal cual, para poder avisar al
/// usuario en vez de perderlo en silencio (que es lo que se hacía antes).
struct LatexIssues {
    /// El documento trae ideogramas chinos, kana japonés o hangul coreano: se
    /// emiten (y el preámbulo se prepara para ellos), pero el .tex ya SOLO compila
    /// con xelatex/lualatex, nunca con pdflatex.
    bool needsUnicodeEngine = false;
    /// Símbolos y emoji sin equivalente en LaTeX que se han descartado para no
    /// romper la compilación. Se descartaban ya; lo nuevo es contarlos.
    int droppedSymbols = 0;
};

/// \brief Serializa el documento a un .tex completo (preámbulo + cuerpo), con babel del
/// idioma dado y, si `title` no está vacío, `\maketitle`.
/// \param outputTexPath ruta del .tex de destino. Si se da, las imágenes que
/// pdflatex no sabe incluir (SVG, GIF, BMP…) se rasterizan a un PNG escrito en esa
/// misma carpeta y se referencian; si está vacía (p.ej. en tests), se referencian
/// tal cual. Salvo por ese efecto de escritura de PNGs, la función es pura.
/// \param issues si no es nulo, recibe lo que no se ha podido trasladar (ver
/// LatexIssues). Quien traduce y muestra el aviso es ExportController: este módulo
/// solo informa de lo que ha pasado.
QString toLatex(const QTextDocument *doc, const Language &language, const QString &title,
                const QString &outputTexPath = QString(), LatexIssues *issues = nullptr);

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

/// \brief Una imagen embebida que produce la serialización: su ruta dentro del paquete,
/// los bytes PNG y el rId con el que la cita el documento.
struct DocxImage {
    QString partName;         ///< p. ej. "media/image1.png"
    QByteArray data;          ///< PNG
    QString relationshipId;   ///< p. ej. "rId3"
};

/// \brief Un enlace externo que produce la serialización. En OOXML el destino NO cabe
/// en el documento: va en una relación aparte (`TargetMode="External"`) que el
/// `<w:hyperlink r:id>` referencia, como escribe Word.
struct DocxHyperlink {
    QString relationshipId;  ///< p. ej. "rId4"
    QString target;          ///< URL de destino
};

/// \brief Serializa el documento a `word/document.xml` (OOXML completo). Si `images` no es
/// nulo, las imágenes se reencodean a PNG y se registran ahí con su relación rId;
/// si es nulo, se omiten. Igual con `hyperlinks`: si es nulo, el texto del enlace
/// se emite con su estilo pero sin destino (no hay dónde declarar la relación).
/// `title`, si no está vacío, se emite como párrafo «Título».
QString toDocxDocumentXml(const QTextDocument *doc, const QString &title,
                          QList<DocxImage> *images = nullptr,
                          QList<DocxHyperlink> *hyperlinks = nullptr);

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

// --- HTML (.html) ---

/// \brief Documento HTML completo para *Exportar a HTML*: el `toHtml()` de Qt más lo
/// que ese no pone y sí tiene el documento.
///   - `lang` en `<html>`: sin él, ni el lector de pantalla ni la separación
///     silábica del navegador saben en qué idioma está el texto;
///   - `<title>`: sin él la pestaña del navegador muestra el nombre del fichero;
///   - las **imágenes embebidas** como `data:` URI. Qt referencia la ruta relativa
///     tal cual (`src="imagen.png"`), así que el .html se veía bien donde se
///     exportó y perdía TODAS las imágenes en cuanto se movía o se enviaba a
///     alguien. Las que no se puedan cargar (una URL remota) se dejan como están.
/// `title` y el idioma pueden ir vacíos. Salvo por la lectura de recursos del
/// documento, es una función pura.
QString toHtmlDocument(const QTextDocument *doc, const Language &language,
                       const QString &title);

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
/// \brief Una entrada del índice del EPUB: un encabezado del documento y el ancla a
/// la que apunta dentro de `content.xhtml`.
struct EpubTocEntry {
    int level;      ///< 1..6, para anidar el índice
    QString text;   ///< rótulo
    QString anchor;  ///< id del ancla (sin `#`)
};

/// \brief Pone un `id` a cada `<h1>`…`<h6>` del cuerpo (en orden de documento) para
/// que el índice pueda enlazarlos, y devuelve el cuerpo modificado. Sin anclas, un
/// índice con capítulos no puede saltar a ninguna parte. Función pura.
QString epubAnchorHeadings(const QString &bodyInner);

/// \brief XML de navegación `nav.xhtml` del EPUB, con el índice anidado por niveles.
/// Con `entries` vacío cae a una única entrada al documento. Pura.
QByteArray epubNavXhtml(const Language &language, const QString &title,
                        const QList<EpubTocEntry> &entries = {});
/// \brief XML `toc.ncx` (tabla de contenidos legada) del EPUB. Pura.
QByteArray epubTocNcx(const QString &title, const QString &uuid,
                      const QList<EpubTocEntry> &entries = {});
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

/// \brief Encoge (nunca agranda) las imágenes de `doc` que sobresalen de `maxWidth`
/// (en unidades del documento), conservando la proporción. Sin esto, una imagen más
/// ancha que la página —un gantt apaisado— sale TRUNCADA en el PDF y la impresión:
/// la maqueta de QTextDocument recorta en el borde en vez de escalar (a diferencia
/// de HTML/DOCX/LaTeX, que ya escalan). `dpiScale` es el factor con el que la
/// maqueta agranda una imagen SIN tamaño explícito al pintar en el dispositivo
/// (dpi del dispositivo / dpi lógico de pantalla); con tamaño explícito no aplica.
/// Solo fija la anchura: la altura sin fijar sigue en automático y conserva la
/// proporción sola. MUTA `doc` (pásale un clon, como hacen las rutas de impresión).
void clampImagesToWidth(QTextDocument *doc, qreal maxWidth, qreal dpiScale = 1.0);

/// \brief Fija en el mapa de recursos PERSISTENTE de `doc` las imágenes que ahora
/// mismo solo se resuelven vía baseUrl (caché). `QTextDocument::print()` clona el
/// documento por dentro, y ese clon copia los recursos pero NO la baseUrl ni la
/// caché: sin esto, las imágenes de ruta relativa desaparecen de la impresión y
/// del PDF cuando los números de página están desactivados (la rama `doc->print`).
/// MUTA `doc` (pásale un clon, como hacen las rutas de impresión).
void bakeImageResources(QTextDocument *doc);

/// \brief Pinta en `painter` la franja `pageBody` de `doc` (una página), con la
/// tinta del PAPEL: el texto sin color propio sale negro.
///
/// No vale `QTextDocument::drawContents`: pinta con un PaintContext cuya paleta es
/// la de la APLICACIÓN, así que con un tema oscuro el cuerpo salía impreso en el
/// color de texto del tema (un crema casi blanco en Solarized Dark) sobre el blanco
/// del papel — ilegible. `QTextDocument::print()` fuerza el negro por su cuenta; al
/// pintar nosotros la paginación hay que hacer lo mismo. Los colores explícitos del
/// documento (enlaces, resaltado del código, admoniciones) viven en los formatos de
/// carácter y no los toca esto.
void paintDocumentPage(QPainter *painter, QTextDocument *doc, const QRectF &pageBody);

/// \brief Vuelca `doc` en `printer` paginando A MANO con QPainter/newPage(), para
/// poder añadir un pie con el número de página (`footerPageNumbers`). Reserva la
/// franja del pie sobre el área imprimible y pinta el contenido de cada página
/// recortado. Alternativa a `QTextDocument::print`, que no admite pies. MUTA el
/// pageSize de `doc` (pásale un clon, como hacen las rutas de exportación).
void paintPaginated(QPrinter *printer, QTextDocument *doc, bool footerPageNumbers);

} // namespace mdexport

#endif // EXPORTERS_H
