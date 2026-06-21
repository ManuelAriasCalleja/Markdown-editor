#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QPalette>
#include <QString>

#include "themespec.h"

class QTextEdit;
class QTextCharFormat;
class QTextCursor;
class QAction;
class QCloseEvent;
class QEvent;
class QKeyEvent;
class QMimeData;
class QObject;
class QLabel;
class QToolBar;
class QTimer;
class FocusEditor;
class SplitViewController;
class DiskWatcher;
class ExportController;
class FileController;
class TableController;
class FormatController;
class FormulaController;
class InsertController;
class DistractionFreeController;
class HelpDialog;
class OutlinePanel;
class RecoveryManager;
class CodeBlockHighlighter;
class DiagramController;
class SpellController;
class FindReplaceBar;
class RecentFilesManager;
class DocumentIo;
class ThemeController;
namespace mdexport { struct Language; }

// Ventana principal del editor de Markdown WYSIWYG.
//
// El usuario edita siempre sobre el texto ya renderizado (nunca ve el código
// Markdown). La barra de formato activa/desactiva cada elemento Markdown
// aplicando formatos de Qt que round-trip-ean limpiamente a Markdown:
//   negrita, cursiva, tachado, código en línea, encabezados H1-H3 y listas.
// Al guardar, el documento se serializa con QTextDocument::toMarkdown().
class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Pruebas de caracterización que acceden a los colaboradores internos y
    // disparan operaciones privadas directamente.
    friend class TestSplitView;
    friend class TestFileController;
    friend class TestFormula;

public:
    explicit MainWindow(QWidget *parent = nullptr);

    // Decide qué mostrar al arrancar (se invoca diferido desde main()): un
    // archivo de la línea de comandos tiene prioridad; si no, se ofrece recuperar
    // un borrador de un cierre anómalo; en su defecto, se reabre el último
    // documento de la sesión anterior. `cmdLineFile` vacío = sin argumento.
    void startSession(const QString &cmdLineFile);

    // Arranque de una ventana recreada tras cambiar de idioma: reabre `reopenPath`
    // (si no está vacío) sin pasar por la recuperación de borrador ni el reabrir
    // del último documento, y arranca el autoguardado. La diferencia con
    // startSession() es que aquí el estado ya lo decidió la ventana anterior.
    void relaunchSession(const QString &reopenPath);

signals:
    // El usuario eligió otro idioma. main() intercambia los traductores y recrea
    // la ventana (rehaciendo todos los tr()) reabriendo `reopenPath`. Se emite
    // solo tras confirmar que no hay cambios sin guardar que perder.
    void languageChangeRequested(const QString &reopenPath);

private:
    // Devuelve el dock del esquema a un ancho de lectura cómodo si el estado
    // restaurado lo dejó desproporcionadamente ancho (no toca el modo sin
    // distracciones, que lo ensancha a propósito).
    void normalizeOutlineWidth();

    // Abre el diálogo «Ir a encabezado» (Ctrl+G) y lleva el cursor al elegido.
    void goToHeading();

    // Tras teclear el ':' de cierre (en `cursor`), si justo antes hay un shortcode
    // `:nombre:` conocido, lo sustituye por su símbolo (ver mdshortcode).
    void expandShortcodeBefore(const QTextCursor &cursor);

    // Id de la referencia de nota al pie renderizada bajo `viewportPos`, o cadena
    // vacía si no hay ninguna. Lo usan el clic y la pista de hover.
    QString footnoteRefIdAt(const QPoint &viewportPos) const;
    // Si `viewportPos` cae sobre una referencia de nota al pie, lleva el cursor a
    // su definición y devuelve true.
    bool jumpToFootnoteAt(const QPoint &viewportPos);

protected:
    void closeEvent(QCloseEvent *event) override;
    // Recoloca el esquema y la columna en el modo sin distracciones al cambiar
    // de tamaño (en pantalla completa, al entrar/salir y al redimensionar).
    void resizeEvent(QResizeEvent *event) override;
    // Captura Ctrl+rueda sobre el editor para hacer zoom de la fuente.
    bool eventFilter(QObject *watched, QEvent *event) override;
    // Sub-manejadores del eventFilter (cada uno devuelve true si consume el
    // evento). Separan las tres responsabilidades que antes convivían en él:
    // ratón/rueda/arrastre sobre el viewport, teclado del editor WYSIWYG y
    // teclado del editor de fuente.
    bool handleViewportEvent(QEvent *event);
    bool handleEditorKeyPress(QKeyEvent *ke);
    bool handleSourceKeyPress(QKeyEvent *ke);

private slots:

    // Enter en el editor de código: continúa la lista/tarea actual o, en un ítem
    // vacío, sale de ella. Devuelve true si gestionó la pulsación.
    bool continueSourceList();

    void zoomInText();
    void zoomOutText();
    void resetZoom();
    // Aplica el nivel de zoom actual (m_zoomDelta) al editor y al resto de la
    // interfaz, y lo persiste para la próxima sesión.
    void applyZoom();
    // Escala la fuente del resto de la interfaz (menú, barras de botones, barra
    // de estado y vista de código fuente) al nivel de zoom actual, para que
    // crezca/encoja junto con el texto del editor.
    void applyChromeZoom();
    // Fija la fuente por defecto de QApplication para las clases QMenuBar y
    // QMenu al tamaño objetivo del zoom actual. Se invoca antes de crear los
    // menús y desde applyChromeZoom: Qt cachea anchuras de QAction en la
    // primera medición, así que la fuente «correcta» tiene que estar antes.
    void applyMenuFontScale();
    // Calcula con la QFontMetrics actual el ancho mínimo que cada QMenu
    // necesita para mostrar todas sus acciones (texto + atajo + flecha de
    // submenú) sin elidir, y se lo fija como minimumWidth. Workaround para un
    // bug de Qt 6.8 + gtk3 en el que el sizeHint de los popups cachea las
    // anchuras al tamaño de fuente original y no se actualiza al re-escalar.
    void forceMenuWidths();

    // Actualiza el contador de palabras/caracteres de la barra de estado.
    void updateWordCount();

    // Muestra un diálogo con las estadísticas del documento (palabras, caracteres,
    // párrafos, frases y tiempo de lectura estimado).
    void showDocumentStatistics();

private:
    // Construye la barra de menús. Orquesta los creadores por menú de abajo y, al
    // final, entrega al controlador de la vista dividida las acciones creadas.
    void createMenusAndActions();
    void createFileMenu();
    void createEditMenu();
    // Crea las acciones de formato (negrita, encabezados, listas…), compartidas
    // por el menú Formato y la barra de botones, y el propio menú Formato.
    void createFormatActions();
    void createInsertMenu();
    void createTableMenu();
    // Menú Ver: vista (fuente/dividida/sin distracciones), esquema, zoom, tema e
    // idioma.
    void createViewMenu();
    void createHelpMenu();
    void createFormatToolBar();
    // (Re)genera los iconos de los botones de lista con el color del tema actual.
    void updateToolBarIcons();

    // Guarda el idioma elegido (código de locale; "" = sistema) y avisa de que
    // se aplicará al reiniciar.
    void setLanguage(const QString &code);
    // Muestra el diálogo «Acerca de» con la foto y los datos del autor.
    void showAboutDialog();
    // Abre la ventana de ayuda (manual de la app + guía de Markdown). Es
    // no modal y única: si ya está abierta, la trae al frente en vez de
    // crear otra.
    void showHelpDialog();
    // Abre `href` (Ctrl+clic sobre un enlace); resuelve rutas relativas con la
    // baseUrl del documento.
    void openLink(const QString &href);
    // Refleja en el título y la lista de recientes el archivo actual (conectado
    // a DocumentIo::currentFileChanged).
    void onCurrentFileChanged(const QString &path);

    // --- Vigilancia de cambios del archivo en disco (ver m_diskWatcher) ---
    // El archivo cambió en disco: recarga si no hay cambios locales, o pregunta.
    // `diskBytes` es el contenido nuevo del disco.
    void onDiskExternalChange(const QByteArray &diskBytes);
    // Recarga el archivo actual desde disco descartando el contenido en memoria.
    void reloadFromDisk();

    // Aplica un borde visible a todas las tablas del documento (las creadas y las
    // cargadas). El borde no se serializa a Markdown, así que no afecta al
    // round-trip ni al estado «modificado».
    void styleTables();

    // Editor actualmente visible (WYSIWYG o fuente); delega en m_split.
    QTextEdit *activeEditor() const;
    // Reemplaza el cuerpo del documento WYSIWYG por el Markdown dado y deja el
    // modelo plenamente al día: protege/renderiza fórmulas, da borde a las
    // tablas, recolorea enlaces y reconstruye el índice. Único punto por el que
    // pasa toda sustitución del cuerpo (volcado del fuente, recuperación) para
    // que ningún paso del pipeline se quede atrás.
    void setBodyMarkdown(const QString &body);

    FocusEditor *m_editor = nullptr;
    HelpDialog *m_helpDialog = nullptr;  // se crea perezoso al pulsar F1
    CodeBlockHighlighter *m_highlighter = nullptr;
    SpellController *m_spellController = nullptr;  // corrector ortográfico
    DiagramController *m_diagrams = nullptr;  // previsualización de diagramas
    qreal m_baseFontPointSize = 0;  // tamaño de fuente base, para "Tamaño normal"
    // Tamaños base de las superficies que siguen al zoom y el desfase (en
    // puntos) que se les aplica para escalar junto con el texto del editor.
    qreal m_baseMenuPointSize = 0;
    qreal m_baseToolBarPointSize = 0;
    qreal m_baseFindBarPointSize = 0;
    qreal m_baseStatusBarPointSize = 0;
    qreal m_baseSourceFontPointSize = 0;
    int m_zoomDelta = 0;
    QToolBar *m_formatToolBar = nullptr;

    // Vista de código fuente / dividida y su sincronización (posee el editor de
    // fuente y el QSplitter central).
    SplitViewController *m_split = nullptr;
    QAction *m_sourceModeAction = nullptr;   // toggle de Ver → Código fuente
    QAction *m_splitAction = nullptr;        // toggle de Ver → Vista dividida
    QList<QAction *> m_wysiwygActions;       // acciones válidas solo en WYSIWYG

    QAction *m_boldAction = nullptr;
    QAction *m_italicAction = nullptr;
    QAction *m_underlineAction = nullptr;
    QAction *m_strikeAction = nullptr;
    QAction *m_codeAction = nullptr;
    QAction *m_linkAction = nullptr;
    QAction *m_quoteAction = nullptr;
    QAction *m_codeBlockAction = nullptr;
    QAction *m_h1Action = nullptr;
    QAction *m_h2Action = nullptr;
    QAction *m_h3Action = nullptr;
    QAction *m_h4Action = nullptr;
    QAction *m_h5Action = nullptr;
    QAction *m_h6Action = nullptr;
    QAction *m_bulletAction = nullptr;
    QAction *m_numberedAction = nullptr;
    QAction *m_taskAction = nullptr;
    QAction *m_langAction = nullptr;      // "Lenguaje del bloque": solo dentro de fence
    QAction *m_indentAction = nullptr;    // sangría: solo dentro de una lista
    QAction *m_outdentAction = nullptr;
    QHash<mdtheme::ThemeId, QAction *> m_themeActions;  // marca la acción del tema activo

    QAction *m_distractionAction = nullptr;   // toggle de Ver → Sin distracciones
    DistractionFreeController *m_distraction = nullptr;  // modo pantalla completa/columna

    RecoveryManager *m_recovery = nullptr;      // borrador de recuperación ante fallos
    FileController *m_file = nullptr;           // abrir/guardar/nuevo/recuperar + autoguardado

    OutlinePanel *m_outline = nullptr;          // panel lateral con el índice (TOC)
    QAction *m_outlineAction = nullptr;         // toggle de Ver → Esquema (F9)
    QTimer *m_outlineTimer = nullptr;           // debounce para reconstruir el índice
    qreal m_baseOutlinePointSize = 0;           // tamaño base del panel, para el zoom

    FindReplaceBar *m_findBar = nullptr;        // barra de buscar/reemplazar (abajo)
    RecentFilesManager *m_recentFiles = nullptr;  // gestor de "Abrir recientes"
    DocumentIo *m_documentIo = nullptr;         // E/S del documento (abrir/guardar)
    ThemeController *m_theme = nullptr;         // tema claro/oscuro y enlaces
    QLabel *m_countLabel = nullptr;             // contador en la barra de estado

    FormatController *m_format = nullptr;        // comandos de formato + estado de acciones
    FormulaController *m_formula = nullptr;      // fórmulas TeX (insertar/editar/proteger)
    InsertController *m_insert = nullptr;        // enlaces, imágenes, tablas, regla
    TableController *m_table = nullptr;         // edición de tablas (contextual)

    DiskWatcher *m_diskWatcher = nullptr;       // vigila cambios externos del archivo
    ExportController *m_export = nullptr;        // exportación e impresión
};

#endif // MAINWINDOW_H
