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

    // --- Documento activo: editor WYSIWYG/fuente + sus 15 colaboradores ---
    m_stack = new EditorStack(m_findBar, m_outline, this);
    setCentralWidget(m_stack);
    m_findBar->setEditor(m_stack->editor());
    connectStack(m_stack);

    FocusEditor *editor = m_stack->editor();
    // Zoom con Ctrl+rueda y detección de enlaces sobre el viewport; teclas
    // interceptadas (fórmulas atómicas, continuación de listas en el fuente). El
    // filtro de eventos es de la ventana y opera sobre el documento activo.
    editor->viewport()->installEventFilter(this);
    editor->viewport()->setMouseTracking(true);  // recibir hover sin botón pulsado
    editor->installEventFilter(this);
    m_stack->split()->sourceEditor()->installEventFilter(this);

    // Tamaños de fuente base, para "Tamaño normal".
    m_baseFontPointSize = editor->font().pointSizeF();
    m_baseSourceFontPointSize = m_stack->split()->sourceEditor()->font().pointSizeF();

    // Mostrar/ocultar el esquema (F9) dentro del modo sin distracciones recoloca el
    // bloque centrado; al mostrarlo, normaliza su ancho (diferido).
    connect(m_outline, &QDockWidget::visibilityChanged, this, [this](bool visible) {
        if (m_distraction && m_distraction->isActive())
            m_distraction->updateLayout();
        else if (visible)
            QTimer::singleShot(0, this, &MainWindow::normalizeOutlineWidth);
    });
    // Clic en una entrada del esquema: lleva el cursor a ese encabezado.
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
    // Reordenar secciones arrastrando en el esquema (diferido; no aplica en fuente).
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
    // El índice se reconstruye al editar (con debounce) y al cargar (de inmediato).
    m_outlineTimer = new QTimer(this);
    m_outlineTimer->setSingleShot(true);
    m_outlineTimer->setInterval(300);
    connect(m_outlineTimer, &QTimer::timeout, this, [this] {
        if (!m_stack->split()->sourceMode())
            m_outline->rebuild(m_stack->editor()->document());
    });
    connect(editor->document(), &QTextDocument::contentsChanged, this, [this] {
        if (!m_stack->split()->sourceMode())
            m_outlineTimer->start();
    });
    connect(m_stack, &EditorStack::documentLoaded, this, [this] {
        m_outline->rebuild(m_stack->editor()->document());
    });

    // --- Zoom de toda la interfaz y construcción de menús ---
    // El nivel de zoom de los menús debe estar fijado ANTES de crearlos (Qt 6.8 +
    // gtk3 cachea anchuras de QAction en la primera medición; ver applyMenuFontScale).
    m_zoomDelta = AppSettings::zoomLevel();
    static const qreal s_baseMenuPointSize = QApplication::font("QMenuBar").pointSizeF();
    m_baseMenuPointSize = s_baseMenuPointSize;
    applyMenuFontScale();

    createMenusAndActions();
    createFormatToolBar();

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
        this, editor, m_stack->split(), m_outline, m_formatToolBar, m_findBar, this);
    connect(m_distractionAction, &QAction::toggled,
            m_distraction, &DistractionFreeController::setActive);
    connect(m_distraction, &DistractionFreeController::activeChanged,
            m_distractionAction, &QAction::setChecked);

    // Contador de palabras/caracteres, anclado a la derecha de la barra de estado.
    m_countLabel = new QLabel(this);
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
    connect(stack, &EditorStack::statusMessage, statusBar(), &QStatusBar::showMessage);
    connect(stack, &EditorStack::windowModifiedChanged, this, &QWidget::setWindowModified);
    connect(stack, &EditorStack::currentFileChanged, this, &MainWindow::onCurrentFileChanged);
    connect(stack, &EditorStack::wordCountShouldUpdate, this, &MainWindow::updateWordCount);
    connect(stack, &EditorStack::loadFailed, this, [this](const QString &path) {
        if (m_recentFiles)
            m_recentFiles->removeFile(path);  // ya no accesible: quitar de recientes
    });
    connect(stack, &EditorStack::diskExternalChange, this, &MainWindow::onDiskExternalChange);
    connect(stack, &EditorStack::diskVanished, this, [this] {
        statusBar()->showMessage(tr("El archivo se eliminó o movió en disco."), 6000);
    });
}


void MainWindow::setLanguage(const QString &code)
{
    if (code == AppSettings::language())
        return;
    // Cambiar de idioma recrea la ventana (la UI se construye a mano, sin un
    // retranslateUi() que reasigne cada cadena). Antes de descartarla, ofrece
    // guardar los cambios pendientes; si el usuario cancela, no cambiamos nada.
    if (!m_stack->file()->maybeSave())
        return;

    AppSettings::setLanguage(code);

    // Persistimos el estado de ventana para que la ventana recreada arranque con
    // el mismo tamaño/posición y disposición (geometría, barras y proporciones de
    // la vista dividida); main() lee estas mismas claves al construirla. El menú
    // de idioma no es accesible en modo sin distracciones, así que basta el
    // estado plano (no el `sessionState` que el cierre usa para ese modo).
    AppSettings::setWindowGeometry(saveGeometry());
    AppSettings::setWindowState(saveState());
    AppSettings::setSplitterState(m_stack->split()->splitView()->saveState());

    // Recuerda dónde estaba el cursor del archivo actual para reabrirlo ahí tras
    // recrear la ventana (igual que hace el cierre).
    m_stack->file()->rememberCursorPosition();

    // main() intercambia los traductores y recrea la ventana, reabriendo el
    // documento actual ya saneado por maybeSave() (ruta vacía = documento nuevo).
    emit languageChangeRequested(m_stack->documentIo()->currentFile());
}

void MainWindow::relaunchSession(const QString &reopenPath)
{
    normalizeOutlineWidth();
    // El estado ya lo decidió la ventana anterior: ni recuperación de borrador ni
    // reabrir el último documento. Solo reabrimos el que estaba abierto (si lo
    // había y sigue existiendo); en su defecto, queda el documento nuevo vacío.
    if (!reopenPath.isEmpty() && QFileInfo::exists(reopenPath))
        m_stack->file()->openFile(reopenPath);
    m_stack->file()->startAutosave();
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

    // Reabrir el último documento real, salvo que ya se abriera/recuperara algo.
    if (cmdLineFile.isEmpty() && !recovered) {
        const QString last = AppSettings::lastFile();
        if (!last.isEmpty() && QFileInfo::exists(last))
            m_stack->file()->openFile(last);
    }

    // Decidida la sesión inicial, ya es seguro autoguardar borradores (no antes:
    // correría durante el diálogo de recuperación con el documento aún vacío).
    m_stack->file()->startAutosave();
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
    if (m_stack->file()->maybeSave()) {
        // Cierre limpio (guardado o descartado a propósito): sin borrador que
        // recuperar la próxima vez.
        m_stack->recovery()->clearDraft();
        m_stack->file()->rememberCursorPosition();  // reabrir el documento donde se dejó
        // Recuerda tamaño/posición y disposición de barras para la próxima vez.
        // Si se cierra en modo sin distracciones, el controlador devuelve el estado
        // previo a entrar (ventana normal con sus barras), no la pantalla completa.
        AppSettings::setWindowGeometry(m_distraction->sessionGeometry());
        AppSettings::setWindowState(m_distraction->sessionState());
        AppSettings::setSplitterState(m_stack->split()->splitView()->saveState());
        event->accept();
    } else {
        event->ignore();
    }
}
