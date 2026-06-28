/// \file
/// \brief Implementación del modo sin distracciones (pantalla completa y columna de lectura centrada).

#include "distractionfreecontroller.h"

#include <QKeySequence>
#include <QMainWindow>
#include <QMenuBar>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QTabBar>
#include <QTimer>
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
    OutlinePanel *outline, QWidget *formatToolBar, QWidget *findBar,
    QTabBar *tabBar, QObject *parent)
    : QObject(parent)
    , m_window(window)
    , m_editor(editor)
    , m_split(split)
    , m_outline(outline)
    , m_formatToolBar(formatToolBar)
    , m_findBar(findBar)
    , m_tabBar(tabBar)
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
        // Ancho a restaurar al salir. Si el esquema está oculto (o flotante) al
        // entrar, su width() no es un ancho de dock acoplado válido — y si fuera
        // 0, la restauración se saltaría (guard `> 0` en updateLayout) y el dock
        // quedaría con el ancho ensanchado del modo. Usamos el ancho por defecto
        // como base: así, si el usuario muestra el TOC dentro del modo, al salir
        // vuelve a un ancho cómodo en vez de ocupar casi toda la ventana.
        m_normalOutlineWidth = (m_outline->isVisible() && !m_outline->isFloating())
                                   ? m_outline->width()
                                   : qRound(kOutlineTreeWidth * m_uiScale);
        m_preGeometry = m_window->saveGeometry();
        m_preState = m_window->saveState();

        m_window->menuBar()->hide();
        m_formatToolBar->hide();
        m_findBar->hide();
        m_window->statusBar()->hide();
        // El modo es de documento único: oculta la barra de pestañas para que se
        // renderice como antes de la edición por pestañas (sin una barra cruzando
        // todo el ancho sobre la columna de lectura) e impide cambiar de pestaña
        // mientras dura el modo. Se hace ANTES de updateLayout para que el cálculo
        // de la columna/relleno vea ya la configuración final de widgets.
        if (m_tabBar)
            m_tabBar->hide();
        // El esquema no se oculta: sigue pegado a la izquierda del área visible
        // (conserva la visibilidad que tuviera al entrar al modo). Su barra de
        // título sí se oculta: con el dock ensanchado por el relleno izquierdo,
        // dejarla visible la estiraría hasta el borde de la pantalla.
        m_outline->setTitleBarWidget(new QWidget(m_outline));

        const int column = qRound(kReadingColumn * m_uiScale);
        m_editor->setReadingColumnWidth(column);
        m_split->sourceEditor()->setReadingColumnWidth(column);

        m_escShortcut->setEnabled(true);
        m_window->showFullScreen();
    } else {
        m_escShortcut->setEnabled(false);

        m_editor->setReadingColumnWidth(0);
        m_split->sourceEditor()->setReadingColumnWidth(0);

        m_window->menuBar()->show();
        m_formatToolBar->show();
        m_window->statusBar()->show();
        if (m_tabBar)
            m_tabBar->show();
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

void DistractionFreeController::setTargets(FocusEditor *editor, SplitViewController *split)
{
    m_editor = editor;
    m_split = split;
}

void DistractionFreeController::retarget(FocusEditor *editor, SplitViewController *split)
{
    // Con el modo inactivo basta recordar el nuevo destino (lo usará la próxima
    // entrada). Si está activo, trasladamos el modo a la pestaña entrante sin salir
    // de él: quitamos la columna del editor saliente y la ponemos en el nuevo.
    if (!m_active) {
        setTargets(editor, split);
        return;
    }
    if (editor == m_editor)
        return;
    m_editor->setReadingColumnWidth(0);
    m_editor->setColumnLeftAligned(false);
    m_split->sourceEditor()->setReadingColumnWidth(0);

    setTargets(editor, split);

    // El modo es de columna única: si la pestaña entrante estaba en vista dividida,
    // salimos de ella, igual que al entrar al modo.
    if (m_split->splitMode())
        m_split->toggleSplitView(false);
    const int column = qRound(kReadingColumn * m_uiScale);
    m_editor->setReadingColumnWidth(column);
    m_split->sourceEditor()->setReadingColumnWidth(column);
    updateLayout();
}

void DistractionFreeController::setUiScale(qreal scale)
{
    if (scale <= 0 || qFuzzyCompare(scale, m_uiScale))
        return;
    m_uiScale = scale;
    // Si el zoom cambia con el modo ya activo, reescala la columna y recoloca.
    if (m_active) {
        const int column = qRound(kReadingColumn * m_uiScale);
        m_editor->setReadingColumnWidth(column);
        m_split->sourceEditor()->setReadingColumnWidth(column);
        updateLayout();
    }
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
        // había ensanchado para el relleno). DIFERIDO: justo tras salir de
        // pantalla completa el layout aún no se ha asentado y un resizeDocks
        // síncrono no se aplica (mismo motivo por el que MainWindow difiere
        // normalizeOutlineWidth). El guard `!m_active` evita restaurar si se
        // reentró al modo antes de que dispare el temporizador.
        if (!m_active && m_normalOutlineWidth > 0) {
            const int width = m_normalOutlineWidth;
            m_normalOutlineWidth = 0;
            QTimer::singleShot(0, this, [this, width] {
                if (!m_active)
                    m_window->resizeDocks({m_outline}, {width}, Qt::Horizontal);
            });
        }
        return;
    }

    const int tree = qRound(kOutlineTreeWidth * m_uiScale);
    const int column = qRound(kReadingColumn * m_uiScale);
    const int total = m_outline->width() + m_split->splitView()->width();  // dock+central
    const int leftPad = qMax(0, (total - tree - column) / 2);
    const int dockWidth = leftPad + tree;

    m_outline->setLeftPadding(leftPad);
    if (m_outline->width() != dockWidth)  // evita reentrar en el layout sin necesidad
        m_window->resizeDocks({m_outline}, {dockWidth}, Qt::Horizontal);
}
