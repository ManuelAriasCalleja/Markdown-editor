#ifndef DISTRACTIONFREECONTROLLER_H
#define DISTRACTIONFREECONTROLLER_H

#include <QByteArray>
#include <QObject>

class QMainWindow;
class QShortcut;
class QWidget;
class FocusEditor;
class SplitViewController;
class OutlinePanel;

// Modo sin distracciones: pantalla completa, oculta menú/barras/estado y centra
// el texto en una columna de lectura, con el esquema (si está visible) pegado a
// su izquierda y el conjunto centrado en pantalla. Se sale con ESC o F11.
//
// Opera a nivel de ventana (pantalla completa, geometría, docks), por eso recibe
// el QMainWindow y los widgets que muestra/oculta. Recuerda el estado previo a
// entrar para restaurarlo —y para que, si se cierra dentro del modo, se persista
// ese (ventana normal con barras) y no la pantalla completa.
class DistractionFreeController : public QObject
{
    Q_OBJECT

public:
    DistractionFreeController(QMainWindow *window, FocusEditor *editor,
                              SplitViewController *split, OutlinePanel *outline,
                              QWidget *formatToolBar, QWidget *findBar, QObject *parent);

    bool isActive() const { return m_active; }

    // Geometría/estado de ventana a persistir al cerrar: si el modo está activo,
    // los de ANTES de entrar (no la pantalla completa con las barras ocultas).
    QByteArray sessionGeometry() const;
    QByteArray sessionState() const;

    // Recoloca el bloque [esquema | columna] centrado. Idempotente. Llámalo al
    // redimensionar y al mostrar/ocultar el esquema.
    void updateLayout();

public slots:
    void setActive(bool on);  // entra/sale del modo (conéctalo a la acción F11)

signals:
    // El modo cambió por una vía distinta a la acción (p. ej. ESC); MainWindow
    // sincroniza la marca de la acción del menú.
    void activeChanged(bool on);

private:
    QMainWindow *m_window = nullptr;
    FocusEditor *m_editor = nullptr;
    SplitViewController *m_split = nullptr;
    OutlinePanel *m_outline = nullptr;
    QWidget *m_formatToolBar = nullptr;
    QWidget *m_findBar = nullptr;
    QShortcut *m_escShortcut = nullptr;  // ESC para salir (solo activo en el modo)

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
