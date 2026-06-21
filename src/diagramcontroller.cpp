#include "diagramcontroller.h"

#include <QCryptographicHash>
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
    // Los fallos (sintaxis incompleta al teclear) son normales: no molestamos con
    // mensajes; la preview simplemente no se actualiza. El caso «falta la
    // herramienta» sí se avisa, desde refresh().
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

void DiagramController::refresh()
{
    if (m_updating)
        return;
    const QList<Region> regions = scanRegions();
    removeOrphanPreviews(regions);

    bool missingTool = false;
    for (const Region &r : regions) {
        if (r.source.trimmed().isEmpty())
            continue;
        if (!DiagramRenderer::isAvailable(r.kind)) {
            missingTool = true;
            continue;
        }
        m_renderer->render(r.kind, r.source);  // async → onRendered coloca la imagen
    }

    // Aviso visible solo si el problema está presente: hay diagramas pero falta
    // la herramienta para renderizarlos.
    if (missingTool && !m_warnedMissingTool) {
        m_warnedMissingTool = true;
        emit statusMessage(
            tr("Hay diagramas, pero falta la herramienta para previsualizarlos "
               "(instala «plantuml» o «mmdc»)."),
            8000);
    } else if (!missingTool) {
        m_warnedMissingTool = false;
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
    const QString hash = sourceHash(source);
    // Encuentra el grupo de diagrama cuya fuente coincide (el documento pudo
    // cambiar desde que se pidió el render).
    for (const Region &r : scanRegions()) {
        if (r.source != source)
            continue;

        QTextDocument *doc = m_editor->document();
        QTextBlock last = doc->findBlockByNumber(r.lastBlockNumber);
        if (!last.isValid())
            return;
        const QTextBlock after = last.next();
        const bool hasPreview =
            after.isValid() && after.blockFormat().boolProperty(mddiagram::PreviewBlockProperty);
        if (hasPreview
            && after.blockFormat().stringProperty(mddiagram::PreviewSourceProperty) == hash)
            return;  // ya está al día

        m_updating = true;
        const bool wasModified = doc->isModified();

        // Recurso de imagen + formato de bloque de preview (centrado, marcado).
        const QString name = QStringLiteral("diagram://") + hash;
        doc->addResource(QTextDocument::ImageResource, QUrl(name), QVariant(image));

        QTextCursor c(doc);
        if (hasPreview) {
            // Reemplaza el contenido del bloque de preview existente.
            c.setPosition(after.position());
            c.movePosition(QTextCursor::StartOfBlock);
            c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
            c.removeSelectedText();
        } else {
            // Inserta un bloque nuevo tras la última línea de código.
            c.setPosition(last.position());
            c.movePosition(QTextCursor::EndOfBlock);
            c.insertBlock();
        }
        QTextBlockFormat bf;
        bf.setProperty(mddiagram::PreviewBlockProperty, true);
        bf.setProperty(mddiagram::PreviewSourceProperty, hash);
        bf.setAlignment(Qt::AlignHCenter);
        c.setBlockFormat(bf);

        QTextImageFormat img;
        img.setName(name);
        const int maxW = m_editor->viewport()->width() - 40;
        if (maxW > 0 && image.width() > maxW) {
            img.setWidth(maxW);
            img.setHeight(image.height() * double(maxW) / image.width());
        } else {
            img.setWidth(image.width());
            img.setHeight(image.height());
        }
        c.insertImage(img);

        doc->setModified(wasModified);
        m_updating = false;
        return;
    }
}
