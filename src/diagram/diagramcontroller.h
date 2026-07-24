#ifndef DIAGRAMCONTROLLER_H
#define DIAGRAMCONTROLLER_H

#include <QImage>
#include <QObject>
#include <QSet>
#include <QString>

#include "diagram.h"

/// \file
/// \brief Controlador que orquesta la previsualización de diagramas en el editor WYSIWYG.

class DiagramRenderer;
class QTextEdit;
class QTimer;

/// Orquesta la previsualización de diagramas en el editor WYSIWYG (opción
/// «imagen bajo el bloque»): detecta los bloques de código `mermaid`/`plantuml`, los manda a
/// renderizar (async, DiagramRenderer) y coloca la imagen resultante en un bloque
/// de presentación justo debajo. Esos bloques se marcan (mddiagram::Preview*) y
/// `documentMarkdown` los quita al serializar, así que el round-trip no los ve.
///
/// Las actualizaciones van con debounce (no en cada tecla) y preservan el estado
/// «modificado»; un flag evita que los propios cambios de preview se realimenten.
class DiagramController : public QObject
{
    Q_OBJECT

public:
    DiagramController(QTextEdit *editor, QObject *parent = nullptr);

    /// Activa/desactiva la previsualización de diagramas. Persiste el ajuste
    /// (global) en AppSettings. Al desactivarla, retira las previews ya colocadas y
    /// deja los bloques como código; al activarla, vuelve a renderizarlos.
    void setEnabled(bool on);
    bool isEnabled() const { return m_enabled; }

public slots:
    /// Programa un refresco (debounce): re-escanea y actualiza las previews.
    void scheduleRefresh();
    /// Re-escanea ya: pide renders de los diagramas y limpia previews huérfanas.
    void refresh();

signals:
    /// \brief Mensaje para la barra de estado de la ventana (texto, ms). Lo emite,
    /// p. ej., un fallo de render de diagrama; EditorStack lo relaya a MainWindow.
    void statusMessage(const QString &text, int timeout);

private:
    struct Region {
        int lastBlockPos;     // posición del último bloque de código del grupo
        int lastBlockNumber;  // su número de bloque
        QString source;       // fuente del diagrama (líneas unidas con \n)
        mddiagram::Kind kind;
    };
    QList<Region> scanRegions() const;
    void onRendered(mddiagram::Kind kind, const QString &source, const QImage &image);
    /// Render fallido. NO avisa de inmediato: un diagrama a medio teclear falla todo
    /// el rato (sintaxis incompleta), y avisar en cada pausa sería molesto. Guarda el
    /// fallo y arranca/reinicia `m_failNotify`; solo si la fuente rota se queda quieta
    /// (sin más fallos) ~1,5 s salta el aviso, una sola vez por fuente (dedup con
    /// m_notifiedFailures, que se rearma al renderizar bien la misma fuente).
    void onFailed(mddiagram::Kind kind, const QString &source, const QString &error);
    void notifyPendingFailure();  // lo dispara m_failNotify cuando el fallo se asienta
    void removeOrphanPreviews(const QList<Region> &regions);
    // Coloca/actualiza el bloque de preview bajo el grupo (imagen o marcador de
    // texto). No hace nada si ya está al día (mismo hash y mismo tipo).
    void setPreviewBlock(int lastBlockNumber, const QString &hash, bool placeholder,
                         const QImage &image, const QString &text);
    // Texto del marcador «herramienta no instalada» con la orden de la plataforma.
    QString placeholderText(mddiagram::Kind kind) const;

    QTextEdit *m_editor = nullptr;
    DiagramRenderer *m_renderer = nullptr;
    QTimer *m_debounce = nullptr;
    bool m_updating = false;  // cambios propios: no re-disparar refresh
    bool m_enabled = true;    // previsualización activada (ajuste global de AppSettings)

    // Aviso de fallo de render «asentado» (ver onFailed).
    QTimer *m_failNotify = nullptr;
    mddiagram::Kind m_pendingFailKind {};
    QString m_pendingFailSource;
    QString m_pendingFailError;
    QSet<QString> m_notifiedFailures;  // hashes ya avisados (dedup; se rearma al renderizar bien)
};

#endif // DIAGRAMCONTROLLER_H
