#ifndef DIAGRAMCONTROLLER_H
#define DIAGRAMCONTROLLER_H

#include <QImage>
#include <QObject>
#include <QString>

#include "diagram.h"

class DiagramRenderer;
class QTextEdit;
class QTimer;

// Orquesta la previsualización de diagramas en el editor WYSIWYG (opción
// «imagen bajo el bloque»): detecta los bloques ```mermaid/plantuml, los manda a
// renderizar (async, DiagramRenderer) y coloca la imagen resultante en un bloque
// de presentación justo debajo. Esos bloques se marcan (mddiagram::Preview*) y
// `documentMarkdown` los quita al serializar, así que el round-trip no los ve.
//
// Las actualizaciones van con debounce (no en cada tecla) y preservan el estado
// «modificado»; un flag evita que los propios cambios de preview se realimenten.
class DiagramController : public QObject
{
    Q_OBJECT

public:
    DiagramController(QTextEdit *editor, QObject *parent = nullptr);

public slots:
    // Programa un refresco (debounce): re-escanea y actualiza las previews.
    void scheduleRefresh();
    // Re-escanea ya: pide renders de los diagramas y limpia previews huérfanas.
    void refresh();

signals:
    void statusMessage(const QString &text, int timeoutMs);

private:
    struct Region {
        int lastBlockPos;     // posición del último bloque de código del grupo
        int lastBlockNumber;  // su número de bloque
        QString source;       // fuente del diagrama (líneas unidas con \n)
        mddiagram::Kind kind;
    };
    QList<Region> scanRegions() const;
    void onRendered(mddiagram::Kind kind, const QString &source, const QImage &image);
    void removeOrphanPreviews(const QList<Region> &regions);

    QTextEdit *m_editor = nullptr;
    DiagramRenderer *m_renderer = nullptr;
    QTimer *m_debounce = nullptr;
    bool m_updating = false;        // cambios propios: no re-disparar refresh
    bool m_warnedMissingTool = false;
};

#endif // DIAGRAMCONTROLLER_H
