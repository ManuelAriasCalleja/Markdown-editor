#ifndef EDITORSTACK_H
#define EDITORSTACK_H

/// \file
/// \brief Conjunto completo de un documento abierto (editor y sus colaboradores): la unidad por pestaña.

#include <QWidget>
#include <QString>

class QByteArray;
class QTextEdit;
class FocusEditor;
class CodeBlockHighlighter;
class DocumentIo;
class ThemeController;
class SpellController;
class DiagramController;
class SplitViewController;
class FormatController;
class TableController;
class FormulaController;
class InsertController;
class ExportController;
class RecoveryManager;
class FileController;
class DiskWatcher;
class FindReplaceBar;
class OutlinePanel;

/// Conjunto completo de un documento abierto: el editor WYSIWYG, su editor de
/// fuente (en `SplitViewController`) y los 15 colaboradores que operan sobre ese
/// documento (E/S, formato, inserción, tablas, fórmulas, exportación, corrector,
/// diagramas, tema, autoguardado, vigilancia de disco…). Es la unidad que, en la
/// edición por pestañas, se multiplica: una instancia por documento abierto.
///
/// `MainWindow` (el *shell*) posee lo que es de ventana —menús, barra de formato,
/// zoom, barra de estado, modo sin distracciones— y habla con el documento activo
/// a través de estos accesores. La barra de búsqueda y el panel de esquema, hoy
/// compartidos por la ventana, se le inyectan en el constructor (los usa
/// `SplitViewController`).
///
/// Las señales reenvían al shell lo que necesita para la ventana (mensajes de
/// estado, marca «modificado», archivo actual, recuentos…); el cableado interno
/// entre colaboradores vive aquí.
class EditorStack : public QWidget
{
    Q_OBJECT

public:
    /// El widget contiene la vista (editor WYSIWYG + fuente). `findBar` y `outline`
    /// son los compartidos de la ventana (los usa SplitViewController). Los
    /// colaboradores se parentan a este widget: así sus diálogos se centran sobre
    /// la ventana y se destruyen al cerrar la pestaña.
    EditorStack(FindReplaceBar *findBar, OutlinePanel *outline, QWidget *parent = nullptr);

    FocusEditor *editor() const { return m_editor; }
    DocumentIo *documentIo() const { return m_documentIo; }
    ThemeController *theme() const { return m_theme; }
    SpellController *spell() const { return m_spell; }
    SplitViewController *split() const { return m_split; }
    FormatController *format() const { return m_format; }
    TableController *table() const { return m_table; }
    FormulaController *formula() const { return m_formula; }
    InsertController *insert() const { return m_insert; }
    ExportController *exporter() const { return m_export; }
    RecoveryManager *recovery() const { return m_recovery; }
    FileController *file() const { return m_file; }
    DiskWatcher *diskWatcher() const { return m_diskWatcher; }

    /// Editor visible ahora mismo (WYSIWYG o fuente); delega en el split.
    QTextEdit *activeEditor() const;

    /// Aplica un borde visible a todas las tablas del documento (no se serializa a
    /// Markdown). Lo dispara también la carga y la inserción de tablas.
    void styleTables();
    /// Reemplaza el cuerpo Markdown del documento WYSIWYG dejando el modelo al día
    /// (protege/renderiza fórmulas, da borde a tablas, recolorea enlaces y
    /// reconstruye el índice). Único punto por el que pasa toda sustitución del
    /// cuerpo (volcado del fuente, recuperación, reordenado del esquema).
    void setBodyMarkdown(const QString &body);

    /// Modo «máquina de escribir»: mantiene la línea del cursor centrada en
    /// vertical mientras se escribe. Lo activa/desactiva la ventana por ajuste; al
    /// activarlo, centra ya el editor activo.
    void setTypewriterMode(bool on);

    /// Interlineado del editor en porcentaje de la altura natural (100 = sencillo).
    /// Es presentación pura (no se serializa); se reaplica tras cargar y en cada
    /// sustitución del cuerpo, porque setMarkdown rehace los formatos de bloque.
    void setLineSpacing(int percent);

    /// Inserta el cuerpo Markdown de un snippet en el editor activo: crudo en la
    /// vista de fuente (es texto Markdown) y renderizado como fragmento en WYSIWYG.
    void insertSnippet(const QString &body);

    /// Limpia/normaliza el Markdown del documento (`mdtidy::tidy`): en la vista de
    /// fuente, sobre su texto; en WYSIWYG, re-serializa, limpia y recarga.
    void cleanMarkdown();

signals:
    /// Mensaje para la barra de estado de la ventana (texto, ms).
    void statusMessage(const QString &text, int timeout);
    /// El contenido difiere (o no) del guardado: la ventana marca el título.
    void windowModifiedChanged(bool modified);
    /// Cambió el archivo asociado (título + recientes + vigilancia de disco).
    void currentFileChanged(const QString &path);
    /// Terminó de cargarse un documento (reconstruir esquema, etc.).
    void documentLoaded();
    /// Hay que recalcular el contador de palabras de la barra de estado.
    void wordCountShouldUpdate();
    /// Hay que refrescar el estado de las acciones de formato (bajo el cursor).
    void formatActionsShouldUpdate();
    /// No se pudo cargar `path` (quitar de recientes).
    void loadFailed(const QString &path);
    /// El archivo abierto cambió/desapareció en disco (recargar o avisar).
    void diskExternalChange(const QByteArray &diskBytes);
    /// El archivo abierto desapareció del disco.
    void diskVanished();

private:
    // Si el cursor del editor WYSIWYG está sobre una fórmula, anuncia su TeX (vía
    // statusMessage → QAccessibleAnnouncementEvent) una sola vez al entrar: una
    // fórmula 2D es un único ObjectReplacementCharacter opaco para los lectores,
    // y las inline son glifos Unicode sin estructura; el TeX es lo legible.
    void announceFormulaUnderCursor();
    // TeX de la fórmula bajo el cursor (vacío si no hay); deja en *start la
    // posición de inicio de su grupo, para no repetir el anuncio al moverse dentro.
    QString formulaAtCursor(int *start) const;
    int m_lastFormulaStart = -1;

    // Si el modo máquina de escribir está activo, desplaza `ed` para que la línea
    // del cursor quede a media altura (lógica pura en `mdtypewriter`).
    void centerCursorLine(QTextEdit *ed);
    // «Foco de línea» (misma palanca que la máquina de escribir): atenúa con
    // QTextEdit::extraSelections todo `ed` salvo el párrafo del cursor. Si el modo
    // está apagado, limpia las selecciones. Los tramos los calcula `mdtypewriter`.
    void applyLineFocus(QTextEdit *ed);
    bool m_typewriter = false;

    // Aplica el interlineado actual (m_lineSpacing) a todos los bloques de `ed`.
    // Presentación pura: preserva la marca «modificado» de Qt (como styleTables).
    void applyLineSpacing(QTextEdit *ed);
    int m_lineSpacing = 100;  // porcentaje; 100 = sencillo

    FocusEditor *m_editor = nullptr;
    CodeBlockHighlighter *m_highlighter = nullptr;
    DocumentIo *m_documentIo = nullptr;
    ThemeController *m_theme = nullptr;
    SpellController *m_spell = nullptr;
    DiagramController *m_diagrams = nullptr;
    SplitViewController *m_split = nullptr;
    FormatController *m_format = nullptr;
    TableController *m_table = nullptr;
    FormulaController *m_formula = nullptr;
    InsertController *m_insert = nullptr;
    ExportController *m_export = nullptr;
    RecoveryManager *m_recovery = nullptr;
    FileController *m_file = nullptr;
    DiskWatcher *m_diskWatcher = nullptr;

    // Compartidos con la ventana (no los posee): los usa el split.
    FindReplaceBar *m_findBar = nullptr;
    OutlinePanel *m_outline = nullptr;
};

#endif // EDITORSTACK_H
