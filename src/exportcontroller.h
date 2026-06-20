#ifndef EXPORTCONTROLLER_H
#define EXPORTCONTROLLER_H

#include <QObject>
#include <QString>

class QTextEdit;
class QWidget;
class DocumentIo;
class SplitViewController;
namespace mdexport { struct Language; }

// Exportación e impresión del documento: PDF, HTML, ODF (.odt), LaTeX (.tex) e
// imprimir. Orquesta los serializadores puros de `mdexport` con los diálogos de
// archivo/idioma y la escritura a disco.
//
// No posee estado: lee el documento del editor WYSIWYG y los metadatos del
// DocumentIo (front matter), vuelca antes el panel de fuente a través del
// SplitViewController, y usa `parent` como padre de los diálogos. Los mensajes de
// estado se piden por señal (como FindReplaceBar).
class ExportController : public QObject
{
    Q_OBJECT

public:
    ExportController(QTextEdit *editor, DocumentIo *documentIo,
                     SplitViewController *split, QWidget *parent);

public slots:
    bool exportPdf();
    bool exportHtml();
    bool exportOdf();
    bool exportLatex();
    bool exportDocx();
    bool exportEpub();
    // Imprime el documento renderizado con el diálogo de impresión del sistema
    // (distinto de exportar a PDF, que escribe a un archivo).
    bool print();
    // Abre una vista previa de impresión (QPrintPreviewDialog) con el documento
    // renderizado; desde ahí se puede ajustar e imprimir.
    bool printPreview();
    // Imprime / exporta a PDF solo el texto seleccionado (avisa si no hay
    // selección). Construye un documento con el fragmento seleccionado.
    bool printSelection();
    bool exportSelectionPdf();
    // Copia el documento completo al portapapeles como HTML (con texto plano de
    // reserva), para pegarlo con formato en Word, correo, etc.
    void copyHtmlToClipboard();

signals:
    // Mensaje para la barra de estado (texto, milisegundos visible).
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
    // Escribe `contents` en `path` en UTF-8; avisa y devuelve false si no se pudo.
    bool writeUtf8File(const QString &path, const QString &contents);

    QTextEdit *m_editor = nullptr;          // editor WYSIWYG (no es propiedad nuestra)
    DocumentIo *m_documentIo = nullptr;
    SplitViewController *m_split = nullptr;
    QWidget *m_parent = nullptr;            // padre de los diálogos
};

#endif // EXPORTCONTROLLER_H
