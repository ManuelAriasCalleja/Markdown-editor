/// \file
/// \brief Implementación de la parte pura de la importación con Pandoc.

#include "pandocimport.h"

#include "markdownrender.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QStandardPaths>
#include <QSysInfo>

#include <utility>

namespace mdimport {

bool pandocAvailable()
{
    return !QStandardPaths::findExecutable(QStringLiteral("pandoc")).isEmpty();
}

QStringList pandocArguments(const QString &inputPath, const QString &mediaDir)
{
    // Salida a stdout en Markdown GitHub (el dialecto del editor); el formato de
    // entrada lo deduce Pandoc de la extensión de `inputPath`.
    // `--standalone` es lo que rescata los METADATOS del documento (el título del
    // estilo «Title» de Word, el autor…): sin él, Pandoc los parsea y los tira,
    // porque solo emite el cuerpo. Con él salen como front matter YAML al principio,
    // que es justo lo que el editor ya conserva verbatim (DocumentIo) y de donde la
    // exportación lee `title`/`lang`. Un documento sin metadatos no gana ningún
    // bloque, y uno vacío sigue produciendo una salida vacía.
    QStringList args{QStringLiteral("--to=gfm"), QStringLiteral("--wrap=none"),
                     QStringLiteral("--standalone")};
    if (!mediaDir.isEmpty())
        args << QStringLiteral("--extract-media=") + mediaDir;
    args << inputPath;
    return args;
}

QString mediaDirFor(const QString &inputPath)
{
    const QFileInfo info(inputPath);
    if (inputPath.isEmpty())
        return QString();
    // Junto al original y con su nombre: dos importaciones en la misma carpeta no
    // se pisan, y se ve de un vistazo a qué documento pertenecen las imágenes.
    return info.absolutePath() + QLatin1Char('/') + info.completeBaseName()
           + QStringLiteral("-media");
}

namespace {

/// Deshace las entidades XML que Pandoc escribe en los atributos de la etiqueta.
QString unescapeEntities(QString s)
{
    s.replace(QLatin1String("&lt;"), QLatin1String("<"));
    s.replace(QLatin1String("&gt;"), QLatin1String(">"));
    s.replace(QLatin1String("&quot;"), QLatin1String("\""));
    s.replace(QLatin1String("&#39;"), QLatin1String("'"));
    s.replace(QLatin1String("&amp;"), QLatin1String("&"));  // el último, o duplicaría
    return s;
}

/// Valor del atributo `name` en el texto de una etiqueta (comillas dobles o simples).
QString attributeValue(const QString &tag, const QString &name)
{
    const QRegularExpression re(
        QStringLiteral("\\b%1\\s*=\\s*(\"([^\"]*)\"|'([^']*)')").arg(name),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch m = re.match(tag);
    if (!m.hasMatch())
        return QString();
    return unescapeEntities(m.captured(2).isNull() ? m.captured(3) : m.captured(2));
}

/// Paso 1: las etiquetas `<img>` que Pandoc emite cuando GFM no expresa la imagen.
QString imgTagsToMarkdown(const QString &markdown)
{
    static const QRegularExpression imgTag(QStringLiteral("<img\\b[^>]*>"),
                                           QRegularExpression::CaseInsensitiveOption);
    QString out;
    qsizetype last = 0;
    QRegularExpressionMatchIterator it = imgTag.globalMatch(markdown);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString src = attributeValue(m.captured(0), QStringLiteral("src"));
        if (src.isEmpty())
            continue;  // sin destino no hay imagen que rescatar: se deja tal cual
        out += markdown.mid(last, m.capturedStart() - last)
               + mdrender::imageMarkdown(
                     src, attributeValue(m.captured(0), QStringLiteral("alt")));
        last = m.capturedEnd();
    }
    return last == 0 ? markdown : out + markdown.mid(last);
}

/// Paso 2: `![](ruta)` → `![nombre](ruta)`. Qt descarta la imagen sin alternativa.
QString fillEmptyAlt(const QString &markdown)
{
    // Destino entre `<...>` o sin espacios, con título opcional (`"…"`).
    static const QRegularExpression emptyAlt(
        QStringLiteral("!\\[\\]\\((<[^>]*>|[^()\\s]*)(\\s+\"[^\"]*\")?\\)"));
    QString out;
    qsizetype last = 0;
    QRegularExpressionMatchIterator it = emptyAlt.globalMatch(markdown);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString dest = m.captured(1);
        if (dest.isEmpty())
            continue;  // `![]()` sin destino: no hay nada que rescatar
        // El destino ya viene con la forma que le dio Pandoc (envuelto o no): se
        // deja como está y solo se rellena el rótulo.
        out += markdown.mid(last, m.capturedStart() - last)
               + QStringLiteral("![%1](%2%3)")
                     .arg(mdrender::imageAltFallback(dest), dest, m.captured(2));
        last = m.capturedEnd();
    }
    return last == 0 ? markdown : out + markdown.mid(last);
}

}  // namespace

QString repairImages(const QString &markdown)
{
    return fillEmptyAlt(imgTagsToMarkdown(markdown));
}

namespace {

/// Filas de una tabla HTML ya convertidas a texto Markdown en línea.
struct HtmlTable {
    QList<QStringList> header;  // filas de <thead> (normalmente una, o ninguna)
    QList<QStringList> body;
};

/// Parsea el fragmento `<table>…</table>` que emite Pandoc. Es XML bien formado
/// (Pandoc cierra todo y escapa las entidades), así que basta QXmlStreamReader; si
/// aun así falla, se devuelve false y el llamador deja el HTML como estaba.
bool parseHtmlTable(const QString &html, HtmlTable *out)
{
    QXmlStreamReader r(html);
    int tableDepth = 0;
    bool inHead = false;
    bool inCell = false;
    QStringList row;
    QString cell;
    // Pila de marcas abiertas: dónde empezó su contenido dentro de `cell` y con qué
    // envolverlo al cerrarse (para negrita/cursiva/código y enlaces).
    struct Open { qsizetype start; QString wrap; QString href; };
    QList<Open> open;

    // Los bloques dentro de una celda (párrafos, elementos de lista, filas de una
    // tabla anidada) no caben en Markdown: se separan con un espacio.
    const auto blockBreak = [&cell] {
        if (!cell.isEmpty() && !cell.endsWith(QLatin1Char(' ')))
            cell += QLatin1Char(' ');
    };

    while (!r.atEnd()) {
        r.readNext();
        if (r.hasError())
            return false;

        if (r.isCharacters()) {
            if (!inCell)
                continue;
            QString text = r.text().toString();
            text.replace(QLatin1Char('\n'), QLatin1Char(' '));
            if (text.trimmed().isEmpty()) {
                blockBreak();
                continue;
            }
            cell += text;
            continue;
        }

        if (r.isStartElement()) {
            const QStringView name = r.name();
            if (name == QLatin1String("table")) {
                ++tableDepth;
                blockBreak();  // tabla anidada: su contenido sigue en esta celda
            } else if (name == QLatin1String("thead") && tableDepth == 1) {
                inHead = true;
            } else if (name == QLatin1String("tr") && tableDepth == 1) {
                row.clear();
            } else if (name == QLatin1String("tr") || name == QLatin1String("p")
                       || name == QLatin1String("li") || name == QLatin1String("div")
                       || name == QLatin1String("br")) {
                blockBreak();
            } else if ((name == QLatin1String("td") || name == QLatin1String("th"))
                       && tableDepth == 1) {
                inCell = true;
                cell.clear();
                open.clear();
            } else if (inCell) {
                if (name == QLatin1String("strong") || name == QLatin1String("b"))
                    open.append({cell.size(), QStringLiteral("**"), QString()});
                else if (name == QLatin1String("em") || name == QLatin1String("i"))
                    open.append({cell.size(), QStringLiteral("*"), QString()});
                else if (name == QLatin1String("code"))
                    open.append({cell.size(), QStringLiteral("`"), QString()});
                else if (name == QLatin1String("a"))
                    open.append({cell.size(), QString(),
                                 r.attributes().value(QLatin1String("href")).toString()});
            }
            continue;
        }

        if (!r.isEndElement())
            continue;
        const QStringView name = r.name();
        if (name == QLatin1String("table")) {
            --tableDepth;
        } else if (name == QLatin1String("thead") && tableDepth == 1) {
            inHead = false;
        } else if ((name == QLatin1String("td") || name == QLatin1String("th"))
                   && tableDepth == 1) {
            inCell = false;
            QString text = cell.simplified();
            text.replace(QLatin1String("|"), QLatin1String("\\|"));  // cortaría la fila
            row << text;
            // `colspan` no existe en Markdown: se rellena con celdas vacías a la
            // derecha para que la fila conserve el número de columnas.
            const int span = r.attributes().value(QLatin1String("colspan")).toInt();
            for (int i = 1; i < span; ++i)
                row << QString();
        } else if (name == QLatin1String("tr") && tableDepth == 1) {
            (inHead ? out->header : out->body) << row;
            row.clear();
        } else if (inCell && !open.isEmpty()
                   && (name == QLatin1String("strong") || name == QLatin1String("b")
                       || name == QLatin1String("em") || name == QLatin1String("i")
                       || name == QLatin1String("code") || name == QLatin1String("a"))) {
            const Open o = open.takeLast();
            const QString inner = cell.mid(o.start).trimmed();
            cell.truncate(o.start);
            if (inner.isEmpty())
                continue;  // marca vacía: no se emite `****` ni `[]()`
            cell += o.href.isEmpty() ? o.wrap + inner + o.wrap
                                     : QStringLiteral("[%1](%2)").arg(inner, o.href);
        }
    }
    return !out->header.isEmpty() || !out->body.isEmpty();
}

/// Emite la tabla como tabla de tuberías, con todas las filas al mismo ancho. Si el
/// HTML no traía `<thead>` el encabezado va vacío (GFM lo exige), igual que hace el
/// propio Pandoc al escribir una tabla sin encabezado.
QString renderPipeTable(const HtmlTable &table)
{
    QList<QStringList> rows = table.header + table.body;
    qsizetype columns = 0;
    for (const QStringList &r : std::as_const(rows))
        columns = qMax(columns, r.size());
    if (columns == 0)
        return QString();

    const auto line = [columns](const QStringList &cells) {
        QString out = QStringLiteral("|");
        for (qsizetype i = 0; i < columns; ++i)
            out += QLatin1Char(' ') + (i < cells.size() ? cells.at(i) : QString())
                   + QStringLiteral(" |");
        return out + QLatin1Char('\n');
    };

    QString out = line(table.header.isEmpty() ? QStringList() : table.header.first());
    out += QStringLiteral("|");
    for (qsizetype i = 0; i < columns; ++i)
        out += QStringLiteral("---|");
    out += QLatin1Char('\n');
    // Un <thead> con varias filas (raro) se degrada a filas normales del cuerpo.
    for (qsizetype i = 1; i < table.header.size(); ++i)
        out += line(table.header.at(i));
    for (const QStringList &r : std::as_const(table.body))
        out += line(r);
    return out;
}

}  // namespace

QString htmlTablesToMarkdown(const QString &markdown)
{
    static const QRegularExpression tableTag(QStringLiteral("</?table\\b"),
                                             QRegularExpression::CaseInsensitiveOption);
    QString out;
    qsizetype last = 0;
    bool inFence = false;
    // Se recorre por líneas para dos cosas: Pandoc siempre abre la tabla al principio
    // de una, y así se pueden SALTAR los bloques de código —un ``` con un `<table>`
    // dentro es texto del usuario, no una tabla, y convertirlo lo destrozaría—.
    for (qsizetype pos = 0; pos < markdown.size();) {
        qsizetype eol = markdown.indexOf(QLatin1Char('\n'), pos);
        if (eol < 0)
            eol = markdown.size();
        const QStringView line = QStringView(markdown).mid(pos, eol - pos).trimmed();

        if (line.startsWith(QLatin1String("```")) || line.startsWith(QLatin1String("~~~"))) {
            inFence = !inFence;
        } else if (!inFence && line.startsWith(QLatin1String("<table"), Qt::CaseInsensitive)) {
            // Cierre EQUILIBRADO: una tabla anidada trae su propio `</table>`, y
            // cortar en el primero partiría el fragmento por la mitad.
            qsizetype end = -1;
            int depth = 0;
            QRegularExpressionMatchIterator it = tableTag.globalMatch(markdown, pos);
            while (it.hasNext()) {
                const QRegularExpressionMatch m = it.next();
                depth += m.captured(0).startsWith(QLatin1String("</")) ? -1 : 1;
                if (depth == 0) {
                    end = markdown.indexOf(QLatin1Char('>'), m.capturedEnd());
                    break;
                }
            }
            if (end < 0)
                break;  // sin cierre: HTML truncado, mejor no tocar nada más
            ++end;
            // El salto que cierra la línea del `</table>` se consume con él: la
            // tabla de tuberías ya trae el suyo y, si no, quedaría un hueco de más.
            if (end < markdown.size() && markdown.at(end) == QLatin1Char('\n'))
                ++end;

            HtmlTable table;
            const qsizetype start = markdown.indexOf(QLatin1Char('<'), pos);
            const QString fragment = markdown.mid(start, end - start);
            const QString pipe =
                parseHtmlTable(fragment, &table) ? renderPipeTable(table) : QString();
            if (!pipe.isEmpty()) {
                out += markdown.mid(last, start - last) + pipe;
                last = end;
            }
            pos = end;
            continue;
        }
        pos = eol + 1;
    }
    return last == 0 ? markdown : out + markdown.mid(last);
}

QString pandocFilePattern()
{
    // Los formatos de entrada más útiles que Pandoc admite y que no tienen ya un
    // importador nativo propio (HTML/EPUB lo tienen, pero se incluyen por comodidad).
    return QStringLiteral(
        "*.docx *.odt *.rtf *.tex *.rst *.org *.textile *.wiki *.epub *.html *.htm "
        "*.man *.docbook");
}

QString pandocInstallCommand()
{
    const QString kernel = QSysInfo::kernelType();  // "linux" / "darwin" / "winnt"
    if (kernel == QLatin1String("darwin"))
        return QStringLiteral("brew install pandoc");
    if (kernel == QLatin1String("winnt"))
        return QStringLiteral("choco install pandoc");
    return QStringLiteral("sudo apt install pandoc");
}

}  // namespace mdimport
