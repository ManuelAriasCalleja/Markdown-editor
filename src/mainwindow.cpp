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
#include "markdownrender.h"
#include "diagramcontroller.h"
#include "mathblocks.h"
#include "outlinepanel.h"
#include "recentfilesmanager.h"
#include "recoverymanager.h"
#include "spellcontroller.h"
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
#include <QLocale>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QResizeEvent>
#include <QToolButton>
#include <QLabel>
#include <memory>

#include <QContextMenuEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QRegularExpression>
#include <QSplitter>
#include <QTextDocumentFragment>
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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    m_editor = new FocusEditor(this);
    m_editor->setAcceptRichText(true);

    // Tamaño de fuente base del editor WYSIWYG, para "Tamaño normal".
    m_baseFontPointSize = m_editor->font().pointSizeF();
    // Resaltado de sintaxis de los bloques de código.
    m_highlighter = new CodeBlockHighlighter(m_editor->document());

    // Previsualización de diagramas (Mermaid/PlantUML) bajo cada bloque de código.
    // Si falta la herramienta, el propio controlador pone un marcador inline con
    // la orden de instalación de la plataforma (no usa la barra de estado).
    m_diagrams = new DiagramController(m_editor, this);
    connect(m_editor->document(), &QTextDocument::contentsChanged,
            m_diagrams, &DiagramController::scheduleRefresh);
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

    // Corrector ortográfico: posee el motor (que enchufa al highlighter para el
    // subrayado) y el menú contextual de sugerencias. Avisa por la barra de
    // estado si falta el diccionario del idioma.
    m_spellController = new SpellController(m_editor, m_highlighter, m_documentIo, this);
    connect(m_spellController, &SpellController::statusMessage,
            statusBar(), &QStatusBar::showMessage);

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
    // El idioma del corrector puede cambiar con el front matter del documento.
    connect(m_documentIo, &DocumentIo::documentLoaded,
            m_spellController, &SpellController::applyLanguage);

    m_documentIo->reset();  // documento nuevo (fija el título inicial)
    m_spellController->applyLanguage();  // idioma inicial (ajuste de la app o locale)

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
        + QStringLiteral("<p>") + tr("Versión %1").arg(QStringLiteral(APP_VERSION))
        + QStringLiteral("</p>")
        + QStringLiteral("<p>") + tr("Desarrollado por Manuel Arias Calleja")
        + QStringLiteral("</p>")
        + QStringLiteral("<p>") + tr("Editor WYSIWYG de Markdown en Qt6 + C++17.")
        + QStringLiteral("</p>"));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
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
    mdrender::setMarkdownWithExtensions(m_editor, body);
    styleTables();
    m_theme->recolorLinks();
    m_outline->rebuild(m_editor->document());
    m_split->endProgrammaticChange(wasSyncing);
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
