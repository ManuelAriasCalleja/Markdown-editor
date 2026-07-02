/// \file
/// \brief Implementación del render asíncrono de diagramas vía QProcess (plantuml/mmdc).

#include "diagramrenderer.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <memory>

namespace {

QString toolName(mddiagram::Kind kind)
{
    switch (kind) {
    case mddiagram::Kind::PlantUml: return QStringLiteral("plantuml");
    case mddiagram::Kind::Mermaid:  return QStringLiteral("mmdc");
    case mddiagram::Kind::None:     break;
    }
    return QString();
}

QString cacheKey(mddiagram::Kind kind, const QString &source)
{
    QCryptographicHash h(QCryptographicHash::Sha1);
    h.addData(QByteArray::number(static_cast<int>(kind)));
    h.addData(source.toUtf8());
    return QString::fromLatin1(h.result().toHex());
}

// PlantUML necesita los delimitadores @startuml/@enduml; muchos bloques de
// Markdown los omiten, así que los añadimos si faltan.
QString wrapPlantUml(const QString &source)
{
    if (source.contains(QStringLiteral("@start")))
        return source;
    return QStringLiteral("@startuml\n") + source + QStringLiteral("\n@enduml\n");
}

}  // namespace

DiagramRenderer::DiagramRenderer(QObject *parent) : QObject(parent) {}

QString DiagramRenderer::toolPath(mddiagram::Kind kind)
{
    const QString name = toolName(kind);
    if (name.isEmpty())
        return QString();
    return QStandardPaths::findExecutable(name);
}

bool DiagramRenderer::isAvailable(mddiagram::Kind kind)
{
    return !toolPath(kind).isEmpty();
}

void DiagramRenderer::render(mddiagram::Kind kind, const QString &source)
{
    if (kind == mddiagram::Kind::None)
        return;
    const QString key = cacheKey(kind, source);
    const auto cached = m_cache.constFind(key);
    if (cached != m_cache.constEnd()) {
        emit rendered(kind, source, cached.value());
        return;
    }
    if (m_inFlight.contains(key))
        return;  // ya en curso: el resultado llegará por la señal
    if (toolPath(kind).isEmpty()) {
        emit failed(kind, source, tr("herramienta no encontrada"));
        return;
    }
    m_inFlight.insert(key);
    if (kind == mddiagram::Kind::PlantUml)
        startPlantUml(key, source);
    else
        startMermaid(key, source);
}

void DiagramRenderer::startPlantUml(const QString &key, const QString &source)
{
    auto *proc = new QProcess(this);
    const auto kind = mddiagram::Kind::PlantUml;
    // Un proceso que crashea emite AMBAS señales (errorOccurred + finished): el
    // guard `done` evita emitir `failed`/`rendered` por duplicado.
    auto done = std::make_shared<bool>(false);
    connect(proc, &QProcess::errorOccurred, this, [=] {
        if (*done)
            return;
        *done = true;
        m_inFlight.remove(key);
        emit failed(kind, source, proc->errorString());
        proc->deleteLater();
    });
    connect(proc, &QProcess::finished, this,
            [=](int code, QProcess::ExitStatus) {
                if (*done)
                    return;
                *done = true;
                m_inFlight.remove(key);
                QImage img;
                if (code == 0)
                    img.loadFromData(proc->readAllStandardOutput(), "PNG");
                if (!img.isNull()) {
                    m_cache.insert(key, img);
                    emit rendered(kind, source, img);
                } else {
                    emit failed(kind, source,
                                QString::fromUtf8(proc->readAllStandardError()));
                }
                proc->deleteLater();
            });
    // -pipe: lee la fuente de stdin y escribe el PNG en stdout.
    proc->start(toolPath(kind),
                {QStringLiteral("-tpng"), QStringLiteral("-pipe"),
                 QStringLiteral("-charset"), QStringLiteral("UTF-8")});
    proc->write(wrapPlantUml(source).toUtf8());
    proc->closeWriteChannel();
}

void DiagramRenderer::startMermaid(const QString &key, const QString &source)
{
    const auto kind = mddiagram::Kind::Mermaid;
    // mmdc trabaja con ficheros: fuente .mmd de entrada, .png de salida.
    auto *dir = new QTemporaryDir;
    if (!dir->isValid()) {
        m_inFlight.remove(key);
        emit failed(kind, source, tr("no se pudo crear un directorio temporal"));
        delete dir;
        return;
    }
    const QString inPath = dir->filePath(QStringLiteral("in.mmd"));
    const QString outPath = dir->filePath(QStringLiteral("out.png"));
    QFile in(inPath);
    if (!in.open(QIODevice::WriteOnly)) {
        m_inFlight.remove(key);
        emit failed(kind, source, tr("no se pudo escribir la fuente"));
        delete dir;
        return;
    }
    in.write(source.toUtf8());
    in.close();

    auto *proc = new QProcess(this);
    // Un proceso que crashea emite AMBAS señales (errorOccurred + finished): el
    // guard `done` garantiza que la limpieza (y el `delete dir`) corre una sola
    // vez y que no se emite `failed`/`rendered` por duplicado.
    auto done = std::make_shared<bool>(false);
    connect(proc, &QProcess::errorOccurred, this, [=] {
        if (*done)
            return;
        *done = true;
        m_inFlight.remove(key);
        emit failed(kind, source, proc->errorString());
        proc->deleteLater();
        delete dir;
    });
    connect(proc, &QProcess::finished, this,
            [=](int code, QProcess::ExitStatus) {
                if (*done)
                    return;
                *done = true;
                m_inFlight.remove(key);
                QImage img;
                if (code == 0)
                    img.load(outPath, "PNG");
                if (!img.isNull()) {
                    m_cache.insert(key, img);
                    emit rendered(kind, source, img);
                } else {
                    emit failed(kind, source,
                                QString::fromUtf8(proc->readAllStandardError()));
                }
                proc->deleteLater();
                delete dir;
            });
    proc->start(toolPath(kind),
                {QStringLiteral("-i"), inPath, QStringLiteral("-o"), outPath,
                 QStringLiteral("-b"), QStringLiteral("white")});
}
