/// \file
/// \brief Exportación a HTML: el cuerpo lo escribe Qt (`toHtml`) y aquí se le añade lo
///        que no pone (idioma, título) y se embeben las imágenes.

#include "exporters.h"
#include "exportutil.h"

#include <QRegularExpression>
#include <QStringList>
#include <QTextDocument>

namespace mdexport {

// ---------------------------------------------------------------------------
// EPUB
// ---------------------------------------------------------------------------

QString htmlBodyToXhtml(const QString &fullHtml)
{
    QString inner;
    int start = fullHtml.indexOf(QLatin1String("<body"));
    if (start >= 0) {
        start = fullHtml.indexOf(QLatin1Char('>'), start);
        const int end = fullHtml.lastIndexOf(QLatin1String("</body>"));
        if (start >= 0 && end > start)
            inner = fullHtml.mid(start + 1, end - start - 1);
    }
    if (inner.isEmpty())
        inner = fullHtml;

    // `&nbsp;` no es una entidad XML válida sin DTD; los elementos vacíos deben
    // ir autocerrados (Qt ya lo hace, pero curamos `<br>`/`<hr>` por seguridad).
    inner.replace(QLatin1String("&nbsp;"), QLatin1String("&#160;"));
    static const QRegularExpression br(QStringLiteral("<br>"),
                                       QRegularExpression::CaseInsensitiveOption);
    inner.replace(br, QStringLiteral("<br/>"));
    static const QRegularExpression hr(QStringLiteral("<hr>"),
                                       QRegularExpression::CaseInsensitiveOption);
    inner.replace(hr, QStringLiteral("<hr/>"));
    return inner.trimmed();
}

namespace {

// `data:` URI de una imagen del documento, o "" si no se puede obtener. Se
// conservan los bytes originales de los formatos que entiende el navegador.
QString imageDataUri(const QString &name, const QTextDocument *doc)
{
    static const QStringList browserFormats = {
        QStringLiteral("png"), QStringLiteral("jpg"), QStringLiteral("jpeg"),
        QStringLiteral("gif"), QStringLiteral("webp"), QStringLiteral("svg")};
    const ImageData image = imageData(name, doc, browserFormats);
    if (image.isNull())
        return QString();  // remota o ilegible: se deja el src original
    return QStringLiteral("data:%1;base64,%2")
        .arg(image.mediaType, QString::fromLatin1(image.bytes.toBase64()));
}

}  // namespace

QString toHtmlDocument(const QTextDocument *doc, const Language &language,
                       const QString &title)
{
    QString html = doc ? doc->toHtml() : QString();

    // Idioma en <html>: lo aprovechan el lector de pantalla, la separación silábica
    // y el corrector del navegador. Qt emite un `<html>` pelado.
    if (!language.odfLang.isEmpty()) {
        const QString tag = QStringLiteral("<html lang=\"%1\">").arg(language.odfLang);
        const int at = html.indexOf(QLatin1String("<html>"));
        if (at >= 0)
            html.replace(at, 6, tag);
    }

    // <title>: sin él el navegador rotula la pestaña con el nombre del fichero.
    if (!title.isEmpty()) {
        const int head = html.indexOf(QLatin1String("<head>"));
        if (head >= 0)
            html.insert(head + 6, QStringLiteral("<title>%1</title>").arg(xmlEsc(title)));
    }

    // Imágenes embebidas: Qt deja la ruta relativa tal cual, así que el .html se
    // quedaba sin ellas en cuanto se movía de carpeta o se enviaba por correo.
    // Se resuelven contra el documento (que trae la baseUrl) y se meten como
    // `data:` URI; lo que no se pueda cargar —una URL remota— se deja igual.
    if (doc) {
        static const QRegularExpression imgRe(QStringLiteral("src=\"([^\"]+)\""));
        QHash<QString, QString> remap;
        auto it = imgRe.globalMatch(html);
        while (it.hasNext()) {
            const QString src = it.next().captured(1);
            if (remap.contains(src) || src.startsWith(QLatin1String("data:")))
                continue;
            const QString data = imageDataUri(src, doc);
            if (!data.isEmpty())
                remap.insert(src, data);
        }
        for (auto i = remap.cbegin(); i != remap.cend(); ++i)
            html.replace(QStringLiteral("src=\"%1\"").arg(i.key()),
                         QStringLiteral("src=\"%1\"").arg(i.value()));
    }
    return html;
}

}  // namespace mdexport
