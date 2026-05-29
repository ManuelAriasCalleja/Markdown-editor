#include "mainwindow.h"

#include "appsettings.h"
#include "blockconstructs.h"
#include "codehighlighter.h"
#include "documentio.h"
#include "exporters.h"
#include "findreplacebar.h"
#include "focuseditor.h"
#include "helpdialog.h"
#include "listcontinuation.h"
#include "mathblocks.h"
#include "outlinepanel.h"
#include "recentfilesmanager.h"
#include "recoverymanager.h"
#include "tableedit.h"
#include "themecontroller.h"

#include <cmath>
#include <memory>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColor>
#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QDir>
#include <QDropEvent>
#include <QMouseEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFont>
#include <QFontMetrics>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QInputDialog>
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
#include <QLineEdit>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QShortcut>
#include <QRegularExpression>
#include <QSplitter>
#include <QStatusBar>
#include <QStringConverter>
#include <QStringList>
#include <QTextCharFormat>
#include <QTextBlock>
#include <QTextCursor>
#include <QSpinBox>
#include <QTextDocument>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextFragment>
#include <QTextFrame>
#include <QTextLength>
#include <QTextList>
#include <QTextStream>
#include <QTextTable>
#include <QTextTableCell>
#include <QScrollBar>
#include <QTimer>
#include <QToolBar>
#include <QUrl>
#include <QWheelEvent>

namespace {

// Ancho de la columna de lectura centrada del modo sin distracciones, y ancho
// del árbol del esquema cuando acompaña a la columna en ese modo.
constexpr int kReadingColumn = 960;
constexpr int kOutlineTreeWidth = 280;

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

    // Vista de código Markdown crudo: texto plano monoespaciado. El widget
    // central es un divisor que puede mostrar el editor WYSIWYG, este, o ambos
    // lado a lado (vista dividida). WYSIWYG a la izquierda, fuente a la derecha.
    m_sourceEditor = new FocusEditor(this);
    m_sourceEditor->setAcceptRichText(false);
    QFont mono = QFont(QStringLiteral("monospace"));
    mono.setStyleHint(QFont::Monospace);
    m_sourceEditor->setFont(mono);
    m_splitView = new QSplitter(Qt::Horizontal, this);
    m_splitView->addWidget(m_editor);
    m_splitView->addWidget(m_sourceEditor);
    m_splitView->setStretchFactor(0, 1);  // ambos paneles crecen por igual
    m_splitView->setStretchFactor(1, 1);
    m_sourceEditor->hide();  // arranca en WYSIWYG
    setCentralWidget(m_splitView);
    m_splitView->restoreState(AppSettings::splitterState());  // proporciones previas

    // Tamaños de fuente base (editor WYSIWYG y vista de código), para poder
    // restaurarlos con "Tamaño normal".
    m_baseFontPointSize = m_editor->font().pointSizeF();
    m_baseSourceFontPointSize = m_sourceEditor->font().pointSizeF();
    // Resaltado de sintaxis de los bloques de código.
    m_highlighter = new CodeBlockHighlighter(m_editor->document());
    // Zoom con Ctrl+rueda del ratón y detección de enlaces bajo el cursor.
    m_editor->viewport()->installEventFilter(this);
    m_editor->viewport()->setMouseTracking(true);  // recibir hover sin botón pulsado
    // Filtro sobre el propio editor para interceptar teclas: las fórmulas
    // renderizadas no se pueden editar tecleando dentro (se editan con doble
    // clic) y Backspace/Delete en su borde borran el grupo entero.
    m_editor->installEventFilter(this);
    // Enter en el editor de código continúa listas/tareas (en WYSIWYG ya lo hace
    // QTextEdit de serie). Se filtran las pulsaciones del propio widget.
    m_sourceEditor->installEventFilter(this);

    // En modo fuente, escribir marca el documento como modificado y actualiza el
    // contador (el contenido vive en m_sourceEditor hasta que se vuelca).
    connect(m_sourceEditor, &QTextEdit::textChanged, this, [this] {
        // Edición real del usuario en el panel de fuente (no un refresco
        // programático, que va con m_syncing): marca el fuente como la versión
        // más fresca, en modo fuente o en vista dividida.
        if (!m_syncing && (m_sourceMode || m_splitMode)) {
            m_sourceDirty = true;
            setWindowModified(true);
            // En vista dividida, mientras el foco está en el fuente, vuelca al
            // WYSIWYG tras una pausa (debounce). Solo el panel sin foco se toca.
            if (m_splitMode && m_sourceEditor->hasFocus())
                m_syncToDocTimer->start();
        }
        updateWordCount();
    });
    connect(m_sourceEditor, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::updateWordCount);
    connect(m_sourceEditor, &QTextEdit::selectionChanged,
            this, &MainWindow::updateWordCount);

    // Colaboradores: E/S del documento y control del tema. Se crean antes del
    // menú porque sus acciones los invocan.
    m_documentIo = new DocumentIo(m_editor, this);
    m_theme = new ThemeController(m_editor, m_highlighter, this);

    // Al pegar o soltar una imagen en el editor, guardarla a disco e insertar
    // `![](ruta)` en vez de incrustarla (que no sobreviviría al round-trip a
    // Markdown). Solo en el editor WYSIWYG; en la vista de fuente es texto.
    m_editor->setMimeInsertHandler(
        [this](const QMimeData *src) { return handlePastedImage(src); });

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
    connect(m_outline, &QDockWidget::visibilityChanged, this, [this] {
        if (m_distractionFree)
            updateDistractionLayout();
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
    // El índice se reconstruye al editar, con un pequeño retardo para no rehacer
    // el árbol en cada pulsación. En modo fuente no se toca (el contenido vive
    // como texto plano, no en la estructura del documento).
    m_outlineTimer = new QTimer(this);
    m_outlineTimer->setSingleShot(true);
    m_outlineTimer->setInterval(300);
    connect(m_outlineTimer, &QTimer::timeout, this, [this] {
        if (!m_sourceMode)
            m_outline->rebuild(m_editor->document());
    });
    connect(m_editor->document(), &QTextDocument::contentsChanged, this, [this] {
        if (!m_sourceMode)
            m_outlineTimer->start();
        // En vista dividida, mientras el foco está en el WYSIWYG, refresca el
        // panel de fuente tras una pausa. m_syncing distingue los cambios del
        // usuario de los provocados por la propia sincronización (anti-bucle).
        if (m_splitMode && !m_syncing && !m_sourceEditor->hasFocus())
            m_syncToSourceTimer->start();
    });

    // Temporizadores de debounce de la vista dividida (~250 ms): refrescan el
    // panel SIN foco con lo editado en el otro. Ver syncSourceFromDocument /
    // syncDocumentFromSource.
    m_syncToSourceTimer = new QTimer(this);
    m_syncToSourceTimer->setSingleShot(true);
    m_syncToSourceTimer->setInterval(250);
    connect(m_syncToSourceTimer, &QTimer::timeout, this, [this] {
        if (!m_sourceEditor->hasFocus())  // si el foco saltó al fuente, no lo pisamos
            syncSourceFromDocument();
    });
    m_syncToDocTimer = new QTimer(this);
    m_syncToDocTimer->setSingleShot(true);
    m_syncToDocTimer->setInterval(250);
    connect(m_syncToDocTimer, &QTimer::timeout, this, &MainWindow::syncDocumentFromSource);

    // Al cambiar el foco entre los dos paneles, vacía de inmediato el sync
    // pendiente para que el panel al que llegas esté al día.
    connect(qApp, &QApplication::focusChanged, this,
            [this](QWidget *old, QWidget *now) {
        flushPendingSync(old);
        // Solo reaccionamos al pasar el foco a uno de los dos editores (no a
        // menús, diálogos o la barra de búsqueda).
        if (m_splitMode && (now == m_editor || now == m_sourceEditor))
            updateActionsForFocus();
    });

    // Autoguardado para recuperación ante fallos: escribe un borrador cada pocos
    // segundos mientras haya cambios sin guardar. m_autosaveDirty evita reescribir
    // el borrador si nada cambió desde el último volcado.
    m_recovery = new RecoveryManager(this);
    m_autosaveTimer = new QTimer(this);
    m_autosaveTimer->setInterval(5000);
    connect(m_autosaveTimer, &QTimer::timeout, this, &MainWindow::autosaveDraft);
    // El temporizador se arranca al final de startSession (tras decidir una
    // posible recuperación): si corriera ya durante el diálogo de recuperación,
    // con el documento aún vacío, borraría el borrador antes de recuperarlo.
    const auto markDirty = [this] { m_autosaveDirty = true; };
    connect(m_editor, &QTextEdit::textChanged, this, markDirty);
    connect(m_sourceEditor, &QTextEdit::textChanged, this, markDirty);

    // El nivel de zoom hay que conocerlo ANTES de crear los menús: Qt 6.8 con
    // plataforma gtk3 cachea las anchuras de las QAction en la primera medición
    // y un setFont posterior no las invalida (resultado visible: items elided
    // como «Acerca de» → «Ace... de» cuando el zoom no es cero). Aplicar el
    // tamaño objetivo en la fuente por defecto del QApplication para las
    // clases QMenuBar/QMenu antes de instanciarlas hace que los menús nazcan
    // ya midiéndose con la fuente correcta.
    m_zoomDelta = AppSettings::zoomLevel();
    m_baseMenuPointSize = QApplication::font("QMenuBar").pointSizeF();
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

    // ESC sale del modo sin distracciones. Solo se activa dentro del modo para
    // no interferir con otros usos de ESC (p. ej. cerrar la barra de búsqueda).
    m_escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    m_escShortcut->setEnabled(false);
    connect(m_escShortcut, &QShortcut::activated, this,
            [this] { m_distractionAction->setChecked(false); });

    // Contador de palabras/caracteres, anclado a la derecha de la barra de estado.
    m_countLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_countLabel);

    // Los botones de formato reflejan en todo momento lo que hay bajo el cursor.
    connect(m_editor, &QTextEdit::currentCharFormatChanged,
            this, &MainWindow::updateFormatActions);
    connect(m_editor, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::updateFormatActions);
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
    // herramienta). El aviso se agrupa con un pequeño retardo para sortear los
    // guardados atómicos (escribir-temporal-y-renombrar), que disparan varios
    // eventos y dejan el archivo un instante inexistente.
    m_fileWatcher = new QFileSystemWatcher(this);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onWatchedFileChanged);
    m_diskCheckTimer = new QTimer(this);
    m_diskCheckTimer->setSingleShot(true);
    m_diskCheckTimer->setInterval(300);
    connect(m_diskCheckTimer, &QTimer::timeout, this, &MainWindow::checkDiskChange);
    connect(m_documentIo, &DocumentIo::currentFileChanged,
            this, &MainWindow::watchCurrentFile);
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
    // Arranca siempre como ventana normal: descarta pantalla completa/maximizado
    // que pudieran venir de una sesión cerrada en modo sin distracciones.
    setWindowState(windowState() & ~(Qt::WindowFullScreen | Qt::WindowMaximized));
    m_findBar->hide();  // la barra de búsqueda siempre arranca oculta

    statusBar()->showMessage(
        tr("Editor Markdown WYSIWYG — escribe y da formato con la barra superior"));
    updateFormatActions();
    updateWordCount();

    // Restaura el tema elegido en la sesión anterior (la señal themeChanged
    // marca la acción del menú correspondiente).
    m_theme->applyTheme(
        mdtheme::idFromKey(AppSettings::themeKey(), mdtheme::ThemeId::Light));
}

void MainWindow::createMenusAndActions()
{
    // --- Menú Archivo ---
    QMenu *fileMenu = menuBar()->addMenu(tr("&Archivo"));

    QAction *newAction = fileMenu->addAction(tr("&Nuevo"));
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::newFile);

    QAction *openAction = fileMenu->addAction(tr("&Abrir..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFileDialog);

    QMenu *recentMenu = fileMenu->addMenu(tr("Abrir &recientes"));
    m_recentFiles = new RecentFilesManager(recentMenu, this);
    connect(m_recentFiles, &RecentFilesManager::fileOpenRequested,
            this, &MainWindow::openFile);

    QAction *saveAction = fileMenu->addAction(tr("&Guardar"));
    saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction, &QAction::triggered, this, &MainWindow::save);

    QAction *saveAsAction = fileMenu->addAction(tr("Guardar &como..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::saveAs);

    fileMenu->addSeparator();

    QMenu *exportMenu = fileMenu->addMenu(tr("&Exportar"));
    QAction *exportPdfAction = exportMenu->addAction(tr("A PDF..."));
    connect(exportPdfAction, &QAction::triggered, this, &MainWindow::exportPdf);
    QAction *exportHtmlAction = exportMenu->addAction(tr("A HTML..."));
    connect(exportHtmlAction, &QAction::triggered, this, &MainWindow::exportHtml);
    QAction *exportOdfAction = exportMenu->addAction(tr("A ODF (ODT)..."));
    connect(exportOdfAction, &QAction::triggered, this, &MainWindow::exportOdf);
    QAction *exportLatexAction = exportMenu->addAction(tr("A LaTeX..."));
    connect(exportLatexAction, &QAction::triggered, this, &MainWindow::exportLatex);

    fileMenu->addSeparator();

    QAction *printAction = fileMenu->addAction(tr("&Imprimir..."));
    printAction->setShortcut(QKeySequence::Print);
    connect(printAction, &QAction::triggered, this, &MainWindow::print);

    fileMenu->addSeparator();

    QAction *quitAction = fileMenu->addAction(tr("&Salir"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    // --- Menú Editar ---
    QMenu *editMenu = menuBar()->addMenu(tr("&Editar"));

    QAction *undoAction = editMenu->addAction(tr("Deshacer"));
    undoAction->setShortcut(QKeySequence::Undo);
    connect(undoAction, &QAction::triggered, this, [this] { activeEditor()->undo(); });

    QAction *redoAction = editMenu->addAction(tr("Rehacer"));
    redoAction->setShortcut(QKeySequence::Redo);
    connect(redoAction, &QAction::triggered, this, [this] { activeEditor()->redo(); });

    editMenu->addSeparator();

    QAction *findAction = editMenu->addAction(tr("Buscar..."));
    findAction->setShortcut(QKeySequence::Find);                       // Ctrl+F
    connect(findAction, &QAction::triggered, m_findBar, &FindReplaceBar::showFind);

    QAction *replaceAction = editMenu->addAction(tr("Reemplazar..."));
    replaceAction->setShortcut(QKeySequence::Replace);                 // Ctrl+H
    connect(replaceAction, &QAction::triggered, m_findBar, &FindReplaceBar::showReplace);

    // --- Acciones de formato (compartidas por el menú y la barra) ---
    // Tabla de descriptores: todas son checkable y solo difieren en texto, atajo
    // y la acción a ejecutar. El tooltip se compone del texto más el atajo en su
    // forma nativa y localizada (p. ej. «Strg+B» en alemán), derivado del propio
    // atajo: así no hay que traducirlo a mano ni puede desincronizarse.
    using Mutator = std::function<void(QTextCharFormat &, const QTextCharFormat &)>;
    const auto charToggle = [this](Mutator m) {
        return [this, m = std::move(m)] { toggleCharFormat(m); };
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
         [this] { insertLink(); }, tr("Insertar o editar enlace")},
        {&m_quoteAction, tr("❝ Cita"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Q),
         [this] { toggleBlockquote(); }, tr("Convertir en cita")},
        {&m_codeBlockAction, tr("Bloque"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K),
         [this] { toggleCodeBlock(); }, tr("Bloque de código")},
        {&m_h1Action, tr("H1"), QKeySequence(Qt::CTRL | Qt::Key_1),
         [this] { applyHeading(1); }},
        {&m_h2Action, tr("H2"), QKeySequence(Qt::CTRL | Qt::Key_2),
         [this] { applyHeading(2); }},
        {&m_h3Action, tr("H3"), QKeySequence(Qt::CTRL | Qt::Key_3),
         [this] { applyHeading(3); }},
        {&m_h4Action, tr("H4"), QKeySequence(Qt::CTRL | Qt::Key_4),
         [this] { applyHeading(4); }},
        {&m_h5Action, tr("H5"), QKeySequence(Qt::CTRL | Qt::Key_5),
         [this] { applyHeading(5); }},
        {&m_h6Action, tr("H6"), QKeySequence(Qt::CTRL | Qt::Key_6),
         [this] { applyHeading(6); }},
        {&m_bulletAction, tr("Lista de viñetas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_U),
         [this] { applyList(QTextListFormat::ListDisc); }},
        {&m_numberedAction, tr("Lista numerada"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O),
         [this] { applyList(QTextListFormat::ListDecimal); }},
        {&m_taskAction, tr("Lista de tareas"), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_T),
         [this] { toggleTaskItem(); }},
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
    connect(m_indentAction, &QAction::triggered, this, &MainWindow::indentList);
    m_wysiwygActions.append(m_indentAction);

    m_outdentAction = formatMenu->addAction(tr("Disminuir sangría"));
    m_outdentAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_BracketLeft));
    connect(m_outdentAction, &QAction::triggered, this, &MainWindow::outdentList);
    m_wysiwygActions.append(m_outdentAction);

    formatMenu->addSeparator();
    formatMenu->addAction(m_quoteAction);
    formatMenu->addAction(m_codeBlockAction);

    m_langAction = formatMenu->addAction(tr("Lenguaje del bloque..."));
    m_langAction->setToolTip(tr("Fija el lenguaje del bloque de código (resaltado)"));
    connect(m_langAction, &QAction::triggered, this, &MainWindow::setCodeLanguage);
    m_wysiwygActions.append(m_langAction);

    // --- Menú Insertar ---
    QMenu *insertMenu = menuBar()->addMenu(tr("&Insertar"));

    QAction *insLink = insertMenu->addAction(tr("Enlace..."));
    connect(insLink, &QAction::triggered, this, &MainWindow::insertLink);

    QAction *insImage = insertMenu->addAction(tr("Imagen..."));
    connect(insImage, &QAction::triggered, this, &MainWindow::insertImage);

    QAction *insPasteImage = insertMenu->addAction(tr("Pegar imagen"));
    insPasteImage->setToolTip(tr("Guarda la imagen del portapapeles y la inserta"));
    connect(insPasteImage, &QAction::triggered, this, &MainWindow::pasteImageFromClipboard);

    QAction *insTable = insertMenu->addAction(tr("Tabla..."));
    connect(insTable, &QAction::triggered, this, &MainWindow::insertTable);

    QAction *insRule = insertMenu->addAction(tr("Regla horizontal"));
    connect(insRule, &QAction::triggered, this, &MainWindow::insertHorizontalRule);

    QAction *insFormula = insertMenu->addAction(tr("Fórmula..."));
    insFormula->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_F));
    insFormula->setToolTip(
        insFormula->text() + QStringLiteral(" (%1)").arg(
            insFormula->shortcut().toString(QKeySequence::NativeText)));
    connect(insFormula, &QAction::triggered, this, &MainWindow::insertFormula);

    // Insertar tampoco aplica en la vista de fuente.
    m_wysiwygActions << insLink << insImage << insPasteImage << insTable << insRule << insFormula;

    // --- Menú Tabla (operaciones sobre la tabla bajo el cursor) ---
    // Sus acciones se habilitan solo cuando el cursor está dentro de una tabla
    // (ver updateTableActions), de ahí que no vayan en m_wysiwygActions.
    QMenu *tableMenu = menuBar()->addMenu(tr("&Tabla"));

    QAction *aRowAbove = tableMenu->addAction(tr("Insertar fila encima"));
    connect(aRowAbove, &QAction::triggered, this, [this] { tableInsertRow(false); });
    QAction *aRowBelow = tableMenu->addAction(tr("Insertar fila debajo"));
    connect(aRowBelow, &QAction::triggered, this, [this] { tableInsertRow(true); });
    QAction *aColLeft = tableMenu->addAction(tr("Insertar columna a la izquierda"));
    connect(aColLeft, &QAction::triggered, this, [this] { tableInsertColumn(false); });
    QAction *aColRight = tableMenu->addAction(tr("Insertar columna a la derecha"));
    connect(aColRight, &QAction::triggered, this, [this] { tableInsertColumn(true); });

    tableMenu->addSeparator();
    QAction *aDelRow = tableMenu->addAction(tr("Eliminar fila"));
    connect(aDelRow, &QAction::triggered, this, &MainWindow::tableDeleteRow);
    QAction *aDelCol = tableMenu->addAction(tr("Eliminar columna"));
    connect(aDelCol, &QAction::triggered, this, &MainWindow::tableDeleteColumn);

    tableMenu->addSeparator();
    QMenu *alignMenu = tableMenu->addMenu(tr("Alinear columna"));
    QAction *aLeft = alignMenu->addAction(tr("Izquierda"));
    connect(aLeft, &QAction::triggered, this, [this] { tableAlignColumn(Qt::AlignLeft); });
    QAction *aCenter = alignMenu->addAction(tr("Centrar"));
    connect(aCenter, &QAction::triggered, this, [this] { tableAlignColumn(Qt::AlignHCenter); });
    QAction *aRight = alignMenu->addAction(tr("Derecha"));
    connect(aRight, &QAction::triggered, this, [this] { tableAlignColumn(Qt::AlignRight); });

    m_tableActions << aRowAbove << aRowBelow << aColLeft << aColRight
                   << aDelRow << aDelCol << alignMenu->menuAction()
                   << aLeft << aCenter << aRight;
    updateTableActions();  // estado inicial (sin tabla bajo el cursor)

    // --- Menú Ver (vista, zoom y tema) ---
    QMenu *viewMenu = menuBar()->addMenu(tr("&Ver"));

    m_sourceModeAction = viewMenu->addAction(tr("Código fuente Markdown"));
    m_sourceModeAction->setCheckable(true);
    m_sourceModeAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
    m_sourceModeAction->setToolTip(
        m_sourceModeAction->text() + QStringLiteral(" (%1)").arg(
            m_sourceModeAction->shortcut().toString(QKeySequence::NativeText)));
    connect(m_sourceModeAction, &QAction::toggled, this, &MainWindow::toggleSourceMode);

    m_splitAction = viewMenu->addAction(tr("Vista dividida"));
    m_splitAction->setCheckable(true);
    m_splitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));
    m_splitAction->setToolTip(
        tr("Editar WYSIWYG y código fuente a la vez, lado a lado") +
        QStringLiteral(" (%1)").arg(
            m_splitAction->shortcut().toString(QKeySequence::NativeText)));
    connect(m_splitAction, &QAction::toggled, this, &MainWindow::toggleSplitView);

    m_distractionAction = viewMenu->addAction(tr("Sin distracciones"));
    m_distractionAction->setCheckable(true);
    m_distractionAction->setShortcut(QKeySequence(Qt::Key_F11));
    m_distractionAction->setToolTip(
        tr("Pantalla completa, sin barras, con el texto centrado (ESC o F11 para salir)"));
    connect(m_distractionAction, &QAction::toggled, this, &MainWindow::toggleDistractionFree);

    // Esquema (índice): toggleViewAction muestra/oculta el dock y mantiene su
    // marca sincronizada con la visibilidad del panel automáticamente.
    m_outlineAction = m_outline->toggleViewAction();
    m_outlineAction->setText(tr("Esquema"));
    m_outlineAction->setShortcut(QKeySequence(Qt::Key_F9));
    m_outlineAction->setToolTip(
        m_outlineAction->text() + QStringLiteral(" (%1)").arg(
            m_outlineAction->shortcut().toString(QKeySequence::NativeText)));
    viewMenu->addAction(m_outlineAction);

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
        // Franjas del modo sin distracciones: color curado del tema.
        const QColor margin = mdtheme::specFor(id).margin;
        m_editor->setMarginColor(margin);
        m_sourceEditor->setMarginColor(margin);
    });

    themeMenu->addSeparator();
    QAction *warmLightAction = themeMenu->addAction(tr("Luz cálida nocturna"));
    warmLightAction->setCheckable(true);
    warmLightAction->setChecked(m_theme->isWarmLight());
    warmLightAction->setToolTip(
        tr("Tiñe el fondo del editor de tono ámbar según la hora, más cálido de noche"));
    connect(warmLightAction, &QAction::toggled, this,
            [this](bool on) { m_theme->setWarmLight(on); });

    // --- Idioma (se aplica al reiniciar) ---
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

    // --- Menú Ayuda ---
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
    AppSettings::setLanguage(code);
    // El idioma se carga al arrancar (la UI ya está construida): avisamos.
    QMessageBox::information(
        this, tr("Idioma"),
        tr("El idioma se aplicará la próxima vez que abras la aplicación."));
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

    QString text = hasSelection ? cursor.selectedText() : ed->toPlainText();
    // selectedText() usa U+2029 como separador de párrafo; lo normalizamos para
    // contar bien palabras y caracteres.
    text.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));

    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    const int words =
        static_cast<int>(text.split(whitespace, Qt::SkipEmptyParts).size());
    const int chars = static_cast<int>(text.size());

    // %n produce el plural correcto en cada idioma (relevante en polaco/rumano,
    // de reglas complejas, y evita el «1 palabra(s)»).
    QString count = tr("%n palabra(s)", nullptr, words)
                    + QStringLiteral(" · ")
                    + tr("%n carácter(es)", nullptr, chars);
    if (hasSelection)
        count.prepend(tr("Selección: "));
    m_countLabel->setText(count);
}

// ---------------------------------------------------------------------------
// Formato de caracteres (negrita, cursiva, tachado, código)
// ---------------------------------------------------------------------------

void MainWindow::mergeCharFormatOnSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = m_editor->textCursor();
    // Sin selección, se aplica a la palabra bajo el cursor; además se fija el
    // formato de inserción para que el texto que se escriba a continuación
    // herede el formato.
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    m_editor->mergeCurrentCharFormat(format);
    m_editor->setFocus();
    updateFormatActions();
}

void MainWindow::toggleCharFormat(
    const std::function<void(QTextCharFormat &, const QTextCharFormat &)> &mutate)
{
    QTextCharFormat fmt;
    mutate(fmt, m_editor->currentCharFormat());
    mergeCharFormatOnSelection(fmt);
}

// ---------------------------------------------------------------------------
// Formato de bloque (encabezados y listas)
// ---------------------------------------------------------------------------

void MainWindow::applyHeading(int level)
{
    QTextCursor cursor = m_editor->textCursor();
    const int current = cursor.blockFormat().headingLevel();
    const int target = (current == level) ? 0 : level;  // volver a pulsar = quitar

    cursor.beginEditBlock();

    // El nivel de encabezado es lo que exporta toMarkdown() como '#'.
    QTextBlockFormat bf;
    bf.setHeadingLevel(target);
    cursor.mergeBlockFormat(bf);

    // Para el WYSIWYG hay que aplicar también el tamaño y la negrita: usamos
    // FontSizeAdjustment = 4 - nivel, igual que hace setMarkdown() de Qt.
    QTextCharFormat cf;
    cf.setProperty(QTextFormat::FontSizeAdjustment, target > 0 ? 4 - target : 0);
    cf.setFontWeight(target > 0 ? QFont::Bold : QFont::Normal);

    QTextCursor blockCursor = cursor;
    blockCursor.movePosition(QTextCursor::StartOfBlock);
    blockCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    blockCursor.mergeCharFormat(cf);

    cursor.endEditBlock();

    m_editor->mergeCurrentCharFormat(cf);  // para el texto que se escriba luego
    m_editor->setFocus();
    updateFormatActions();
}

void MainWindow::applyList(QTextListFormat::Style style)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList *currentList = cursor.currentList();

    cursor.beginEditBlock();
    if (currentList && currentList->format().style() == style) {
        // Ya es ese tipo de lista: la quitamos (desligamos el bloque). También
        // se quita el marcador de tarea, si lo hubiera, para que no quede un
        // checkbox huérfano fuera de toda lista.
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setObjectIndex(-1);
        bf.setIndent(0);
        bf.setMarker(QTextBlockFormat::MarkerType::NoMarker);
        cursor.setBlockFormat(bf);
    } else {
        QTextListFormat lf;
        lf.setStyle(style);
        cursor.createList(lf);
    }
    cursor.endEditBlock();

    m_editor->setFocus();
    updateFormatActions();
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

void MainWindow::updateFormatActions()
{
    const QTextCharFormat cf = m_editor->currentCharFormat();
    m_boldAction->setChecked(cf.fontWeight() >= QFont::Bold);
    m_italicAction->setChecked(cf.fontItalic());
    m_underlineAction->setChecked(cf.fontUnderline());
    m_strikeAction->setChecked(cf.fontStrikeOut());
    m_codeAction->setChecked(cf.fontFixedPitch());
    m_linkAction->setChecked(cf.isAnchor());

    const QTextCursor cursor = m_editor->textCursor();
    const int level = cursor.blockFormat().headingLevel();
    m_h1Action->setChecked(level == 1);
    m_h2Action->setChecked(level == 2);
    m_h3Action->setChecked(level == 3);
    m_h4Action->setChecked(level == 4);
    m_h5Action->setChecked(level == 5);
    m_h6Action->setChecked(level == 6);

    // En un encabezado, el formato de carácter (negrita, cursiva, etc.) no
    // round-trip-ea a Markdown: el `#` ya implica el peso del título y Qt no
    // serializa cursiva/subrayado dentro de un heading. Para no engañar al
    // usuario (ni dejarle «quitar» la negrita del título), se deshabilitan.
    const bool heading = level > 0;
    for (QAction *a : {m_boldAction, m_italicAction, m_underlineAction,
                       m_strikeAction, m_codeAction})
        a->setEnabled(!heading);

    const QTextList *list = cursor.currentList();
    const QTextListFormat::Style style =
        list ? list->format().style() : QTextListFormat::ListStyleUndefined;
    const QTextBlockFormat bf = cursor.blockFormat();
    const bool isTask = list && bf.marker() != QTextBlockFormat::MarkerType::NoMarker;
    // Una lista de tareas es internamente una ListDisc con marcador: marca solo
    // «tareas», no «viñetas». Una viñeta normal (sin marcador) marca solo «viñetas».
    m_bulletAction->setChecked(style == QTextListFormat::ListDisc && !isTask);
    m_numberedAction->setChecked(style == QTextListFormat::ListDecimal);
    m_taskAction->setChecked(isTask);

    // La sangría de lista solo tiene efecto dentro de una lista.
    m_indentAction->setEnabled(list != nullptr);
    m_outdentAction->setEnabled(list != nullptr);

    m_quoteAction->setChecked(bf.intProperty(QTextFormat::BlockQuoteLevel) > 0);
    m_codeBlockAction->setChecked(bf.hasProperty(QTextFormat::BlockCodeFence));
    // El lenguaje solo se puede fijar con el cursor dentro de un bloque de código.
    m_langAction->setEnabled(bf.hasProperty(QTextFormat::BlockCodeFence));

    updateTableActions();  // el menú Tabla depende de si el cursor está en una
}

// ---------------------------------------------------------------------------
// Citas y bloques de código. La lógica de alternado (reescribir el Markdown del
// bloque y dejar que el lector de Qt construya la estructura, que sí
// round-trip-ea) vive en BlockConstruct; aquí solo se invoca.
// ---------------------------------------------------------------------------

void MainWindow::toggleBlockquote()
{
    Blockquote().toggle(m_editor);
    updateFormatActions();
}

void MainWindow::toggleCodeBlock()
{
    CodeBlock().toggle(m_editor);
    updateFormatActions();
}

void MainWindow::setCodeLanguage()
{
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.blockFormat().hasProperty(QTextFormat::BlockCodeFence)) {
        statusBar()->showMessage(
            tr("Coloca el cursor dentro de un bloque de código"), 3000);
        return;
    }

    const QString current =
        cursor.blockFormat().stringProperty(QTextFormat::BlockCodeLanguage);

    const QStringList langs = {QString(), QStringLiteral("cpp"),
        QStringLiteral("c"), QStringLiteral("python"),
        QStringLiteral("javascript"), QStringLiteral("typescript"),
        QStringLiteral("java"), QStringLiteral("go"), QStringLiteral("rust"),
        QStringLiteral("bash"), QStringLiteral("json"), QStringLiteral("sql"),
        QStringLiteral("html"), QStringLiteral("css")};
    const int currentIndex = qMax(0, langs.indexOf(current));

    bool ok = false;
    const QString lang = QInputDialog::getItem(
        this, tr("Lenguaje del bloque"),
        tr("Lenguaje (vacío = ninguno):"), langs, currentIndex,
        /*editable=*/true, &ok);
    if (!ok)
        return;

    // Aplica el lenguaje a todos los bloques contiguos del bloque de código.
    QTextDocument *doc = m_editor->document();
    int first = cursor.block().blockNumber();
    int last = first;
    while (first > 0 && doc->findBlockByNumber(first - 1).blockFormat()
                            .hasProperty(QTextFormat::BlockCodeFence))
        --first;
    while (last < doc->blockCount() - 1 && doc->findBlockByNumber(last + 1).blockFormat()
                                               .hasProperty(QTextFormat::BlockCodeFence))
        ++last;

    QTextCursor edit(doc);
    edit.beginEditBlock();
    for (int i = first; i <= last; ++i) {
        QTextCursor bc(doc->findBlockByNumber(i));
        QTextBlockFormat add;
        add.setProperty(QTextFormat::BlockCodeLanguage, lang);
        bc.mergeBlockFormat(add);
    }
    edit.endEditBlock();

    m_highlighter->rehighlight();
    m_editor->setFocus();
}

// ---------------------------------------------------------------------------
// Enlaces, imágenes, tablas, regla horizontal, tareas y sangría
// ---------------------------------------------------------------------------

namespace {

// Diálogo de dos campos de texto. Con `browseV2`, el segundo campo lleva un
// botón "..." para elegir un archivo. Devuelve true si el usuario acepta.
bool twoFieldDialog(QWidget *parent, const QString &title,
                    const QString &label1, QString &value1,
                    const QString &label2, QString &value2,
                    bool browseV2 = false)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    auto *form = new QFormLayout(&dialog);

    auto *edit1 = new QLineEdit(value1, &dialog);
    form->addRow(label1, edit1);

    auto *edit2 = new QLineEdit(value2, &dialog);
    if (browseV2) {
        auto *row = new QWidget(&dialog);
        auto *h = new QHBoxLayout(row);
        h->setContentsMargins(0, 0, 0, 0);
        h->addWidget(edit2);
        auto *browse = new QPushButton(QObject::tr("..."), row);
        QObject::connect(browse, &QPushButton::clicked, [parent, edit2] {
            const QString f = QFileDialog::getOpenFileName(
                parent, QObject::tr("Elegir imagen"), QString(),
                QObject::tr("Imágenes (*.png *.jpg *.jpeg *.gif *.bmp *.svg);;Todos (*)"));
            if (!f.isEmpty())
                edit2->setText(f);
        });
        h->addWidget(browse);
        form->addRow(label2, row);
    } else {
        form->addRow(label2, edit2);
    }

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return false;
    value1 = edit1->text();
    value2 = edit2->text();
    return true;
}

} // namespace

void MainWindow::insertLink()
{
    QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat current = m_editor->currentCharFormat();

    QString text = cursor.selectedText();
    QString url = current.isAnchor() ? current.anchorHref() : QString();

    if (!twoFieldDialog(this, tr("Enlace"), tr("Texto:"), text, tr("URL:"), url))
        return;

    QTextCharFormat fmt;
    if (url.isEmpty()) {
        // Sin URL no se crea ningún enlace: se inserta texto plano (un enlace sin
        // destino, [texto](), no aporta nada y confunde). Si tampoco hay texto,
        // no hay nada que insertar.
        if (text.isEmpty())
            return;
        fmt.setAnchor(false);
        fmt.setAnchorHref(QString());
        fmt.setForeground(palette().color(QPalette::Text));
        fmt.setFontUnderline(false);
        cursor.insertText(text, fmt);
    } else {
        fmt.setAnchor(true);
        fmt.setAnchorHref(url);
        fmt.setForeground(palette().color(QPalette::Link));
        fmt.setFontUnderline(true);
        if (text.isEmpty())
            text = url;
        // insertText reemplaza la selección si la hay.
        cursor.insertText(text, fmt);
    }
    m_editor->setFocus();
    updateFormatActions();
}

void MainWindow::insertImage()
{
    QString alt = tr("imagen");
    QString path;
    if (!twoFieldDialog(this, tr("Insertar imagen"),
                        tr("Texto alternativo:"), alt,
                        tr("Ruta o URL:"), path, /*browseV2=*/true))
        return;
    if (path.isEmpty())
        return;

    // Si el documento está guardado y la imagen es local, usar ruta relativa
    // para que el Markdown sea portable.
    const QString currentFile = m_documentIo->currentFile();
    if (!currentFile.isEmpty() && !path.contains(QLatin1String("://"))) {
        const QDir docDir(QFileInfo(currentFile).absolutePath());
        const QString rel = docDir.relativeFilePath(path);
        if (!rel.startsWith(QLatin1String("..")))
            path = rel;
    }

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(QTextDocumentFragment::fromMarkdown(
        QStringLiteral("![%1](%2)").arg(alt, path)));
    cursor.endEditBlock();
    m_editor->setFocus();
}

void MainWindow::pasteImageFromClipboard()
{
    const QMimeData *mime = QApplication::clipboard()->mimeData();
    if (!mime || !handlePastedImage(mime))
        QMessageBox::information(
            this, tr("Pegar imagen"),
            tr("El portapapeles no contiene ninguna imagen."));
}

bool MainWindow::handlePastedImage(const QMimeData *source)
{
    if (!source || !source->hasImage())
        return false;
    const QImage image = qvariant_cast<QImage>(source->imageData());
    if (image.isNull())
        return false;

    // Texto alternativo de la imagen (puede dejarse vacío). Si se cancela, no se
    // inserta nada pero el evento se considera gestionado (no incrustar la imagen).
    bool ok = false;
    const QString alt = QInputDialog::getText(
        this, tr("Pegar imagen"), tr("Texto alternativo:"),
        QLineEdit::Normal, tr("imagen pegada"), &ok);
    if (!ok)
        return true;

    // Nombre con marca de tiempo para evitar colisiones; si aun así existe (mismo
    // segundo), se añade un sufijo incremental.
    const QString stamp =
        QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss"));
    const auto fileNameFor = [&stamp](int n) {
        return n == 0 ? QStringLiteral("imagen-%1.png").arg(stamp)
                      : QStringLiteral("imagen-%1-%2.png").arg(stamp).arg(n);
    };

    QString savePath;  // ruta absoluta donde se escribe el PNG
    QString mdPath;    // lo que va dentro de ![](...)

    const QString currentFile = m_documentIo->currentFile();
    if (!currentFile.isEmpty()) {
        // Documento guardado: junto al .md, con ruta relativa para que sea
        // portable (igual criterio que insertImage()).
        const QDir dir(QFileInfo(currentFile).absolutePath());
        int n = 0;
        do {
            savePath = dir.filePath(fileNameFor(n++));
        } while (QFileInfo::exists(savePath));
        mdPath = dir.relativeFilePath(savePath);
    } else {
        // Documento sin guardar: no hay carpeta de referencia; se pide dónde
        // guardarla y se inserta la ruta elegida.
        savePath = QFileDialog::getSaveFileName(
            this, tr("Guardar imagen pegada"),
            QDir::home().filePath(fileNameFor(0)), tr("Imagen PNG (*.png)"));
        if (savePath.isEmpty())
            return true;  // cancelado: gestionado (no incrustar la imagen)
        mdPath = savePath;
    }

    if (!image.save(savePath, "PNG")) {
        QMessageBox::warning(
            this, tr("Pegar imagen"),
            tr("No se pudo guardar la imagen en «%1».").arg(savePath));
        return true;
    }

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(QTextDocumentFragment::fromMarkdown(
        QStringLiteral("![%1](%2)").arg(alt, mdPath)));
    cursor.endEditBlock();
    m_editor->setFocus();
    return true;
}

void MainWindow::insertTable()
{
    // Un solo diálogo con columnas y filas a la vez (más legible que dos seguidos).
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Insertar tabla"));
    auto *colsSpin = new QSpinBox(&dlg);
    colsSpin->setRange(1, 20);
    colsSpin->setValue(2);
    auto *rowsSpin = new QSpinBox(&dlg);
    rowsSpin->setRange(1, 100);
    rowsSpin->setValue(2);
    auto *form = new QFormLayout;
    form->addRow(tr("Columnas:"), colsSpin);
    form->addRow(tr("Filas de datos:"), rowsSpin);
    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    auto *layout = new QVBoxLayout(&dlg);
    layout->addLayout(form);
    layout->addWidget(buttons);
    if (dlg.exec() != QDialog::Accepted)
        return;
    const int cols = colsSpin->value();
    const int rows = rowsSpin->value();

    QString header = QStringLiteral("|");
    QString separator = QStringLiteral("|");
    for (int c = 0; c < cols; ++c) {
        header += QStringLiteral("   |");
        separator += QStringLiteral("---|");
    }
    QString row = QStringLiteral("|");
    for (int c = 0; c < cols; ++c)
        row += QStringLiteral("   |");

    QString md = header + QLatin1Char('\n') + separator + QLatin1Char('\n');
    for (int r = 0; r < rows; ++r)
        md += row + QLatin1Char('\n');

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.insertFragment(QTextDocumentFragment::fromMarkdown(md));
    cursor.endEditBlock();
    styleTables();  // que la tabla recién creada muestre sus bordes
    m_editor->setFocus();
}

bool MainWindow::askFormula(QString *tex, bool *block,
                            const QString &initialTex, bool initialBlock)
{
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Insertar fórmula"));
    auto *form = new QFormLayout;
    auto *edit = new QPlainTextEdit(&dlg);
    edit->setPlaceholderText(tr("Expresión TeX, p. ej. E = mc^2"));
    edit->setMinimumWidth(360);
    edit->setPlainText(initialTex);
    auto *combo = new QComboBox(&dlg);
    combo->addItem(tr("En línea ($...$)"));
    combo->addItem(tr("Bloque ($$...$$)"));
    combo->setCurrentIndex(initialBlock ? 1 : 0);
    // Vista previa en vivo: alimentamos un QTextEdit (no un QLabel) con los
    // runs que renderTexAsRuns produce, para que los super/subíndices de Qt
    // se vean tal cual aparecerán en el editor.
    auto *preview = new QTextEdit(&dlg);
    preview->setReadOnly(true);
    preview->setMinimumWidth(360);
    preview->setMaximumHeight(70);
    preview->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    preview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    preview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFont previewFont = preview->font();
    previewFont.setPointSizeF(previewFont.pointSizeF() * 1.25);
    preview->setFont(previewFont);
    auto updatePreview = [edit, preview] {
        preview->clear();
        const QString tex = edit->toPlainText().trimmed();
        if (tex.isEmpty()) {
            preview->setPlainText(QStringLiteral("—"));
            return;
        }
        QTextCursor c(preview->document());
        // El char-format base lleva las propiedades de math (cursiva, etc.);
        // no importa que el preview no las consuma — solo nos interesan los
        // runs con AlignSuperScript/SubScript.
        const QTextCharFormat base = mdmath::mathCharFormat(tex, /*block=*/false);
        for (const mdmath::MathRun &r : mdmath::renderTexAsRuns(tex, base))
            c.insertText(r.text, r.fmt);
    };
    QObject::connect(edit, &QPlainTextEdit::textChanged, &dlg, updatePreview);
    updatePreview();
    form->addRow(tr("TeX:"), edit);
    form->addRow(tr("Tipo:"), combo);
    form->addRow(tr("Vista previa:"), preview);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    auto *layout = new QVBoxLayout(&dlg);
    layout->addLayout(form);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() != QDialog::Accepted)
        return false;
    const QString text = edit->toPlainText().trimmed();
    if (text.isEmpty())
        return false;
    *tex = text;
    *block = combo->currentIndex() == 1;
    return true;
}

void MainWindow::insertFormula()
{
    QString tex;
    bool block = false;
    if (!askFormula(&tex, &block))
        return;

    // Inserta la fórmula como una secuencia de runs (varios fragmentos con
    // super/subíndice de verdad, no solo Unicode). Todos comparten el TeX en
    // la propiedad MathTex, lo que permite agruparlos al serializar y al
    // editar con doble clic.
    const QTextCharFormat base = mdmath::mathCharFormat(tex, block);
    const QList<mdmath::MathRun> runs = mdmath::renderTexAsRuns(tex, base);

    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    if (block && !cursor.atBlockStart())
        cursor.insertText(QStringLiteral("\n"));
    for (const mdmath::MathRun &r : runs)
        cursor.insertText(r.text, r.fmt);
    if (block)
        cursor.insertText(QStringLiteral("\n"), QTextCharFormat());
    cursor.endEditBlock();
    // Restaura el formato de inserción para que lo que el usuario teclee a
    // continuación no herede las propiedades de math.
    m_editor->setCurrentCharFormat(QTextCharFormat());
    m_editor->setFocus();
}

void MainWindow::guardPasteAgainstMath()
{
    QTextCursor cursor = m_editor->textCursor();
    const QList<QPair<int, int>> groups = mdmath::mathGroupBounds(m_editor->document());
    if (groups.isEmpty())
        return;

    int newStart = cursor.selectionStart();
    int newEnd   = cursor.selectionEnd();
    const bool hadSelection = cursor.hasSelection();

    if (!hadSelection) {
        // Sin selección: si el cursor está dentro de un grupo (no en su
        // borde), seleccionamos el grupo entero para que el pegado lo
        // reemplace en bloque en vez de incrustarse a mitad de fórmula.
        const int pos = cursor.position();
        for (const auto &g : groups) {
            if (pos > g.first && pos < g.second) {
                newStart = g.first;
                newEnd = g.second;
                break;
            }
        }
    } else {
        // Con selección: extiéndela a los grupos que solape.
        for (const auto &g : groups) {
            if (g.second <= newStart || g.first >= newEnd)
                continue;  // sin solape
            if (g.first < newStart) newStart = g.first;
            if (g.second > newEnd)  newEnd = g.second;
        }
    }

    if (newStart != cursor.selectionStart() || newEnd != cursor.selectionEnd()) {
        QTextCursor c = m_editor->textCursor();
        c.setPosition(newStart);
        c.setPosition(newEnd, QTextCursor::KeepAnchor);
        m_editor->setTextCursor(c);
    }
}

bool MainWindow::handleMathKeyPress(QKeyEvent *event)
{
    QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection())
        return false;

    QTextDocument *doc = m_editor->document();
    const int pos = cursor.position();
    const int docLen = doc->characterCount() - 1;
    auto fmtAt = [doc](int p) {
        QTextCursor c(doc);
        c.setPosition(p);
        return c.charFormat();
    };

    const QTextCharFormat before = pos > 0 ? fmtAt(pos - 1) : QTextCharFormat();
    const QTextCharFormat after  = pos < docLen ? fmtAt(pos) : QTextCharFormat();
    const bool mathBefore = before.boolProperty(mdmath::IsMathProperty);
    const bool mathAfter  = after.boolProperty(mdmath::IsMathProperty);

    if (!mathBefore && !mathAfter)
        return false;  // sin contacto con math: pasa al editor

    const bool insideMath = mathBefore && mathAfter
        && before.property(mdmath::MathTexProperty).toString()
               == after.property(mdmath::MathTexProperty).toString();

    // Expande [start,end) al grupo de fragmentos con el mismo MathTex que toca
    // en `pivot`. `pivot` apunta al char con IsMath (antes o después del cursor).
    auto groupBounds = [&](int pivot, int *start, int *end) {
        const QString tex = fmtAt(pivot).property(mdmath::MathTexProperty).toString();
        int s = pivot, e = pivot + 1;
        while (s > 0) {
            const QTextCharFormat f = fmtAt(s - 1);
            if (!f.boolProperty(mdmath::IsMathProperty)
                || f.property(mdmath::MathTexProperty).toString() != tex)
                break;
            --s;
        }
        while (e < docLen) {
            const QTextCharFormat f = fmtAt(e);
            if (!f.boolProperty(mdmath::IsMathProperty)
                || f.property(mdmath::MathTexProperty).toString() != tex)
                break;
            ++e;
        }
        *start = s; *end = e;
    };

    auto removeRange = [&](int start, int end) {
        QTextCursor c(doc);
        c.setPosition(start);
        c.setPosition(end, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        m_editor->setCurrentCharFormat(QTextCharFormat());
    };

    // Backspace tocando el borde derecho (mathBefore): borra el grupo entero.
    if (event->key() == Qt::Key_Backspace && mathBefore) {
        int s, e;
        groupBounds(pos - 1, &s, &e);
        removeRange(s, e);
        return true;
    }
    // Delete tocando el borde izquierdo (mathAfter): borra el grupo entero.
    if (event->key() == Qt::Key_Delete && mathAfter) {
        int s, e;
        groupBounds(pos, &s, &e);
        removeRange(s, e);
        return true;
    }

    // Tecla imprimible dentro de la fórmula: se ignora y se avisa.
    if (insideMath && !event->text().isEmpty() && event->text().at(0).isPrint()) {
        statusBar()->showMessage(
            tr("Doble clic en la fórmula para editarla (Ctrl+Shift+F para insertar otra)."),
            3000);
        return true;
    }

    // Tecla imprimible en el BORDE de una fórmula (no dentro): si dejáramos que
    // la insertara el editor, Qt heredaría el formato del carácter contiguo y el
    // texto nuevo quedaría marcado como parte de la fórmula (mismo color y, peor,
    // la serialización se lo tragaría, sin aparecer en el fuente). Lo insertamos
    // nosotros con un formato limpio para que sea texto normal. Se excluyen los
    // atajos (Ctrl/Alt/Meta), que no son escritura.
    const bool typing = !event->text().isEmpty() && event->text().at(0).isPrint()
        && !(event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier));
    if (typing) {
        const QTextCharFormat clean;  // sin IsMath/MathTex ni super/subíndice
        cursor.insertText(event->text(), clean);
        m_editor->setTextCursor(cursor);
        m_editor->setCurrentCharFormat(clean);
        return true;
    }

    return false;
}

bool MainWindow::editFormulaAt(const QPoint &viewportPos)
{
    const int pos = m_editor->cursorForPosition(viewportPos).position();
    // El char-format del carácter justo antes del cursor (o del actual, si está
    // al inicio del documento) decide si estamos sobre una fórmula.
    QTextCursor probe(m_editor->document());
    probe.setPosition(pos > 0 ? pos - 1 : 0);
    QTextCharFormat cf = probe.charFormat();
    if (!cf.boolProperty(mdmath::IsMathProperty)) {
        probe.setPosition(pos);
        cf = probe.charFormat();
        if (!cf.boolProperty(mdmath::IsMathProperty))
            return false;
    }
    // Expande hasta cubrir todo el fragmento de la fórmula (los caracteres con
    // las mismas propiedades IsMath/MathTex).
    const QString tex = cf.property(mdmath::MathTexProperty).toString();
    const bool block = cf.boolProperty(mdmath::MathBlockProperty);
    int start = pos > 0 ? pos - 1 : 0;
    int end = start + 1;
    QTextCursor walker(m_editor->document());
    while (start > 0) {
        walker.setPosition(start - 1);
        const QTextCharFormat f = walker.charFormat();
        if (!f.boolProperty(mdmath::IsMathProperty)
            || f.property(mdmath::MathTexProperty).toString() != tex)
            break;
        --start;
    }
    const int docLen = m_editor->document()->characterCount();
    while (end < docLen - 1) {
        walker.setPosition(end);
        const QTextCharFormat f = walker.charFormat();
        if (!f.boolProperty(mdmath::IsMathProperty)
            || f.property(mdmath::MathTexProperty).toString() != tex)
            break;
        ++end;
    }

    QString newTex;
    bool newBlock = block;
    if (!askFormula(&newTex, &newBlock, tex, block))
        return true;  // canceló pero atendimos el clic

    const QTextCharFormat base = mdmath::mathCharFormat(newTex, newBlock);
    const QList<mdmath::MathRun> runs = mdmath::renderTexAsRuns(newTex, base);
    QTextCursor c(m_editor->document());
    c.beginEditBlock();
    c.setPosition(start);
    c.setPosition(end, QTextCursor::KeepAnchor);
    c.removeSelectedText();
    for (const mdmath::MathRun &r : runs)
        c.insertText(r.text, r.fmt);
    c.endEditBlock();
    m_editor->setCurrentCharFormat(QTextCharFormat());
    return true;
}

// ---------------------------------------------------------------------------
// Edición de tablas. Operan sobre el QTextTable bajo el cursor. La alineación se
// fija en el formato de bloque de todas las celdas de la columna y se preserva
// al guardar mediante mdtable (toMarkdown de Qt no la emite por sí solo).
// ---------------------------------------------------------------------------

void MainWindow::tableInsertRow(bool below)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const QTextTableCell cell = table->cellAt(cursor);
    table->insertRows(cell.row() + (below ? 1 : 0), 1);
    m_editor->setFocus();
}

void MainWindow::tableInsertColumn(bool right)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const QTextTableCell cell = table->cellAt(cursor);
    table->insertColumns(cell.column() + (right ? 1 : 0), 1);
    m_editor->setFocus();
}

void MainWindow::tableDeleteRow()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table || table->rows() <= 1)  // dejar al menos la fila de cabecera
        return;
    table->removeRows(table->cellAt(cursor).row(), 1);
    m_editor->setFocus();
}

void MainWindow::tableDeleteColumn()
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table || table->columns() <= 1)
        return;
    table->removeColumns(table->cellAt(cursor).column(), 1);
    m_editor->setFocus();
}

void MainWindow::tableAlignColumn(Qt::Alignment alignment)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextTable *table = cursor.currentTable();
    if (!table)
        return;
    const int col = table->cellAt(cursor).column();

    cursor.beginEditBlock();
    // Se alinean todas las filas (incluida la cabecera) para que la columna se
    // vea homogénea y mdtable detecte la alineación de forma fiable.
    for (int r = 0; r < table->rows(); ++r) {
        const QTextTableCell c = table->cellAt(r, col);
        QTextCursor cc = c.firstCursorPosition();
        cc.setPosition(c.lastCursorPosition().position(), QTextCursor::KeepAnchor);
        QTextBlockFormat bf;
        bf.setAlignment(alignment);
        cc.mergeBlockFormat(bf);
    }
    cursor.endEditBlock();
    // El contenido cambió (la alineación va al Markdown vía mdtable): refresca el
    // estado de "modificado".
    setWindowModified(m_documentIo->isModified());
    m_editor->setFocus();
}

void MainWindow::updateTableActions()
{
    // En vista dividida, las acciones de tabla solo cuando el foco está en el
    // WYSIWYG (es donde viven las tablas); con el foco en el fuente, inhabilitadas.
    const bool wysiwygActive = !m_sourceMode && (!m_splitMode || !m_sourceEditor->hasFocus());
    const bool inTable = wysiwygActive && m_editor->textCursor().currentTable() != nullptr;
    for (QAction *a : m_tableActions)
        a->setEnabled(inTable);
}

void MainWindow::insertHorizontalRule()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertBlock();
    // La regla horizontal es un bloque con esta propiedad (lo que produce el
    // lector de Markdown de Qt y exporta como '- - -').
    QTextBlockFormat hr;
    hr.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth,
                   QTextLength(QTextLength::PercentageLength, 100));
    cursor.setBlockFormat(hr);
    // Bloque normal a continuación para seguir escribiendo.
    cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    cursor.endEditBlock();
    m_editor->setFocus();
}

void MainWindow::toggleTaskItem()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    if (!cursor.currentList()) {
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        cursor.createList(lf);
    }
    QTextBlockFormat bf = cursor.blockFormat();
    const bool isTask = bf.marker() != QTextBlockFormat::MarkerType::NoMarker;
    bf.setMarker(isTask ? QTextBlockFormat::MarkerType::NoMarker
                        : QTextBlockFormat::MarkerType::Unchecked);
    cursor.setBlockFormat(bf);
    cursor.endEditBlock();
    m_editor->setFocus();
    updateFormatActions();
}

void MainWindow::indentList()
{
    changeListIndent(+1);
}

void MainWindow::outdentList()
{
    changeListIndent(-1);
}

void MainWindow::changeListIndent(int delta)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList *list = cursor.currentList();
    if (!list)
        return;  // la sangría de lista solo aplica dentro de una lista

    QTextListFormat lf = list->format();
    const int newIndent = qMax(1, lf.indent() + delta);
    if (newIndent == lf.indent())
        return;
    lf.setIndent(newIndent);
    cursor.createList(lf);  // crea/ajusta la (sub)lista al nuevo nivel
    m_editor->setFocus();
    updateFormatActions();
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
    f.setPointSizeF(qMax(qreal(1), m_baseFontPointSize + m_zoomDelta));
    m_editor->setFont(f);

    applyChromeZoom();
    AppSettings::setZoomLevel(m_zoomDelta);  // se recuerda para la próxima sesión
}

void MainWindow::applyMenuFontScale()
{
    if (m_baseMenuPointSize <= 0)
        return;
    QFont mf = QApplication::font("QMenuBar");
    mf.setPointSizeF(qMax(qreal(1), m_baseMenuPointSize + m_zoomDelta));
    // Cambiar la fuente por clase obliga a Qt a recalcular el sizeHint de los
    // popups la próxima vez que se muestren — sin esto, las anchuras de las
    // QAction quedan congeladas al tamaño con el que se construyeron.
    QApplication::setFont(mf, "QMenuBar");
    QApplication::setFont(mf, "QMenu");
}

void MainWindow::forceMenuWidths()
{
    // Qt 6.8 con la plataforma gtk3 cachea las anchuras de las QAction al
    // primer cálculo, y los cambios de fuente posteriores no las invalidan:
    // ítems largos como «Insertar columna a la izquierda» o «Acerca de» se
    // ven elided cuando la fuente del zoom es mayor que la base. Calculamos
    // a mano el ancho mínimo que necesita cada menú con la QFontMetrics actual
    // (texto sin el mnemonic `&` + columnas de atajo y flecha de submenú) y se
    // lo aplicamos como minimumWidth: Qt no puede entonces elidir.
    for (QMenu *menu : findChildren<QMenu *>()) {
        const QFontMetrics fm(menu->font());
        int textMax = 0;
        int shortcutMax = 0;
        bool anyShortcut = false;
        bool anySubmenu = false;
        bool anyCheckable = false;
        for (const QAction *a : menu->actions()) {
            if (a->isSeparator())
                continue;
            QString t = a->text();
            t.remove(QLatin1Char('&'));  // mnemonic prefix no se pinta
            textMax = qMax(textMax, fm.horizontalAdvance(t));
            const QKeySequence sc = a->shortcut();
            if (!sc.isEmpty()) {
                anyShortcut = true;
                shortcutMax = qMax(
                    shortcutMax,
                    fm.horizontalAdvance(sc.toString(QKeySequence::NativeText)));
            }
            if (a->menu())
                anySubmenu = true;
            if (a->isCheckable())
                anyCheckable = true;
        }
        if (textMax <= 0)
            continue;
        // Estimación generosa de paddings/separadores entre columnas. Mejor
        // pasarse y dejar el menú un pelín más ancho que quedarse corto y
        // volver al elide: el estilo del SO añade su propio padding encima,
        // así que el ancho real terminará un poco mayor que este mínimo.
        const int em = fm.horizontalAdvance(QChar('M'));
        int width = textMax + em * 3;      // padding lateral + icono opcional
        if (anyCheckable)
            width += em * 2;               // columna del indicador de check
        if (anyShortcut)
            width += shortcutMax + em * 4; // separación texto–atajo
        if (anySubmenu)
            width += em * 2;               // flecha de submenú
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
        f.setPointSizeF(qMax(qreal(1), base + m_zoomDelta));
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
    scale(m_sourceEditor, m_baseSourceFontPointSize);
    scale(m_outline, m_baseOutlinePointSize);
}

// ---------------------------------------------------------------------------
// Modo sin distracciones
// ---------------------------------------------------------------------------

void MainWindow::toggleDistractionFree(bool on)
{
    if (on == m_distractionFree)
        return;
    m_distractionFree = on;

    if (on) {
        // El modo sin distracciones es de columna única: salimos de la vista
        // dividida (si estaba activa) para concentrarse en la escritura.
        if (m_splitMode)
            toggleSplitView(false);
        // Recuerda el estado para restaurarlo al salir (y para persistirlo si
        // se cierra la app dentro del modo, en vez del estado a pantalla
        // completa con las barras ocultas).
        m_wasMaximized = isMaximized();
        m_findBarWasVisible = m_findBar->isVisible();
        m_normalOutlineWidth = m_outline->width();  // para restaurar el dock al salir
        m_preDistractionGeometry = saveGeometry();
        m_preDistractionState = saveState();

        menuBar()->hide();
        m_formatToolBar->hide();
        m_findBar->hide();
        statusBar()->hide();
        // El esquema no se oculta: sigue pegado a la izquierda del área visible
        // (conserva la visibilidad que tuviera al entrar al modo). Su barra de
        // título sí se oculta: con el dock ensanchado por el relleno izquierdo,
        // dejarla visible la estiraría hasta el borde de la pantalla.
        m_outline->setTitleBarWidget(new QWidget(m_outline));

        m_editor->setReadingColumnWidth(kReadingColumn);
        m_sourceEditor->setReadingColumnWidth(kReadingColumn);

        m_escShortcut->setEnabled(true);
        showFullScreen();
    } else {
        m_escShortcut->setEnabled(false);

        m_editor->setReadingColumnWidth(0);
        m_sourceEditor->setReadingColumnWidth(0);

        menuBar()->show();
        m_formatToolBar->show();
        statusBar()->show();
        // Devuelve al dock su barra de título nativa.
        if (QWidget *tb = m_outline->titleBarWidget()) {
            m_outline->setTitleBarWidget(nullptr);
            tb->deleteLater();
        }
        if (m_findBarWasVisible)
            m_findBar->show();

        if (m_wasMaximized)
            showMaximized();
        else
            showNormal();
    }
    updateDistractionLayout();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    if (m_distractionFree)
        updateDistractionLayout();  // el tamaño en pantalla completa llega aquí
}

void MainWindow::updateDistractionLayout()
{
    // Esquema visible y acoplado: centra el bloque [esquema | columna] en
    // pantalla. La columna se pega al borde derecho del dock (alineada a la
    // izquierda) y el dock se ensancha con un relleno izquierdo igual al hueco
    // que queda a la derecha de la columna, de modo que el conjunto quede
    // simétrico (franjas oscuras iguales a ambos lados).
    const bool group = m_distractionFree && m_outline->isVisible()
                       && !m_outline->isFloating();

    m_editor->setColumnLeftAligned(group);
    m_sourceEditor->setColumnLeftAligned(group);

    if (!group) {
        m_outline->setLeftPadding(0);
        // Al salir del modo, devuelve al dock su ancho previo (resizeDocks lo
        // había ensanchado para el relleno).
        if (!m_distractionFree && m_normalOutlineWidth > 0) {
            resizeDocks({m_outline}, {m_normalOutlineWidth}, Qt::Horizontal);
            m_normalOutlineWidth = 0;
        }
        return;
    }

    const int total = m_outline->width() + m_splitView->width();  // espacio dock+central
    const int leftPad = qMax(0, (total - kOutlineTreeWidth - kReadingColumn) / 2);
    const int dockWidth = leftPad + kOutlineTreeWidth;

    m_outline->setLeftPadding(leftPad);
    if (m_outline->width() != dockWidth)  // evita reentrar en el layout sin necesidad
        resizeDocks({m_outline}, {dockWidth}, Qt::Horizontal);
}

// ---------------------------------------------------------------------------
// Vista de código Markdown crudo
// ---------------------------------------------------------------------------

QTextEdit *MainWindow::activeEditor() const
{
    // En fuente a pantalla completa manda el editor de fuente; en vista dividida,
    // el que tenga el foco; en WYSIWYG, el editor.
    if (m_sourceMode)
        return m_sourceEditor;
    if (m_splitMode && m_sourceEditor->hasFocus())
        return m_sourceEditor;
    return m_editor;
}

void MainWindow::updateEditorVisibility()
{
    m_editor->setVisible(!m_sourceMode);                       // oculto solo en fuente-completo
    m_sourceEditor->setVisible(m_sourceMode || m_splitMode);   // visible en fuente y en dividido
}

void MainWindow::updateActionsForFocus()
{
    if (!m_splitMode)
        return;  // en los otros modos lo gestiona toggleSourceMode
    const bool wysiwygFocused = !m_sourceEditor->hasFocus();
    // El formato/inserción solo tiene sentido sobre el WYSIWYG.
    for (QAction *a : m_wysiwygActions)
        a->setEnabled(wysiwygFocused);
    // La búsqueda actúa sobre el panel donde está el cursor.
    m_findBar->setEditor(wysiwygFocused ? m_editor : m_sourceEditor);
    if (wysiwygFocused)
        updateFormatActions();  // refina habilitado (encabezado, lista, fence…) y marcas
    else
        updateTableActions();   // deja las de tabla inhabilitadas con el foco en el fuente
}

void MainWindow::commitSourceToDocument()
{
    // Vuelca los cambios del editor de fuente al documento WYSIWYG, que es el
    // que se serializa al guardar/exportar. setMarkdown re-renderiza y deja el
    // modelo al día. Funciona tanto en modo fuente como en vista dividida (el
    // disparador es m_sourceDirty, no el modo).
    if (m_sourceDirty) {
        m_syncing = true;
        m_editor->setMarkdown(mdmath::protectMath(m_sourceEditor->toPlainText()));
        mdmath::renderMathInDocument(m_editor->document());
        styleTables();
        m_syncing = false;
        m_sourceDirty = false;
    }
}

void MainWindow::toggleSourceMode(bool on)
{
    if (on == m_sourceMode)
        return;

    if (on) {
        // Fuente a pantalla completa y vista dividida son excluyentes.
        if (m_splitMode)
            toggleSplitView(false);
        // Entrar: vuelca el Markdown actual al editor de fuente (con la alineación
        // de tablas preservada, igual que al guardar).
        m_syncing = true;
        m_sourceEditor->setPlainText(mdtable::documentMarkdown(m_editor->document()));
        m_syncing = false;
        m_findBar->setEditor(m_sourceEditor);
    } else {
        // Salir: aplica los cambios del fuente y vuelve al WYSIWYG.
        commitSourceToDocument();
        m_findBar->setEditor(m_editor);
        m_theme->recolorLinks();  // los enlaces recargados toman el color del tema
    }
    m_sourceMode = on;
    m_sourceDirty = false;
    updateEditorVisibility();

    // Las acciones de formato/inserción no aplican al texto plano: se desactivan
    // (también sus atajos) en modo fuente.
    for (QAction *a : m_wysiwygActions)
        a->setEnabled(!on);
    updateTableActions();  // y las de tabla (en fuente, siempre inhabilitadas)

    // El índice se nutre de la estructura del documento WYSIWYG, no del texto
    // plano: se deshabilita en modo fuente y se reconstruye al volver (el
    // contenido pudo cambiar en el editor de fuente).
    m_outline->setEnabled(!on);
    if (!on)
        m_outline->rebuild(m_editor->document());

    if (m_sourceModeAction->isChecked() != on)
        m_sourceModeAction->setChecked(on);  // mantiene el menú en sincronía
    activeEditor()->setFocus();
    updateWordCount();
}

void MainWindow::toggleSplitView(bool on)
{
    if (on == m_splitMode)
        return;

    if (on) {
        // Vista dividida y fuente a pantalla completa son excluyentes.
        if (m_sourceMode)
            toggleSourceMode(false);
        // Rellena el panel de fuente con el Markdown actual del documento.
        m_syncing = true;
        m_sourceEditor->setPlainText(mdtable::documentMarkdown(m_editor->document()));
        m_syncing = false;
        m_sourceDirty = false;
    } else {
        // Salir: vuelca cualquier edición pendiente del fuente al documento.
        commitSourceToDocument();
        m_theme->recolorLinks();
    }
    m_splitMode = on;
    updateEditorVisibility();

    if (on) {
        // Si el panel de fuente quedó sin anchura (p. ej. estaba oculto y no había
        // estado guardado), reparte el espacio a partes iguales.
        const QList<int> sizes = m_splitView->sizes();
        if (sizes.size() == 2 && (sizes[0] < 50 || sizes[1] < 50)) {
            const int half = qMax(1, m_splitView->width() / 2);
            m_splitView->setSizes({half, half});
        }
    } else {
        // Al volver a WYSIWYG, reactiva el formato (pudo quedar deshabilitado si
        // el foco estaba en el fuente) y refresca su estado.
        for (QAction *a : m_wysiwygActions)
            a->setEnabled(true);
        m_findBar->setEditor(m_editor);
        updateFormatActions();
    }

    if (m_splitAction->isChecked() != on)
        m_splitAction->setChecked(on);  // mantiene el menú en sincronía
    m_editor->setFocus();
    updateWordCount();
}

void MainWindow::syncSourceFromDocument()
{
    // WYSIWYG -> fuente. La guarda de "no pisar el panel con foco" la aplica el
    // disparador del temporizador; flushPendingSync llama aquí directamente al
    // salir del WYSIWYG (cuando el foco ya está en el fuente) para dejarlo al día
    // justo antes de que el usuario lo edite.
    if (!m_splitMode)
        return;
    m_syncing = true;
    // Preserva cursor/selección y scroll: setPlainText los reinicia, y al cambiar
    // de panel el usuario espera encontrar el cursor donde lo dejó. Se guarda por
    // offset y se reajusta (clamp) al nuevo tamaño, porque el round-trip puede
    // normalizar ligeramente el Markdown.
    const QTextCursor before = m_sourceEditor->textCursor();
    const int anchor = before.anchor();
    const int pos = before.position();
    const int scroll = m_sourceEditor->verticalScrollBar()->value();

    m_sourceEditor->setPlainText(mdtable::documentMarkdown(m_editor->document()));

    const int last = m_sourceEditor->document()->characterCount() - 1;
    QTextCursor restored = m_sourceEditor->textCursor();
    restored.setPosition(qBound(0, anchor, last));
    restored.setPosition(qBound(0, pos, last),
                         pos == anchor ? QTextCursor::MoveAnchor : QTextCursor::KeepAnchor);
    m_sourceEditor->setTextCursor(restored);
    m_sourceEditor->verticalScrollBar()->setValue(scroll);  // no saltar la vista
    m_syncing = false;
    m_sourceDirty = false;  // el fuente acaba de igualarse al documento
}

void MainWindow::syncDocumentFromSource()
{
    // fuente -> WYSIWYG. Solo si seguimos en split y el WYSIWYG no tiene el foco.
    if (!m_splitMode || m_editor->hasFocus() || !m_sourceDirty)
        return;
    const int scroll = m_editor->verticalScrollBar()->value();
    commitSourceToDocument();  // gestiona m_syncing y re-renderiza
    m_editor->verticalScrollBar()->setValue(scroll);  // no saltar la vista
}

void MainWindow::flushPendingSync(QWidget *losingFocus)
{
    if (!m_splitMode)
        return;
    // Al salir del fuente con un volcado pendiente, aplícalo ya (para que el
    // WYSIWYG esté al día al llegar). Al salir del WYSIWYG, refresca el fuente.
    if (losingFocus == m_sourceEditor && m_syncToDocTimer->isActive()) {
        m_syncToDocTimer->stop();
        const int scroll = m_editor->verticalScrollBar()->value();
        commitSourceToDocument();
        m_editor->verticalScrollBar()->setValue(scroll);
    } else if (losingFocus == m_editor && m_syncToSourceTimer->isActive()) {
        m_syncToSourceTimer->stop();
        syncSourceFromDocument();
    }
}

// ---------------------------------------------------------------------------
// Exportar
// ---------------------------------------------------------------------------

bool MainWindow::print()
{
    commitSourceToDocument();  // en modo fuente, imprime el contenido al día
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);
    dialog.setWindowTitle(tr("Imprimir"));
    if (dialog.exec() != QDialog::Accepted)
        return false;
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    flat->print(&printer);
    statusBar()->showMessage(tr("Documento enviado a la impresora."), 4000);
    return true;
}

bool MainWindow::exportPdf()
{
    commitSourceToDocument();
    const QString currentFile = m_documentIo->currentFile();
    const QString suggested = currentFile.isEmpty()
        ? QDir::homePath() + QStringLiteral("/documento.pdf")
        : QFileInfo(currentFile).absolutePath() + QLatin1Char('/') +
              QFileInfo(currentFile).completeBaseName() + QStringLiteral(".pdf");

    QString path = QFileDialog::getSaveFileName(
        this, tr("Exportar a PDF"), suggested, tr("PDF (*.pdf)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".pdf");

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(path);
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    flat->print(&printer);

    statusBar()->showMessage(tr("Exportado a PDF: %1").arg(path), 4000);
    return true;
}

bool MainWindow::exportHtml()
{
    commitSourceToDocument();
    const QString currentFile = m_documentIo->currentFile();
    const QString suggested = currentFile.isEmpty()
        ? QDir::homePath() + QStringLiteral("/documento.html")
        : QFileInfo(currentFile).absolutePath() + QLatin1Char('/') +
              QFileInfo(currentFile).completeBaseName() + QStringLiteral(".html");

    QString path = QFileDialog::getSaveFileName(
        this, tr("Exportar a HTML"), suggested, tr("HTML (*.html *.htm)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".html");

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo escribir:\n%1\n\n%2")
                                 .arg(path, file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    out << flat->toHtml();
    file.close();

    statusBar()->showMessage(tr("Exportado a HTML: %1").arg(path), 4000);
    return true;
}

QString MainWindow::exportTitle() const
{
    return mdexport::frontMatterValue(m_documentIo->frontMatter(),
                                      QStringLiteral("title"));
}

bool MainWindow::chooseExportLanguage(mdexport::Language *out)
{
    // Idioma por defecto: `lang`/`language` del front matter; si no, el de la
    // app; si tampoco, el del sistema.
    const QString fm = m_documentIo->frontMatter();
    QString code = mdexport::frontMatterValue(fm, QStringLiteral("lang"));
    if (code.isEmpty())
        code = mdexport::frontMatterValue(fm, QStringLiteral("language"));
    if (code.isEmpty())
        code = AppSettings::language();
    if (code.isEmpty())
        code = QLocale::system().name();  // p. ej. "es_ES"
    const mdexport::Language def = mdexport::languageForCode(code);

    const QList<mdexport::Language> langs = mdexport::languages();
    QStringList names;
    int current = 0;
    for (int i = 0; i < langs.size(); ++i) {
        names << langs.at(i).name;
        if (langs.at(i).code == def.code)
            current = i;
    }

    bool ok = false;
    const QString chosen = QInputDialog::getItem(
        this, tr("Idioma del documento"), tr("Idioma para la exportación:"),
        names, current, /*editable=*/false, &ok);
    if (!ok)
        return false;
    *out = langs.at(names.indexOf(chosen));
    return true;
}

bool MainWindow::exportOdf()
{
    commitSourceToDocument();
    mdexport::Language language;
    if (!chooseExportLanguage(&language))
        return false;

    const QString currentFile = m_documentIo->currentFile();
    const QString suggested = currentFile.isEmpty()
        ? QDir::homePath() + QStringLiteral("/documento.odt")
        : QFileInfo(currentFile).absolutePath() + QLatin1Char('/') +
              QFileInfo(currentFile).completeBaseName() + QStringLiteral(".odt");

    QString path = QFileDialog::getSaveFileName(
        this, tr("Exportar a ODF"), suggested, tr("Documento ODF (*.odt)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".odt");

    QString error;
    std::unique_ptr<QTextDocument> flat(
        mdexport::cloneForExport(m_editor->document()));
    if (!mdexport::writeOdf(flat.get(), path, language, exportTitle(), &error)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo exportar a ODF:\n%1\n\n%2").arg(path, error));
        return false;
    }
    statusBar()->showMessage(tr("Exportado a ODF: %1").arg(path), 4000);
    return true;
}

bool MainWindow::exportLatex()
{
    commitSourceToDocument();
    mdexport::Language language;
    if (!chooseExportLanguage(&language))
        return false;

    const QString currentFile = m_documentIo->currentFile();
    const QString suggested = currentFile.isEmpty()
        ? QDir::homePath() + QStringLiteral("/documento.tex")
        : QFileInfo(currentFile).absolutePath() + QLatin1Char('/') +
              QFileInfo(currentFile).completeBaseName() + QStringLiteral(".tex");

    QString path = QFileDialog::getSaveFileName(
        this, tr("Exportar a LaTeX"), suggested, tr("Documento LaTeX (*.tex)"));
    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".tex");

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo escribir:\n%1\n\n%2")
                                 .arg(path, file.errorString()));
        return false;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << mdexport::toLatex(m_editor->document(), language, exportTitle());
    file.close();

    statusBar()->showMessage(tr("Exportado a LaTeX: %1").arg(path), 4000);
    return true;
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
                        openFile(path);
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
                && editFormulaAt(me->position().toPoint()))
                return true;
        }
        // Ctrl+clic izquierdo sobre un enlace lo abre en la aplicación externa.
        else if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton
                && (me->modifiers() & Qt::ControlModifier)) {
                const QString href = m_editor->anchorAt(me->position().toPoint());
                if (!href.isEmpty()) {
                    openLink(href);
                    return true;  // no mover el cursor de texto
                }
            }
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
            guardPasteAgainstMath();
        if (handleMathKeyPress(ke))
            return true;
    }
    // Continuación inteligente de listas en el editor de código fuente.
    else if (watched == m_sourceEditor && event->type() == QEvent::KeyPress) {
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
    QTextCursor cursor = m_sourceEditor->textCursor();
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
    m_sourceEditor->setTextCursor(cursor);
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

// ---------------------------------------------------------------------------
// Archivos
// ---------------------------------------------------------------------------

void MainWindow::newFile()
{
    toggleSourceMode(false);  // un documento nuevo se edita en WYSIWYG
    if (!maybeSave())
        return;
    m_documentIo->reset();
}

void MainWindow::openFileDialog()
{
    const QString currentFile = m_documentIo->currentFile();
    const QString startDir = currentFile.isEmpty()
        ? QDir::homePath()
        : QFileInfo(currentFile).absolutePath();

    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Abrir archivo Markdown"),
        startDir,
        tr("Archivos Markdown (*.md *.markdown *.mdown *.mkd);;Todos los archivos (*)"));

    if (!path.isEmpty())
        openFile(path);
}

void MainWindow::openFile(const QString &path)
{
    toggleSourceMode(false);  // el archivo cargado se muestra en WYSIWYG
    if (!maybeSave())
        return;

    QString error;
    if (!m_documentIo->load(path, &error)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo abrir el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        // Si venía de la lista de recientes y ya no es accesible, lo quitamos.
        if (m_recentFiles)
            m_recentFiles->removeFile(path);
        return;
    }
    // El front matter se conserva pero no se muestra: avisamos para que no
    // parezca que se ha perdido.
    if (m_documentIo->hasFrontMatter())
        statusBar()->showMessage(tr("%1 — front matter conservado").arg(path));
    else
        statusBar()->showMessage(path);
}

bool MainWindow::save()
{
    const QString currentFile = m_documentIo->currentFile();
    return currentFile.isEmpty() ? saveAs() : writeToFile(currentFile);
}

bool MainWindow::saveAs()
{
    const QString currentFile = m_documentIo->currentFile();
    QString path = QFileDialog::getSaveFileName(
        this,
        tr("Guardar como"),
        currentFile.isEmpty() ? QDir::homePath() : currentFile,
        tr("Archivos Markdown (*.md *.markdown);;Todos los archivos (*)"));

    if (path.isEmpty())
        return false;
    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".md");

    return writeToFile(path);
}

bool MainWindow::writeToFile(const QString &path)
{
    commitSourceToDocument();  // si estamos en fuente, aplica los cambios primero
    QString error;
    if (!m_documentIo->write(path, &error)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo guardar el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        return false;
    }
    // Guardado en disco: el borrador de recuperación ya no hace falta.
    m_recovery->clearDraft();
    m_autosaveDirty = false;
    statusBar()->showMessage(tr("Guardado: %1").arg(path), 4000);
    return true;
}

bool MainWindow::maybeSave()
{
    commitSourceToDocument();  // que isModified vea también los cambios del fuente
    if (!m_documentIo->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("md-editor"),
        tr("El documento tiene cambios sin guardar.\n¿Quieres guardarlos?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        return true;  // Discard
    }
}

QString MainWindow::currentBody() const
{
    // El cuerpo Markdown editable. Si el panel de fuente tiene los cambios más
    // frescos sin volcar (modo fuente o vista dividida), se toma de él; si no, se
    // serializa desde el documento WYSIWYG.
    return m_sourceDirty ? m_sourceEditor->toPlainText()
                         : mdtable::documentMarkdown(m_editor->document());
}

void MainWindow::autosaveDraft()
{
    if (!m_autosaveDirty)
        return;  // nada cambió desde el último volcado: no reescribir
    m_autosaveDirty = false;

    // Hay cambios sin guardar si el documento difiere de lo último guardado, o si
    // se está editando en modo fuente (cuyos cambios aún no se han volcado).
    const bool modified = m_documentIo->isModified() || m_sourceDirty;
    if (modified)
        m_recovery->saveDraft(m_documentIo->currentFile(), currentBody());
    else
        m_recovery->clearDraft();  // el documento coincide con el disco
}

bool MainWindow::recoverDraft()
{
    if (!m_recovery->hasDraft())
        return false;

    const QString body = m_recovery->draftBody();
    const QString original = m_recovery->draftOriginalPath();

    if (!original.isEmpty() && QFileInfo::exists(original)) {
        // Abre el documento guardado (fija ruta, baseUrl y la línea base) y le
        // superpone el cuerpo del borrador: así queda asociado a su archivo pero
        // marcado como modificado (el borrador difiere de lo guardado).
        openFile(original);
        m_editor->setMarkdown(mdmath::protectMath(body));
        mdmath::renderMathInDocument(m_editor->document());
        styleTables();
    } else {
        // Sin archivo asociado (o ya no existe): se recupera como sin título.
        toggleSourceMode(false);
        m_documentIo->reset();
        m_editor->setMarkdown(mdmath::protectMath(body));
        mdmath::renderMathInDocument(m_editor->document());
        styleTables();
        m_theme->recolorLinks();
        m_outline->rebuild(m_editor->document());
    }

    setWindowModified(m_documentIo->isModified());
    statusBar()->showMessage(tr("Documento recuperado de la sesión anterior"), 5000);
    return true;
}

void MainWindow::startSession(const QString &cmdLineFile)
{
    // Prioridad: archivo de la línea de comandos > recuperar borrador > reabrir
    // el último documento. Sin returns prematuros: todas las ramas confluyen en
    // el arranque del autoguardado al final.
    bool recovered = false;
    if (!cmdLineFile.isEmpty()) {
        openFile(cmdLineFile);
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
            recovered = recoverDraft();
        else
            m_recovery->clearDraft();  // descartado: seguir con el flujo normal
    }

    // Reabrir el último documento real, salvo que ya se abriera/recuperara algo.
    if (cmdLineFile.isEmpty() && !recovered) {
        const QString last = AppSettings::lastFile();
        if (!last.isEmpty() && QFileInfo::exists(last))
            openFile(last);
    }

    // Decidida la sesión inicial, ya es seguro autoguardar borradores. Lo abierto
    // al arrancar no cuenta como cambio pendiente; el borrador recuperado, en su
    // caso, ya está en disco y se mantiene hasta que el usuario edite o guarde.
    m_autosaveDirty = false;
    m_autosaveTimer->start();
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

void MainWindow::watchCurrentFile(const QString &path)
{
    // Deja de vigilar lo anterior y vigila el archivo actual (si lo hay en disco).
    const QStringList watched = m_fileWatcher->files();
    if (!watched.isEmpty())
        m_fileWatcher->removePaths(watched);
    if (!path.isEmpty() && QFileInfo::exists(path))
        m_fileWatcher->addPath(path);
    // La instantánea se pone al día con lo que acabamos de cargar/guardar, de modo
    // que nuestro propio guardado no se confunda luego con un cambio externo.
    updateDiskSnapshot();
}

void MainWindow::updateDiskSnapshot()
{
    m_diskSnapshot.clear();
    const QString path = m_documentIo->currentFile();
    if (path.isEmpty())
        return;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        m_diskSnapshot = file.readAll();
}

void MainWindow::onWatchedFileChanged(const QString &path)
{
    if (path == m_documentIo->currentFile())
        m_diskCheckTimer->start();  // (re)arranca el debounce
}

void MainWindow::checkDiskChange()
{
    if (m_diskPromptOpen)
        return;  // ya hay un diálogo abierto: no apilar otro (exec corre anidado)
    const QString path = m_documentIo->currentFile();
    if (path.isEmpty())
        return;

    // Un guardado atómico recrea el archivo y rompe la vigilancia: hay que volver
    // a añadirlo si volvió a existir.
    if (QFileInfo::exists(path) && !m_fileWatcher->files().contains(path))
        m_fileWatcher->addPath(path);

    if (!QFileInfo::exists(path)) {
        statusBar()->showMessage(
            tr("El archivo se eliminó o movió en disco."), 6000);
        return;
    }

    QByteArray disk;
    QFile file(path);
    if (file.open(QIODevice::ReadOnly))
        disk = file.readAll();
    if (disk == m_diskSnapshot)
        return;  // sin cambios reales (o fue nuestro propio guardado)

    const bool locallyModified =
        m_documentIo->isModified() || m_sourceDirty;

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
    m_diskPromptOpen = true;
    box.exec();
    m_diskPromptOpen = false;

    if (box.clickedButton() == reloadButton) {
        reloadFromDisk();
    } else {
        // Conserva lo del usuario; recuerda el contenido del disco como referencia
        // para no volver a preguntar por este mismo cambio externo.
        m_diskSnapshot = disk;
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
    if (m_sourceMode || m_splitMode) {
        m_syncing = true;
        m_sourceEditor->setPlainText(mdtable::documentMarkdown(m_editor->document()));
        m_syncing = false;
        m_sourceDirty = false;
    }
    QTextEdit *ed = activeEditor();
    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(qMin(caret, ed->document()->characterCount() - 1));
    ed->setTextCursor(cursor);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        // Cierre limpio (guardado o descartado a propósito): sin borrador que
        // recuperar la próxima vez.
        m_recovery->clearDraft();
        // Recuerda tamaño/posición y disposición de barras para la próxima vez.
        // Si se cierra en modo sin distracciones, se guarda el estado previo a
        // entrar (ventana normal con sus barras), no la pantalla completa.
        if (m_distractionFree) {
            AppSettings::setWindowGeometry(m_preDistractionGeometry);
            AppSettings::setWindowState(m_preDistractionState);
        } else {
            AppSettings::setWindowGeometry(saveGeometry());
            AppSettings::setWindowState(saveState());
        }
        AppSettings::setSplitterState(m_splitView->saveState());
        event->accept();
    } else {
        event->ignore();
    }
}
