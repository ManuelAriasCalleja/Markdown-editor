#include "mainwindow.h"

#include "appsettings.h"
#include "blockconstructs.h"
#include "chromezoom.h"
#include "footnotes.h"
#include "tasklist.h"
#include "diskwatcher.h"
#include "distractionfreecontroller.h"
#include "documentio.h"
#include "editorstack.h"
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
#include <QTabBar>
#include <QTabWidget>
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
    // --- Superficies compartidas de la ventana (las usa el documento activo) ---
    // Panel de esquema (índice de encabezados). Vive siempre pegado a la izquierda
    // (ni flota ni va a otra zona). Se crea antes del menú porque su acción de
    // mostrar/ocultar (toggleViewAction) se añade a Ver.
    m_outline = new OutlinePanel(this);
    m_outline->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_outline->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    addDockWidget(Qt::LeftDockWidgetArea, m_outline);

    // Barra inferior de buscar/reemplazar. Su editor objetivo lo fija el documento
    // activo (abajo y al cambiar de foco).
    m_findBar = new FindReplaceBar(nullptr, this);
    addToolBar(Qt::BottomToolBarArea, m_findBar);
    connect(m_findBar, &FindReplaceBar::statusMessage,
            statusBar(), &QStatusBar::showMessage);

    // --- Documentos en pestañas: uno por archivo abierto ---
    m_tabs = new QTabWidget(this);
    m_tabs->setTabsClosable(true);
    m_tabs->setMovable(true);
    m_tabs->setDocumentMode(true);
    // Sin esto, una sola pestaña se estira hasta llenar todo el ancho (el defecto
    // de QTabBar es expandir): se vería desproporcionada, sobre todo en el modo
    // sin distracciones. Con expanding=false cada pestaña se dimensiona a su texto.
    m_tabs->tabBar()->setExpanding(false);
    setCentralWidget(m_tabs);
    connect(m_tabs, &QTabWidget::currentChanged, this, [this](int index) {
        if (EditorStack *s = stackAt(index))
            setActiveStack(s);
    });
    connect(m_tabs, &QTabWidget::tabCloseRequested, this, &MainWindow::closeTab);

    // Esquema: mostrar/ocultar (F9) recoloca la columna sin distracciones; clic
    // lleva el cursor al encabezado; arrastre reordena la sección. Operan sobre el
    // documento ACTIVO (m_stack). Su reconstrucción la conduce setActiveStack.
    connect(m_outline, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (m_distraction && m_distraction->isActive())
            m_distraction->updateLayout();
        else if (visible)
            QTimer::singleShot(0, this, &MainWindow::normalizeOutlineWidth);
    });
    connect(m_outline, &OutlinePanel::headingActivated, this, [this](int blockNumber) {
        FocusEditor *ed = m_stack->editor();
        const QTextBlock block = ed->document()->findBlockByNumber(blockNumber);
        if (!block.isValid())
            return;
        QTextCursor cursor(block);
        ed->setTextCursor(cursor);
        ed->ensureCursorVisible();
        ed->setFocus();
    });
    connect(m_outline, &OutlinePanel::sectionMoveRequested, this,
            [this](int from, int to, bool placeAfter) {
        if (m_stack->split()->sourceMode())
            return;
        QTimer::singleShot(0, this, [this, from, to, placeAfter] {
            const QString body = mdtable::documentMarkdown(m_stack->editor()->document());
            const QString moved = mdoutline::moveSection(body, from, to, placeAfter);
            if (moved == body)
                return;
            m_stack->setBodyMarkdown(moved);
            setWindowModified(true);
        });
    });
    // El índice se reconstruye al editar (con debounce). La conexión al documento
    // activo la (re)hace setActiveStack; este timer es de la ventana.
    m_outlineTimer = new QTimer(this);
    m_outlineTimer->setSingleShot(true);
    m_outlineTimer->setInterval(300);
    connect(m_outlineTimer, &QTimer::timeout, this, [this] {
        if (!m_stack->split()->sourceMode())
            m_outline->rebuild(m_stack->editor()->document());
    });

    // Primer documento. addTab lo crea, lo cablea y lo hace activo (setActiveStack,
    // que de momento omite lo dependiente de los menús, aún por crear).
    addTab();
    FocusEditor *editor = m_stack->editor();
    m_baseFontPointSize = editor->font().pointSizeF();
    m_baseSourceFontPointSize = m_stack->split()->sourceEditor()->font().pointSizeF();

    // --- Zoom de toda la interfaz y construcción de menús ---
    // El nivel de zoom de los menús debe estar fijado ANTES de crearlos (Qt 6.8 +
    // gtk3 cachea anchuras de QAction en la primera medición; ver applyMenuFontScale).
    m_zoomDelta = AppSettings::zoomLevel();
    static const qreal s_baseMenuPointSize = QApplication::font("QMenuBar").pointSizeF();
    m_baseMenuPointSize = s_baseMenuPointSize;
    applyMenuFontScale();

    createMenusAndActions();
    createFormatToolBar();
    // Ya existen las acciones compartidas: entrégaselas al primer documento (el
    // resto las recibe en addTab) y refresca su estado para el documento activo.
    configureStack(m_stack);
    setActiveStack(m_stack);

    // Tamaños base del resto de la interfaz, para escalar con el zoom.
    m_baseToolBarPointSize = m_formatToolBar->font().pointSizeF();
    m_baseFindBarPointSize = m_findBar->font().pointSizeF();
    m_baseStatusBarPointSize = statusBar()->font().pointSizeF();
    m_baseOutlinePointSize = m_outline->font().pointSizeF();
    applyZoom();

    // En el modo sin distracciones se ocultan menú y barras; registra sus atajos a
    // nivel de ventana para que sigan funcionando.
    const auto allActions = findChildren<QAction *>();
    for (QAction *a : allActions)
        if (!a->shortcut().isEmpty())
            addAction(a);

    // Modo sin distracciones (pantalla completa, columna centrada).
    m_distraction = new DistractionFreeController(
        this, editor, m_stack->split(), m_outline, m_formatToolBar, m_findBar,
        m_tabs->tabBar(), this);
    connect(m_distractionAction, &QAction::toggled,
            m_distraction, &DistractionFreeController::setActive);
    connect(m_distraction, &DistractionFreeController::activeChanged,
            m_distractionAction, &QAction::setChecked);

    // Contador de palabras/caracteres, anclado a la derecha de la barra de estado.
    m_countLabel = new QLabel(this);
    m_countLabel->setAccessibleName(tr("Contador de palabras"));
    m_countLabel->setVisible(AppSettings::showWordCount());
    statusBar()->addPermanentWidget(m_countLabel);

    // Documento inicial (nuevo) e idioma del corrector. Tras conectar el stack para
    // que el esquema se reconstruya y el título se fije.
    m_stack->documentIo()->reset();
    m_stack->spell()->applyLanguage();

    // Restaura tamaño/posición y disposición de barras de la sesión anterior.
    const QByteArray geometry = AppSettings::windowGeometry();
    if (geometry.isEmpty())
        resize(900, 700);
    else
        restoreGeometry(geometry);
    restoreState(AppSettings::windowState());
    // restoreState puede reponer el esquema flotante o en otra zona; re-anclar.
    if (m_outline->isFloating() || dockWidgetArea(m_outline) != Qt::LeftDockWidgetArea) {
        m_outline->setFloating(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_outline);
    }
    // Arranca siempre como ventana normal (descarta pantalla completa/maximizado).
    setWindowState(windowState() & ~(Qt::WindowFullScreen | Qt::WindowMaximized));
    m_findBar->hide();  // la barra de búsqueda siempre arranca oculta

    statusBar()->showMessage(
        tr("Editor Markdown WYSIWYG — escribe y da formato con la barra superior"));
    m_stack->format()->updateActions();
    updateWordCount();

    // Restaura el tema de la sesión anterior (la señal themeChanged marca la acción
    // del menú). Si se sigue el SO, se deriva de su esquema actual.
    m_stack->theme()->applyTheme(
        m_stack->theme()->followsSystem()
            ? m_stack->theme()->systemTheme()
            : mdtheme::idFromKey(AppSettings::themeKey(), mdtheme::ThemeId::Light));
}

void MainWindow::connectStack(EditorStack *stack)
{
    // La mayoría de señales solo afectan a la VENTANA cuando provienen del
    // documento activo (título, modificado, recuento, esquema, disco): los demás
    // documentos viven en segundo plano sin tocar el cromo de la ventana.
    connect(stack, &EditorStack::statusMessage, statusBar(), &QStatusBar::showMessage);
    connect(stack, &EditorStack::windowModifiedChanged, this, [this, stack](bool m) {
        if (stack == m_stack)
            setWindowModified(m);
        updateTabLabel(stack);  // el [*] de la pestaña, venga del activo o no
    });
    connect(stack, &EditorStack::currentFileChanged, this, &MainWindow::onCurrentFileChanged);
    connect(stack, &EditorStack::wordCountShouldUpdate, this, [this, stack] {
        if (stack == m_stack)
            updateWordCount();
    });
    connect(stack, &EditorStack::loadFailed, this, [this](const QString &path) {
        if (m_recentFiles)
            m_recentFiles->removeFile(path);  // ya no accesible: quitar de recientes
    });
    connect(stack, &EditorStack::diskExternalChange, this, [this, stack](const QByteArray &b) {
        if (stack == m_stack)
            onDiskExternalChange(b);
    });
    connect(stack, &EditorStack::diskVanished, this, [this, stack] {
        if (stack == m_stack)
            statusBar()->showMessage(tr("El archivo se eliminó o movió en disco."), 6000);
    });
    connect(stack, &EditorStack::documentLoaded, this, [this, stack] {
        if (stack == m_stack && !stack->split()->sourceMode())
            m_outline->rebuild(stack->editor()->document());
    });
}


void MainWindow::setLanguage(const QString &code)
{
    if (code == AppSettings::language())
        return;
    // Cambiar de idioma recrea la ventana (la UI se construye a mano, sin un
    // retranslateUi() que reasigne cada cadena). Antes de descartarla, ofrece
    // guardar los cambios pendientes de CADA pestaña; si alguna cancela, no
    // cambiamos nada.
    for (int i = 0; i < m_tabs->count(); ++i) {
        EditorStack *s = stackAt(i);
        if (!s)
            continue;
        m_tabs->setCurrentIndex(i);
        if (!s->file()->maybeSave())
            return;
    }

    AppSettings::setLanguage(code);

    // Persistimos el estado de ventana para que la ventana recreada arranque con
    // el mismo tamaño/posición y disposición; main() lee estas mismas claves al
    // construirla. El menú de idioma no es accesible en modo sin distracciones,
    // así que basta el estado plano (no el `sessionState` del cierre).
    AppSettings::setWindowGeometry(saveGeometry());
    AppSettings::setWindowState(saveState());
    AppSettings::setSplitterState(m_stack->split()->splitView()->saveState());

    // Sesión de pestañas + cursores, para que la ventana recreada reabra todos los
    // documentos donde se dejaron.
    QStringList openFiles;
    for (int i = 0; i < m_tabs->count(); ++i) {
        EditorStack *s = stackAt(i);
        if (!s)
            continue;
        s->file()->rememberCursorPosition();
        const QString f = s->documentIo()->currentFile();
        if (!f.isEmpty())
            openFiles << f;
    }
    AppSettings::setOpenFiles(openFiles);

    // main() intercambia los traductores y recrea la ventana, que reabrirá la
    // sesión (ver relaunchSession). Pasa el documento activo por compatibilidad.
    emit languageChangeRequested(m_stack->documentIo()->currentFile());
}

void MainWindow::relaunchSession(const QString &reopenPath)
{
    normalizeOutlineWidth();
    // El estado lo decidió la ventana anterior: sin recuperación de borrador. Se
    // reabren todas las pestañas que estaban abiertas (sesión guardada en
    // setLanguage); a falta de sesión, el `reopenPath` suelto (compatibilidad).
    QStringList session = AppSettings::openFiles();
    if (session.isEmpty() && !reopenPath.isEmpty())
        session << reopenPath;
    for (const QString &path : session)
        if (QFileInfo::exists(path))
            openPathInTab(path);
    for (int i = 0; i < m_tabs->count(); ++i)
        if (EditorStack *s = stackAt(i))
            s->file()->startAutosave();
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

    QTextEdit *ed = m_stack->activeEditor();
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
    const mdstats::DocStats st = mdstats::analyze(m_stack->activeEditor()->toPlainText());
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

    QFont f = m_stack->editor()->font();
    f.setPointSizeF(chromezoom::scaledPointSize(m_baseFontPointSize, m_zoomDelta));
    m_stack->editor()->setFont(f);

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
    scale(m_stack->split()->sourceEditor(), m_baseSourceFontPointSize);
    scale(m_outline, m_baseOutlinePointSize);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // m_distraction puede no existir aún durante la construcción.
    if (m_distraction && m_distraction->isActive())
        m_distraction->updateLayout();  // el tamaño en pantalla completa llega aquí
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
        m_stack->file()->openFile(cmdLineFile);
    } else if (m_stack->recovery()->hasDraft()) {
        // ¿Quedó un borrador de un cierre anómalo? Ofrecer recuperarlo.
        const QString original = m_stack->recovery()->draftOriginalPath();
        const QString name = original.isEmpty() ? tr("(sin título)")
                                                : QFileInfo(original).fileName();
        const QString when = m_stack->recovery()->draftTimestamp().toString(QStringLiteral("yyyy-MM-dd HH:mm"));
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
            recovered = m_stack->file()->recoverDraft();
        else
            m_stack->recovery()->clearDraft();  // descartado: seguir con el flujo normal
    }

    // Reabrir la sesión de pestañas anterior (todos los documentos abiertos),
    // salvo que ya se abriera/recuperara algo. openPathInTab reusa la pestaña
    // vacía inicial para el primero y añade una por cada documento siguiente.
    if (cmdLineFile.isEmpty() && !recovered) {
        QStringList session = AppSettings::openFiles();
        if (session.isEmpty()) {
            const QString last = AppSettings::lastFile();  // compatibilidad: una sola
            if (!last.isEmpty())
                session << last;
        }
        for (const QString &path : session)
            if (QFileInfo::exists(path))
                openPathInTab(path);
    }

    // Decidida la sesión inicial, ya es seguro autoguardar borradores en cada
    // pestaña (no antes: correría durante el diálogo de recuperación con el
    // documento aún vacío).
    for (int i = 0; i < m_tabs->count(); ++i)
        if (EditorStack *s = stackAt(i))
            s->file()->startAutosave();
}

EditorStack *MainWindow::stackAt(int index) const
{
    if (!m_tabs)
        return nullptr;
    if (index < 0)
        index = m_tabs->currentIndex();
    return qobject_cast<EditorStack *>(m_tabs->widget(index));
}

EditorStack *MainWindow::addTab()
{
    auto *stack = new EditorStack(m_findBar, m_outline, this);
    // El filtro de eventos de la ventana consulta m_stack; fíjalo ya (aún antes de
    // añadir la pestaña) para que ningún evento de layout llegue con m_stack a un
    // documento previo o nulo. setActiveStack lo reconfirmará al activarse.
    m_stack = stack;
    connectStack(stack);
    // El filtro de eventos de la ventana opera sobre los editores de cada documento
    // (zoom con Ctrl+rueda, enlaces, fórmulas atómicas, listas en el fuente).
    stack->editor()->viewport()->installEventFilter(this);
    stack->editor()->viewport()->setMouseTracking(true);
    stack->editor()->installEventFilter(this);
    stack->split()->sourceEditor()->installEventFilter(this);
    if (m_sourceModeAction)        // si los menús ya existen, dale las acciones
        configureStack(stack);
    const int idx = m_tabs->addTab(stack, tr("Sin título"));
    m_tabs->setCurrentIndex(idx);  // dispara setActiveStack
    return stack;
}

void MainWindow::setActiveStack(EditorStack *stack)
{
    if (!stack)
        return;
    m_stack = stack;

    // «Editar → reconstruir esquema» se reengancha al documento activo (solo él
    // alimenta el esquema compartido).
    disconnect(m_outlineEditConn);
    m_outlineEditConn = connect(stack->editor()->document(),
                                &QTextDocument::contentsChanged, this, [this] {
        if (!m_stack->split()->sourceMode())
            m_outlineTimer->start();
    });

    // Aplica el zoom actual a los editores de este documento (las otras pestañas
    // mantienen su fuente hasta que se activan).
    if (m_baseFontPointSize > 0) {
        QFont ef = stack->editor()->font();
        ef.setPointSizeF(chromezoom::scaledPointSize(m_baseFontPointSize, m_zoomDelta));
        stack->editor()->setFont(ef);
        QFont sf = stack->split()->sourceEditor()->font();
        sf.setPointSizeF(chromezoom::scaledPointSize(m_baseSourceFontPointSize, m_zoomDelta));
        stack->split()->sourceEditor()->setFont(sf);
    }

    // Barra de búsqueda al editor activo (oculta al cambiar); esquema del activo.
    if (m_findBar) {
        m_findBar->setEditor(stack->activeEditor());
        m_findBar->hide();
    }
    if (!stack->split()->sourceMode())
        m_outline->rebuild(stack->editor()->document());

    // Estado de las acciones dependientes del documento (si ya hay menús).
    if (m_sourceModeAction) {
        m_sourceModeAction->setChecked(stack->split()->sourceMode());
        m_splitAction->setChecked(stack->split()->splitMode());
        stack->format()->updateActions();
        stack->table()->updateActions();
    }
    // El modo sin distracciones apunta a un editor concreto; al cambiar de
    // documento se sale (reorientarlo no compensa la complejidad).
    if (m_distraction && m_distraction->isActive())
        m_distraction->setActive(false);

    // Título, indicador de modificado y recuento del documento activo.
    setWindowModified(stack->documentIo()->isModified());
    const QString file = stack->documentIo()->currentFile();
    setWindowTitle(tr("%1[*] — md-editor")
                       .arg(file.isEmpty() ? tr("Sin título") : QFileInfo(file).fileName()));
    updateWordCount();
}

void MainWindow::newTab()
{
    addTab();  // documento nuevo vacío en una pestaña nueva
}

void MainWindow::openInTab()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("Abrir"), QString(),
        tr("Archivos Markdown (*.md *.markdown *.txt);;Todos los archivos (*)"));
    if (!path.isEmpty())
        openPathInTab(path);
}

void MainWindow::openPathInTab(const QString &path)
{
    // Si ya está abierto, salta a esa pestaña.
    for (int i = 0; i < m_tabs->count(); ++i)
        if (EditorStack *s = stackAt(i); s && s->documentIo()->currentFile() == path) {
            m_tabs->setCurrentIndex(i);
            return;
        }
    // Reusa la pestaña actual si es un documento nuevo vacío y sin cambios; si no,
    // abre en una pestaña nueva.
    EditorStack *target = (m_stack->documentIo()->currentFile().isEmpty()
                           && !m_stack->documentIo()->isModified())
                          ? m_stack : addTab();
    m_tabs->setCurrentWidget(target);
    target->file()->openFile(path);
}

void MainWindow::closeTab(int index)
{
    EditorStack *stack = stackAt(index);
    if (!stack)
        return;
    m_tabs->setCurrentWidget(stack);  // mostrarlo para el posible diálogo de guardado
    if (!stack->file()->maybeSave())
        return;  // el usuario canceló
    if (m_tabs->count() == 1) {
        stack->documentIo()->reset();  // última pestaña: queda como documento nuevo
        return;
    }
    m_tabs->removeTab(m_tabs->indexOf(stack));
    stack->deleteLater();
}

void MainWindow::updateTabLabel(EditorStack *stack)
{
    if (!m_tabs)
        return;
    const int idx = m_tabs->indexOf(stack);
    if (idx < 0)
        return;
    const QString file = stack->documentIo()->currentFile();
    const QString name = file.isEmpty() ? tr("Sin título") : QFileInfo(file).fileName();
    // Un punto delante marca los cambios sin guardar (las pestañas no soportan [*]).
    m_tabs->setTabText(idx, stack->documentIo()->isModified()
                                ? QStringLiteral("• %1").arg(name) : name);
}

void MainWindow::onCurrentFileChanged(const QString &path)
{
    auto *stack = qobject_cast<EditorStack *>(sender());

    // Registra el archivo en recientes y recuerda el último real (para reabrirlo al
    // arrancar; sobrevive a un cierre inesperado). addFile/lastFile ignoran la ruta
    // vacía del documento nuevo.
    if (m_recentFiles)
        m_recentFiles->addFile(path);
    if (!path.isEmpty())
        AppSettings::setLastFile(path);

    if (stack)
        updateTabLabel(stack);

    // Título e indicador [*]: solo manda el documento activo (o una llamada directa
    // sin remitente). Acaba de cargarse/guardarse, así que está limpio.
    if (!stack || stack == m_stack) {
        setWindowModified(false);
        const QString shown = path.isEmpty() ? tr("Sin título")
                                             : QFileInfo(path).fileName();
        setWindowTitle(tr("%1[*] — md-editor").arg(shown));
    }
}

void MainWindow::onDiskExternalChange(const QByteArray &diskBytes)
{
    const QString path = m_stack->documentIo()->currentFile();
    const bool locallyModified =
        m_stack->documentIo()->isModified() || m_stack->split()->isSourceDirty();

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
    m_stack->diskWatcher()->setSuspended(true);
    box.exec();
    m_stack->diskWatcher()->setSuspended(false);

    if (box.clickedButton() == reloadButton) {
        reloadFromDisk();
    } else {
        // Conserva lo del usuario; recuerda el contenido del disco como referencia
        // para no volver a preguntar por este mismo cambio externo.
        m_stack->diskWatcher()->setSnapshot(diskBytes);
    }
}

void MainWindow::reloadFromDisk()
{
    const QString path = m_stack->documentIo()->currentFile();
    if (path.isEmpty())
        return;

    // Conserva aproximadamente la posición del cursor del editor activo.
    const int caret = m_stack->activeEditor()->textCursor().position();

    QString error;
    if (!m_stack->documentIo()->load(path, &error)) {  // emite currentFileChanged → revigila
        QMessageBox::warning(this, tr("Error"),
                             tr("No se pudo recargar el archivo:\n%1\n\n%2")
                                 .arg(path, error));
        return;
    }
    // Si el panel de fuente está visible (modo fuente o vista dividida), su texto
    // plano hay que refrescarlo con lo recargado (load() solo toca el documento
    // WYSIWYG).
    m_stack->split()->refreshSourceFromDocument();
    QTextEdit *ed = m_stack->activeEditor();
    QTextCursor cursor = ed->textCursor();
    cursor.setPosition(qMin(caret, ed->document()->characterCount() - 1));
    ed->setTextCursor(cursor);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Pregunta por CADA documento con cambios sin guardar; si alguno se cancela, no
    // se cierra la ventana. Se muestra cada pestaña antes de su diálogo.
    for (int i = 0; i < m_tabs->count(); ++i) {
        EditorStack *s = stackAt(i);
        if (!s)
            continue;
        m_tabs->setCurrentIndex(i);
        if (!s->file()->maybeSave()) {
            event->ignore();
            return;
        }
    }

    // Cierre limpio: limpia borradores, recuerda cursores y guarda la sesión de
    // pestañas (rutas abiertas) para reabrirla la próxima vez.
    QStringList openFiles;
    for (int i = 0; i < m_tabs->count(); ++i) {
        EditorStack *s = stackAt(i);
        if (!s)
            continue;
        s->recovery()->clearDraft();
        s->file()->rememberCursorPosition();
        const QString f = s->documentIo()->currentFile();
        if (!f.isEmpty())
            openFiles << f;
    }
    AppSettings::setOpenFiles(openFiles);
    // Tamaño/posición y disposición de barras (si se cierra sin distracciones, el
    // controlador devuelve el estado previo a entrar, no la pantalla completa).
    AppSettings::setWindowGeometry(m_distraction->sessionGeometry());
    AppSettings::setWindowState(m_distraction->sessionState());
    AppSettings::setSplitterState(m_stack->split()->splitView()->saveState());
    event->accept();
}
