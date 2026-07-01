#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/// \file
/// \brief Ventana principal (shell): pestañas, menús, barra de formato, zoom y sesión del editor WYSIWYG.

#include <QMainWindow>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QPalette>
#include <QString>

#include "themespec.h"
#include "formatcontroller.h"  // FormatController::Actions (miembro por valor)

class QTextEdit;
class QTabWidget;
class QTextCharFormat;
class QTextCursor;
class QAction;
class QCloseEvent;
class QEvent;
class QKeyEvent;
class QMenu;
class QMimeData;
class QObject;
class QLabel;
class QToolBar;
class QTimer;
class EditorStack;
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

/// \brief Ventana principal del editor de Markdown WYSIWYG.
///
/// El usuario edita siempre sobre el texto ya renderizado (nunca ve el código
/// Markdown). La barra de formato activa/desactiva cada elemento Markdown
/// aplicando formatos de Qt que round-trip-ean limpiamente a Markdown:
/// negrita, cursiva, tachado, código en línea, encabezados H1-H3 y listas.
/// Al guardar, el documento se serializa con QTextDocument::toMarkdown().
class MainWindow : public QMainWindow
{
    Q_OBJECT

    // Pruebas de caracterización que acceden a los colaboradores internos y
    // disparan operaciones privadas directamente.
    friend class TestSplitView;
    friend class TestFileController;
    friend class TestFormula;
    friend class TestExtraSelections;
    friend class TestHeadingShift;
    friend class TestTableSortUi;
    friend class TestLineCommands;
    friend class TestInputRules;

public:
    explicit MainWindow(QWidget *parent = nullptr);

    /// \brief Decide qué mostrar al arrancar (se invoca diferido desde main()): un
    /// archivo de la línea de comandos tiene prioridad; si no, se ofrece recuperar
    /// un borrador de un cierre anómalo; en su defecto, se reabre el último
    /// documento de la sesión anterior.
    /// \param cmdLineFile ruta del archivo de la línea de comandos; vacío = sin argumento.
    void startSession(const QString &cmdLineFile);

    /// \brief Arranque de una ventana recreada tras cambiar de idioma: reabre `reopenPath`
    /// (si no está vacío) sin pasar por la recuperación de borrador ni el reabrir
    /// del último documento, y arranca el autoguardado.
    ///
    /// La diferencia con startSession() es que aquí el estado ya lo decidió la
    /// ventana anterior.
    void relaunchSession(const QString &reopenPath);

signals:
    /// \brief El usuario eligió otro idioma. main() intercambia los traductores y recrea
    /// la ventana (rehaciendo todos los tr()) reabriendo `reopenPath`.
    ///
    /// Se emite solo tras confirmar que no hay cambios sin guardar que perder.
    void languageChangeRequested(const QString &reopenPath);

private:
    // Devuelve el dock del esquema a un ancho de lectura cómodo si el estado
    // restaurado lo dejó desproporcionadamente ancho (no toca el modo sin
    // distracciones, que lo ensancha a propósito).
    void normalizeOutlineWidth();

    // Abre el diálogo «Ir a encabezado» (Ctrl+G) y lleva el cursor al elegido.
    void goToHeading();

    // Pide un número de línea (Ctrl+L) y lleva el cursor a esa línea del editor
    // activo (sirve también en la vista de fuente y dividida).
    void goToLine();

    // Abre la paleta de comandos (Ctrl+Shift+P): busca cualquier acción de los
    // menús por su nombre (filtro difuso) y la dispara al aceptar.
    void commandPalette();

    // Regla de entrada: si el bloque del cursor (párrafo normal) empieza justo por un
    // marcador Markdown de bloque (`#`, `>`, `-`, `1.`…) y se acaba de teclear un
    // espacio, borra el marcador y aplica el formato equivalente. Devuelve true si
    // transformó (el espacio se consume, no se inserta literal).
    bool applyInputRule();

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
    /// \brief Recoloca el esquema y la columna en el modo sin distracciones al cambiar
    /// de tamaño (en pantalla completa, al entrar/salir y al redimensionar).
    void resizeEvent(QResizeEvent *event) override;
    /// \brief Regenera los iconos de la barra de formato cuando la ventana ya está en
    /// pantalla.
    ///
    /// Durante la construcción la ventana aún no tiene pantalla asignada, así que
    /// en monitores HiDPI los iconos se hornean a devicePixelRatio = 1 y se ven
    /// borrosos/«aguados» (sin contraste) hasta el primer relayout; aquí, ya
    /// mostrada, la dpr es la definitiva.
    void showEvent(QShowEvent *event) override;
    /// \brief Rehornea los iconos de la barra cuando cambia la paleta (tema, esquema del
    /// SO, luz cálida) o la densidad de pantalla (otro monitor / Wayland tras el
    /// show). Es el punto único que mantiene su tinta contrastando con el fondo.
    void changeEvent(QEvent *event) override;
    /// \brief Despachador de eventos: delega en los tres sub-manejadores de abajo
    /// (viewport, teclado WYSIWYG, teclado de fuente); el primero que consume gana.
    bool eventFilter(QObject *watched, QEvent *event) override;
    /// \brief Sub-manejador del viewport: zoom con Ctrl+rueda, enlaces, arrastrar-soltar,
    /// tareas y notas al pie. \return true si consume el evento.
    bool handleViewportEvent(QEvent *event);
    /// \brief Sub-manejador del teclado del editor WYSIWYG (fórmulas, shortcodes,
    /// auto-emparejado). \return true si consume la pulsación.
    bool handleEditorKeyPress(QKeyEvent *ke);
    /// \brief Sub-manejador del teclado del editor de fuente (continuación de listas,
    /// auto-emparejado). \return true si consume la pulsación.
    bool handleSourceKeyPress(QKeyEvent *ke);
    /// \brief Auto-emparejado: si la tecla `ke` (un carácter imprimible, sin Ctrl/Alt/Meta)
    /// dispara una acción de `mdautopair` sobre `ed`, la aplica.
    /// \return true si aplicó el emparejado.
    bool applyAutoPair(QTextEdit *ed, QKeyEvent *ke);

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
    // Factor de escala de la interfaz al nivel de zoom actual (1.0 sin zoom). Es
    // el cociente entre la fuente escalada del editor y su base; lo usa el modo
    // sin distracciones para escalar la columna de lectura (en px) como crece la
    // fuente. Devuelve 1.0 si la fuente base no usa puntos.
    qreal uiScaleFactor() const;
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

    // Actualiza el indicador de línea y columna del cursor (barra de estado).
    void updateLineColumn();

    // Muestra un mensaje en la barra de estado y, además, lo anuncia a los lectores
    // de pantalla (que no leen la barra de estado por sí sola). Punto único por el
    // que pasa el feedback efímero importante (guardado, exportado, «no encontrado»,
    // cambios en disco…); las pistas de hover van por showMessage directo, sin
    // anunciarse. El anuncio accesible necesita Qt 6.8+ (degrada a solo visual).
    void showStatusMessage(const QString &text, int timeout = 0);

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
    // Repuebla el submenú Insertar → Snippet con los snippets guardados (uno por
    // entrada que los inserta) más «Gestionar snippets…». Se llama al construir el
    // menú y tras editar la lista en el diálogo gestor.
    void rebuildSnippetsMenu();
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
    // Acción «Revertir a lo guardado»: pide confirmación (si hay cambios) y recarga
    // del disco vía reloadFromDisk(). No hace nada si no hay archivo.
    void revertToSaved();

    // Conecta las señales de un EditorStack (mensajes de estado, marca de
    // modificado, archivo actual, recargas de disco, recuento, esquema) a la
    // ventana. Se llama una vez por documento (pestaña).
    void connectStack(EditorStack *stack);

    // --- Edición por pestañas ---
    // Crea un documento nuevo, lo cablea (connectStack + filtro de eventos + sus
    // acciones) y lo añade como pestaña activa. Devuelve el stack creado.
    EditorStack *addTab();
    // Cambia el documento activo: re-vincula barra de búsqueda, esquema, estado de
    // las acciones, título y modo de vista al stack dado.
    void setActiveStack(EditorStack *stack);
    // Entrega a un documento las acciones compartidas (formato, tabla, modo).
    void configureStack(EditorStack *stack);
    // Cierra la pestaña `index` (pregunta si tiene cambios sin guardar). No cierra
    // la última: la deja como documento nuevo.
    void closeTab(int index);
    // Reabre la última pestaña cerrada que tenía archivo en disco (Ctrl+Shift+R).
    void reopenClosedTab();
    // Documento de la pestaña `index` (o la actual con -1), o nullptr.
    EditorStack *stackAt(int index) const;
    // Documento nuevo en una pestaña nueva (Archivo → Nuevo).
    void newTab();
    // Cambia a la pestaña +delta/-delta (con envoltura), si hay más de una.
    // Atajos Ctrl+AvPág/RePág y Ctrl+Tab/Ctrl+Shift+Tab.
    void cycleTab(int delta);
    // Alterna el foco de teclado entre el esquema (TOC) y el editor (F6):
    // muestra el esquema si está oculto, lo enfoca; si ya lo tiene, vuelve al editor.
    void toggleOutlineFocus();
    // Diálogo de abrir → abre en pestaña (reusa la actual si está vacía).
    void openInTab();
    // Diálogo de importar → convierte un HTML a Markdown y lo abre como documento
    // nuevo sin título en una pestaña nueva (no pisa el original).
    void importHtml();
    // Abre `path` en una pestaña: si ya está abierto salta a él; reusa la actual si
    // es un documento nuevo vacío; si no, una pestaña nueva.
    void openPathInTab(const QString &path);
    // Refleja en la etiqueta de la pestaña del `stack` su nombre de archivo y un
    // punto si tiene cambios sin guardar.
    void updateTabLabel(EditorStack *stack);

    QTabWidget *m_tabs = nullptr;    // un documento por pestaña
    EditorStack *m_stack = nullptr;  // documento ACTIVO (pestaña actual)
    QStringList m_closedTabs;        // pila de rutas de pestañas cerradas (reabrir)
    // Tema/paleta/tinte cálido: ÚNICO de la ventana (global a la app). Se reapunta al
    // editor de la pestaña activa en setActiveStack; las pestañas no lo poseen.
    ThemeController *m_theme = nullptr;
    // Acciones compartidas cuyo estado sincroniza el documento activo con el
    // cursor; se entregan a cada pestaña en configureStack().
    FormatController::Actions m_formatActions;
    QList<QAction *> m_tableActions;
    // Conexión «editar → reconstruir esquema» del documento activo; se rehace al
    // cambiar de pestaña (solo el activo alimenta el esquema compartido).
    QMetaObject::Connection m_outlineEditConn;
    HelpDialog *m_helpDialog = nullptr;  // se crea perezoso al pulsar F1
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
    QAction *m_promoteHeadingAction = nullptr;  // promover/degradar el encabezado del cursor
    QAction *m_demoteHeadingAction = nullptr;
    QHash<mdtheme::ThemeId, QAction *> m_themeActions;  // marca la acción del tema activo

    QAction *m_distractionAction = nullptr;   // toggle de Ver → Sin distracciones
    QAction *m_typewriterAction = nullptr;    // toggle de Ver → Máquina de escribir
    QMenu *m_snippetsMenu = nullptr;          // submenú dinámico Insertar → Snippet
    DistractionFreeController *m_distraction = nullptr;  // modo pantalla completa/columna

    OutlinePanel *m_outline = nullptr;          // panel lateral con el índice (TOC)
    QAction *m_outlineAction = nullptr;         // toggle de Ver → Esquema (F9)
    QTimer *m_outlineTimer = nullptr;           // debounce para reconstruir el índice
    qreal m_baseOutlinePointSize = 0;           // tamaño base del panel, para el zoom

    FindReplaceBar *m_findBar = nullptr;        // barra de buscar/reemplazar (abajo)
    RecentFilesManager *m_recentFiles = nullptr;  // gestor de "Abrir recientes"
    QLabel *m_countLabel = nullptr;             // contador en la barra de estado
    QLabel *m_lineColLabel = nullptr;           // indicador Ln/Col en la barra de estado
};

#endif // MAINWINDOW_H
