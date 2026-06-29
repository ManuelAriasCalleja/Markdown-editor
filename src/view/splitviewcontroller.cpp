/// \file
/// \brief Implementación de la vista de fuente, la vista dividida y su sincronización con debounce.

#include "splitviewcontroller.h"

#include <QAction>
#include <QApplication>
#include <QFont>
#include <QScrollBar>
#include <QSplitter>
#include <QTextCursor>
#include <QTextEdit>
#include <QTimer>

#include "appsettings.h"
#include "findreplacebar.h"
#include "focuseditor.h"
#include "outlinepanel.h"
#include "tableedit.h"
#include "themecontroller.h"

SplitViewController::SplitViewController(FocusEditor *editor,
                                         FindReplaceBar *findBar,
                                         OutlinePanel *outline,
                                         ThemeController *theme,
                                         QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_findBar(findBar)
    , m_outline(outline)
    , m_theme(theme)
{
    // Vista de código Markdown crudo: texto plano monoespaciado. El widget central
    // es un divisor que puede mostrar el editor WYSIWYG, este, o ambos lado a lado
    // (vista dividida). WYSIWYG a la izquierda, fuente a la derecha.
    m_sourceEditor = new FocusEditor(parent);
    m_sourceEditor->setAcceptRichText(false);
    m_sourceEditor->setAccessibleName(tr("Código fuente Markdown"));
    QFont mono = QFont(QStringLiteral("monospace"));
    mono.setStyleHint(QFont::Monospace);
    m_sourceEditor->setFont(mono);
    m_splitView = new QSplitter(Qt::Horizontal, parent);
    m_splitView->addWidget(m_editor);
    m_splitView->addWidget(m_sourceEditor);
    m_splitView->setStretchFactor(0, 1);  // ambos paneles crecen por igual
    m_splitView->setStretchFactor(1, 1);
    m_sourceEditor->hide();  // arranca en WYSIWYG
    m_splitView->restoreState(AppSettings::splitterState());  // proporciones previas

    // En modo fuente, escribir marca el documento como modificado y actualiza el
    // contador (el contenido vive en m_sourceEditor hasta que se vuelca).
    connect(m_sourceEditor, &QTextEdit::textChanged,
            this, &SplitViewController::onSourceTextChanged);

    // Mientras el foco está en el WYSIWYG, refresca el panel de fuente tras una
    // pausa. El índice (que también escucha contentsChanged) lo lleva MainWindow.
    connect(m_editor->document(), &QTextDocument::contentsChanged,
            this, &SplitViewController::onDocumentContentsChanged);

    // Temporizadores de debounce de la vista dividida (~250 ms): refrescan el
    // panel SIN foco con lo editado en el otro.
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
    connect(m_syncToDocTimer, &QTimer::timeout, this, &SplitViewController::syncDocumentFromSource);

    // Al cambiar el foco entre los dos paneles, vacía de inmediato el sync
    // pendiente para que el panel al que llegas esté al día.
    connect(qApp, &QApplication::focusChanged, this, &SplitViewController::onFocusChanged);
}

void SplitViewController::setModeActions(QAction *sourceModeAction, QAction *splitAction)
{
    m_sourceModeAction = sourceModeAction;
    m_splitAction = splitAction;
}

QTextEdit *SplitViewController::activeEditor() const
{
    // En fuente a pantalla completa manda el editor de fuente; en vista dividida,
    // el que tenga el foco; en WYSIWYG, el editor.
    if (m_sourceMode)
        return m_sourceEditor;
    if (m_splitMode && m_sourceEditor->hasFocus())
        return m_sourceEditor;
    return m_editor;
}

bool SplitViewController::beginProgrammaticChange()
{
    const bool previous = m_syncing;
    m_syncing = true;
    return previous;
}

void SplitViewController::setWysiwygActionsEnabled(bool enabled)
{
    for (QAction *a : m_wysiwygActions)
        a->setEnabled(enabled);
}

void SplitViewController::updateEditorVisibility()
{
    m_editor->setVisible(!m_sourceMode);                       // oculto solo en fuente-completo
    m_sourceEditor->setVisible(m_sourceMode || m_splitMode);   // visible en fuente y en dividido
}

void SplitViewController::updateActionsForFocus()
{
    if (!m_splitMode)
        return;  // en los otros modos lo gestiona toggleSourceMode
    const bool wysiwygFocused = !m_sourceEditor->hasFocus();
    // El formato/inserción solo tiene sentido sobre el WYSIWYG.
    setWysiwygActionsEnabled(wysiwygFocused);
    // La búsqueda actúa sobre el panel donde está el cursor.
    m_findBar->setEditor(wysiwygFocused ? m_editor : m_sourceEditor);
    if (wysiwygFocused)
        emit formatActionsShouldUpdate();  // refina habilitado (encabezado, lista, fence…) y marcas
    else
        emit tableActionsShouldUpdate();   // deja las de tabla inhabilitadas con el foco en el fuente
}

void SplitViewController::onSourceTextChanged()
{
    // Edición real del usuario en el panel de fuente (no un refresco programático,
    // que va con m_syncing): marca el fuente como la versión más fresca, en modo
    // fuente o en vista dividida.
    if (!m_syncing && (m_sourceMode || m_splitMode)) {
        m_sourceDirty = true;
        emit documentModified();
        // En vista dividida, mientras el foco está en el fuente, vuelca al
        // WYSIWYG tras una pausa (debounce). Solo el panel sin foco se toca.
        if (m_splitMode && m_sourceEditor->hasFocus())
            m_syncToDocTimer->start();
    }
    emit wordCountShouldUpdate();
}

void SplitViewController::onDocumentContentsChanged()
{
    // En vista dividida, mientras el foco está en el WYSIWYG, refresca el panel de
    // fuente tras una pausa. m_syncing distingue los cambios del usuario de los
    // provocados por la propia sincronización (anti-bucle).
    if (m_splitMode && !m_syncing && !m_sourceEditor->hasFocus())
        m_syncToSourceTimer->start();
}

void SplitViewController::onFocusChanged(QWidget *old, QWidget *now)
{
    flushPendingSync(old);
    // Solo reaccionamos al pasar el foco a uno de los dos editores (no a menús,
    // diálogos o la barra de búsqueda).
    if (m_splitMode && (now == m_editor || now == m_sourceEditor))
        updateActionsForFocus();
}

void SplitViewController::commitSourceToDocument()
{
    // Vuelca los cambios del editor de fuente al documento WYSIWYG, que es el que
    // se serializa al guardar/exportar. El re-render lo hace m_renderBody, que deja
    // el modelo al día. Funciona tanto en modo fuente como en vista dividida (el
    // disparador es m_sourceDirty, no el modo).
    if (m_sourceDirty) {
        if (m_renderBody)
            m_renderBody(m_sourceEditor->toPlainText());
        m_sourceDirty = false;
    }
}

void SplitViewController::refreshSourceFromDocument()
{
    if (m_sourceMode || m_splitMode) {
        m_syncing = true;
        m_sourceEditor->setPlainText(mdtable::documentMarkdown(m_editor->document()));
        m_syncing = false;
        m_sourceDirty = false;
    }
}

void SplitViewController::toggleSourceMode(bool on)
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
        m_theme->recolorLinks(m_editor);  // los enlaces recargados toman el color del tema
    }
    m_sourceMode = on;
    m_sourceDirty = false;
    updateEditorVisibility();

    // Las acciones de formato/inserción no aplican al texto plano: se desactivan
    // (también sus atajos) en modo fuente.
    setWysiwygActionsEnabled(!on);
    emit tableActionsShouldUpdate();  // y las de tabla (en fuente, siempre inhabilitadas)

    // El índice se nutre de la estructura del documento WYSIWYG, no del texto
    // plano: se deshabilita en modo fuente y se reconstruye al volver (el contenido
    // pudo cambiar en el editor de fuente).
    m_outline->setEnabled(!on);
    if (!on)
        m_outline->rebuild(m_editor->document());

    if (m_sourceModeAction && m_sourceModeAction->isChecked() != on)
        m_sourceModeAction->setChecked(on);  // mantiene el menú en sincronía
    activeEditor()->setFocus();
    emit wordCountShouldUpdate();
}

void SplitViewController::toggleSplitView(bool on)
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
        m_theme->recolorLinks(m_editor);
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
        // Al volver a WYSIWYG, reactiva el formato (pudo quedar deshabilitado si el
        // foco estaba en el fuente) y refresca su estado.
        setWysiwygActionsEnabled(true);
        m_findBar->setEditor(m_editor);
        emit formatActionsShouldUpdate();
    }

    if (m_splitAction && m_splitAction->isChecked() != on)
        m_splitAction->setChecked(on);  // mantiene el menú en sincronía
    m_editor->setFocus();
    emit wordCountShouldUpdate();
}

void SplitViewController::syncSourceFromDocument()
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

void SplitViewController::syncDocumentFromSource()
{
    // fuente -> WYSIWYG. Solo si seguimos en split y el WYSIWYG no tiene el foco.
    if (!m_splitMode || m_editor->hasFocus() || !m_sourceDirty)
        return;
    const int scroll = m_editor->verticalScrollBar()->value();
    commitSourceToDocument();  // gestiona m_syncing y re-renderiza
    m_editor->verticalScrollBar()->setValue(scroll);  // no saltar la vista
}

void SplitViewController::flushPendingSync(QWidget *losingFocus)
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
