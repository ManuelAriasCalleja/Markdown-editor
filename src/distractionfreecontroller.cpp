#include "distractionfreecontroller.h"

#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QWidget>

#include "focuseditor.h"
#include "outlinepanel.h"
#include "splitviewcontroller.h"

namespace {
// Ancho de la columna de lectura y del árbol del esquema en el modo (px).
constexpr int kReadingColumn = 960;
constexpr int kOutlineTreeWidth = 280;
}  // namespace

DistractionFreeController::DistractionFreeController(
    QMainWindow *window, FocusEditor *editor, SplitViewController *split,
    OutlinePanel *outline, QWidget *formatToolBar, QWidget *findBar, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_editor(editor)
    , m_split(split)
    , m_outline(outline)
    , m_formatToolBar(formatToolBar)
    , m_findBar(findBar)
{
    // ESC sale del modo. Solo se activa dentro del modo para no interferir con
    // otros usos de ESC (p. ej. cerrar la barra de búsqueda).
    m_escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), window);
    m_escShortcut->setEnabled(false);
    connect(m_escShortcut, &QShortcut::activated, this, [this] { setActive(false); });
}

QByteArray DistractionFreeController::sessionGeometry() const
{
    return m_active ? m_preGeometry : m_window->saveGeometry();
}

QByteArray DistractionFreeController::sessionState() const
{
    return m_active ? m_preState : m_window->saveState();
}

void DistractionFreeController::setActive(bool on)
{
    if (on == m_active)
        return;
    m_active = on;

    if (on) {
        // El modo sin distracciones es de columna única: salimos de la vista
        // dividida (si estaba activa) para concentrarse en la escritura.
        if (m_split->splitMode())
            m_split->toggleSplitView(false);
        // Recuerda el estado para restaurarlo al salir (y para persistirlo si
        // se cierra la app dentro del modo, en vez del estado a pantalla
        // completa con las barras ocultas).
        m_wasMaximized = m_window->isMaximized();
        m_findBarWasVisible = m_findBar->isVisible();
        m_normalOutlineWidth = m_outline->width();  // para restaurar el dock al salir
        m_preGeometry = m_window->saveGeometry();
        m_preState = m_window->saveState();

        m_window->menuBar()->hide();
        m_formatToolBar->hide();
        m_findBar->hide();
        m_window->statusBar()->hide();
        // El esquema no se oculta: sigue pegado a la izquierda del área visible
        // (conserva la visibilidad que tuviera al entrar al modo). Su barra de
        // título sí se oculta: con el dock ensanchado por el relleno izquierdo,
        // dejarla visible la estiraría hasta el borde de la pantalla.
        m_outline->setTitleBarWidget(new QWidget(m_outline));

        m_editor->setReadingColumnWidth(kReadingColumn);
        m_split->sourceEditor()->setReadingColumnWidth(kReadingColumn);

        m_escShortcut->setEnabled(true);
        m_window->showFullScreen();
    } else {
        m_escShortcut->setEnabled(false);

        m_editor->setReadingColumnWidth(0);
        m_split->sourceEditor()->setReadingColumnWidth(0);

        m_window->menuBar()->show();
        m_formatToolBar->show();
        m_window->statusBar()->show();
        // Devuelve al dock su barra de título nativa.
        if (QWidget *tb = m_outline->titleBarWidget()) {
            m_outline->setTitleBarWidget(nullptr);
            tb->deleteLater();
        }
        if (m_findBarWasVisible)
            m_findBar->show();

        if (m_wasMaximized)
            m_window->showMaximized();
        else
            m_window->showNormal();
    }
    updateLayout();
    emit activeChanged(on);
}

void DistractionFreeController::updateLayout()
{
    // Esquema visible y acoplado: centra el bloque [esquema | columna] en
    // pantalla. La columna se pega al borde derecho del dock (alineada a la
    // izquierda) y el dock se ensancha con un relleno izquierdo igual al hueco
    // que queda a la derecha de la columna, de modo que el conjunto quede
    // simétrico (franjas oscuras iguales a ambos lados).
    const bool group = m_active && m_outline->isVisible() && !m_outline->isFloating();

    m_editor->setColumnLeftAligned(group);
    m_split->sourceEditor()->setColumnLeftAligned(group);

    if (!group) {
        m_outline->setLeftPadding(0);
        // Al salir del modo, devuelve al dock su ancho previo (resizeDocks lo
        // había ensanchado para el relleno).
        if (!m_active && m_normalOutlineWidth > 0) {
            m_window->resizeDocks({m_outline}, {m_normalOutlineWidth}, Qt::Horizontal);
            m_normalOutlineWidth = 0;
        }
        return;
    }

    const int total = m_outline->width() + m_split->splitView()->width();  // dock+central
    const int leftPad = qMax(0, (total - kOutlineTreeWidth - kReadingColumn) / 2);
    const int dockWidth = leftPad + kOutlineTreeWidth;

    m_outline->setLeftPadding(leftPad);
    if (m_outline->width() != dockWidth)  // evita reentrar en el layout sin necesidad
        m_window->resizeDocks({m_outline}, {dockWidth}, Qt::Horizontal);
}
