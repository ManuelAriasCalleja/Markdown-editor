#ifndef EXPORTCONTROLLER_H
#define EXPORTCONTROLLER_H

/// \file
/// \brief Controlador de exportación e impresión del documento (PDF, HTML, ODF, LaTeX, DOCX, EPUB, imprimir).

#include <QObject>
#include <QString>

#include <functional>

class QMimeData;
class QPrinter;
class QTextEdit;
class QTextDocument;
class QWidget;
class DocumentIo;
class SplitViewController;
namespace mdexport { struct Language; struct LatexIssues; }

/// \brief Exportación e impresión del documento: PDF, HTML, ODF (.odt), LaTeX (.tex),
/// DOCX (.docx), EPUB (.epub) e imprimir. Orquesta los serializadores puros de
/// `mdexport` con los diálogos de archivo/idioma y la escritura a disco. Los
/// formatos basados en archivo comparten `runExport` (dirigido por `FileExporter`).
///
/// No posee estado: lee el documento del editor WYSIWYG y los metadatos del
/// DocumentIo (front matter), vuelca antes el panel de fuente a través del
/// SplitViewController, y usa `parent` como padre de los diálogos. Los mensajes de
/// estado se piden por señal (como FindReplaceBar).
class ExportController : public QObject
{
    Q_OBJECT

public:
    /// \brief Construye el controlador sobre el editor WYSIWYG, su DocumentIo (front
    /// matter), el SplitViewController (para volcar el panel de fuente) y `parent`
    /// como padre de los diálogos. No toma propiedad de ninguno.
    ExportController(QTextEdit *editor, DocumentIo *documentIo,
                     SplitViewController *split, QWidget *parent);

public slots:
    /// \brief Exporta el documento a PDF (a un archivo).
    bool exportPdf();
    /// \brief Exporta el documento a HTML.
    bool exportHtml();
    /// \brief Exporta el documento a ODF (.odt), con el idioma incrustado.
    bool exportOdf();
    /// \brief Exporta el documento a LaTeX (.tex), con babel del idioma elegido.
    bool exportLatex();
    /// \brief Exporta el documento a DOCX (.docx).
    bool exportDocx();
    /// \brief Exporta el documento a EPUB (.epub).
    bool exportEpub();
    /// \brief Exporta el documento a texto plano (.txt), sin formato Markdown.
    bool exportPlainText();
    /// \brief Imprime el documento renderizado con el diálogo de impresión del sistema
    /// (distinto de exportar a PDF, que escribe a un archivo).
    bool print();
    /// \brief Abre una vista previa de impresión (QPrintPreviewDialog) con el documento
    /// renderizado; desde ahí se puede ajustar e imprimir.
    bool printPreview();
    /// \brief Imprime / exporta a PDF solo el texto seleccionado (avisa si no hay
    /// selección). Construye un documento con el fragmento seleccionado.
    bool printSelection();
    /// \brief Exporta a PDF solo el texto seleccionado.
    bool exportSelectionPdf();
    /// \brief Copia el documento completo al portapapeles como HTML (con texto plano de
    /// reserva), para pegarlo con formato en Word, correo, etc.
    void copyHtmlToClipboard();
    /// \brief Copia al portapapeles como Markdown la selección (si la hay) o el
    /// documento completo, usando la serialización canónica del proyecto.
    void copyMarkdownToClipboard();

signals:
    /// \brief Mensaje para la barra de estado (texto, milisegundos visible).
    void statusMessage(const QString &text, int timeoutMs);

private:
    // Título para la exportación: el `title` del front matter, o "" si no hay.
    QString exportTitle() const;
    // Pide al usuario el idioma del documento (por defecto, el `lang`/`language`
    // del front matter, o el de la app, o el del sistema). false si cancela.
    bool chooseExportLanguage(mdexport::Language *out);
    // Ruta sugerida para exportar con la extensión `ext` (sin punto): junto al
    // archivo actual con ese sufijo, o «documento.<ext>» en HOME si no hay archivo.
    QString suggestedExportPath(const QString &ext) const;
    // Muestra el diálogo de guardar (con `title`, `filter` y la ruta sugerida para
    // `ext`) y, si el usuario no puso extensión, le añade `.ext`. "" si cancela.
    QString promptSavePath(const QString &title, const QString &filter, const QString &ext);
    // Escribe `contents` en `path` en UTF-8; devuelve false y rellena *error si no
    // se pudo (el aviso lo muestra runExport, uniforme para todos los formatos).
    static bool writeUtf8File(const QString &path, const QString &contents, QString *error);
    // Pone `mime` en el portapapeles (toma su propiedad) y anuncia `statusMsg` (un
    // literal QT_TRANSLATE_NOOP("MainWindow", ...)). Sumidero común de los «Copiar
    // como …».
    void setClipboardMime(QMimeData *mime, const char *statusMsg);
    // Incrusta en el PDF el título y el autor del front matter (setDocName/setCreator).
    // Solo tiene efecto con la salida en PdfFormat. Punto único de los dos exportadores PDF.
    void applyPdfMetadata(QPrinter *printer) const;

    // Punto ÚNICO de volcado a impresora/PDF (R2): lo llaman todas las rutas
    // (imprimir, imprimir selección, vista previa, exportar PDF). Si el ajuste de
    // números de página está activo, pagina a mano y añade el pie; si no, usa el
    // print() de Qt (conducta original).
    void renderToPrinter(QTextDocument *doc, QPrinter *printer) const;

    // Avisa si el documento lleva texto chino, japonés o coreano y en el sistema no
    // hay NINGUNA fuente que sepa dibujarlo: ese texto no aparece en el PDF ni en el
    // papel, y no deja ni una caja vacía en su sitio. Es el mismo criterio que en
    // LaTeX (reportLatexIssues): no se bloquea la exportación —el resto del documento
    // sale bien— pero tampoco se pierde texto sin decirlo. Sin CJK no hace nada, que
    // es el caso de casi todos los documentos.
    void reportMissingCjkFont(const QTextDocument *doc) const;

    // Descriptor de un formato de exportación basado en archivo (HTML/ODF/LaTeX/
    // DOCX/EPUB). Los textos van como QT_TRANSLATE_NOOP("MainWindow", ...) para que
    // lupdate los extraiga sin traducirlos aquí; runExport los traduce al usarlos.
    struct FileExporter {
        const char *title;     // título del diálogo de guardar
        const char *filter;    // filtro de archivos del diálogo
        QString ext;           // extensión por defecto (sin punto)
        const char *errorMsg;  // mensaje de error (con %1=ruta, %2=detalle)
        const char *okMsg;     // mensaje de éxito (con %1=ruta)
        bool needsLanguage;    // ¿preguntar el idioma del documento?
        bool useFlatClone;     // ¿exportar el clon «plano» (cloneForExport) o el original?
        bool needsBaseUrl;     // ¿copiar la baseUrl al clon (para imágenes relativas)?
        // Serializa `doc` a `path`. Devuelve false y rellena *error si falla.
        std::function<bool(const QTextDocument *, const QString &,
                           const mdexport::Language &, const QString &, QString *)> write;
    };
    // Flujo común de exportación a archivo: vuelca la fuente, (pide idioma), pide
    // ruta, prepara el documento, llama a `exp.write` y muestra el resultado.
    bool runExport(const FileExporter &exp);

    // Cuenta lo que el .tex no ha podido trasladar tal cual (ver mdexport::toLatex).
    // NO bloquea la exportación: el archivo ya está escrito y es válido; esto solo
    // evita que el usuario se entere al compilar, o peor, nunca. Es aquí y no en
    // `mdexport` porque el módulo es puro y no traduce.
    void reportLatexIssues(const mdexport::LatexIssues &issues) const;

    QTextEdit *m_editor = nullptr;          // editor WYSIWYG (no es propiedad nuestra)
    DocumentIo *m_documentIo = nullptr;
    SplitViewController *m_split = nullptr;
    QWidget *m_parent = nullptr;            // padre de los diálogos
};

#endif // EXPORTCONTROLLER_H
