#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QByteArray>
#include <QHash>
#include <QList>
#include <QPalette>
#include <QString>
#include <QTextListFormat>

#include <functional>

#include "themespec.h"

class QTextEdit;
class QTextCharFormat;
class QAction;
class QCloseEvent;
class QEvent;
class QMimeData;
class QObject;
class QLabel;
class QSplitter;
class QToolBar;
class QShortcut;
class QTimer;
class QFileSystemWatcher;
class FocusEditor;
class HelpDialog;
class OutlinePanel;
class RecoveryManager;
class CodeBlockHighlighter;
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

public:
    explicit MainWindow(QWidget *parent = nullptr);

    // Carga y renderiza el archivo indicado. Público para abrir un archivo
    // pasado por línea de comandos desde main().
    void openFile(const QString &path);

    // Decide qué mostrar al arrancar (se invoca diferido desde main()): un
    // archivo de la línea de comandos tiene prioridad; si no, se ofrece recuperar
    // un borrador de un cierre anómalo; en su defecto, se reabre el último
    // documento de la sesión anterior. `cmdLineFile` vacío = sin argumento.
    void startSession(const QString &cmdLineFile);

protected:
    void closeEvent(QCloseEvent *event) override;
    // Recoloca el esquema y la columna en el modo sin distracciones al cambiar
    // de tamaño (en pantalla completa, al entrar/salir y al redimensionar).
    void resizeEvent(QResizeEvent *event) override;
    // Captura Ctrl+rueda sobre el editor para hacer zoom de la fuente.
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void newFile();
    void openFileDialog();
    bool save();
    bool saveAs();

    void toggleBlockquote();
    void toggleCodeBlock();
    void toggleTaskItem();
    void setCodeLanguage();

    void insertLink();
    void insertImage();
    void insertTable();
    void insertHorizontalRule();
    void insertFormula();
    // Lanza el diálogo de fórmula con `initialTex` y `initialBlock` precargados.
    // Si el usuario acepta, escribe el resultado en *tex/*block y devuelve true.
    bool askFormula(QString *tex, bool *block,
                    const QString &initialTex = QString(), bool initialBlock = false);
    // Si en `viewportPos` (coordenadas del viewport de m_editor) hay un fragmento
    // de fórmula, abre el diálogo precargado y sustituye la fórmula. Devuelve
    // true si actuó. Lo dispara el doble clic.
    bool editFormulaAt(const QPoint &viewportPos);
    // Si la selección del editor toca o atraviesa fórmulas, la extiende para
    // incluir los grupos enteros — así un pegado posterior reemplaza la
    // fórmula completa en vez de partirla por la mitad.
    void guardPasteAgainstMath();
    // Intercepta pulsaciones de tecla cerca o dentro de una fórmula renderizada:
    // bloquea el tecleo de caracteres imprimibles (las fórmulas se editan con
    // doble clic) y convierte Backspace/Delete en el borde en borrado atómico
    // del grupo entero. Devuelve true si consumió el evento.
    bool handleMathKeyPress(QKeyEvent *event);
    // Inserta la imagen del portapapeles (acción del menú Insertar).
    void pasteImageFromClipboard();
    // Enter en el editor de código: continúa la lista/tarea actual o, en un ítem
    // vacío, sale de ella. Devuelve true si gestionó la pulsación.
    bool continueSourceList();
    // Si los datos traen una imagen, la guarda a disco e inserta `![](ruta)`.
    // Devuelve true si la gestionó (para que el editor no la incruste).
    bool handlePastedImage(const QMimeData *source);
    void indentList();
    void outdentList();

    // --- Edición de tablas (solo con el cursor dentro de una tabla) ---
    void tableInsertRow(bool below);          // inserta una fila encima/debajo
    void tableInsertColumn(bool right);       // inserta una columna a izda./dcha.
    void tableDeleteRow();                    // elimina la fila actual
    void tableDeleteColumn();                 // elimina la columna actual
    void tableAlignColumn(Qt::Alignment alignment);  // alinea la columna actual
    // Habilita/inhabilita las acciones de tabla según si el cursor está en una.
    void updateTableActions();

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

    // Alterna entre la vista WYSIWYG y la del código Markdown crudo.
    void toggleSourceMode(bool on);

    // Activa/desactiva la vista dividida: WYSIWYG y fuente lado a lado, ambos
    // editables. Es excluyente con el modo fuente a pantalla completa.
    void toggleSplitView(bool on);

    // Sincronización de la vista dividida (solo se actualiza el panel SIN foco,
    // para no estorbar a lo que el usuario está escribiendo):
    //   refresca el panel de fuente con el Markdown del documento WYSIWYG.
    void syncSourceFromDocument();
    //   vuelca el texto del panel de fuente al documento WYSIWYG (re-renderiza).
    void syncDocumentFromSource();
    // Vacía de inmediato el temporizador pendiente al cambiar de panel, para que
    // el otro panel quede al día antes de empezar a editarlo.
    void flushPendingSync(QWidget *losingFocus);

    // Entra/sale del modo sin distracciones: pantalla completa, oculta menú,
    // barras y estado, y centra el texto en una columna. Se sale con ESC o F11.
    void toggleDistractionFree(bool on);
    // En el modo sin distracciones, centra en pantalla el bloque [esquema|columna]:
    // ensancha el dock con un relleno izquierdo y pega la columna a su borde. Fuera
    // del modo (o sin esquema visible) deshace ambos ajustes. Idempotente.
    void updateDistractionLayout();

    bool exportPdf();
    bool exportHtml();
    // Imprime el documento renderizado mostrando el diálogo de impresión del
    // sistema (distinto de exportar a PDF, que escribe a un archivo).
    bool print();
    bool exportOdf();
    bool exportLatex();
    // Pide al usuario el idioma del documento para exportar (por defecto, el del
    // front matter `lang`/`language`, o el de la app). false si cancela.
    bool chooseExportLanguage(mdexport::Language *out);
    // Título para la exportación: el `title` del front matter, o "" si no hay.
    QString exportTitle() const;

    // Mantiene los botones de la barra reflejando el formato bajo el cursor.
    void updateFormatActions();
    // Actualiza el contador de palabras/caracteres de la barra de estado.
    void updateWordCount();
    // Escribe el borrador de recuperación si hay cambios sin guardar (o lo borra
    // si el documento está limpio). Lo dispara el temporizador de autoguardado.
    void autosaveDraft();

private:
    void createMenusAndActions();
    void createFormatToolBar();
    // (Re)genera los iconos de los botones de lista con el color del tema actual.
    void updateToolBarIcons();

    void changeListIndent(int delta);
    // Recupera el borrador en el editor: si su documento original existe, lo
    // abre y superpone el borrador encima (queda como modificado); si no, lo
    // recupera como documento sin título. Devuelve false si no había borrador.
    bool recoverDraft();
    // Cuerpo Markdown actual para el borrador (texto del editor activo).
    QString currentBody() const;
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

    // --- Vigilancia de cambios del archivo en disco ---
    // Pasa a vigilar `path` (y deja de vigilar el anterior); fija la instantánea.
    void watchCurrentFile(const QString &path);
    // Relee y guarda los bytes del archivo actual como instantánea de referencia.
    void updateDiskSnapshot();
    // QFileSystemWatcher::fileChanged: agenda la comprobación con un pequeño retardo.
    void onWatchedFileChanged(const QString &path);
    // Compara el disco con la instantánea y, si cambió de verdad, avisa/recarga.
    void checkDiskChange();
    // Recarga el archivo actual desde disco descartando el contenido en memoria.
    void reloadFromDisk();

    // Fija/quita el encabezado de nivel `level` en el bloque actual (toggle).
    void applyHeading(int level);
    // Fija/quita una lista del estilo dado en el bloque actual (toggle).
    void applyList(QTextListFormat::Style style);

    // Aplica un borde visible a todas las tablas del documento (las creadas y las
    // cargadas). El borde no se serializa a Markdown, así que no afecta al
    // round-trip ni al estado «modificado».
    void styleTables();

    // Aplica un toggle de formato de carácter: `mutate` recibe el formato a
    // rellenar y el formato actual bajo el cursor (Strategy de los botones
    // negrita/cursiva/tachado/código).
    void toggleCharFormat(
        const std::function<void(QTextCharFormat &, const QTextCharFormat &)> &mutate);
    void mergeCharFormatOnSelection(const QTextCharFormat &format);
    bool maybeSave();
    bool writeToFile(const QString &path);

    // Editor actualmente visible (WYSIWYG o fuente).
    QTextEdit *activeEditor() const;
    // Si estamos en modo fuente y hay cambios, los vuelca al documento WYSIWYG.
    void commitSourceToDocument();

    FocusEditor *m_editor = nullptr;
    HelpDialog *m_helpDialog = nullptr;  // se crea perezoso al pulsar F1
    CodeBlockHighlighter *m_highlighter = nullptr;
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

    QSplitter *m_splitView = nullptr;        // contiene WYSIWYG y fuente lado a lado
    FocusEditor *m_sourceEditor = nullptr;   // vista de código Markdown crudo
    QAction *m_sourceModeAction = nullptr;   // toggle de Ver → Código fuente
    QAction *m_splitAction = nullptr;        // toggle de Ver → Vista dividida
    bool m_sourceMode = false;               // fuente a pantalla completa
    bool m_splitMode = false;                // WYSIWYG + fuente lado a lado
    bool m_sourceDirty = false;              // el fuente tiene cambios sin volcar
    bool m_syncing = false;                  // actualización programática en curso (anti-bucle)
    QTimer *m_syncToSourceTimer = nullptr;   // debounce WYSIWYG -> fuente
    QTimer *m_syncToDocTimer = nullptr;      // debounce fuente -> WYSIWYG
    QList<QAction *> m_wysiwygActions;       // acciones válidas solo en WYSIWYG

    // Muestra/oculta cada editor según el modo de vista activo (WYSIWYG / fuente
    // / dividido).
    void updateEditorVisibility();
    // En vista dividida, ajusta a qué panel apuntan las acciones según el foco:
    // el formato/inserción solo aplica al WYSIWYG, así que se deshabilita cuando
    // el foco está en el fuente; la barra de búsqueda sigue al panel con foco.
    void updateActionsForFocus();

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
    QShortcut *m_escShortcut = nullptr;       // ESC para salir del modo (solo activo en él)
    bool m_distractionFree = false;
    bool m_wasMaximized = false;              // estado de ventana previo, para restaurar
    bool m_findBarWasVisible = false;         // visibilidad previa de la barra de búsqueda
    // Geometría y disposición de barras justo antes de entrar al modo, para
    // restaurarlas al salir y, sobre todo, para persistir esas (y no las del
    // modo a pantalla completa con barras ocultas) si se cierra dentro del modo.
    QByteArray m_preDistractionGeometry;
    QByteArray m_preDistractionState;
    int m_normalOutlineWidth = 0;             // ancho del dock fuera del modo, para restaurarlo

    RecoveryManager *m_recovery = nullptr;      // borrador de recuperación ante fallos
    QTimer *m_autosaveTimer = nullptr;          // dispara el autoguardado periódico
    bool m_autosaveDirty = false;               // hubo cambios desde el último autoguardado

    OutlinePanel *m_outline = nullptr;          // panel lateral con el índice (TOC)
    QAction *m_outlineAction = nullptr;         // toggle de Ver → Esquema (F9)
    QTimer *m_outlineTimer = nullptr;           // debounce para reconstruir el índice
    qreal m_baseOutlinePointSize = 0;           // tamaño base del panel, para el zoom

    FindReplaceBar *m_findBar = nullptr;        // barra de buscar/reemplazar (abajo)
    RecentFilesManager *m_recentFiles = nullptr;  // gestor de "Abrir recientes"
    DocumentIo *m_documentIo = nullptr;         // E/S del documento (abrir/guardar)
    ThemeController *m_theme = nullptr;         // tema claro/oscuro y enlaces
    QLabel *m_countLabel = nullptr;             // contador en la barra de estado

    QList<QAction *> m_tableActions;            // acciones del menú Tabla (contextuales)

    QFileSystemWatcher *m_fileWatcher = nullptr;  // vigila cambios externos del archivo
    QTimer *m_diskCheckTimer = nullptr;           // debounce para los avisos de cambio
    QByteArray m_diskSnapshot;                    // últimos bytes en disco (cargados/guardados)
    bool m_diskPromptOpen = false;                // hay un diálogo de "cambió en disco" abierto
};

#endif // MAINWINDOW_H
