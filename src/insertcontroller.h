#ifndef INSERTCONTROLLER_H
#define INSERTCONTROLLER_H

#include <QObject>

class QMimeData;
class QTextEdit;
class QWidget;
class DocumentIo;
class SymbolPicker;

// Comandos de inserción del editor WYSIWYG: enlaces, imágenes (con diálogo o
// pegadas del portapapeles), tablas y reglas horizontales. Las imágenes locales
// se referencian con ruta relativa al .md cuando es posible; las pegadas se
// guardan a disco como PNG (no se incrustan, para que el Markdown round-trip-ee).
//
// Recibe el editor y el DocumentIo (para la ruta del documento); usa `parent`
// como padre de los diálogos. Pide por señal los refrescos que tocan a otros
// colaboradores (estado de las acciones de formato tras un enlace, borde de las
// tablas recién insertadas).
class InsertController : public QObject
{
    Q_OBJECT

public:
    InsertController(QTextEdit *editor, DocumentIo *documentIo, QWidget *parent);

    // Si los datos del portapapeles/arrastre traen una imagen, la guarda a disco e
    // inserta `![](ruta)`. Devuelve true si la gestionó (para que el editor no la
    // incruste). Lo invoca el handler de pegado/soltado de FocusEditor.
    bool handlePastedImage(const QMimeData *source);

    // Si se pega una sola URL habiendo una selección (de un único bloque), la
    // envuelve como `[selección](url)`. Devuelve true si lo gestionó. Encadenado
    // tras handlePastedImage en el handler de pegado.
    bool handlePastedUrl(const QMimeData *source);

public slots:
    void insertLink();
    void insertImage();
    void insertTable();
    void insertHorizontalRule();
    void insertTableOfContents();    // índice (TOC) con los encabezados del documento
    void insertFootnote();           // referencia `[^n]` + definición autonumeradas
    void insertSymbol();             // abre el diálogo de símbolos especiales
    void insertDate();               // fecha actual (formato largo localizado)
    void insertDateTime();           // fecha y hora actuales (localizadas)
    void pasteImageFromClipboard();  // acción del menú Insertar

signals:
    // Tras insertar un enlace: refrescar el estado de las acciones de formato.
    void formatActionsShouldRefresh();
    // Tras insertar una tabla: aplicarle borde visible (styleTables).
    void tableInserted();

private:
    QTextEdit *m_editor = nullptr;
    DocumentIo *m_documentIo = nullptr;
    QWidget *m_parent = nullptr;  // padre de los diálogos
    SymbolPicker *m_symbolPicker = nullptr;  // no modal, reutilizado entre aperturas
};

#endif // INSERTCONTROLLER_H
