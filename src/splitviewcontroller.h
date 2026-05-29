#ifndef SPLITVIEWCONTROLLER_H
#define SPLITVIEWCONTROLLER_H

#include <QObject>
#include <QList>

#include <functional>

class QAction;
class QSplitter;
class QTextEdit;
class QTimer;
class QWidget;
class FocusEditor;
class FindReplaceBar;
class OutlinePanel;
class ThemeController;

// Controla la vista del código Markdown crudo: el modo fuente a pantalla
// completa, la vista dividida (WYSIWYG + fuente lado a lado, ambos editables) y
// la sincronización con debounce entre los dos paneles.
//
// Posee el editor de fuente, el QSplitter central y los dos temporizadores de
// sincronización, junto con la maquinaria de los flags de modo, «fuente sucio» y
// anti-bucle. El editor WYSIWYG y los colaboradores que necesita (barra de
// búsqueda, índice, tema) se inyectan por referencia, no son propiedad suya. Los
// refrescos propios de MainWindow (contador de palabras, acciones contextuales,
// marca de modificado) se piden por señal.
//
// Regla de sincronización: solo se actualiza el panel SIN foco, nunca el que el
// usuario está editando, para no provocar saltos de cursor ni reescribirle el
// texto. Ver syncSourceFromDocument / syncDocumentFromSource.
//
// Modo fuente a pantalla completa y vista dividida son excluyentes.
class SplitViewController : public QObject
{
    Q_OBJECT

    friend class TestSplitView;  // pruebas de caracterización (sync interno)

public:
    // `parent` actúa como padre QObject y como padre de los widgets creados (el
    // QSplitter y el editor de fuente). `editor` es el editor WYSIWYG ya creado.
    SplitViewController(FocusEditor *editor,
                        FindReplaceBar *findBar,
                        OutlinePanel *outline,
                        ThemeController *theme,
                        QWidget *parent);

    // Widgets que MainWindow necesita para el zoom, el modo sin distracciones, el
    // filtro de eventos y el guardado del estado del divisor.
    FocusEditor *sourceEditor() const { return m_sourceEditor; }
    QSplitter *splitView() const { return m_splitView; }

    // Acciones de menú que reflejan/disparan los modos (se conectan a los toggles)
    // y la lista de acciones válidas solo en WYSIWYG. Se fijan tras crear los menús.
    void setModeActions(QAction *sourceModeAction, QAction *splitAction);
    void setWysiwygActions(const QList<QAction *> &actions) { m_wysiwygActions = actions; }
    // Renderiza un cuerpo Markdown en el documento WYSIWYG dejándolo plenamente al
    // día (MainWindow::setBodyMarkdown). Es el único punto de re-render.
    void setRenderBody(std::function<void(const QString &)> fn) { m_renderBody = std::move(fn); }

    // Editor actualmente visible/activo (fuente en modo fuente; según el foco en
    // dividido; WYSIWYG en otro caso).
    QTextEdit *activeEditor() const;
    bool sourceMode() const { return m_sourceMode; }
    bool splitMode() const { return m_splitMode; }
    bool isSourceDirty() const { return m_sourceDirty; }

    // Si el panel de fuente tiene cambios sin volcar, los aplica al documento.
    void commitSourceToDocument();
    // Refresca el panel de fuente con el Markdown actual del documento (si está
    // visible). Lo usa MainWindow tras recargar el archivo desde disco.
    void refreshSourceFromDocument();

    // Guarda/restaura el flag anti-bucle para envolver una sustitución
    // programática del documento (la usa MainWindow::setBodyMarkdown): los
    // contentsChanged que provoque no deben realimentar la sincronización.
    bool beginProgrammaticChange();
    void endProgrammaticChange(bool previous) { m_syncing = previous; }

public slots:
    void toggleSourceMode(bool on);
    void toggleSplitView(bool on);

signals:
    void wordCountShouldUpdate();
    void tableActionsShouldUpdate();
    void formatActionsShouldUpdate();
    void documentModified();

private:
    void syncSourceFromDocument();
    void syncDocumentFromSource();
    void flushPendingSync(QWidget *losingFocus);
    void onSourceTextChanged();
    void onDocumentContentsChanged();
    void onFocusChanged(QWidget *old, QWidget *now);
    void updateEditorVisibility();
    void updateActionsForFocus();
    void setWysiwygActionsEnabled(bool enabled);

    FocusEditor *m_editor = nullptr;        // WYSIWYG (no es propiedad nuestra)
    FocusEditor *m_sourceEditor = nullptr;  // vista de código Markdown crudo
    QSplitter *m_splitView = nullptr;       // contiene WYSIWYG y fuente lado a lado
    FindReplaceBar *m_findBar = nullptr;
    OutlinePanel *m_outline = nullptr;
    ThemeController *m_theme = nullptr;
    QAction *m_sourceModeAction = nullptr;
    QAction *m_splitAction = nullptr;
    QList<QAction *> m_wysiwygActions;
    std::function<void(const QString &)> m_renderBody;

    bool m_sourceMode = false;              // fuente a pantalla completa
    bool m_splitMode = false;               // WYSIWYG + fuente lado a lado
    bool m_sourceDirty = false;             // el fuente tiene cambios sin volcar
    bool m_syncing = false;                 // actualización programática en curso (anti-bucle)
    QTimer *m_syncToSourceTimer = nullptr;  // debounce WYSIWYG -> fuente
    QTimer *m_syncToDocTimer = nullptr;     // debounce fuente -> WYSIWYG
};

#endif // SPLITVIEWCONTROLLER_H
