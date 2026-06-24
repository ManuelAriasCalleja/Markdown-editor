#ifndef FILECONTROLLER_H
#define FILECONTROLLER_H

/// \file
/// \brief Operaciones de archivo (nuevo/abrir/guardar) y autoguardado del borrador.

#include <QObject>
#include <QString>

#include <functional>

class QTextEdit;
class QTimer;
class QWidget;
class DocumentIo;
class RecoveryManager;
class SplitViewController;

/// \brief Operaciones de archivo del documento: nuevo, abrir, guardar / guardar como,
/// «¿guardar cambios?» antes de descartar, y el autoguardado del borrador de
/// recuperación. Orquesta DocumentIo (E/S) y RecoveryManager (borrador), volcando
/// antes el panel de fuente a través del SplitViewController y mostrando los
/// diálogos necesarios con `parent` como padre.
///
/// No posee DocumentIo ni RecoveryManager (siguen en MainWindow, con su cableado
/// de señales): se le inyectan. Sí posee el temporizador de autoguardado. Los
/// mensajes de estado, la marca «modificado» de la ventana y el fallo de carga se
/// comunican por señal, para no acoplarse a la ventana.
class FileController : public QObject
{
    Q_OBJECT

public:
    FileController(QTextEdit *editor, DocumentIo *documentIo, SplitViewController *split,
                   RecoveryManager *recovery, QWidget *parent);

    /// \brief Re-render del cuerpo Markdown en el documento (MainWindow::setBodyMarkdown),
    /// usado al recuperar un borrador. Se inyecta porque toca tema e índice.
    void setRenderBody(std::function<void(const QString &)> fn) { m_renderBody = std::move(fn); }

    /// \brief Cuerpo Markdown editable: del panel de fuente si tiene cambios sin volcar
    /// (modo fuente o vista dividida); si no, serializado del documento WYSIWYG.
    QString currentBody() const;

    /// \brief Guarda el documento en `path` (vuelca antes el fuente). false si falla.
    bool writeToFile(const QString &path);
    /// \brief Si hay cambios sin guardar, pregunta. Devuelve false solo si el usuario
    /// cancela (true si guarda o descarta). Llámalo antes de descartar el documento.
    bool maybeSave();
    /// \brief Recupera el borrador de la sesión anterior, si lo hay. Devuelve false si no
    /// había borrador.
    bool recoverDraft();

    /// \brief Marca que hubo cambios desde el último autoguardado (lo conecta MainWindow a
    /// los textChanged de los editores).
    void markDirty() { m_autosaveDirty = true; }
    /// \brief Arranca el autoguardado periódico. Se llama al final de startSession (no
    /// antes: correría durante el diálogo de recuperación con el documento vacío).
    void startAutosave();

    /// \brief Guarda en AppSettings la posición del cursor del archivo actual, para
    /// reabrirlo donde se dejó. Lo llama MainWindow al cerrar (los cambios de
    /// documento ya lo hacen internamente).
    void rememberCursorPosition();

public slots:
    /// \brief Empieza un documento nuevo y vacío (preguntando antes si hay cambios).
    void newFile();
    /// \brief Documento nuevo prerrellenado con el cuerpo Markdown de una plantilla.
    void newFromTemplate(const QString &body);
    /// \brief Pide un archivo con un diálogo y lo abre.
    void openFileDialog();
    /// \brief Carga y renderiza `path` (preguntando antes si hay cambios sin guardar).
    void openFile(const QString &path);
    /// \brief Guarda en la ruta actual, o pide una con «Guardar como» si no la hay.
    bool save();
    /// \brief Pide una ruta con un diálogo y guarda en ella.
    bool saveAs();
    /// \brief Abre, en el gestor de archivos del sistema, la carpeta del documento actual
    /// (multiplataforma vía QDesktopServices). Avisa si aún no está guardado.
    void openContainingFolder();

signals:
    /// \brief Mensaje para la barra de estado (`timeoutMs` 0 = permanente).
    void statusMessage(const QString &text, int timeoutMs);
    /// \brief Cambió el estado «modificado» de la ventana.
    void windowModifiedChanged(bool modified);
    /// \brief La carga de `path` falló (p. ej. para quitarlo de los recientes).
    void loadFailed(const QString &path);

private:
    void autosaveDraft();
    // Lleva el cursor a la posición recordada del archivo actual (si la hay).
    void restoreCursorPosition();

    QTextEdit *m_editor = nullptr;          // editor WYSIWYG (no es propiedad nuestra)
    DocumentIo *m_documentIo = nullptr;
    SplitViewController *m_split = nullptr;
    RecoveryManager *m_recovery = nullptr;
    QWidget *m_parent = nullptr;            // padre de los diálogos
    std::function<void(const QString &)> m_renderBody;

    QTimer *m_autosaveTimer = nullptr;
    bool m_autosaveDirty = false;           // hubo cambios desde el último autoguardado
};

#endif // FILECONTROLLER_H
