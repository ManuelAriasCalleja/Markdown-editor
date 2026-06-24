#ifndef DIAGRAMRENDERER_H
#define DIAGRAMRENDERER_H

#include <QHash>
#include <QImage>
#include <QObject>
#include <QSet>
#include <QString>

#include "diagram.h"

/// \file
/// \brief Render asíncrono de diagramas Mermaid/PlantUML ejecutando la herramienta externa.

/// Renderiza diagramas Mermaid/PlantUML ejecutando la herramienta externa
/// correspondiente (degradación elegante: si no está instalada, no rompe nada,
/// solo no hay imagen). El render es ASÍNCRONO (QProcess) porque arrancar Java /
/// Node tarda; los resultados se cachean por (motor, fuente) para no re-renderizar
/// lo que no cambia.
///
/// Dependencia OPCIONAL en tiempo de ejecución (como el corrector con Hunspell):
///   - PlantUML: ejecutable `plantuml` (necesita Java).
///   - Mermaid:  ejecutable `mmdc` (mermaid-cli, necesita Node).
/// El núcleo no enlaza nada de terceros; solo lanza procesos si existen.
class DiagramRenderer : public QObject
{
    Q_OBJECT

public:
    explicit DiagramRenderer(QObject *parent = nullptr);

    /// ¿Hay herramienta instalada para este motor? (cacheado tras la 1ª consulta).
    static bool isAvailable(mddiagram::Kind kind);
    /// Ruta del ejecutable de la herramienta, o vacío si no está.
    static QString toolPath(mddiagram::Kind kind);

    /// Lanza (async) el render de `source` con el motor `kind`. Si ya está en
    /// caché, emite `rendered` de inmediato; si ya hay una petición igual en
    /// vuelo, no duplica el proceso. Sin herramienta, emite `failed`.
    void render(mddiagram::Kind kind, const QString &source);

signals:
    /// Emitida con la imagen lista cuando el render termina con éxito (o desde caché).
    void rendered(mddiagram::Kind kind, const QString &source, const QImage &image);
    /// Emitida si el render falla (herramienta ausente, error de sintaxis o de proceso).
    void failed(mddiagram::Kind kind, const QString &source, const QString &error);

private:
    void startPlantUml(const QString &key, const QString &source);
    void startMermaid(const QString &key, const QString &source);

    QHash<QString, QImage> m_cache;  // clave: hash de (kind, source)
    QSet<QString> m_inFlight;        // peticiones en curso (evita duplicar procesos)
};

#endif // DIAGRAMRENDERER_H
