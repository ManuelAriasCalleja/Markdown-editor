/// \file
/// \brief Piezas COMUNES de la exportación: catálogo de idiomas, lectura del front
///        matter, el clon plano que usan casi todos los formatos y la paginación
///        del PDF. Cada formato vive en su propio `export<formato>.cpp`.

#include "exporters.h"

#include "mathblocks.h"
#include "printdecor.h"

#include <QAbstractTextDocumentLayout>
#include <QImage>
#include <QPixmap>
#include <QTextImageFormat>
#include <QUrl>
#include <QVariant>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPrinter>
#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextLayout>

#include <algorithm>

namespace mdexport {

QList<Language> languages()
{
    // code, nombre nativo, babel, fo:language, fo:country.
    return {
        {QStringLiteral("es"), QStringLiteral("Español"),   QStringLiteral("spanish"),    QStringLiteral("es"), QStringLiteral("ES")},
        {QStringLiteral("en"), QStringLiteral("English"),   QStringLiteral("english"),    QStringLiteral("en"), QStringLiteral("US")},
        {QStringLiteral("de"), QStringLiteral("Deutsch"),   QStringLiteral("ngerman"),    QStringLiteral("de"), QStringLiteral("DE")},
        {QStringLiteral("fr"), QStringLiteral("Français"),  QStringLiteral("french"),     QStringLiteral("fr"), QStringLiteral("FR")},
        {QStringLiteral("it"), QStringLiteral("Italiano"),  QStringLiteral("italian"),    QStringLiteral("it"), QStringLiteral("IT")},
        {QStringLiteral("pt"), QStringLiteral("Português"), QStringLiteral("portuguese"), QStringLiteral("pt"), QStringLiteral("PT")},
        {QStringLiteral("pl"), QStringLiteral("Polski"),    QStringLiteral("polish"),     QStringLiteral("pl"), QStringLiteral("PL")},
        {QStringLiteral("nl"), QStringLiteral("Nederlands"),QStringLiteral("dutch"),      QStringLiteral("nl"), QStringLiteral("NL")},
        {QStringLiteral("ro"), QStringLiteral("Română"),    QStringLiteral("romanian"),   QStringLiteral("ro"), QStringLiteral("RO")},
    };
}

Language languageForCode(const QString &code)
{
    // Normaliza "es-ES"/"es_ES" → "es".
    const QString base = code.left(2).toLower();
    const QList<Language> langs = languages();
    for (const Language &l : langs)
        if (l.code == base)
            return l;
    return langs.at(1);  // inglés como recurso seguro
}

QString frontMatterValue(const QString &frontMatter, const QString &key)
{
    const QRegularExpression re(
        QStringLiteral("(?im)^[ \\t]*%1[ \\t]*[:=][ \\t]*(.*)$").arg(key));
    const QRegularExpressionMatch m = re.match(frontMatter);
    if (!m.hasMatch())
        return QString();
    QString value = m.captured(1).trimmed();
    // Quita comillas envolventes (YAML/TOML).
    if (value.size() >= 2
        && ((value.startsWith(QLatin1Char('"')) && value.endsWith(QLatin1Char('"')))
            || (value.startsWith(QLatin1Char('\'')) && value.endsWith(QLatin1Char('\'')))))
        value = value.mid(1, value.size() - 2);
    return value;
}

PdfInfo pdfDocumentInfo(const QString &frontMatter)
{
    PdfInfo info;
    info.title = frontMatterValue(frontMatter, QStringLiteral("title"));
    info.creator = frontMatterValue(frontMatter, QStringLiteral("author"));
    if (info.creator.isEmpty())
        info.creator = frontMatterValue(frontMatter, QStringLiteral("creator"));
    return info;
}

namespace {
// El resaltado de sintaxis lo pinta un QSyntaxHighlighter como overlay de la
// maqueta (block.layout()->formats()), no como formato de carácter del documento,
// así que clone() no lo copia y el código se exportaría sin color. Lo «horneamos»
// en el clon copiando esos formatos como formato de carácter real, SOLO en los
// bloques de código (BlockCodeFence): así no arrastramos el subrayado ortográfico
// (que vive en la prosa) ni el color de las fórmulas (que se expanden aparte). Se
// hace antes de las ediciones de math: solo cambia formatos, no longitudes, así
// que las posiciones siguen coincidiendo con `src`.
void bakeCodeHighlighting(const QTextDocument *src, QTextDocument *out)
{
    QTextCursor cursor(out);
    for (QTextBlock b = src->begin(); b.isValid(); b = b.next()) {
        if (!b.blockFormat().hasProperty(QTextFormat::BlockCodeFence))
            continue;
        const QTextLayout *layout = b.layout();
        if (!layout)
            continue;
        const int base = b.position();
        const QList<QTextLayout::FormatRange> ranges = layout->formats();
        for (const QTextLayout::FormatRange &r : ranges) {
            if (r.length <= 0)
                continue;
            cursor.setPosition(base + r.start);
            cursor.setPosition(base + r.start + r.length, QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(r.format);
        }
    }
}
} // namespace

QTextDocument *cloneForExport(const QTextDocument *src)
{
    // Clon directo: preserva fragmentos, formatos y vertical-align (lo que
    // hace que HTML/PDF/ODF muestren los super/subíndices reales). Después:
    //   - los runs de math inline: solo limpiamos sus propiedades custom (son
    //     internas del editor y no las entiende ningún writer);
    //   - las fórmulas 2D (un carácter objeto MathObjectType, que los writers
    //     tampoco saben pintar): las EXPANDIMOS a esos mismos runs inline
    //     (cursiva + super/subíndice de Qt), la representación que sí exporta a
    //     HTML/ODF/PDF/DOCX. La maquetación 2D es solo de pantalla.
    // Recolectamos primero y aplicamos en orden descendente para no invalidar
    // posiciones (las expansiones cambian la longitud).
    QTextDocument *out = src->clone();
    // clone() NO copia la baseUrl (ni la caché de recursos): sin ella, las
    // imágenes de ruta relativa (`![](imagen.png)`) no se resuelven y desaparecen
    // del PDF, la impresión, la vista previa y el ODF. La copiamos para que
    // doc->resource() las cargue desde disco.
    out->setBaseUrl(src->baseUrl());

    // Normaliza el TAMAÑO de la fuente por defecto a un cuerpo estándar. El zoom de
    // interfaz se aplica agrandando la fuente del editor (editor->setFont), y clone()
    // copia esa fuente aumentada como fuente por defecto del documento: sin esto, el
    // zoom de PANTALLA se colaría en la impresión, el PDF y las exportaciones
    // (HTML/ODF/EPUB), que saldrían con una letra desproporcionada. Se conserva la
    // familia y demás atributos; los encabezados usan pasos relativos
    // (FontSizeAdjustment), así que reescalan proporcionalmente solos. DOCX y LaTeX
    // no pasan por aquí: llevan sus propios tamaños fijos. 11 pt = cuerpo estándar,
    // igual que el estilo Normal del DOCX.
    constexpr qreal kExportBodyPointSize = 11.0;
    QFont df = out->defaultFont();
    df.setPointSizeF(kExportBodyPointSize);
    out->setDefaultFont(df);

    bakeCodeHighlighting(src, out);  // conserva el color del código (overlay -> char format)
    QTextCursor c(out);
    struct MathEdit { int start; int end; bool isObject; QString tex; QTextCharFormat cleared; };
    QList<MathEdit> edits;
    for (QTextBlock b = out->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid())
                continue;
            QTextCharFormat cf = frag.charFormat();
            if (!cf.boolProperty(mdmath::IsMathProperty))
                continue;
            if (cf.objectType() == mdmath::MathObjectType) {
                edits.append({frag.position(), frag.position() + frag.length(), true,
                              cf.property(mdmath::MathTexProperty).toString(), {}});
            } else {
                cf.clearProperty(mdmath::IsMathProperty);
                cf.clearProperty(mdmath::MathTexProperty);
                cf.clearProperty(mdmath::MathBlockProperty);
                edits.append({frag.position(), frag.position() + frag.length(), false,
                              QString(), cf});
            }
        }
    }
    std::sort(edits.begin(), edits.end(),
              [](const MathEdit &a, const MathEdit &b) { return a.start > b.start; });
    for (const MathEdit &e : edits) {
        c.setPosition(e.start);
        c.setPosition(e.end, QTextCursor::KeepAnchor);
        if (!e.isObject) {
            c.setCharFormat(e.cleared);
            continue;
        }
        c.removeSelectedText();
        QTextCharFormat base;
        base.setFontItalic(true);  // convención tipográfica de las matemáticas
        c.setPosition(e.start);
        for (const mdmath::MathRun &r : mdmath::renderTexAsRuns(e.tex, base))
            c.insertText(r.text, r.fmt);
    }

    // Qt hornea un tamaño de fuente ABSOLUTO en los runs de código (inline y
    // bloque): setMarkdown les fija `FontPointSize` = el tamaño por defecto AL
    // IMPORTAR, que con el zoom de interfaz es el tamaño de pantalla. Normalizar
    // solo el defaultFont (arriba) no los toca, así que saldrían más grandes (o más
    // pequeños) que el cuerpo. Se quita ese tamaño absoluto para que hereden el
    // cuerpo normalizado (conservando la familia monospace y el color). Es el único
    // formato con tamaño absoluto: los encabezados usan pasos relativos
    // (FontSizeAdjustment) y el resto hereda el defaultFont.
    for (QTextBlock b = out->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || !frag.charFormat().hasProperty(QTextFormat::FontPointSize))
                continue;
            QTextCharFormat cleared = frag.charFormat();
            cleared.clearProperty(QTextFormat::FontPointSize);
            c.setPosition(frag.position());
            c.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            c.setCharFormat(cleared);
        }
    }
    return out;
}

void bakeImageResources(QTextDocument *doc)
{
    if (!doc)
        return;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || !frag.charFormat().isImageFormat())
                continue;
            const QString name = frag.charFormat().toImageFormat().name();
            if (name.isEmpty())
                continue;
            // resource() resuelve por baseUrl y deja el resultado en la caché; se
            // re-registra como recurso explícito, que clone() sí copia.
            const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(name));
            if (res.isValid())
                doc->addResource(QTextDocument::ImageResource, QUrl(name), res);
        }
    }
}

void clampImagesToWidth(QTextDocument *doc, qreal maxWidth, qreal dpiScale)
{
    if (!doc || maxWidth <= 0 || dpiScale <= 0)
        return;
    // La maqueta multiplica por `dpiScale` TODOS los tamaños al pintar en el
    // dispositivo: tanto el intrínseco de la imagen como un width/height explícito
    // del formato (que Qt trata como «px de pantalla»). Por eso el tope se traduce
    // a unidades de formato; fijar aquí píxeles de dispositivo saldría re-escalado
    // otra vez y la imagen quedaría gigante.
    const qreal maxFormatWidth = maxWidth / dpiScale;
    // Recoge primero y aplica después: cambiar formatos no mueve texto, pero no
    // conviene editar mientras se itera sobre los fragmentos.
    struct Clamp { int start; int length; QTextImageFormat fmt; };
    QList<Clamp> clamps;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || !frag.charFormat().isImageFormat())
                continue;
            QTextImageFormat fmt = frag.charFormat().toImageFormat();

            // Anchura con la que la maqueta va a pintarla: la fijada en el formato
            // (unidades de documento, tal cual) o, si no hay, la intrínseca del
            // recurso escalada al dpi del dispositivo.
            // Anchura en unidades de formato: la explícita, o la intrínseca del
            // recurso (que la maqueta usa tal cual como base antes del re-escalado).
            qreal width = fmt.width();
            if (width <= 0) {
                const QVariant res =
                    doc->resource(QTextDocument::ImageResource, QUrl(fmt.name()));
                QImage img = qvariant_cast<QImage>(res);
                if (img.isNull()) {
                    const QPixmap pm = qvariant_cast<QPixmap>(res);
                    if (!pm.isNull())
                        img = pm.toImage();
                }
                if (img.isNull())
                    continue;  // no se puede medir (remota/ilegible): se deja estar
                width = img.width();
            }
            if (width <= maxFormatWidth)
                continue;

            // Solo la anchura: si la altura estaba en automático, sigue en
            // automático y la proporción se conserva sola; si estaba fijada, se
            // reescala con el mismo factor.
            if (fmt.height() > 0)
                fmt.setHeight(fmt.height() * maxFormatWidth / width);
            fmt.setWidth(maxFormatWidth);
            clamps.append({frag.position(), frag.length(), fmt});
        }
    }
    for (const Clamp &c : clamps) {
        QTextCursor cursor(doc);
        cursor.setPosition(c.start);
        cursor.setPosition(c.start + c.length, QTextCursor::KeepAnchor);
        cursor.setCharFormat(c.fmt);
    }
}

void paintPaginated(QPrinter *printer, QTextDocument *doc, bool footerPageNumbers)
{
    if (!printer || !doc)
        return;
    QPainter painter(printer);
    if (!painter.isActive())
        return;
    // Métricas del dispositivo de impresión: la maqueta usa su DPI (como hace el
    // propio QTextDocument::print internamente).
    doc->documentLayout()->setPaintDevice(printer);

    const QRectF printable = printer->pageRect(QPrinter::DevicePixel);
    // Franja del pie: dos alturas de línea de la fuente por defecto (aire + texto).
    const QFontMetricsF fm(doc->defaultFont(), printer);
    const qreal footerH = footerPageNumbers ? fm.height() * 2.0 : 0.0;
    const QSizeF bodySize(printable.width(), qMax<qreal>(1.0, printable.height() - footerH));
    doc->setPageSize(bodySize);

    const int pages = qMax(1, doc->pageCount());
    painter.translate(printable.topLeft());  // (0,0) = esquina de la zona imprimible
    for (int i = 0; i < pages; ++i) {
        if (i > 0)
            printer->newPage();
        // Contenido de la página i: se desplaza su franja al origen y se recorta a
        // la altura del cuerpo (para que no invada el pie ni la página siguiente).
        painter.save();
        const QRectF pageBody(0, bodySize.height() * i, bodySize.width(), bodySize.height());
        painter.setClipRect(QRectF(0, 0, bodySize.width(), bodySize.height()));
        painter.translate(0, -pageBody.top());
        doc->drawContents(&painter, pageBody);
        painter.restore();
        // Pie con el número de página, centrado bajo el cuerpo.
        if (footerPageNumbers) {
            painter.save();
            painter.setFont(doc->defaultFont());
            const QRectF footer(0, bodySize.height(), printable.width(), footerH);
            painter.drawText(footer, Qt::AlignHCenter | Qt::AlignVCenter,
                             mdprintdecor::pageNumberText(i + 1, pages));
            painter.restore();
        }
    }
}

} // namespace mdexport
