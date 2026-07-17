/// \file
/// \brief Implementación del controlador de previsualización de diagramas en el editor.

#include "diagramcontroller.h"

#include <QCryptographicHash>
#include <QPalette>
#include <QSysInfo>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QTimer>
#include <QUrl>

#include "diagramdoc.h"
#include "diagramrenderer.h"

namespace {
QString sourceHash(const QString &source)
{
    return QString::fromLatin1(
        QCryptographicHash::hash(source.toUtf8(), QCryptographicHash::Sha1).toHex());
}

QString toolDisplayName(mddiagram::Kind kind)
{
    return kind == mddiagram::Kind::Mermaid ? QStringLiteral("Mermaid (mmdc)")
                                            : QStringLiteral("PlantUML");
}

// Orden de instalación según el sistema operativo en EJECUCIÓN (QSysInfo, sin
// `#ifdef Q_OS_*`). Mermaid va por npm en las tres plataformas; PlantUML cambia.
QString installCommand(mddiagram::Kind kind)
{
    if (kind == mddiagram::Kind::Mermaid)
        return QStringLiteral("npm install -g @mermaid-js/mermaid-cli");
    const QString kernel = QSysInfo::kernelType();  // "linux" / "darwin" / "winnt"
    if (kernel == QLatin1String("darwin"))
        return QStringLiteral("brew install plantuml");
    if (kernel == QLatin1String("winnt"))
        return QStringLiteral("choco install plantuml");
    return QStringLiteral("sudo apt install plantuml");
}
}  // namespace

DiagramController::DiagramController(QTextEdit *editor, QObject *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_renderer(new DiagramRenderer(this))
    , m_debounce(new QTimer(this))
{
    m_debounce->setSingleShot(true);
    m_debounce->setInterval(600);  // no renderizar en cada tecla
    connect(m_debounce, &QTimer::timeout, this, &DiagramController::refresh);
    connect(m_renderer, &DiagramRenderer::rendered, this, &DiagramController::onRendered);

    // Aviso de fallo de render. Un diagrama a medio teclear falla todo el rato
    // (sintaxis incompleta), así que NO se avisa en crudo: el fallo se «asienta» en
    // m_failNotify y solo se avisa si la fuente rota se queda quieta este tiempo.
    // (El caso «falta la herramienta» no llega aquí: refresh pone un MARCADOR inline
    // y ni siquiera llama a render() — ver refresh.)
    m_failNotify = new QTimer(this);
    m_failNotify->setSingleShot(true);
    m_failNotify->setInterval(1500);
    connect(m_failNotify, &QTimer::timeout, this, &DiagramController::notifyPendingFailure);
    connect(m_renderer, &DiagramRenderer::failed, this, &DiagramController::onFailed);
}

void DiagramController::scheduleRefresh()
{
    if (m_updating)
        return;  // cambios provocados por nosotros: no realimentar
    m_debounce->start();
}

QList<DiagramController::Region> DiagramController::scanRegions() const
{
    QList<Region> regions;
    const QTextDocument *doc = m_editor->document();
    QTextBlock b = doc->begin();
    while (b.isValid()) {
        const QTextBlockFormat bf = b.blockFormat();
        if (!bf.hasProperty(QTextFormat::BlockCodeFence)) {
            b = b.next();
            continue;
        }
        // Grupo de bloques de código contiguos con el mismo lenguaje (un bloque
        // vallado = un QTextBlock por línea).
        const QString lang = bf.stringProperty(QTextFormat::BlockCodeLanguage);
        QStringList lines;
        QTextBlock last = b;
        QTextBlock n = b;
        while (n.isValid() && n.blockFormat().hasProperty(QTextFormat::BlockCodeFence)
               && n.blockFormat().stringProperty(QTextFormat::BlockCodeLanguage) == lang) {
            lines << n.text();
            last = n;
            n = n.next();
        }
        const mddiagram::Kind kind = mddiagram::kindForLanguage(lang);
        if (kind != mddiagram::Kind::None)
            regions.append({last.position(), last.blockNumber(), lines.join(QLatin1Char('\n')), kind});
        b = n;
    }
    return regions;
}

QString DiagramController::placeholderText(mddiagram::Kind kind) const
{
    // El símbolo de aviso fuera del tr() para no obligar a traducirlo; la orden
    // tampoco se traduce. %1 = nombre de la herramienta, %2 = orden de instalación.
    return QStringLiteral("⚠ ")
           + tr("%1 no está instalado. Para previsualizar este diagrama: %2")
                 .arg(toolDisplayName(kind), installCommand(kind));
}

void DiagramController::refresh()
{
    if (m_updating)
        return;
    const QList<Region> regions = scanRegions();
    removeOrphanPreviews(regions);

    for (const Region &r : regions) {
        if (r.source.trimmed().isEmpty())
            continue;
        const QString hash = sourceHash(r.source);
        if (DiagramRenderer::isAvailable(r.kind)) {
            m_renderer->render(r.kind, r.source);  // async → onRendered coloca la imagen
        } else {
            // Herramienta ausente: marcador inline con la orden de la plataforma.
            setPreviewBlock(r.lastBlockNumber, hash, /*placeholder=*/true, QImage(),
                            placeholderText(r.kind));
        }
    }
}

void DiagramController::removeOrphanPreviews(const QList<Region> &regions)
{
    QTextDocument *doc = m_editor->document();
    // Preview válida = está justo tras el último bloque de un grupo de diagrama y
    // su hash coincide con la fuente de ese grupo (imagen al día).
    QHash<int, QString> expected;  // blockNumber del último bloque → hash de fuente
    for (const Region &r : regions)
        expected.insert(r.lastBlockNumber, sourceHash(r.source));

    QList<int> orphanPositions;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        if (!b.blockFormat().boolProperty(mddiagram::PreviewBlockProperty))
            continue;
        const QTextBlock prev = b.previous();
        const bool valid =
            prev.isValid() && expected.contains(prev.blockNumber())
            && expected.value(prev.blockNumber())
                   == b.blockFormat().stringProperty(mddiagram::PreviewSourceProperty);
        if (!valid)
            orphanPositions.append(b.position());
    }
    if (orphanPositions.isEmpty())
        return;

    m_updating = true;
    const bool wasModified = doc->isModified();
    QTextCursor c(doc);
    c.beginEditBlock();
    for (auto it = orphanPositions.crbegin(); it != orphanPositions.crend(); ++it) {
        QTextCursor del(doc);
        del.setPosition(*it);
        const QTextBlock block = del.block();
        del.movePosition(QTextCursor::StartOfBlock);
        if (block.next().isValid())
            del.setPosition(block.next().position(), QTextCursor::KeepAnchor);
        else {
            del.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            if (block.position() > 0) {
                del.setPosition(block.position() - 1);
                del.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            }
        }
        del.removeSelectedText();
    }
    c.endEditBlock();
    doc->setModified(wasModified);
    m_updating = false;
}

void DiagramController::onRendered(mddiagram::Kind, const QString &source, const QImage &image)
{
    // Render OK: esta fuente ya no está en fallo. La saca del dedup (si vuelve a
    // romperse, se volverá a avisar) y cancela un aviso pendiente sobre ella.
    m_notifiedFailures.remove(sourceHash(source));
    if (m_pendingFailSource == source) {
        m_failNotify->stop();
        m_pendingFailSource.clear();
    }

    // Encuentra el grupo de diagrama cuya fuente coincide (el documento pudo
    // cambiar desde que se pidió el render) y coloca la imagen bajo él.
    for (const Region &r : scanRegions()) {
        if (r.source != source)
            continue;
        setPreviewBlock(r.lastBlockNumber, sourceHash(source), /*placeholder=*/false,
                        image, QString());
        return;
    }
}

void DiagramController::onFailed(mddiagram::Kind kind, const QString &source,
                                 const QString &error)
{
    // No avisar de inmediato (ver la cabecera): un diagrama a medio teclear falla en
    // cada pausa. Guarda el fallo y (re)arranca el temporizador de «asentamiento»;
    // si esta misma fuente ya se avisó, no insiste.
    if (m_notifiedFailures.contains(sourceHash(source)))
        return;
    m_pendingFailKind = kind;
    m_pendingFailSource = source;
    m_pendingFailError = error;
    m_failNotify->start();
}

void DiagramController::notifyPendingFailure()
{
    // La fuente rota lleva quieta el intervalo de m_failNotify: avisa una vez.
    if (m_pendingFailSource.isEmpty())
        return;
    const QString hash = sourceHash(m_pendingFailSource);
    if (m_notifiedFailures.contains(hash))
        return;
    m_notifiedFailures.insert(hash);
    emit statusMessage(tr("No se pudo previsualizar el diagrama %1: %2")
                           .arg(toolDisplayName(m_pendingFailKind), m_pendingFailError),
                       6000);
}

void DiagramController::setPreviewBlock(int lastBlockNumber, const QString &hash,
                                        bool placeholder, const QImage &image,
                                        const QString &text)
{
    QTextDocument *doc = m_editor->document();
    const QTextBlock last = doc->findBlockByNumber(lastBlockNumber);
    if (!last.isValid())
        return;
    const QTextBlock after = last.next();
    const bool hasPreview =
        after.isValid() && after.blockFormat().boolProperty(mddiagram::PreviewBlockProperty);
    // Ya al día si coincide el hash Y el tipo (imagen vs marcador): así, cuando
    // aparece la herramienta, la imagen reemplaza al marcador aunque el hash sea
    // el mismo.
    if (hasPreview
        && after.blockFormat().stringProperty(mddiagram::PreviewSourceProperty) == hash
        && after.blockFormat().boolProperty(mddiagram::PreviewPlaceholderProperty) == placeholder)
        return;

    m_updating = true;
    const bool wasModified = doc->isModified();

    QTextCursor c(doc);
    if (hasPreview) {
        c.setPosition(after.position());
        c.movePosition(QTextCursor::StartOfBlock);
        c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        c.removeSelectedText();
    } else {
        c.setPosition(last.position());
        c.movePosition(QTextCursor::EndOfBlock);
        c.insertBlock();
    }
    QTextBlockFormat bf;
    bf.setProperty(mddiagram::PreviewBlockProperty, true);
    bf.setProperty(mddiagram::PreviewSourceProperty, hash);
    bf.setProperty(mddiagram::PreviewPlaceholderProperty, placeholder);
    bf.setAlignment(Qt::AlignHCenter);
    c.setBlockFormat(bf);

    if (placeholder) {
        // Marcador de texto atenuado (no parece contenido del documento) y
        // seleccionable, para poder copiar la orden.
        QTextCharFormat cf;
        cf.setFontItalic(true);
        cf.setForeground(m_editor->palette().placeholderText().color());
        c.insertText(text, cf);
    } else {
        const QString name = QStringLiteral("diagram://") + hash;
        doc->addResource(QTextDocument::ImageResource, QUrl(name), QVariant(image));
        QTextImageFormat img;
        img.setName(name);
        // El tamaño de presentación lo decide el handler de imágenes (ImageObject):
        // nativo · zoom, acotado a la vista. Aquí solo se referencia el recurso; el
        // ancho/alto del formato es indiferente (el handler los recalcula).
        c.insertImage(img);
    }

    doc->setModified(wasModified);
    m_updating = false;
}
