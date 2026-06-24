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
};

#endif // DIAGRAMCONTROLLER_H
