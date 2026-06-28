#ifndef DISTRACTIONFREECONTROLLER_H
#define DISTRACTIONFREECONTROLLER_H

/// \file
/// \brief Modo sin distracciones: pantalla completa con el texto centrado en una columna de lectura.

#include <QByteArray>
#include <QObject>

class QMainWindow;
class QShortcut;
class QTabBar;
class QWidget;
class FocusEditor;
class SplitViewController;
class OutlinePanel;

/// Modo sin distracciones: pantalla completa, oculta menú/barras/estado y centra
/// el texto en una columna de lectura, con el esquema (si está visible) pegado a
/// su izquierda y el conjunto centrado en pantalla. Se sale con ESC o F11.
///
/// Opera a nivel de ventana (pantalla completa, geometría, docks), por eso recibe
/// el QMainWindow y los widgets que muestra/oculta. Recuerda el estado previo a
/// entrar para restaurarlo —y para que, si se cierra dentro del modo, se persista
/// ese (ventana normal con barras) y no la pantalla completa.
class DistractionFreeController : public QObject
{
    Q_OBJECT

public:
    DistractionFreeController(QMainWindow *window, FocusEditor *editor,
                              SplitViewController *split, OutlinePanel *outline,
                              QWidget *formatToolBar, QWidget *findBar,
                              QTabBar *tabBar, QObject *parent);

    /// \brief Indica si el modo sin distracciones está activo.
    bool isActive() const { return m_active; }

    /// Geometría/estado de ventana a persistir al cerrar: si el modo está activo,
    /// los de ANTES de entrar (no la pantalla completa con las barras ocultas).
    QByteArray sessionGeometry() const;
    /// \brief Estado de ventana a persistir al cerrar (ver sessionGeometry()).
    QByteArray sessionState() const;

    /// Recoloca el bloque [esquema | columna] centrado. Idempotente. Llámalo al
    /// redimensionar y al mostrar/ocultar el esquema.
    void updateLayout();

    /// Reapunta el modo al editor y la vista dividida de la pestaña activa. El
    /// controlador es de la ventana (único), pero el editor y el `split` son de
    /// cada documento, así que al cambiar de pestaña hay que reorientarlo o el
    /// modo aplicaría la columna sobre el editor de la pestaña anterior (oculto) y
    /// el visible quedaría a todo el ancho. Llámalo con el modo ya inactivo.
    void setTargets(FocusEditor *editor, SplitViewController *split);

    /// Traslada el modo a la pestaña activa al cambiar de documento. Si el modo
    /// está activo, lo mantiene: quita la columna del editor saliente y la aplica
    /// al entrante (sin salir de pantalla completa). Si está inactivo, equivale a
    /// setTargets(). Sustituye al antiguo «salir del modo al cambiar de pestaña».
    void retarget(FocusEditor *editor, SplitViewController *split);

    /// Factor de escala de la interfaz (1.0 sin zoom). La columna de lectura y el
    /// ancho del esquema están en píxeles, pero la fuente crece con el zoom; sin
    /// escalarlos la columna se queda demasiado estrecha al ampliar. MainWindow lo
    /// fija al arrancar y en cada cambio de zoom; si el modo está activo, recoloca
    /// en el acto.
    void setUiScale(qreal scale);

public slots:
    /// \brief Entra/sale del modo (conéctalo a la acción F11).
    void setActive(bool on);

signals:
    /// El modo cambió por una vía distinta a la acción (p. ej. ESC); MainWindow
    /// sincroniza la marca de la acción del menú.
    void activeChanged(bool on);

private:
    QMainWindow *m_window = nullptr;
    FocusEditor *m_editor = nullptr;
    SplitViewController *m_split = nullptr;
    OutlinePanel *m_outline = nullptr;
    QWidget *m_formatToolBar = nullptr;
    QWidget *m_findBar = nullptr;
    QTabBar *m_tabBar = nullptr;          // barra de pestañas (se oculta en el modo)
    QShortcut *m_escShortcut = nullptr;  // ESC para salir (solo activo en el modo)

    qreal m_uiScale = 1.0;                // factor de zoom de la interfaz (1.0 = sin zoom)
    bool m_active = false;
    bool m_wasMaximized = false;          // estado de ventana previo, para restaurar
    bool m_findBarWasVisible = false;     // visibilidad previa de la barra de búsqueda
    // Geometría y disposición de barras justo antes de entrar, para restaurarlas
    // al salir y, sobre todo, para persistir esas si se cierra dentro del modo.
    QByteArray m_preGeometry;
    QByteArray m_preState;
    int m_normalOutlineWidth = 0;         // ancho del dock fuera del modo, para restaurarlo
};

#endif // DISTRACTIONFREECONTROLLER_H
