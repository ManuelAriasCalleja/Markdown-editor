#include "mainwindow.h"

#include "appsettings.h"
#include "blockconstructs.h"
#include "chromezoom.h"
#include "footnotes.h"
#include "tasklist.h"
#include "codehighlighter.h"
#include "diskwatcher.h"
#include "distractionfreecontroller.h"
#include "documentio.h"
#include "exportcontroller.h"
#include "exporters.h"
#include "filecontroller.h"
#include "formatcontroller.h"
#include "formulacontroller.h"
#include "insertcontroller.h"
#include "tablecontroller.h"
#include "findreplacebar.h"
#include "focuseditor.h"
#include "helpdialog.h"
#include "docstats.h"
#include "texttransform.h"
#include "listcontinuation.h"
#include "mathblocks.h"
#include "gotoheadingdialog.h"
#include "outlinepanel.h"
#include "shortcodes.h"
#include "recentfilesmanager.h"
#include "recoverymanager.h"
#include "splitviewcontroller.h"
#include "tableedit.h"
#include "themecontroller.h"

#include <cmath>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QDesktopServices>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QFormLayout>
#include <QKeyEvent>
#include <QKeySequence>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QResizeEvent>
#include <QToolButton>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QStatusBar>
#include <QStringList>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextFragment>
#include <QTextFrame>
#include <QTextLength>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QWheelEvent>

namespace {

// Devuelve un color de «tinta» (casi negro o casi blanco) que contraste con el
// fondo dado. La decisión se toma sobre la luminancia relativa WCAG: si el
// fondo es luminoso se devuelve tinta oscura, y al revés. Se usan tonos
// ligeramente atenuados respecto a #000/#fff para que el antialiasing no se
// vea agresivo a tamaños pequeños.
QColor contrastingInk(const QColor &background)
{
    auto channel = [](int c) {
        const double v = c / 255.0;
        return v <= 0.04045 ? v / 12.92 : std::pow((v + 0.055) / 1.055, 2.4);
    };
    const double L = 0.2126 * channel(background.red())
                   + 0.7152 * channel(background.green())
                   + 0.0722 * channel(background.blue());
    return L > 0.5 ? QColor(0x1a, 0x1a, 0x1a) : QColor(0xf0, 0xf0, 0xf0);
}

enum class ListIconKind { Bullet, Numbered, Task };

// Dibuja un icono monocromo para los botones de lista, del color dado (el del
// texto de los botones, para que siga al tema claro/oscuro). Tres filas con una
// «línea de texto» a la derecha y, a la izquierda, el marcador propio de cada
// tipo: viñetas, números o casillas de verificación.
QIcon makeListIcon(ListIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);

    const qreal N = px;                       // se pinta en coordenadas lógicas
    const qreal markerRight = N * 0.42;       // fin de la zona del marcador
    const qreal lineRight   = N * 0.88;       // fin de la línea de texto
    const qreal rows[3] = {N * 0.26, N * 0.5, N * 0.74};
    // Trazo grueso: a tamaños de icono pequeños un trazo más fino se ve
    // «aguado» y pierde contraste contra el fondo, aunque la tinta sea
    // máxima. Se pisa un poco más que el grosor «de proporción» natural.
    const qreal stroke = qMax(qreal(1.5), N * 0.10);

    QPen linePen(color, stroke);
    linePen.setCapStyle(Qt::RoundCap);
    p.setPen(linePen);
    for (const qreal y : rows)
        p.drawLine(QPointF(markerRight, y), QPointF(lineRight, y));

    switch (kind) {
    case ListIconKind::Bullet: {
        p.setPen(Qt::NoPen);
        p.setBrush(color);
        const qreal r = N * 0.10;
        for (const qreal y : rows)
            p.drawEllipse(QPointF(N * 0.16, y), r, r);
        break;
    }
    case ListIconKind::Numbered: {
        QFont f = p.font();
        f.setPixelSize(int(N * 0.34));
        f.setBold(true);
        p.setFont(f);
        p.setPen(color);
        const char *nums[3] = {"1", "2", "3"};
        for (int i = 0; i < 3; ++i)
            p.drawText(QRectF(0, rows[i] - N * 0.22, markerRight - N * 0.10, N * 0.44),
                       Qt::AlignRight | Qt::AlignVCenter, QString::fromLatin1(nums[i]));
        break;
    }
    case ListIconKind::Task: {
        const qreal s = N * 0.22;
        QPen boxPen(color, qMax(qreal(1.2), N * 0.09));
        boxPen.setJoinStyle(Qt::MiterJoin);
        for (int i = 0; i < 3; ++i) {
            const qreal y = rows[i];
            const QRectF box(N * 0.08, y - s / 2, s, s);
            p.setPen(boxPen);
            p.setBrush(Qt::NoBrush);
            p.drawRect(box);
            if (i == 0) {  // la primera casilla, marcada
                QPen chk(color, qMax(qreal(1.2), N * 0.10));
                chk.setCapStyle(Qt::RoundCap);
                chk.setJoinStyle(Qt::RoundJoin);
                p.setPen(chk);
                QPolygonF check;
                check << QPointF(box.left() + s * 0.18, y + s * 0.02)
                      << QPointF(box.left() + s * 0.42, y + s * 0.28)
                      << QPointF(box.left() + s * 0.84, y - s * 0.30);
                p.drawPolyline(check);
            }
        }
        break;
    }
    }

    p.end();
    return QIcon(pm);
}

enum class FormatIconKind { Bold, Italic, Underline, Strike };

// Dibuja el icono de un botón de formato de carácter: la inicial española del
// efecto, pintada con ese mismo efecto, de modo que la letra se explica sola
// (N negrita, C cursiva, S subrayada, T tachada). Monocromo, del color del
// texto, para seguir al tema como los iconos de lista.
QIcon makeFormatIcon(FormatIconKind kind, const QColor &color, int px, qreal dpr)
{
    QPixmap pm(QSize(px, px) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);

    const qreal N = px;
    QChar glyph;
    QFont f = p.font();
    f.setPixelSize(int(N * 0.66));
    // Todos los glifos arrancan con peso DemiBold para que C/S/T no se vean
    // más finos —y por tanto más «aguados»— que la N. La negrita se sigue
    // distinguiendo porque sube a Black; el efecto característico de cada
    // botón (cursiva, subrayado, tachado) se mantiene aparte.
    f.setWeight(QFont::DemiBold);
    switch (kind) {
    case FormatIconKind::Bold:      glyph = u'N'; f.setWeight(QFont::Black); break;
    case FormatIconKind::Italic:    glyph = u'C'; f.setItalic(true);    break;
    case FormatIconKind::Underline: glyph = u'S'; f.setUnderline(true); break;
    case FormatIconKind::Strike:    glyph = u'T'; f.setStrikeOut(true); break;
    }
    p.setFont(f);
    p.setPen(color);
    p.drawText(QRectF(0, 0, N, N), Qt::AlignCenter, QString(glyph));

    p.end();
    return QIcon(pm);
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_editor = new FocusEditor(this);
    m_editor->setAcceptRichText(true);

    // Tamaño de fuente base del editor WYSIWYG, para "Tamaño normal".
    m_baseFontPointSize = m_editor->font().pointSizeF();
    // Resaltado de sintaxis de los bloques de código.
    m_highlighter = new CodeBlockHighlighter(m_editor->document());
    // Zoom con Ctrl+rueda del ratón y detección de enlaces bajo el cursor.
    m_editor->viewport()->installEventFilter(this);
    m_editor->viewport()->setMouseTracking(true);  // recibir hover sin botón pulsado
    // Filtro sobre el propio editor para interceptar teclas: las fórmulas
    // renderizadas no se pueden editar tecleando dentro (se editan con doble
    // clic) y Backspace/Delete en su borde borran el grupo entero.
    m_editor->installEventFilter(this);

    // Colaboradores: E/S del documento y control del tema. Se crean antes del
    // menú porque sus acciones los invocan.
    m_documentIo = new DocumentIo(m_editor, this);
    m_theme = new ThemeController(m_editor, m_highlighter, this);

    // Al pegar o soltar una imagen en el editor, guardarla a disco e insertar
    // `![](ruta)` en vez de incrustarla (que no sobreviviría al round-trip a
    // Markdown). Solo en el editor WYSIWYG; en la vista de fuente es texto.
    m_editor->setMimeInsertHandler([this](const QMimeData *src) {
        // Primero imágenes (a disco); si no, auto-enlazar una URL sobre la selección.
        return m_insert->handlePastedImage(src) || m_insert->handlePastedUrl(src);
    });

    // Barra inferior de buscar/reemplazar (creada antes del menú que la invoca).
    m_findBar = new FindReplaceBar(m_editor, this);
    addToolBar(Qt::BottomToolBarArea, m_findBar);
    connect(m_findBar, &FindReplaceBar::statusMessage,
            statusBar(), &QStatusBar::showMessage);

    // Panel de esquema (índice de encabezados). Se crea antes del menú porque su
    // acción de mostrar/ocultar (toggleViewAction) se añade a Ver. Un clic en una
    // entrada lleva el cursor a ese encabezado y devuelve el foco al editor.
    m_outline = new OutlinePanel(this);
    // El esquema vive siempre pegado a la izquierda: ni flota ni va a otra zona
    // (evita que un estado guardado lo deje como ventana suelta de ancho completo).
    // Conserva mostrar/ocultar y el reordenado, pero no el desacople.
    m_outline->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_outline->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, m_outline);
    // Mostrar/ocultar el esquema (F9) dentro del modo sin distracciones recoloca
    // el bloque centrado.
    connect(m_outline, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        // m_distraction puede no existir aún si esto se emite durante la
        // construcción (al acoplar el dock), antes de crear el controlador.
        if (m_distraction && m_distraction->isActive())
            m_distraction->updateLayout();
        // Al mostrar el esquema (F9), QMainWindow repone el ancho que tuviera
        // en el estado guardado, que puede ser desproporcionado. Lo normalizamos
        // diferido: el ancho restaurado se aplica tras asentarse el layout.
        else if (visible)
            QTimer::singleShot(0, this, &MainWindow::normalizeOutlineWidth);
    });
    connect(m_outline, &OutlinePanel::headingActivated, this, [this](int blockNumber) {
        const QTextBlock block = m_editor->document()->findBlockByNumber(blockNumber);
        if (!block.isValid())
            return;
        QTextCursor cursor(block);
        m_editor->setTextCursor(cursor);
        m_editor->ensureCursorVisible();
        m_editor->setFocus();
    });
    // Reordenar secciones arrastrando en el esquema: serializa el cuerpo, mueve la
    // sección con la función pura y re-renderiza. Diferido para no reconstruir el
    // árbol mientras se procesa el propio drop. En vista de fuente no aplica (el
    // documento WYSIWYG no es el que se edita).
    connect(m_outline, &OutlinePanel::sectionMoveRequested, this,
            [this](int from, int to, bool placeAfter) {
        if (m_split->sourceMode())
            return;  // editando la fuente a pantalla completa: no aplica
        QTimer::singleShot(0, this, [this, from, to, placeAfter] {
            const QString body = mdtable::documentMarkdown(m_editor->document());
            const QString moved = mdoutline::moveSection(body, from, to, placeAfter);
            if (moved == body)
                return;  // movimiento no válido: nada que hacer
            setBodyMarkdown(moved);
            setWindowModified(true);
        });
    });
    // Vista de código fuente / dividida y su sincronización. Posee el editor de
    // fuente y el QSplitter central; se crea tras el editor WYSIWYG, la barra de
    // búsqueda, el índice y el tema, de los que depende.
    m_split = new SplitViewController(m_editor, m_findBar, m_outline, m_theme, this);
    setCentralWidget(m_split->splitView());
    // Tamaño de fuente base del editor de fuente, para "Tamaño normal".
    m_baseSourceFontPointSize = m_split->sourceEditor()->font().pointSizeF();
    // Enter en el editor de código continúa listas/tareas (en WYSIWYG ya lo hace
    // QTextEdit de serie). Se filtran las pulsaciones del propio widget.
    m_split->sourceEditor()->installEventFilter(this);
    // El panel de fuente actualiza el contador de palabras igual que el WYSIWYG.
    connect(m_split->sourceEditor(), &QTextEdit::cursorPositionChanged,
            this, &MainWindow::updateWordCount);
    connect(m_split->sourceEditor(), &QTextEdit::selectionChanged,
            this, &MainWindow::updateWordCount);
    // Refrescos de la ventana que el controlador pide por señal.
    connect(m_split, &SplitViewController::wordCountShouldUpdate,
            this, &MainWindow::updateWordCount);
    connect(m_split, &SplitViewController::formatActionsShouldUpdate,
            this, [this] { m_format->updateActions(); });
    connect(m_split, &SplitViewController::documentModified,
            this, [this] { setWindowModified(true); });
    // Volcado del fuente: re-render del cuerpo Markdown dejando el modelo al día.
    m_split->setRenderBody([this](const QString &body) { setBodyMarkdown(body); });

    // Comandos de formato (negrita, encabezados, listas, citas…) y sincronización
    // de sus acciones con el formato bajo el cursor.
    m_format = new FormatController(m_editor, m_highlighter, this);
    connect(m_format, &FormatController::statusMessage,
            statusBar(), &QStatusBar::showMessage);

    // Edición de tablas (contextual: solo con el cursor dentro de una tabla).
    m_table = new TableController(m_editor, m_split, m_documentIo, this);
    connect(m_split, &SplitViewController::tableActionsShouldUpdate,
            m_table, &TableController::updateActions);
    connect(m_table, &TableController::modifiedChanged,
            this, &QWidget::setWindowModified);
    // Tras refrescar el formato, refresca también las acciones de tabla.
    connect(m_format, &FormatController::actionsUpdated,
            m_table, &TableController::updateActions);

    // Inserción y edición de fórmulas TeX + protección del teclado/pegado.
    m_formula = new FormulaController(m_editor, this);
    connect(m_formula, &FormulaController::statusMessage,
            statusBar(), &QStatusBar::showMessage);

    // Comandos de inserción (enlaces, imágenes, tablas, regla horizontal).
    m_insert = new InsertController(m_editor, m_documentIo, this);
    connect(m_insert, &InsertController::formatActionsShouldRefresh,
            m_format, &FormatController::updateActions);
    connect(m_insert, &InsertController::tableInserted, this, &MainWindow::styleTables);

    // Exportación e impresión (PDF/HTML/ODF/LaTeX). Sus mensajes van a la barra
    // de estado.
    m_export = new ExportController(m_editor, m_documentIo, m_split, this);
    connect(m_export, &ExportController::statusMessage,
            statusBar(), &QStatusBar::showMessage);

    // El índice se reconstruye al editar, con un pequeño retardo para no rehacer
    // el árbol en cada pulsación. En modo fuente no se toca (el contenido vive
    // como texto plano, no en la estructura del documento). La sincronización
    // WYSIWYG→fuente de la vista dividida la lleva el propio m_split.
    m_outlineTimer = new QTimer(this);
    m_outlineTimer->setSingleShot(true);
    m_outlineTimer->setInterval(300);
    connect(m_outlineTimer, &QTimer::timeout, this, [this] {
        if (!m_split->sourceMode())
            m_outline->rebuild(m_editor->document());
    });
    connect(m_editor->document(), &QTextDocument::contentsChanged, this, [this] {
        if (!m_split->sourceMode())
            m_outlineTimer->start();
    });

    // Operaciones de archivo (nuevo/abrir/guardar/recuperar) y autoguardado del
    // borrador. DocumentIo y RecoveryManager siguen siendo de MainWindow (con su
    // cableado de señales) y se le inyectan.
    m_recovery = new RecoveryManager(this);
    m_file = new FileController(m_editor, m_documentIo, m_split, m_recovery, this);
    m_file->setRenderBody([this](const QString &body) { setBodyMarkdown(body); });
    connect(m_file, &FileController::statusMessage,
            statusBar(), &QStatusBar::showMessage);
    connect(m_file, &FileController::windowModifiedChanged,
            this, &QWidget::setWindowModified);
    connect(m_file, &FileController::loadFailed, this, [this](const QString &path) {
        if (m_recentFiles)
            m_recentFiles->removeFile(path);  // ya no accesible: quitar de recientes
    });
    // Escribir en cualquiera de los dos editores marca el borrador como pendiente.
    connect(m_editor, &QTextEdit::textChanged, m_file, &FileController::markDirty);
    connect(m_split->sourceEditor(), &QTextEdit::textChanged, m_file, &FileController::markDirty);

    // El nivel de zoom hay que conocerlo ANTES de crear los menús: Qt 6.8 con
    // plataforma gtk3 cachea las anchuras de las QAction en la primera medición
    // y un setFont posterior no las invalida (resultado visible: items elided
    // como «Acerca de» → «Ace... de» cuando el zoom no es cero). Aplicar el
    // tamaño objetivo en la fuente por defecto del QApplication para las
    // clases QMenuBar/QMenu antes de instanciarlas hace que los menús nazcan
    // ya midiéndose con la fuente correcta.
    m_zoomDelta = AppSettings::zoomLevel();
    // applyMenuFontScale() muta la fuente de clase QMenuBar/QMenu de QApplication
    // (global y persistente entre ventanas). Al recrear la ventana (cambio de
    // idioma) y releer esa fuente ya escalada, el zoom de los menús se compondría
    // en cada cambio. Por eso capturamos el tamaño base pristino una sola vez por
    // proceso: la primera ventana lo lee antes de cualquier escalado y las
    // recreadas reutilizan ese valor. (El resto de superficies escalan su fuente
    // de widget, no la de clase de la app, así que no acumulan.)
    static const qreal s_baseMenuPointSize = QApplication::font("QMenuBar").pointSizeF();
    m_baseMenuPointSize = s_baseMenuPointSize;
    applyMenuFontScale();  // antes de createMenusAndActions

    createMenusAndActions();
    createFormatToolBar();

    // Tamaños base del resto de la interfaz (barras y estado), para
    // escalarlos con el zoom y poder restaurarlos con "Tamaño normal".
    m_baseToolBarPointSize = m_formatToolBar->font().pointSizeF();
    m_baseFindBarPointSize = m_findBar->font().pointSizeF();
    m_baseStatusBarPointSize = statusBar()->font().pointSizeF();
    m_baseOutlinePointSize = m_outline->font().pointSizeF();

    applyZoom();

    // En el modo sin distracciones se ocultan menú y barras; para que sus
    // atajos sigan funcionando, se registran también a nivel de ventana (que
    // siempre está visible). Añadir una acción ya existente es inocuo.
    const auto allActions = findChildren<QAction *>();
    for (QAction *a : allActions)
        if (!a->shortcut().isEmpty())
            addAction(a);

    // Modo sin distracciones (pantalla completa, columna centrada). Se crea tras
    // la barra de formato, que oculta/muestra. La acción del menú (F11) lo conmuta
    // y se mantiene marcada según el estado (p. ej. al salir con ESC).
    m_distraction = new DistractionFreeController(
        this, m_editor, m_split, m_outline, m_formatToolBar, m_findBar, this);
    connect(m_distractionAction, &QAction::toggled,
            m_distraction, &DistractionFreeController::setActive);
    connect(m_distraction, &DistractionFreeController::activeChanged,
            m_distractionAction, &QAction::setChecked);

    // Contador de palabras/caracteres, anclado a la derecha de la barra de estado.
    // Su visibilidad se recuerda entre sesiones (acción del menú Ver).
    m_countLabel = new QLabel(this);
    m_countLabel->setVisible(AppSettings::showWordCount());
    statusBar()->addPermanentWidget(m_countLabel);

    // Los botones de formato reflejan en todo momento lo que hay bajo el cursor.
    connect(m_editor, &QTextEdit::currentCharFormatChanged,
            this, [this] { m_format->updateActions(); });
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, [this] { m_format->updateActions(); });
    // Marca el título con '*' cuando el contenido difiere del guardado. Se
    // recalcula en cada cambio de contenido (DocumentIo compara con su línea
    // base, así que el artefacto de trazado de QTextEdit no lo ensucia).
    connect(m_editor->document(), &QTextDocument::contentsChanged, this,
            [this] { setWindowModified(m_documentIo->isModified()); });
    // Mantiene el contador al día con el contenido y la selección.
    connect(m_editor, &QTextEdit::textChanged, this, &MainWindow::updateWordCount);
    connect(m_editor, &QTextEdit::selectionChanged, this, &MainWindow::updateWordCount);

    // El título y los recientes siguen al archivo actual; tras cargar, se
    // recolorean los enlaces según el tema.
    connect(m_documentIo, &DocumentIo::currentFileChanged,
            this, &MainWindow::onCurrentFileChanged);

    // Vigilancia de cambios externos del archivo abierto (editar con git u otra
    // herramienta). El watcher solo señala; la reacción (recargar o preguntar) la
    // decide MainWindow porque toca los editores y puede abrir un diálogo.
    m_diskWatcher = new DiskWatcher(this);
    connect(m_documentIo, &DocumentIo::currentFileChanged,
            m_diskWatcher, &DiskWatcher::watch);
    connect(m_diskWatcher, &DiskWatcher::externalChange,
            this, &MainWindow::onDiskExternalChange);
    connect(m_diskWatcher, &DiskWatcher::vanished, this, [this] {
        statusBar()->showMessage(tr("El archivo se eliminó o movió en disco."), 6000);
    });
    connect(m_documentIo, &DocumentIo::documentLoaded,
            m_theme, &ThemeController::recolorLinks);
    // Al cargar un documento, el índice se reconstruye de inmediato (sin esperar
    // al debounce de edición) para que esté listo nada más abrir.
    connect(m_documentIo, &DocumentIo::documentLoaded, this,
            [this] { m_outline->rebuild(m_editor->document()); });
    // Las tablas cargadas no traen borde; se lo damos para que sean visibles.
    connect(m_documentIo, &DocumentIo::documentLoaded, this, &MainWindow::styleTables);

    m_documentIo->reset();  // documento nuevo (fija el título inicial)

    // Restaura tamaño/posición y disposición de barras de la sesión anterior.
    const QByteArray geometry = AppSettings::windowGeometry();
    if (geometry.isEmpty())
        resize(900, 700);
    else
        restoreGeometry(geometry);
    restoreState(AppSettings::windowState());
    // restoreState puede reponer un estado antiguo con el esquema flotante o en
    // otra zona; lo re-anclamos a la izquierda sin tocar su visibilidad.
    if (m_outline->isFloating() || dockWidgetArea(m_outline) != Qt::LeftDockWidgetArea) {
        m_outline->setFloating(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_outline);
    }
    // El ancho del dock del esquema se normaliza en startSession(), ya con la
    // ventana mostrada y el layout asentado (aquí, en el constructor, el ancho
    // restaurado aún no se ha aplicado y width() no es fiable).
    // Arranca siempre como ventana normal: descarta pantalla completa/maximizado
    // que pudieran venir de una sesión cerrada en modo sin distracciones.
    setWindowState(windowState() & ~(Qt::WindowFullScreen | Qt::WindowMaximized));
    m_findBar->hide();  // la barra de búsqueda siempre arranca oculta

    statusBar()->showMessage(
        tr("Editor Markdown WYSIWYG — escribe y da formato con la barra superior"));
    m_format->updateActions();
    updateWordCount();

    // Restaura el tema de la sesión anterior (la señal themeChanged marca la
    // acción del menú). Si se sigue el SO, se deriva de su esquema actual en vez
    // de la clave guardada.
    m_theme->applyTheme(
        m_theme->followsSystem()
            ? m_theme->systemTheme()
            : mdtheme::idFromKey(AppSettings::themeKey(), mdtheme::ThemeId::Light));
}

void MainWindow::createMenusAndActions()
{
    createFileMenu();
    createEditMenu();
    createFormatActions();
    createInsertMenu();
    createTableMenu();
    createViewMenu();
    createHelpMenu();

    // Entrega al controlador de la vista dividida las acciones recién creadas: las
    // de modo (que conmuta y mantiene marcadas) y las válidas solo en WYSIWYG (que
    // deshabilita en modo fuente).
    m_split->setModeActions(m_sourceModeAction, m_splitAction);
    m_split->setWysiwygActions(m_wysiwygActions);
}

void MainWindow::createFileMenu()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Archivo"));

    QAction *newAction = fileMenu->addAction(tr("&Nuevo"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, m_file, &FileController::newFile);

    QAction *openAction = fileMenu->addAction(tr("&Abrir..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, m_file, &FileController::openFileDialog);

    QMenu *recentMenu = fileMenu->addMenu(tr("Abrir &recientes"));
    m_recentFiles = new RecentFilesManager(recentMenu, this);
    connect(m_recentFiles, &RecentFilesManager::fileOpenRequested,
            m_file, &FileController::openFile);

    QAction *saveAction = fileMenu->addAction(tr("&Guardar"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, m_file, &FileController::save);

    QAction *saveAsAction = fileMenu->addAction(tr("Guardar &como..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, m_file, &FileController::saveAs);

    QAction *openFolderAction = fileMenu->addAction(tr("Abrir &carpeta contenedora"));
    connect(openFolderAction, &QAction::triggered, m_file, &FileController::openContainingFolder);

    fileMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu(tr("&Exportar"));
    QAction *exportPdfAction = exportMenu->addAction(tr("A PDF..."));
    connect(exportPdfAction, &QAction::triggered, m_export, &ExportController::exportPdf);
    QAction *exportHtmlAction = exportMenu->addAction(tr("A HTML..."));
    connect(exportHtmlAction, &QAction::triggered, m_export, &ExportController::exportHtml);
    QAction *exportOdfAction = exportMenu->addAction(tr("A ODF (ODT)..."));
    connect(exportOdfAction, &QAction::triggered, m_export, &ExportController::exportOdf);
    QAction *exportDocxAction = exportMenu->addAction(tr("A DOCX (Word)..."));
    connect(exportDocxAction, &QAction::triggered, m_export, &ExportController::exportDocx);
    QAction *exportLatexAction = exportMenu->addAction(tr("A LaTeX..."));
    connect(exportLatexAction, &QAction::triggered, m_export, &ExportController::exportLatex);
    exportMenu->addSeparator();
    QAction *exportSelPdfAction = exportMenu->addAction(tr("Selección a PDF..."));
    exportSelPdfAction->setToolTip(tr("Exporta a PDF solo el texto seleccionado"));
    connect(exportSelPdfAction, &QAction::triggered, m_export, &ExportController::exportSelectionPdf);

    fileMenu->addSeparator();

    QAction *printPreviewAction = fileMenu->addAction(tr("&Vista previa de impresión..."));
    connect(printPreviewAction, &QAction::triggered, m_export, &ExportController::printPreview);

    QAction *printAction = fileMenu->addAction(tr("&Imprimir..."));
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, m_export, &ExportController::print);

    QAction *printSelAction = fileMenu->addAction(tr("Imprimir &selección..."));
    printSelAction->setToolTip(tr("Imprime solo el texto seleccionado"));
    connect(printSelAction, &QAction::triggered, m_export, &ExportController::printSelection);

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction(tr("&Salir"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::createEditMenu()
{
    QMenu *editMenu = menuBar()->addMenu(tr("&Editar"));

    QAction *undoAction = editMenu->addAction(tr("Deshacer"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this] { activeEditor()->undo(); });

    QAction *redoAction = editMenu->addAction(tr("Rehacer"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this] { activeEditor()->redo(); });

    editMenu->addSeparator();

    QAction *pastePlainAction = editMenu->addAction(tr("Pegar como texto plano"));
    pastePlainAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V));
    pastePlainAction->setToolTip(
        pastePlainAction->text() + QStringLiteral(" (%1)").arg(
            pastePlainAction->shortcut().toString(QKeySequence::NativeText)));
    connect(pastePlainAction, &QAction::triggered, this, [this] {
        const QString text = QApplication::clipboard()->text();
        if (!text.isEmpty())
            activeEditor()->insertPlainText(text);  // ignora formato/HTML del portapapeles
    });

    QAction *copyHtmlAction = editMenu->addAction(tr("Copiar como HTML"));
    connect(copyHtmlAction, &QAction::triggered, m_export, &ExportController::copyHtmlToClipboard);

    editMenu->addSeparator();

    // Transformaciones de texto sobre la selección (o la palabra bajo el cursor,
    // para las de mayúsculas/minúsculas). La lógica vive en mdtext (puro).
    QMenu *transformMenu = editMenu->addMenu(tr("Transformar texto"));
    const auto applyToSelection = [this](QString (*fn)(const QString &)) {
        QTextEdit *ed = activeEditor();
        QTextCursor c = ed->textCursor();
        if (!c.hasSelection())
            c.select(QTextCursor::WordUnderCursor);
        if (c.hasSelection())
            c.insertText(fn(c.selectedText()));  // U+2029 sobrevive: preserva párrafos
    };
    QAction *upperAction = transformMenu->addAction(tr("MAYÚSCULAS"));
    connect(upperAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::toUpper); });
    QAction *lowerAction = transformMenu->addAction(tr("minúsculas"));
    connect(lowerAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::toLower); });
    QAction *capAction = transformMenu->addAction(tr("Capitalizar"));
    connect(capAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::capitalize); });
    transformMenu->addSeparator();
    QAction *typographyAction = transformMenu->addAction(tr("Tipografía inteligente"));
    typographyAction->setToolTip(tr("Convierte -- — ... y comillas rectas en sus formas tipográficas"));
    connect(typographyAction, &QAction::triggered, this,
            [applyToSelection] { applyToSelection(mdtext::smartTypography); });

    QAction *sortAction = editMenu->addAction(tr("Ordenar líneas"));
    connect(sortAction, &QAction::triggered, this, [this] {
        QTextEdit *ed = activeEditor();
        QTextCursor c = ed->textCursor();
        if (!c.hasSelection())
            return;  // ordenar exige un rango explícito de líneas
        QString sel = c.selectedText();
        // selectedText() usa U+2029 entre párrafos; lo paso a '\n' para mdtext y lo
        // restauro antes de reinsertar (insertText vuelve a partir por U+2029).
        sel.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));
        QString out = mdtext::sortLines(sel, true);
        out.replace(QLatin1Char('\n'), QChar(QChar::ParagraphSeparator));
        c.insertText(out);
    });

    editMenu->addSeparator();

    QAction *findAction = editMenu->addAction(tr("Buscar..."));
    findAction->setShortcut(QKeySequence::Find);                       // Ctrl+F
    connect(findAction, &QAction::triggered, m_findBar, &FindReplaceBar::showFind);

    QAction *replaceAction = editMenu->addAction(tr("Reemplazar..."));
    replaceAction->setShortcut(QKeySequence::Replace);                 // Ctrl+H
    connect(replaceAction, &QAction::triggered, m_findBar, &FindReplaceBar::showReplace);
}

void MainWindow::createFormatActions()
{
    // --- Acciones de formato (compartidas por el menú y la barra) ---
    // Tabla de descriptores: todas son checkable y solo difieren en texto, atajo
    // y la acción a ejecutar. El tooltip se compone del texto más el atajo en su
    // forma nativa y localizada (p. ej. «Strg+B» en alemán), derivado del propio
    // atajo: así no hay que traducirlo a mano ni puede desincronizarse.
    using Mutator = std::function<void(QTextCharFormat &, const QTextCharFormat &)>;
    const auto charToggle = [this](Mutator m) {
        return [this, m = std::move(m)] { m_format->toggleCharFormat(m); };
    };

    struct FormatActionDef {
        QAction **slot;             // dónde guardar el QAction creado
        QString text;
        QKeySequence shortcut;      // vacío = sin atajo
        std::function<void()> handler;
        QString tooltip;            // globo descriptivo; vacío = usar `text`
    };

    const FormatActionDef defs[] = {
        {&m_boldAction, tr("Negrita"), QKeySequence::Bold,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontWeight(cur.fontWeight() >= QFont::Bold ? QFont::Normal : QFont::Bold);
         })},
        {&m_italicAction, tr("Cursiva"), QKeySequence::Italic,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontItalic(!cur.fontItalic());
         })},
        {&m_underlineAction, tr("Subrayado"), QKeySequence::Underline,
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             // toMarkdown() de Qt serializa el subrayado como `_texto_` (su
             // dialecto reserva `*…*` para la cursiva), así que hace round-trip.
             f.setFontUnderline(!cur.fontUnderline());
         })},
        {&m_strikeAction, tr("Tachado"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_X),
         charToggle([](QTextCharFormat &f, const QTextCharFormat &cur) {
             f.setFontStrikeOut(!cur.fontStrikeOut());
         })},
        {&m_codeAction, tr("Código"), QKeySequence(Qt::CTRL | Qt::Key_E),
         charToggle([this](QTextCharFormat &f, const QTextCharFormat &cur) {
             const bool enable = !cur.fontFixedPitch();
             f.setFontFixedPitch(enable);
             // El conversor a Markdown emite `código` con fuente de paso fijo.
             f.setFontFamilies({enable ? QStringLiteral("monospace")
                                       : m_editor->document()->defaultFont().family()});
         }),
         tr("Código en línea")},
        {&m_linkAction, tr("Enlace"), QKeySequence(Qt::CTRL | Qt::Key_K),
         [this] { m_insert->insertLink(); }, tr("Insertar o editar enlace")},
        {&m_quoteAction, tr("❝ Cita"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q),
         [this] { m_format->toggleBlockquote(); }, tr("Convertir en cita")},
        {&m_codeBlockAction, tr("Bloque"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K),
         [this] { m_format->toggleCodeBlock(); }, tr("Bloque de código")},
        {&m_h1Action, tr("H1"), QKeySequence(Qt::CTRL | Qt::Key_1),
         [this] { m_format->applyHeading(1); }},
        {&m_h2Action, tr("H2"), QKeySequence(Qt::CTRL | Qt::Key_2),
         [this] { m_format->applyHeading(2); }},
        {&m_h3Action, tr("H3"), QKeySequence(Qt::CTRL | Qt::Key_3),
         [this] { m_format->applyHeading(3); }},
        {&m_h4Action, tr("H4"), QKeySequence(Qt::CTRL | Qt::Key_4),
         [this] { m_format->applyHeading(4); }},
        {&m_h5Action, tr("H5"), QKeySequence(Qt::CTRL | Qt::Key_5),
         [this] { m_format->applyHeading(5); }},
        {&m_h6Action, tr("H6"), QKeySequence(Qt::CTRL | Qt::Key_6),
         [this] { m_format->applyHeading(6); }},
        {&m_bulletAction, tr("Lista de viñetas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U),
         [this] { m_format->applyList(QTextListFormat::ListDisc); }},
        {&m_numberedAction, tr("Lista numerada"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O),
         [this] { m_format->applyList(QTextListFormat::ListDecimal); }},
        {&m_taskAction, tr("Lista de tareas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T),
         [this] { m_format->toggleTaskItem(); }},
    };

    for (const FormatActionDef &d : defs) {
        auto *action = new QAction(d.text, this);
        action->setCheckable(true);
        QString tip = d.tooltip.isEmpty() ? d.text : d.tooltip;
        if (!d.shortcut.isEmpty()) {
            action->setShortcut(d.shortcut);
            tip += QStringLiteral(" (%1)")
                       .arg(d.shortcut.toString(QKeySequence::NativeText));
        }
        action->setToolTip(tip);
        connect(action, &QAction::triggered, this, d.handler);
        *d.slot = action;
        m_wysiwygActions.append(action);  // sin sentido en la vista de fuente
    }

    // --- Menú Formato (las mismas acciones, accesibles por teclado) ---
    QMenu *formatMenu = menuBar()->addMenu(tr("&Formato"));
    formatMenu->addAction(m_boldAction);
    formatMenu->addAction(m_italicAction);
    formatMenu->addAction(m_underlineAction);
    formatMenu->addAction(m_strikeAction);
    formatMenu->addAction(m_codeAction);
    formatMenu->addAction(m_linkAction);
    formatMenu->addSeparator();
    formatMenu->addAction(m_h1Action);
    formatMenu->addAction(m_h2Action);
    formatMenu->addAction(m_h3Action);
    formatMenu->addAction(m_h4Action);
    formatMenu->addAction(m_h5Action);
    formatMenu->addAction(m_h6Action);
    formatMenu->addSeparator();
    formatMenu->addAction(m_bulletAction);
    formatMenu->addAction(m_numberedAction);
    formatMenu->addAction(m_taskAction);

    m_indentAction = formatMenu->addAction(tr("Aumentar sangría"));
    m_indentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketRight));
    connect(m_indentAction, &QAction::triggered, m_format, &FormatController::indentList);
    m_wysiwygActions.append(m_indentAction);

    m_outdentAction = formatMenu->addAction(tr("Disminuir sangría"));
    m_outdentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    connect(m_outdentAction, &QAction::triggered, m_format, &FormatController::outdentList);
    m_wysiwygActions.append(m_outdentAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_quoteAction);
    formatMenu->addAction(m_codeBlockAction);

    m_langAction = formatMenu->addAction(tr("Lenguaje del bloque..."));
    m_langAction->setToolTip(tr("Fija el lenguaje del bloque de código (resaltado)"));
    connect(m_langAction, &QAction::triggered, m_format, &FormatController::setCodeLanguage);
    m_wysiwygActions.append(m_langAction);

    // Entrega a FormatController las acciones que sincroniza con el formato bajo
    // el cursor (las comparten este menú y la barra de botones).
    m_format->setActions({
        m_boldAction, m_italicAction, m_underlineAction, m_strikeAction, m_codeAction,
        m_linkAction, m_quoteAction, m_codeBlockAction, m_langAction,
        m_h1Action, m_h2Action, m_h3Action, m_h4Action, m_h5Action, m_h6Action,
        m_bulletAction, m_numberedAction, m_taskAction, m_indentAction, m_outdentAction});
}

void MainWindow::createInsertMenu()
{
    QMenu *insertMenu = menuBar()->addMenu(tr("&Insertar"));

    QAction *insLink = insertMenu->addAction(tr("Enlace..."));
    connect(insLink, &QAction::triggered, m_insert, &InsertController::insertLink);

    QAction *insImage = insertMenu->addAction(tr("Imagen..."));
    connect(insImage, &QAction::triggered, m_insert, &InsertController::insertImage);

    QAction *insPasteImage = insertMenu->addAction(tr("Pegar imagen"));
    insPasteImage->setToolTip(tr("Guarda la imagen del portapapeles y la inserta"));
    connect(insPasteImage, &QAction::triggered, m_insert, &InsertController::pasteImageFromClipboard);

    QAction *insTable = insertMenu->addAction(tr("Tabla..."));
    connect(insTable, &QAction::triggered, m_insert, &InsertController::insertTable);

    QAction *insRule = insertMenu->addAction(tr("Regla horizontal"));
    connect(insRule, &QAction::triggered, m_insert, &InsertController::insertHorizontalRule);

    QAction *insToc = insertMenu->addAction(tr("Índice (TOC)"));
    insToc->setToolTip(tr("Inserta un índice con los encabezados del documento"));
    connect(insToc, &QAction::triggered, m_insert, &InsertController::insertTableOfContents);

    QAction *insFormula = insertMenu->addAction(tr("Fórmula..."));
    insFormula->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    insFormula->setToolTip(
        insFormula->text() + QStringLiteral(" (%1)").arg(
            insFormula->shortcut().toString(QKeySequence::NativeText)));
    connect(insFormula, &QAction::triggered, m_formula, &FormulaController::insertFormula);

    QAction *insFootnote = insertMenu->addAction(tr("Nota al pie"));
    insFootnote->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
    insFootnote->setToolTip(
        tr("Inserta una referencia [^n] y su definición al final del documento")
        + QStringLiteral(" (%1)").arg(
            insFootnote->shortcut().toString(QKeySequence::NativeText)));
    connect(insFootnote, &QAction::triggered, m_insert, &InsertController::insertFootnote);

    QAction *insSymbol = insertMenu->addAction(tr("Símbolos especiales..."));
    insSymbol->setToolTip(tr("Inserta símbolos no habituales, por categorías"));
    connect(insSymbol, &QAction::triggered, m_insert, &InsertController::insertSymbol);

    QAction *insDate = insertMenu->addAction(tr("Fecha"));
    insDate->setToolTip(tr("Inserta la fecha actual en formato local"));
    connect(insDate, &QAction::triggered, m_insert, &InsertController::insertDate);

    QAction *insDateTime = insertMenu->addAction(tr("Fecha y hora"));
    insDateTime->setToolTip(tr("Inserta la fecha y la hora actuales en formato local"));
    connect(insDateTime, &QAction::triggered, m_insert, &InsertController::insertDateTime);

    // Insertar tampoco aplica en la vista de fuente.
    m_wysiwygActions << insLink << insImage << insPasteImage << insTable << insRule
                     << insToc << insFormula << insFootnote << insSymbol
                     << insDate << insDateTime;
}

void MainWindow::createTableMenu()
{
    // --- Menú Tabla (operaciones sobre la tabla bajo el cursor) ---
    // Sus acciones se habilitan solo cuando el cursor está dentro de una tabla
    // (ver updateTableActions), de ahí que no vayan en m_wysiwygActions.
    QMenu *tableMenu = menuBar()->addMenu(tr("&Tabla"));

    QAction *aRowAbove = tableMenu->addAction(tr("Insertar fila encima"));
    connect(aRowAbove, &QAction::triggered, this, [this] { m_table->insertRow(false); });
    QAction *aRowBelow = tableMenu->addAction(tr("Insertar fila debajo"));
    connect(aRowBelow, &QAction::triggered, this, [this] { m_table->insertRow(true); });
    QAction *aColLeft = tableMenu->addAction(tr("Insertar columna a la izquierda"));
    connect(aColLeft, &QAction::triggered, this, [this] { m_table->insertColumn(false); });
    QAction *aColRight = tableMenu->addAction(tr("Insertar columna a la derecha"));
    connect(aColRight, &QAction::triggered, this, [this] { m_table->insertColumn(true); });

    tableMenu->addSeparator();
    QAction *aDelRow = tableMenu->addAction(tr("Eliminar fila"));
    connect(aDelRow, &QAction::triggered, m_table, &TableController::deleteRow);
    QAction *aDelCol = tableMenu->addAction(tr("Eliminar columna"));
    connect(aDelCol, &QAction::triggered, m_table, &TableController::deleteColumn);

    tableMenu->addSeparator();
    QMenu *alignMenu = tableMenu->addMenu(tr("Alinear columna"));
    QAction *aLeft = alignMenu->addAction(tr("Izquierda"));
    connect(aLeft, &QAction::triggered, this, [this] { m_table->alignColumn(Qt::AlignLeft); });
    QAction *aCenter = alignMenu->addAction(tr("Centrar"));
    connect(aCenter, &QAction::triggered, this, [this] { m_table->alignColumn(Qt::AlignHCenter); });
    QAction *aRight = alignMenu->addAction(tr("Derecha"));
    connect(aRight, &QAction::triggered, this, [this] { m_table->alignColumn(Qt::AlignRight); });

    m_table->setActions({aRowAbove, aRowBelow, aColLeft, aColRight,
                         aDelRow, aDelCol, alignMenu->menuAction(),
                         aLeft, aCenter, aRight});
    m_table->updateActions();  // estado inicial (sin tabla bajo el cursor)
}

void MainWindow::createViewMenu()
{
    QMenu *viewMenu = menuBar()->addMenu(tr("&Ver"));

    m_sourceModeAction = viewMenu->addAction(tr("Código fuente Markdown"));
    m_sourceModeAction->setCheckable(true);
    m_sourceModeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
    m_sourceModeAction->setToolTip(
        m_sourceModeAction->text() + QStringLiteral(" (%1)").arg(
            m_sourceModeAction->shortcut().toString(QKeySequence::NativeText)));
    connect(m_sourceModeAction, &QAction::toggled, m_split, &SplitViewController::toggleSourceMode);

    m_splitAction = viewMenu->addAction(tr("Vista dividida"));
    m_splitAction->setCheckable(true);
    // Ctrl+Shift+D (no Ctrl+\\: en teclados español/ISO la «\» exige AltGr y el
    // atajo resulta imposible de pulsar). «D» de «Dividida».
    m_splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
    m_splitAction->setToolTip(
        tr("Editar WYSIWYG y código fuente a la vez, lado a lado") +
        QStringLiteral(" (%1)").arg(
            m_splitAction->shortcut().toString(QKeySequence::NativeText)));
    connect(m_splitAction, &QAction::toggled, m_split, &SplitViewController::toggleSplitView);

    m_distractionAction = viewMenu->addAction(tr("Sin distracciones"));
    m_distractionAction->setCheckable(true);
    m_distractionAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_distractionAction->setToolTip(
        tr("Pantalla completa, sin barras, con el texto centrado (ESC o F11 para salir)"));
    // La conexión con el controlador se hace en el ctor (m_distraction se crea
    // después de los menús, tras la barra de formato que oculta/muestra).

    // Esquema (índice): toggleViewAction muestra/oculta el dock y mantiene su
    // marca sincronizada con la visibilidad del panel automáticamente.
    m_outlineAction = m_outline->toggleViewAction();
    m_outlineAction->setText(tr("Esquema"));
    m_outlineAction->setShortcut(QKeySequence(Qt::Key_F9));
    m_outlineAction->setToolTip(
        m_outlineAction->text() + QStringLiteral(" (%1)").arg(
            m_outlineAction->shortcut().toString(QKeySequence::NativeText)));
    viewMenu->addAction(m_outlineAction);

    QAction *goToHeadingAction = viewMenu->addAction(tr("Ir a encabezado..."));
    goToHeadingAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
    goToHeadingAction->setToolTip(
        tr("Salta a un encabezado del documento") + QStringLiteral(" (%1)").arg(
            goToHeadingAction->shortcut().toString(QKeySequence::NativeText)));
    connect(goToHeadingAction, &QAction::triggered, this, &MainWindow::goToHeading);

    viewMenu->addSeparator();

    QAction *zoomInAction = viewMenu->addAction(tr("Aumentar letra"));
    zoomInAction->setShortcuts({QKeySequence::ZoomIn,
                                QKeySequence(Qt::CTRL | Qt::Key_Equal)});  // Ctrl++ y Ctrl+=
    connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomInText);

    QAction *zoomOutAction = viewMenu->addAction(tr("Reducir letra"));
    zoomOutAction->setShortcut(QKeySequence::ZoomOut);                     // Ctrl+-
    connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOutText);

    QAction *zoomResetAction = viewMenu->addAction(tr("Tamaño normal"));
    zoomResetAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_0));      // Ctrl+0
    connect(zoomResetAction, &QAction::triggered, this, &MainWindow::resetZoom);

    viewMenu->addSeparator();

    QAction *statsAction = viewMenu->addAction(tr("Estadísticas del documento..."));
    connect(statsAction, &QAction::triggered, this, &MainWindow::showDocumentStatistics);

    QAction *wordCountAction = viewMenu->addAction(tr("Mostrar contador de palabras"));
    wordCountAction->setCheckable(true);
    // m_countLabel aún no existe aquí (los menús se crean antes que la barra de
    // estado); el estado inicial sale del ajuste, con el que arrancará el label.
    wordCountAction->setChecked(AppSettings::showWordCount());
    connect(wordCountAction, &QAction::toggled, this, [this](bool on) {
        m_countLabel->setVisible(on);
        AppSettings::setShowWordCount(on);
    });

    viewMenu->addSeparator();
    QMenu *themeMenu = viewMenu->addMenu(tr("Tema"));
    auto *themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    // Literales de los nombres de tema traducibles, marcados para que lupdate
    // los extraiga (el bucle los traduce en runtime con tr(), que no expone el
    // literal). Deben coincidir con los displayName de los temas con
    // nameTranslated == true en mdtheme::allThemes().
    [[maybe_unused]] static const char *const kThemeNameMarkers[] = {
        QT_TRANSLATE_NOOP("MainWindow", "Claro"),
        QT_TRANSLATE_NOOP("MainWindow", "Oscuro"),
        QT_TRANSLATE_NOOP("MainWindow", "Alto contraste"),
    };

    // Una entrada por tema del catálogo (mdtheme::allThemes()): añadir un tema
    // allí lo trae al menú sin tocar esto. Los nombres con nameTranslated pasan
    // por tr(); los de nombre propio (GitHub, Monokai) se muestran verbatim.
    for (const mdtheme::ThemeSpec &spec : mdtheme::allThemes()) {
        const QString name = spec.nameTranslated
            ? tr(spec.displayName.toUtf8().constData())
            : spec.displayName;
        QAction *action = themeMenu->addAction(name);
        action->setCheckable(true);
        themeGroup->addAction(action);
        m_themeActions.insert(spec.id, action);
        const mdtheme::ThemeId id = spec.id;
        connect(action, &QAction::triggered, this,
                [this, id] { m_theme->applyTheme(id); });
    }

    // El control del tema avisa para mantener marcada la acción del tema activo.
    connect(m_theme, &ThemeController::themeChanged, this,
            [this](mdtheme::ThemeId id) {
        if (QAction *action = m_themeActions.value(id))
            action->setChecked(true);
        updateToolBarIcons();  // los iconos generados siguen el color del tema
    });

    themeMenu->addSeparator();
    QAction *followSystemAction = themeMenu->addAction(tr("Seguir el sistema"));
    followSystemAction->setCheckable(true);
    followSystemAction->setChecked(m_theme->followsSystem());
    followSystemAction->setToolTip(
        tr("Usa el tema claro u oscuro según la configuración del sistema operativo"));
    // Mientras se sigue el SO, la elección manual de tema queda deshabilitada.
    auto setManualThemeEnabled = [this](bool follow) {
        for (QAction *action : m_themeActions.values())
            action->setEnabled(!follow);
    };
    setManualThemeEnabled(m_theme->followsSystem());
    connect(followSystemAction, &QAction::toggled, this,
            [this, setManualThemeEnabled](bool on) {
                m_theme->setFollowSystem(on);
                setManualThemeEnabled(on);
            });

    themeMenu->addSeparator();
    QAction *warmLightAction = themeMenu->addAction(tr("Luz cálida nocturna"));
    warmLightAction->setCheckable(true);
    warmLightAction->setChecked(m_theme->isWarmLight());
    warmLightAction->setToolTip(
        tr("Tiñe el fondo del editor de tono ámbar según la hora, más cálido de noche"));
    connect(warmLightAction, &QAction::toggled, this,
            [this](bool on) { m_theme->setWarmLight(on); });

    // --- Idioma (se aplica al instante: recrea la ventana, ver setLanguage) ---
    // Código de locale por idioma; "" = automático (el del sistema). Los
    // nombres se muestran en su propia lengua (autónimos), no se traducen.
    // Añadir un idioma = una línea aquí + su .ts en CMakeLists.txt.
    struct Lang { QString code; QString name; };
    const QList<Lang> languages = {
        { QString(),               tr("Automático (sistema)") },
        { QStringLiteral("es"),    QStringLiteral("Español") },
        { QStringLiteral("en"),    QStringLiteral("English") },
        { QStringLiteral("de"),    QStringLiteral("Deutsch") },
        { QStringLiteral("fr"),    QStringLiteral("Français") },
        { QStringLiteral("it"),    QStringLiteral("Italiano") },
        { QStringLiteral("pt"),    QStringLiteral("Português") },
        { QStringLiteral("pl"),    QStringLiteral("Polski") },
        { QStringLiteral("nl"),    QStringLiteral("Nederlands") },
        { QStringLiteral("ro"),    QStringLiteral("Română") },
    };

    QMenu *langMenu = viewMenu->addMenu(tr("Idioma"));
    auto *langGroup = new QActionGroup(this);
    langGroup->setExclusive(true);
    const QString currentLang = AppSettings::language();
    for (const Lang &lang : languages) {
        QAction *action = langMenu->addAction(lang.name);
        action->setCheckable(true);
        action->setChecked(lang.code == currentLang);
        langGroup->addAction(action);
        connect(action, &QAction::triggered, this,
                [this, code = lang.code] { setLanguage(code); });
    }
}

void MainWindow::createHelpMenu()
{
    QMenu *helpMenu = menuBar()->addMenu(tr("A&yuda"));
    QAction *manualAction = helpMenu->addAction(tr("&Manual"));
    manualAction->setShortcut(QKeySequence::HelpContents);  // F1 en Linux/Windows
    connect(manualAction, &QAction::triggered, this, &MainWindow::showHelpDialog);
    helpMenu->addSeparator();
    QAction *aboutAction = helpMenu->addAction(tr("&Acerca de"));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);
}

void MainWindow::setLanguage(const QString &code)
{
    if (code == AppSettings::language())
        return;
    // Cambiar de idioma recrea la ventana (la UI se construye a mano, sin un
    // retranslateUi() que reasigne cada cadena). Antes de descartarla, ofrece
    // guardar los cambios pendientes; si el usuario cancela, no cambiamos nada.
    if (!m_file->maybeSave())
        return;

    AppSettings::setLanguage(code);

    // Persistimos el estado de ventana para que la ventana recreada arranque con
    // el mismo tamaño/posición y disposición (geometría, barras y proporciones de
    // la vista dividida); main() lee estas mismas claves al construirla. El menú
    // de idioma no es accesible en modo sin distracciones, así que basta el
    // estado plano (no el `sessionState` que el cierre usa para ese modo).
    AppSettings::setWindowGeometry(saveGeometry());
    AppSettings::setWindowState(saveState());
    AppSettings::setSplitterState(m_split->splitView()->saveState());

    // Recuerda dónde estaba el cursor del archivo actual para reabrirlo ahí tras
    // recrear la ventana (igual que hace el cierre).
    m_file->rememberCursorPosition();

    // main() intercambia los traductores y recrea la ventana, reabriendo el
    // documento actual ya saneado por maybeSave() (ruta vacía = documento nuevo).
    emit languageChangeRequested(m_documentIo->currentFile());
}

void MainWindow::relaunchSession(const QString &reopenPath)
{
    normalizeOutlineWidth();
    // El estado ya lo decidió la ventana anterior: ni recuperación de borrador ni
    // reabrir el último documento. Solo reabrimos el que estaba abierto (si lo
    // había y sigue existiendo); en su defecto, queda el documento nuevo vacío.
    if (!reopenPath.isEmpty() && QFileInfo::exists(reopenPath))
        m_file->openFile(reopenPath);
    m_file->startAutosave();
}

void MainWindow::showHelpDialog()
{
    // Único y no modal: si ya existe, solo lo trae al frente. Así el usuario
    // puede dejarlo abierto a un lado mientras escribe.
    if (!m_helpDialog)
        m_helpDialog = new HelpDialog(this);
    m_helpDialog->show();
    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

void MainWindow::showAboutDialog()
{
    QMessageBox box(this);
    box.setWindowTitle(tr("Acerca de"));
    box.setIconPixmap(
        QPixmap(QStringLiteral(":/icons/manuel-arias-calleja.jpg"))
            .scaledToWidth(160, Qt::SmoothTransformation));
    box.setTextFormat(Qt::RichText);
    box.setText(
        QStringLiteral("<h3>md-editor</h3>")
        + QStringLiteral("<p>") + tr("Versión 1.0") + QStringLiteral("</p>")
        + QStringLiteral("<p>") + tr("Desarrollado por Manuel Arias Calleja")
        + QStringLiteral("</p>")
        + QStringLiteral("<p>") + tr("Editor WYSIWYG de Markdown en Qt6 + C++17.")
        + QStringLiteral("</p>"));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void MainWindow::createFormatToolBar()
{
    QToolBar *bar = addToolBar(tr("Formato"));
    m_formatToolBar = bar;
    bar->setObjectName(QStringLiteral("formatToolBar"));  // para save/restoreState
    bar->setToolButtonStyle(Qt::ToolButtonTextOnly);
    bar->addAction(m_boldAction);
    bar->addAction(m_italicAction);
    bar->addAction(m_underlineAction);
    bar->addAction(m_strikeAction);
    bar->addAction(m_codeAction);
    bar->addAction(m_linkAction);
    bar->addSeparator();
    bar->addAction(m_h1Action);
    bar->addAction(m_h2Action);
    bar->addAction(m_h3Action);
    bar->addSeparator();
    bar->addAction(m_bulletAction);
    bar->addAction(m_numberedAction);
    bar->addAction(m_taskAction);
    bar->addSeparator();
    bar->addAction(m_quoteAction);
    bar->addAction(m_codeBlockAction);

    // Los botones de formato de carácter y de lista se muestran como iconos (el
    // resto de la barra es de texto): se ponen en modo solo-icono y se les genera
    // el icono según el tema. El tamaño lo fija updateToolBarIcons() a partir de
    // la fuente de la barra.
    for (QAction *a : {m_boldAction, m_italicAction, m_underlineAction, m_strikeAction,
                       m_bulletAction, m_numberedAction, m_taskAction})
        if (auto *btn = qobject_cast<QToolButton *>(bar->widgetForAction(a)))
            btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    updateToolBarIcons();
}

void MainWindow::updateToolBarIcons()
{
    if (!m_formatToolBar)
        return;
    // El icono sigue a la altura del texto de la barra (la de los botones «H1»,
    // «H2»…) para que los botones de icono se vean igual de grandes y acompañen
    // al zoom de la interfaz.
    const int px =
        qMax(18, qRound(QFontMetricsF(m_formatToolBar->font()).height() * 1.3));
    m_formatToolBar->setIconSize(QSize(px, px));
    const qreal dpr = m_formatToolBar->devicePixelRatioF();
    // El color del glifo se deriva del fondo de la barra (no se lee de la
    // paleta) para garantizar contraste sea cual sea el tema: tinta oscura
    // sobre fondos luminosos, tinta clara sobre fondos oscuros. Los botones de
    // la barra no pintan fondo de botón salvo en hover, así que el contraste
    // real es contra el color Window de la barra.
    //
    // Se consulta la paleta de la aplicación, no la del widget: cuando esta
    // función se llama desde la señal themeChanged del ThemeController, la
    // paleta de qApp ya está actualizada, pero la del widget aún arrastra la
    // anterior hasta que Qt propague el cambio. Si leyéramos la del widget,
    // los iconos se regenerarían con el color del tema previo y se verían
    // «aguados» al cambiar de tema.
    const QColor color =
        contrastingInk(qApp->palette().color(QPalette::Window));
    m_boldAction->setIcon(makeFormatIcon(FormatIconKind::Bold, color, px, dpr));
    m_italicAction->setIcon(makeFormatIcon(FormatIconKind::Italic, color, px, dpr));
    m_underlineAction->setIcon(makeFormatIcon(FormatIconKind::Underline, color, px, dpr));
    m_strikeAction->setIcon(makeFormatIcon(FormatIconKind::Strike, color, px, dpr));
    m_bulletAction->setIcon(makeListIcon(ListIconKind::Bullet, color, px, dpr));
    m_numberedAction->setIcon(makeListIcon(ListIconKind::Numbered, color, px, dpr));
    m_taskAction->setIcon(makeListIcon(ListIconKind::Task, color, px, dpr));
}

void MainWindow::updateWordCount()
{
    if (!m_countLabel)
        return;

    QTextEdit *ed = activeEditor();
    const QTextCursor cursor = ed->textCursor();
    const bool hasSelection = cursor.hasSelection();

    const QString text = hasSelection ? cursor.selectedText() : ed->toPlainText();
    const mdstats::DocStats st = mdstats::analyze(text);

    // %n produce el plural correcto en cada idioma (relevante en polaco/rumano,
    // de reglas complejas, y evita el «1 palabra(s)»).
    QString count = tr("%n palabra(s)", nullptr, st.words)
                    + QStringLiteral(" · ")
                    + tr("%n carácter(es)", nullptr, st.chars);
    // Tiempo de lectura estimado: cualquier texto cuenta al menos como 1 min.
    const int minutes =
        st.words > 0 ? std::max(1, static_cast<int>(std::ceil(st.readingMinutes))) : 0;
    if (minutes > 0)
        count += QStringLiteral(" · ") + tr("~%n min", nullptr, minutes);
    if (hasSelection)
        count.prepend(tr("Selección: "));
    m_countLabel->setText(count);
}

void MainWindow::showDocumentStatistics()
{
    const mdstats::DocStats st = mdstats::analyze(activeEditor()->toPlainText());
    const int minutes =
        st.words > 0 ? std::max(1, static_cast<int>(std::ceil(st.readingMinutes))) : 0;

    QDialog dlg(this);
    dlg.setWindowTitle(tr("Estadísticas del documento"));
    auto *form = new QFormLayout(&dlg);
    form->addRow(tr("Palabras:"), new QLabel(QString::number(st.words), &dlg));
    form->addRow(tr("Caracteres:"), new QLabel(QString::number(st.chars), &dlg));
    form->addRow(tr("Caracteres (sin espacios):"),
                 new QLabel(QString::number(st.charsNoSpaces), &dlg));
    form->addRow(tr("Párrafos:"), new QLabel(QString::number(st.paragraphs), &dlg));
    form->addRow(tr("Frases:"), new QLabel(QString::number(st.sentences), &dlg));
    form->addRow(tr("Tiempo de lectura:"),
                 new QLabel(tr("~%n min", nullptr, minutes), &dlg));

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Close, &dlg);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    form->addRow(buttons);

    dlg.exec();
}

void MainWindow::styleTables()
{
    QTextDocument *doc = m_editor->document();
    const QColor borderColor = palette().color(QPalette::Mid);

    // El borde es presentación pura: no lo serializa toMarkdown(), así que ni el
    // round-trip ni el estado «modificado» se ven afectados. Aun así preservamos
    // la marca de modificado de Qt, como hace recolorLinks().
    const bool wasModified = doc->isModified();
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    const QList<QTextFrame *> frames = doc->rootFrame()->childFrames();
    for (QTextFrame *frame : frames) {
        auto *table = qobject_cast<QTextTable *>(frame);
        if (!table)
            continue;
        QTextTableFormat fmt = table->format();
        fmt.setBorder(1);
        fmt.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
        fmt.setBorderBrush(borderColor);
        fmt.setBorderCollapse(true);  // un solo trazo entre celdas, no doble
        fmt.setCellPadding(4);
        fmt.setCellSpacing(0);
        table->setFormat(fmt);
    }
    cursor.endEditBlock();
    doc->setModified(wasModified);
}

// ---------------------------------------------------------------------------
// Zoom de la fuente
// ---------------------------------------------------------------------------

void MainWindow::zoomInText()
{
    ++m_zoomDelta;
    applyZoom();
}

void MainWindow::zoomOutText()
{
    --m_zoomDelta;
    applyZoom();
}

void MainWindow::resetZoom()
{
    m_zoomDelta = 0;
    applyZoom();
}

void MainWindow::applyZoom()
{
    // No dejar que el zoom reduzca la fuente por debajo de 1 punto.
    m_zoomDelta = qMax(m_zoomDelta, int(1 - m_baseFontPointSize));

    QFont f = m_editor->font();
    f.setPointSizeF(chromezoom::scaledPointSize(m_baseFontPointSize, m_zoomDelta));
    m_editor->setFont(f);

    applyChromeZoom();
    AppSettings::setZoomLevel(m_zoomDelta);  // se recuerda para la próxima sesión
}

void MainWindow::applyMenuFontScale()
{
    if (m_baseMenuPointSize <= 0)
        return;
    QFont mf = QApplication::font("QMenuBar");
    mf.setPointSizeF(chromezoom::scaledPointSize(m_baseMenuPointSize, m_zoomDelta));
    // Cambiar la fuente por clase obliga a Qt a recalcular el sizeHint de los
    // popups la próxima vez que se muestren — sin esto, las anchuras de las
    // QAction quedan congeladas al tamaño con el que se construyeron.
    QApplication::setFont(mf, "QMenuBar");
    QApplication::setFont(mf, "QMenu");
}

void MainWindow::forceMenuWidths()
{
    // Qt 6.8 con la plataforma gtk3 cachea las anchuras de las QAction al primer
    // cálculo, y los cambios de fuente posteriores no las invalidan: ítems largos
    // como «Insertar columna a la izquierda» o «Acerca de» se ven elided cuando la
    // fuente del zoom es mayor que la base. Le fijamos a cada menú el mínimo que
    // necesita (calculado en chromezoom::menuMinimumWidth) para que no pueda elidir.
    for (QMenu *menu : findChildren<QMenu *>()) {
        const int width = chromezoom::menuMinimumWidth(*menu);
        if (width > 0)
            menu->setMinimumWidth(width);
    }
}

void MainWindow::applyChromeZoom()
{
    // Escala cada superficie al mismo desfase que el editor, partiendo de su
    // tamaño base. Se ignora si la fuente base no usa puntos (pointSizeF() < 0),
    // para no romper nada. La barra de estado propaga su fuente al contador.
    const auto scale = [this](QWidget *w, qreal base) {
        if (!w || base <= 0)
            return;
        QFont f = w->font();
        f.setPointSizeF(chromezoom::scaledPointSize(base, m_zoomDelta));
        w->setFont(f);
    };
    // Menús: aplicamos la fuente vía QApplication para que afecte a la
    // medición interna de Qt (ver applyMenuFontScale y el comentario del
    // constructor). Tras eso, también la propagamos a las instancias ya
    // creadas para que el cambio sea visible sin reiniciar.
    applyMenuFontScale();
    scale(menuBar(), m_baseMenuPointSize);
    for (QMenu *menu : findChildren<QMenu *>())
        scale(menu, m_baseMenuPointSize);
    // Y forzamos el ancho mínimo para evitar el elided que Qt 6.8 + gtk3
    // mete cuando la fuente del zoom difiere de la base.
    forceMenuWidths();
    scale(m_formatToolBar, m_baseToolBarPointSize);
    updateToolBarIcons();  // el tamaño de los iconos sigue a la fuente de la barra
    scale(m_findBar, m_baseFindBarPointSize);
    scale(statusBar(), m_baseStatusBarPointSize);
    scale(m_split->sourceEditor(), m_baseSourceFontPointSize);
    scale(m_outline, m_baseOutlinePointSize);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // m_distraction puede no existir aún durante la construcción.
    if (m_distraction && m_distraction->isActive())
        m_distraction->updateLayout();  // el tamaño en pantalla completa llega aquí
}

// ---------------------------------------------------------------------------
// Vista de código Markdown crudo
// ---------------------------------------------------------------------------

QTextEdit *MainWindow::activeEditor() const
{
    return m_split->activeEditor();
}

void MainWindow::setBodyMarkdown(const QString &body)
{
    // El flag anti-bucle del controlador envuelve la sustitución para que los
    // contentsChanged que provoca (incluido el de recolorLinks) no realimenten la
    // sincronización de la vista dividida. Se guarda/restaura por reentrancia.
    const bool wasSyncing = m_split->beginProgrammaticChange();
    m_editor->setMarkdown(mdmath::protectMath(mdfootnote::protectFootnotes(body)));
    mdmath::renderMathInDocument(m_editor->document());
    mdfootnote::renderFootnotesInDocument(m_editor->document());
    styleTables();
    m_theme->recolorLinks();
    m_outline->rebuild(m_editor->document());
    m_split->endProgrammaticChange(wasSyncing);
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_editor->viewport()) {
        if (event->type() == QEvent::Wheel) {
            auto *wheel = static_cast<QWheelEvent *>(event);
            if (wheel->modifiers() & Qt::ControlModifier) {
                if (wheel->angleDelta().y() > 0)
                    zoomInText();
                else if (wheel->angleDelta().y() < 0)
                    zoomOutText();
                return true;  // consumimos el evento: no desplazar
            }
        }
        // Arrastrar y soltar un archivo lo abre (en lugar de que el editor lo
        // inserte como texto). El arrastre de texto interno no trae URLs y pasa.
        else if (event->type() == QEvent::DragEnter ||
                 event->type() == QEvent::DragMove ||
                 event->type() == QEvent::Drop) {
            auto *drop = static_cast<QDropEvent *>(event);
            if (drop->mimeData()->hasUrls()) {
                if (event->type() == QEvent::Drop) {
                    const QString path =
                        drop->mimeData()->urls().constFirst().toLocalFile();
                    if (!path.isEmpty())
                        m_file->openFile(path);
                }
                drop->acceptProposedAction();
                return true;
            }
        }
        // Al pasar por encima de un enlace: cursor de mano y pista de cómo
        // abrirlo. (El clic normal coloca el cursor para editar; Ctrl+clic abre.)
        else if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->buttons() == Qt::NoButton) {
                // Sobre la casilla de una tarea: cursor de mano y pista de clic.
                if (mdtask::isCheckboxAt(m_editor, me->position().toPoint())) {
                    m_editor->viewport()->setCursor(Qt::PointingHandCursor);
                    statusBar()->showMessage(tr("Clic para marcar o desmarcar la tarea"));
                    return true;
                }
                // Sobre una referencia de nota al pie: cursor de mano y pista.
                if (!footnoteRefIdAt(me->position().toPoint()).isEmpty()) {
                    m_editor->viewport()->setCursor(Qt::PointingHandCursor);
                    statusBar()->showMessage(tr("Clic para ir a la nota al pie"));
                    return true;
                }
                const QString href = m_editor->anchorAt(me->position().toPoint());
                if (!href.isEmpty()) {
                    m_editor->viewport()->setCursor(Qt::PointingHandCursor);
                    statusBar()->showMessage(
                        tr("Ctrl+clic para abrir el enlace: %1").arg(href));
                    return true;  // si no, QTextEdit restablecería el cursor a I-beam
                }
                // Acabamos de salir de un enlace: restablece cursor y pista.
                if (m_editor->viewport()->cursor().shape() == Qt::PointingHandCursor) {
                    m_editor->viewport()->setCursor(Qt::IBeamCursor);
                    statusBar()->clearMessage();
                }
            }
        }
        // Doble clic sobre una fórmula renderizada: abre el diálogo de edición
        // con el TeX precargado. Sin esto las fórmulas serían de solo lectura.
        else if (event->type() == QEvent::MouseButtonDblClick) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton
                && m_formula->editFormulaAt(me->position().toPoint()))
                return true;
        }
        else if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            // Ctrl+clic izquierdo sobre un enlace lo abre en la aplicación externa.
            if (me->button() == Qt::LeftButton
                && (me->modifiers() & Qt::ControlModifier)) {
                const QString href = m_editor->anchorAt(me->position().toPoint());
                if (!href.isEmpty()) {
                    openLink(href);
                    return true;  // no mover el cursor de texto
                }
            }
            // Clic izquierdo simple sobre una referencia de nota al pie: salta a
            // su definición al final del documento.
            if (me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier
                && jumpToFootnoteAt(me->position().toPoint()))
                return true;
            // Clic izquierdo simple sobre la casilla de un ítem de tarea: la
            // marca/desmarca (round-trip a `- [x]`/`- [ ]` lo da Qt solo).
            if (me->button() == Qt::LeftButton && me->modifiers() == Qt::NoModifier
                && mdtask::toggleCheckboxAt(m_editor, me->position().toPoint()))
                return true;  // consumido: no coloca el cursor ni inicia selección
        }
    }
    // Interceptación de teclas sobre el editor WYSIWYG para proteger las
    // fórmulas renderizadas frente a edición accidental con el teclado.
    else if (watched == m_editor && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        // Antes de un pegado, ajusta la selección para que coja los grupos
        // de math enteros (no a media fórmula). El paste real lo sigue
        // haciendo QTextEdit con la selección ya extendida.
        if (ke->matches(QKeySequence::Paste))
            m_formula->guardPasteAgainstMath();
        if (m_formula->handleMathKeyPress(ke))
            return true;
        // Shortcodes `:nombre:`: al teclear el ':' de cierre, si delante hay un
        // `:nombre:` conocido se sustituye por su símbolo. Insertamos el ':' y
        // expandimos nosotros (solo en el editor WYSIWYG, solo al teclear).
        if (ke->text() == QStringLiteral(":")
            && !(ke->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier))) {
            QTextCursor cursor = m_editor->textCursor();
            cursor.insertText(QStringLiteral(":"));
            expandShortcodeBefore(cursor);
            return true;
        }
    }
    // Continuación inteligente de listas en el editor de código fuente. Se
    // comprueba m_split porque durante su construcción (al reparentar el editor en
    // el QSplitter) ya llegan eventos aquí, antes de que el puntero esté asignado.
    else if (m_split && watched == m_split->sourceEditor() && event->type() == QEvent::KeyPress) {
        auto *ke = static_cast<QKeyEvent *>(event);
        const auto mods = ke->modifiers() & ~Qt::KeypadModifier;
        if ((ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
            && mods == Qt::NoModifier && continueSourceList())
            return true;
    }
    return QMainWindow::eventFilter(watched, event);
}

bool MainWindow::continueSourceList()
{
    QTextEdit *source = m_split->sourceEditor();
    QTextCursor cursor = source->textCursor();
    if (cursor.hasSelection())
        return false;  // con selección, Enter la reemplaza: comportamiento normal

    const mdlist::Continuation c = mdlist::analyze(cursor.block().text());
    if (!c.isItem)
        return false;

    cursor.beginEditBlock();
    if (c.empty) {
        // Ítem vacío: se sale de la lista quitando el marcador (la línea queda en
        // blanco), en vez de insertar otro ítem.
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
    } else {
        // Nueva línea con la misma sangría y marcador (número incrementado,
        // tarea sin marcar).
        cursor.insertText(QLatin1Char('\n') + c.nextPrefix);
    }
    cursor.endEditBlock();
    source->setTextCursor(cursor);
    return true;
}

void MainWindow::openLink(const QString &href)
{
    if (href.isEmpty())
        return;
    // Los enlaces relativos se resuelven respecto al directorio del documento
    // (la misma baseUrl con la que se cargaron imágenes/enlaces al abrir).
    QUrl url(href);
    if (url.isRelative())
        url = m_editor->document()->baseUrl().resolved(url);
    QDesktopServices::openUrl(url);
}

void MainWindow::normalizeOutlineWidth()
{
    // El ancho del dock se persiste en windowState y puede volver
    // desproporcionado (p. ej. heredado del modo sin distracciones, que lo
    // ensancha a propósito). Solo aplica fuera de ese modo y con el dock
    // visible; si ocupa más de un tercio de la ventana lo devolvemos a un
    // ancho de lectura cómodo para que el editor no quede en una franja.
    if (!m_outline->isVisible() || (m_distraction && m_distraction->isActive()))
        return;
    constexpr int kNormalOutlineWidth = 280;
    if (m_outline->width() > qMax(kNormalOutlineWidth, width() / 3))
        resizeDocks({m_outline}, {kNormalOutlineWidth}, Qt::Horizontal);
}

void MainWindow::goToHeading()
{
    const QList<OutlineHeading> headings = mdoutline::headingsOf(m_editor->document());
    if (headings.isEmpty()) {
        statusBar()->showMessage(tr("El documento no tiene encabezados."));
        return;
    }
    GoToHeadingDialog dialog(headings, this);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const int blockNumber = dialog.selectedBlockNumber();
    const QTextBlock block = m_editor->document()->findBlockByNumber(blockNumber);
    if (!block.isValid())
        return;
    QTextCursor cursor(block);
    m_editor->setTextCursor(cursor);
    m_editor->ensureCursorVisible();
    m_editor->setFocus();
}

void MainWindow::expandShortcodeBefore(const QTextCursor &cursor)
{
    const QTextBlock block = cursor.block();
    const int blockStart = block.position();
    const int end = cursor.position() - blockStart;  // tras el ':' de cierre, en el bloque
    const QString text = block.text();
    if (end < 2 || text.at(end - 1) != QLatin1Char(':'))
        return;
    // Busca el ':' de apertura hacia atrás; el nombre solo admite [A-Za-z0-9_].
    int open = -1;
    for (int i = end - 2; i >= 0; --i) {
        const QChar c = text.at(i);
        if (c == QLatin1Char(':')) {
            open = i;
            break;
        }
        if (!(c.isLetterOrNumber() || c == QLatin1Char('_')))
            return;  // carácter inválido en el nombre: no es un shortcode
    }
    if (open < 0)
        return;
    const QString name = text.mid(open + 1, (end - 1) - (open + 1));
    const QString symbol = mdshortcode::expand(name);
    if (symbol.isEmpty())
        return;
    // Sustituye `:nombre:` (ambos dos puntos incluidos) por el símbolo.
    QTextCursor replace(m_editor->document());
    replace.setPosition(blockStart + open);
    replace.setPosition(blockStart + end, QTextCursor::KeepAnchor);
    replace.insertText(symbol);
}

QString MainWindow::footnoteRefIdAt(const QPoint &viewportPos) const
{
    const int pos = m_editor->cursorForPosition(viewportPos).position();
    // charFormat() devuelve el formato del carácter inmediatamente anterior al
    // cursor; probamos pos-1 y pos para cubrir el carácter bajo el clic.
    QTextCursor probe(m_editor->document());
    probe.setPosition(pos > 0 ? pos - 1 : 0);
    QTextCharFormat cf = probe.charFormat();
    if (!cf.boolProperty(mdfootnote::IsFootnoteRefProperty)) {
        probe.setPosition(pos);
        cf = probe.charFormat();
        if (!cf.boolProperty(mdfootnote::IsFootnoteRefProperty))
            return QString();
    }
    return cf.property(mdfootnote::FootnoteIdProperty).toString();
}

bool MainWindow::jumpToFootnoteAt(const QPoint &viewportPos)
{
    const QString id = footnoteRefIdAt(viewportPos);
    if (id.isEmpty())
        return false;
    const int blockNo = mdfootnote::definitionBlockNumber(m_editor->document(), id);
    if (blockNo < 0) {
        statusBar()->showMessage(tr("La nota [^%1] no tiene definición").arg(id));
        return false;
    }
    const QTextBlock block = m_editor->document()->findBlockByNumber(blockNo);
    QTextCursor dest(block);
    dest.movePosition(QTextCursor::EndOfBlock);
    m_editor->setTextCursor(dest);
    m_editor->ensureCursorVisible();
    m_editor->setFocus();
    return true;
}

void MainWindow::startSession(const QString &cmdLineFile)
{
    // Si el esquema arranca visible, normaliza ya su ancho (con el layout
    // asentado). Si arranca oculto, lo hará el primer F9 (visibilityChanged).
    normalizeOutlineWidth();

    // Prioridad: archivo de la línea de comandos > recuperar borrador > reabrir
    // el último documento. Sin returns prematuros: todas las ramas confluyen en
    // el arranque del autoguardado al final.
    bool recovered = false;
    if (!cmdLineFile.isEmpty()) {
        m_file->openFile(cmdLineFile);
    } else if (m_recovery->hasDraft()) {
        // ¿Quedó un borrador de un cierre anómalo? Ofrecer recuperarlo.
        const QString original = m_recovery->draftOriginalPath();
        const QString name = original.isEmpty() ? tr("(sin título)")
                                                : QFileInfo(original).fileName();
        const QString when = m_recovery->draftTimestamp().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
        QMessageBox box(this);
        box.setIcon(QMessageBox::Question);
        box.setWindowTitle(tr("Recuperar documento"));
        box.setText(tr("Se encontró un documento con cambios sin guardar de una "
                       "sesión anterior:\n%1 (%2).\n\n¿Quieres recuperarlo?")
                        .arg(name, when));
        QPushButton *recoverBtn = box.addButton(tr("Recuperar"), QMessageBox::AcceptRole);
        box.addButton(tr("Descartar"), QMessageBox::DestructiveRole);
        box.exec();
        if (box.clickedButton() == recoverBtn)
            recovered = m_file->recoverDraft();
        else
            m_recovery->clearDraft();  // descartado: seguir con el flujo normal
    }

    // Reabrir el último documento real, salvo que ya se abriera/recuperara algo.
    if (cmdLineFile.isEmpty() && !recovered) {
        const QString last = AppSettings::lastFile();
        if (!last.isEmpty() && QFileInfo::exists(last))
            m_file->openFile(last);
    }

    // Decidida la sesión inicial, ya es seguro autoguardar borradores (no antes:
    // correría durante el diálogo de recuperación con el documento aún vacío).
    m_file->startAutosave();
}

void MainWindow::onCurrentFileChanged(const QString &path)
{
    // Acaba de cargarse/guardarse: el documento está limpio.
    setWindowModified(false);

    // Registra el archivo en la lista de recientes (al abrir o al guardar;
    // addFile ignora la ruta vacía del documento nuevo).
    if (m_recentFiles)
        m_recentFiles->addFile(path);

    // Recuerda el último archivo real para reabrirlo al arrancar. Se persiste
    // aquí (no solo al cerrar) para sobrevivir a un cierre inesperado. Solo se
    // guardan rutas no vacías: la ruta vacía del documento nuevo (al arrancar o
    // tras «Nuevo») NO debe pisar el último archivo, o si no quedaría vacío en
    // cada arranque y nunca habría nada que reabrir.
    if (!path.isEmpty())
        AppSettings::setLastFile(path);

    const QString shown = path.isEmpty() ? tr("Sin título")
                                         : QFileInfo(path).fileName();
    // El marcador [*] se muestra automáticamente cuando hay cambios sin guardar.
    setWindowTitle(tr("%1[*] — md-editor").arg(shown));
}

void MainWindow::onDiskExternalChange(const QByteArray &diskBytes)
{
    const QString path = m_documentIo->currentFile();
    const bool locallyModified =
        m_documentIo->isModified() || m_split->isSourceDirty();

    if (!locallyModified) {
        // Sin cambios locales: se recarga sin molestar.
        reloadFromDisk();
        statusBar()->showMessage(tr("El archivo cambió en disco: recargado."), 4000);
        return;
    }

    // Hay cambios locales sin guardar: que decida el usuario.
    QMessageBox box(this);
    box.setIcon(QMessageBox::Warning);
    box.setWindowTitle(tr("Archivo modificado en disco"));
    box.setText(tr("«%1» ha cambiado en disco y tienes cambios sin guardar.")
                    .arg(QFileInfo(path).fileName()));
    box.setInformativeText(
        tr("¿Recargar la versión del disco (perderás tus cambios) o conservar los tuyos?"));
    QPushButton *reloadButton = box.addButton(tr("Recargar"), QMessageBox::AcceptRole);
    QPushButton *keepButton =
        box.addButton(tr("Conservar los míos"), QMessageBox::RejectRole);
    box.setDefaultButton(keepButton);
    // Mientras el diálogo modal está abierto, suspende la comprobación (su exec
    // corre un bucle anidado y el watcher podría volver a dispararse).
    m_diskWatcher->setSuspended(true);
    box.exec();
    m_diskWatcher->setSuspended(false);

    if (box.clickedButton() == reloadButton) {
        reloadFromDisk();
    } else {
        // Conserva lo del usuario; recuerda el contenido del disco como referencia
        // para no volver a preguntar por este mismo cambio externo.
        m_diskWatcher->setSnapshot(diskBytes);
    }
}

void MainWindow::reloadFromDisk()
{
    const QString path = m_documentIo->currentFile();
    if (path.isEmpty())
        return;

    // Conserva aproximadamente la posición del cursor del editor activo.
    const int caret = activeEditor()->textCursor().position();

    QString error;
    if (!m_documentIo->load(path, &error)) {  // emite currentFileChanged → revigila
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo recargar el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        return;
    }
    // Si el panel de fuente está visible (modo fuente o vista dividida), su texto
    // plano hay que refrescarlo con lo recargado (load() solo toca el documento
    // WYSIWYG).
    m_split->refreshSourceFromDocument();
    QTextEdit *ed = activeEditor();
    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(qMin(caret, ed->document()->characterCount() - 1));
    ed->setTextCursor(cursor);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_file->maybeSave()) {
        // Cierre limpio (guardado o descartado a propósito): sin borrador que
        // recuperar la próxima vez.
        m_recovery->clearDraft();
        m_file->rememberCursorPosition();  // reabrir el documento donde se dejó
        // Recuerda tamaño/posición y disposición de barras para la próxima vez.
        // Si se cierra en modo sin distracciones, el controlador devuelve el estado
        // previo a entrar (ventana normal con sus barras), no la pantalla completa.
        AppSettings::setWindowGeometry(m_distraction->sessionGeometry());
        AppSettings::setWindowState(m_distraction->sessionState());
        AppSettings::setSplitterState(m_split->splitView()->saveState());
        event->accept();
    } else {
        event->ignore();
    }
}
