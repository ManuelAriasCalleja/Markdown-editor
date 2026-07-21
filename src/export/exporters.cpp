/// \file
/// \brief Implementación de los serializadores de exportación a ODF, LaTeX, DOCX y EPUB.

#include "exporters.h"

#include "mathblocks.h"
#include "printdecor.h"
#include "tableedit.h"

#include <QAbstractTextDocumentLayout>
#include <QBuffer>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QFont>
#include <QFontMetricsF>
#include <QHash>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPrinter>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextFragment>
#include <QTextLayout>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>
#include <QUrl>
#include <QUuid>

#include <algorithm>

#include <private/qzipreader_p.h>
#include <private/qzipwriter_p.h>

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

// --------------------------------------------------------------------------
// LaTeX
// --------------------------------------------------------------------------

namespace {

// Por debajo de este punto de código se confía en inputenc/fontspec (latino y
// puntuación habitual: acentos, raya —, comillas tipográficas, …). A partir de
// aquí están las flechas, símbolos varios, dingbats y emoji, que pdflatex (con
// inputenc utf8 + T1) no sabe componer y abortarían la compilación.
constexpr uint kHighSymbolStart = 0x2190;

// Equivalentes LaTeX de algunos símbolos altos frecuentes en Markdown; el resto
// (≥ kHighSymbolStart) se omite para no romper pdflatex.
const QHash<uint, QString> &highSymbolMap()
{
    static const QHash<uint, QString> m = {
        {0x2705, QStringLiteral("$\\checkmark$")},        // ✅
        {0x2714, QStringLiteral("$\\checkmark$")},        // ✔
        {0x2713, QStringLiteral("$\\checkmark$")},        // ✓
        {0x2611, QStringLiteral("$\\boxtimes$")},         // ☑
        {0x2612, QStringLiteral("$\\boxtimes$")},         // ☒
        {0x2610, QStringLiteral("$\\square$")},           // ☐
        {0x25A1, QStringLiteral("$\\square$")},           // □
        {0x274C, QStringLiteral("$\\times$")},            // ❌
        {0x2716, QStringLiteral("$\\times$")},            // ✖
        {0x2717, QStringLiteral("$\\times$")},            // ✗
        {0x2718, QStringLiteral("$\\times$")},            // ✘
        {0x2605, QStringLiteral("$\\bigstar$")},          // ★
        {0x2606, QStringLiteral("$\\star$")},             // ☆
        {0x2190, QStringLiteral("$\\leftarrow$")},        // ←
        {0x2192, QStringLiteral("$\\rightarrow$")},       // →
        {0x2194, QStringLiteral("$\\leftrightarrow$")},   // ↔
        {0x21D2, QStringLiteral("$\\Rightarrow$")},       // ⇒
        {0x21D0, QStringLiteral("$\\Leftarrow$")},        // ⇐
        {0x275D, QStringLiteral("``")},                   // ❝
        {0x275E, QStringLiteral("''")},                   // ❞
        {0x2122, QStringLiteral("\\texttrademark{}")},    // ™
    };
    return m;
}

QString latexEscape(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const uint cp : s.toUcs4()) {
        switch (cp) {
        case u'\\': out += QStringLiteral("\\textbackslash{}"); continue;
        case u'&':  out += QStringLiteral("\\&"); continue;
        case u'%':  out += QStringLiteral("\\%"); continue;
        case u'$':  out += QStringLiteral("\\$"); continue;
        case u'#':  out += QStringLiteral("\\#"); continue;
        case u'_':  out += QStringLiteral("\\_"); continue;
        case u'{':  out += QStringLiteral("\\{"); continue;
        case u'}':  out += QStringLiteral("\\}"); continue;
        case u'~':  out += QStringLiteral("\\textasciitilde{}"); continue;
        case u'^':  out += QStringLiteral("\\textasciicircum{}"); continue;
        default:    break;
        }
        // Caracteres «técnicos» (super/subíndices, griego, operadores, letras
        // matemáticas…) que pdflatex+T1 no compone → su equivalente LaTeX en modo
        // matemático. Cubre por igual los que hoy se colaban crudos y abortaban la
        // compilación (`₁`, `φ`, `ℝ`…) y los altos que se descartaban (`⊕`, `∈`…).
        if (const QString math = mdmath::unicodeToLatex(cp); !math.isEmpty())
            out += math;
        else if (cp < kHighSymbolStart)
            out += QChar(static_cast<char16_t>(cp));  // latino/puntuación: lo compone el motor
        else if (const auto it = highSymbolMap().constFind(cp); it != highSymbolMap().cend())
            out += it.value();
        // else: símbolo/emoji sin equivalente → se omite (evita el fallo de pdflatex)
    }
    return out;
}

// Versión para entornos verbatim, donde no caben comandos LaTeX: conserva el
// texto literal y solo descarta los símbolos/emoji altos que romperían pdflatex.
QString verbatimSanitize(const QString &s)
{
    QString out;
    out.reserve(s.size());
    for (const uint cp : s.toUcs4())
        if (cp < kHighSymbolStart)
            out += QChar(static_cast<char16_t>(cp));
    return out;
}

// Estado para traer las imágenes junto al .tex y así hacer el export
// autocontenido. Con `outDir` vacío (p.ej. en los tests) no se toca nada: las
// imágenes se referencian por su ruta original, como siempre.
struct LatexImages {
    const QTextDocument *doc = nullptr;
    QString outDir;   ///< carpeta del .tex donde escribir las imágenes traídas
    QString stem;     ///< prefijo de nombre de los ficheros (ya saneado, sin puntos)
    int counter = 0;  ///< secuencia para nombres de fichero únicos
};

// Extensiones que pdflatex (driver pdftex) incluye de forma nativa. Las demás
// (SVG, GIF, BMP, TIFF, WebP…) abortan la compilación con «Unknown graphics
// extension», así que hay que rasterizarlas antes.
bool pdflatexIncludable(const QString &ext)
{
    return ext == QLatin1String("pdf") || ext == QLatin1String("png")
        || ext == QLatin1String("jpg") || ext == QLatin1String("jpeg");
}

// Ruta local legible del recurso `name` resuelta contra `baseUrl` (rutas
// relativas del Markdown), o vacío si es remoto/no existe.
QString localFileFor(const QString &name, const QUrl &baseUrl)
{
    QUrl u(name);
    if (u.isRelative())
        u = baseUrl.resolved(u);
    if (u.isLocalFile()) {
        const QString p = u.toLocalFile();
        if (QFileInfo::exists(p))
            return p;
    }
    if (QFileInfo::exists(name))  // por si era ya una ruta de sistema de ficheros
        return name;
    return QString();
}

// Escribe el siguiente sidecar (`<stem>-imgN.<ext>`) junto al .tex y devuelve su
// nombre de fichero, o vacío si la escritura falla. Solo avanza el contador si
// realmente escribe, para que los nombres no salten números.
QString writeSidecar(LatexImages &images, const QString &ext, const QByteArray &bytes)
{
    QString file =
        QStringLiteral("%1-img%2.%3").arg(images.stem).arg(images.counter + 1).arg(ext);
    QFile out(images.outDir + QLatin1Char('/') + file);
    if (!out.open(QIODevice::WriteOnly) || out.write(bytes) != bytes.size())
        return QString();
    ++images.counter;
    return file;
}

QString imagePlaceholder(const QString &name)
{
    // Marcador visible e inocuo (nunca un `%`, que comentaría el resto de la línea
    // del párrafo) para cuando la imagen no se puede traer.
    return QStringLiteral("\\texttt{[imagen: %1]}").arg(latexEscape(QFileInfo(name).fileName()));
}

// LaTeX para una imagen, dejando el export AUTOCONTENIDO: toda imagen se copia o
// convierte a un fichero junto al .tex y se referencia por ese nombre, de modo que
// el .tex compila esté donde esté. Las de formato incluible (pdf/png/jpg) se copian
// byte a byte (conservan formato y calidad); las que pdflatex no soporta (SVG, GIF,
// BMP…) —o una incluible que no se pueda leer del disco, p.ej. remota— se
// rasterizan a PNG vía `doc->resource()` (que resuelve la ruta relativa por baseUrl
// y rasteriza el SVG con el mismo plugin que ya lo muestra en el editor). La ruta
// del sidecar se escapa (`%`, `#`, `{`, `}`, `\` romperían el .tex). Sin carpeta de
// salida (tests) se mantiene la conducta previa: referencia directa de las
// incluibles, marcador para el resto.
QString imageLatex(const QString &name, LatexImages &images)
{
    if (name.isEmpty())
        return QString();
    const QString incl = QStringLiteral("\\includegraphics[max width=\\linewidth]{%1}");
    const QString ext = QFileInfo(name).suffix().toLower();

    if (images.outDir.isEmpty() || !images.doc)
        return pdflatexIncludable(ext) ? incl.arg(latexEscape(name)) : imagePlaceholder(name);

    // 1) Formato incluible con fichero local → copia los bytes tal cual.
    if (pdflatexIncludable(ext)) {
        const QString src = localFileFor(name, images.doc->baseUrl());
        QFile in(src);
        if (!src.isEmpty() && in.open(QIODevice::ReadOnly)) {
            const QString file = writeSidecar(images, ext, in.readAll());
            if (!file.isEmpty())
                return incl.arg(latexEscape(file));
        }
    }
    // 2) Formato no incluible (o incluible ilegible/remota) → rasteriza a PNG.
    const QVariant res = images.doc->resource(QTextDocument::ImageResource, QUrl(name));
    QImage img = qvariant_cast<QImage>(res);
    if (img.isNull()) {
        const QPixmap pm = qvariant_cast<QPixmap>(res);
        if (!pm.isNull())
            img = pm.toImage();
    }
    if (!img.isNull()) {
        QByteArray png;
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
        const QString file = writeSidecar(images, QStringLiteral("png"), png);
        if (!file.isEmpty())
            return incl.arg(latexEscape(file));
    }
    return imagePlaceholder(name);
}

// Texto en línea de un bloque, con el formato de carácter convertido a comandos
// LaTeX (negrita, cursiva, subrayado, tachado, código, enlaces e imágenes).
// `ignoreBold` evita el \textbf en los encabezados (que Qt marca en negrita y en
// LaTeX ya lo son), para no producir \section{\textbf{...}}.
QString inlineLatex(const QTextBlock &block, LatexImages &images, bool ignoreBold = false)
{
    QString out;
    QString lastMathTex;  // sigue el grupo de fórmula abierto (evita repetir)
    for (auto it = block.begin(); it != block.end(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid())
            continue;
        const QTextCharFormat cf = frag.charFormat();
        // Fórmula: las fórmulas viven como secuencia de fragmentos consecutivos
        // con IsMathProperty + el mismo MathTex (los runs de super/subíndice).
        // Emitimos `$tex$`/`$$tex$$` UNA sola vez por grupo a partir de la
        // propiedad, sin pasar por el texto visible (que es Unicode).
        if (cf.boolProperty(mdmath::IsMathProperty)) {
            const QString tex = cf.property(mdmath::MathTexProperty).toString();
            if (tex == lastMathTex)
                continue;  // mismo grupo abierto: ya emitido
            const bool isBlock = cf.boolProperty(mdmath::MathBlockProperty);
            out += mdmath::wrapTex(tex, isBlock);
            lastMathTex = tex;
            continue;
        }
        lastMathTex.clear();
        if (cf.isImageFormat()) {
            out += imageLatex(cf.toImageFormat().name(), images);
            continue;
        }
        QString t = latexEscape(frag.text());
        if (t.isEmpty())
            continue;
        if (cf.fontFixedPitch())
            t = QStringLiteral("\\texttt{%1}").arg(t);
        if (cf.fontStrikeOut())
            t = QStringLiteral("\\sout{%1}").arg(t);
        if (cf.fontUnderline() && !cf.isAnchor())
            t = QStringLiteral("\\uline{%1}").arg(t);
        if (cf.fontItalic())
            t = QStringLiteral("\\textit{%1}").arg(t);
        if (!ignoreBold && cf.fontWeight() >= QFont::Bold)
            t = QStringLiteral("\\textbf{%1}").arg(t);
        if (cf.isAnchor() && !cf.anchorHref().isEmpty())
            // El destino se escapa: un `%` (URLs codificadas) rompería el .tex y un
            // destino con `}{\...}` inyectaría comandos LaTeX.
            t = QStringLiteral("\\href{%1}{%2}").arg(latexEscape(cf.anchorHref()), t);
        out += t;
    }
    return out;
}

QString columnSpec(const QTextTable *table)
{
    QString spec;
    const int row = table->rows() > 1 ? 1 : 0;
    for (int c = 0; c < table->columns(); ++c) {
        const Qt::Alignment a =
            table->cellAt(row, c).firstCursorPosition().blockFormat().alignment();
        if (a & Qt::AlignHCenter)
            spec += QLatin1Char('c');
        else if (a & Qt::AlignRight)
            spec += QLatin1Char('r');
        else
            spec += QLatin1Char('l');
    }
    return spec;
}

QString tableLatex(QTextTable *table, LatexImages &images)
{
    QString out = QStringLiteral("\\begin{tabular}{%1}\n\\hline\n").arg(columnSpec(table));
    for (int r = 0; r < table->rows(); ++r) {
        QStringList cells;
        for (int c = 0; c < table->columns(); ++c) {
            const QTextTableCell cell = table->cellAt(r, c);
            QString text;
            QTextFrame::iterator fit = cell.begin();
            for (; !fit.atEnd(); ++fit) {
                const QTextBlock b = fit.currentBlock();
                if (b.isValid())
                    text += inlineLatex(b, images);
            }
            cells << text;
        }
        out += cells.join(QStringLiteral(" & ")) + QStringLiteral(" \\\\\n\\hline\n");
    }
    out += QStringLiteral("\\end{tabular}\n");
    return out;
}

QString headingCommand(int level, const QString &text)
{
    switch (level) {
    case 1: return QStringLiteral("\\section{%1}\n").arg(text);
    case 2: return QStringLiteral("\\subsection{%1}\n").arg(text);
    case 3: return QStringLiteral("\\subsubsection{%1}\n").arg(text);
    case 4: return QStringLiteral("\\paragraph{%1}\n").arg(text);
    case 5: return QStringLiteral("\\subparagraph{%1}\n").arg(text);
    default: return QStringLiteral("\\textbf{%1}\n\n").arg(text);  // H6
    }
}

} // namespace

// Preámbulo del documento LaTeX: desde \documentclass hasta \begin{document} (con
// \maketitle si hay título). Portable entre motores: pdfLaTeX usa inputenc/T1;
// LuaLaTeX y XeLaTeX usan fontspec (Unicode nativo). Así el .tex compila con
// cualquiera. El cuerpo y el \end{document} los añade toLatex.
static QString latexPreamble(const Language &language, const QString &title)
{
    QString out;
    out += QStringLiteral("\\documentclass[11pt]{article}\n");
    out += QStringLiteral("\\usepackage{iftex}\n");
    out += QStringLiteral("\\ifPDFTeX\n"
                          "  \\usepackage[utf8]{inputenc}\n"
                          "  \\usepackage[T1]{fontenc}\n"
                          "\\else\n"
                          "  \\usepackage{fontspec}\n"
                          "\\fi\n");
    out += QStringLiteral("\\usepackage[%1]{babel}\n").arg(language.babel);
    out += QStringLiteral("\\usepackage{amsmath}\n");
    out += QStringLiteral("\\usepackage{amssymb}\n");
    out += QStringLiteral("\\usepackage[normalem]{ulem}\n");
    out += QStringLiteral("\\usepackage{graphicx}\n");
    out += QStringLiteral("\\usepackage[export]{adjustbox}\n");  // max width en imágenes
    out += QStringLiteral("\\usepackage{hyperref}\n");
    if (!title.isEmpty())
        out += QStringLiteral("\\title{%1}\n\\author{}\n\\date{}\n").arg(latexEscape(title));
    out += QStringLiteral("\\begin{document}\n");
    if (!title.isEmpty())
        out += QStringLiteral("\\maketitle\n");
    return out;
}

QString toLatex(const QTextDocument *doc, const Language &language, const QString &title,
                const QString &outputTexPath)
{
    // Contexto de conversión de imágenes: si nos dan la ruta del .tex, las imágenes
    // que pdflatex no soporta (SVG…) se rasterizan a un PNG junto a él. Sin ruta
    // (tests), la conversión queda inactiva y las imágenes se referencian tal cual.
    LatexImages images;
    images.doc = doc;
    if (!outputTexPath.isEmpty()) {
        const QFileInfo fi(outputTexPath);
        images.outDir = fi.absolutePath();
        // Nombre base saneado (sin puntos ni espacios) para que graphicx no se líe
        // con las extensiones y no haya problemas de rutas con espacios.
        QString stem = fi.baseName();
        stem.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("-"));
        images.stem = stem.isEmpty() ? QStringLiteral("figura") : stem;
    }

    QStringList openLists;  // entornos de lista abiertos (de fuera hacia dentro)
    bool inQuote = false;
    bool inCode = false;
    QString body;

    const auto closeLists = [&] {
        while (!openLists.isEmpty())
            body += QStringLiteral("\\end{%1}\n").arg(openLists.takeLast());
    };
    const auto closeQuote = [&] {
        if (inQuote) { body += QStringLiteral("\\end{quote}\n"); inQuote = false; }
    };
    const auto closeCode = [&] {
        if (inCode) { body += QStringLiteral("\\end{verbatim}\n"); inCode = false; }
    };

    QList<QTextTable *> doneTables;
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        const QTextBlockFormat bf = block.blockFormat();

        // Tablas: se emiten una vez, al ver su primer bloque.
        if (QTextTable *table = QTextCursor(block).currentTable()) {
            if (!doneTables.contains(table)) {
                closeLists(); closeQuote(); closeCode();
                doneTables.append(table);
                body += tableLatex(table, images) + QLatin1Char('\n');
            }
            continue;
        }

        // Bloque de código → verbatim (agrupando líneas consecutivas).
        if (bf.hasProperty(QTextFormat::BlockCodeFence)) {
            closeLists(); closeQuote();
            if (!inCode) { body += QStringLiteral("\\begin{verbatim}\n"); inCode = true; }
            body += verbatimSanitize(block.text()) + QLatin1Char('\n');
            continue;
        }
        closeCode();

        // Regla horizontal.
        if (bf.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
            closeLists(); closeQuote();
            body += QStringLiteral("\\noindent\\rule{\\linewidth}{0.4pt}\n\n");
            continue;
        }

        const QString text = inlineLatex(block, images);

        // Listas (viñetas, numeradas y tareas) con anidamiento por sangría.
        if (QTextList *list = block.textList()) {
            closeQuote();
            const int depth = qMax(1, list->format().indent());
            const bool ordered = list->format().style() == QTextListFormat::ListDecimal;
            const QString want = ordered ? QStringLiteral("enumerate")
                                         : QStringLiteral("itemize");
            while (openLists.size() > depth)
                body += QStringLiteral("\\end{%1}\n").arg(openLists.takeLast());
            if (openLists.size() == depth && openLists.last() != want) {
                body += QStringLiteral("\\end{%1}\n").arg(openLists.takeLast());
            }
            while (openLists.size() < depth) {
                body += QStringLiteral("\\begin{%1}\n").arg(want);
                openLists.append(want);
            }
            // Tarea: prefijo con casilla (requiere amssymb).
            QString marker;
            if (bf.marker() == QTextBlockFormat::MarkerType::Checked)
                marker = QStringLiteral("$\\boxtimes$ ");
            else if (bf.marker() == QTextBlockFormat::MarkerType::Unchecked)
                marker = QStringLiteral("$\\square$ ");
            body += QStringLiteral("\\item %1%2\n").arg(marker, text);
            continue;
        }
        closeLists();

        if (text.isEmpty())
            continue;

        // Cita en bloque.
        if (bf.intProperty(QTextFormat::BlockQuoteLevel) > 0) {
            if (!inQuote) { body += QStringLiteral("\\begin{quote}\n"); inQuote = true; }
            body += text + QLatin1Char('\n');
            continue;
        }
        closeQuote();

        // Encabezado o párrafo normal.
        const int level = bf.headingLevel();
        if (level >= 1)
            body += headingCommand(level, inlineLatex(block, images, /*ignoreBold=*/true));
        else
            body += text + QStringLiteral("\n\n");
    }
    closeLists(); closeQuote(); closeCode();

    return latexPreamble(language, title) + body + QStringLiteral("\\end{document}\n");
}

// --------------------------------------------------------------------------
// ODF
// --------------------------------------------------------------------------

QByteArray odfStylesXml(const Language &language)
{
    // Un default-style de párrafo con el idioma: LibreOffice lo aplica a todo el
    // documento (corrector, separación silábica). Los idiomas asiático/complejo
    // se marcan "zxx" (ninguno) para no confundir al corrector.
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<office:document-styles"
        " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
        " xmlns:style=\"urn:oasis:names:tc:opendocument:xmlns:style:1.0\""
        " xmlns:fo=\"urn:oasis:names:tc:opendocument:xmlns:xsl-fo-compatible:1.0\""
        " office:version=\"1.2\">\n"
        " <office:styles>\n"
        "  <style:default-style style:family=\"paragraph\">\n"
        "   <style:text-properties fo:language=\"%1\" fo:country=\"%2\""
        " style:language-asian=\"zxx\" style:country-asian=\"none\""
        " style:language-complex=\"zxx\" style:country-complex=\"none\"/>\n"
        "  </style:default-style>\n"
        " </office:styles>\n"
        "</office:document-styles>\n")
        .arg(language.odfLang, language.odfCountry);
    return xml.toUtf8();
}

QByteArray odfMetaXml(const Language &language, const QString &title)
{
    QString meta;
    if (!title.isEmpty())
        meta += QStringLiteral("  <dc:title>%1</dc:title>\n").arg(title.toHtmlEscaped());
    meta += QStringLiteral("  <dc:language>%1-%2</dc:language>\n")
                .arg(language.odfLang, language.odfCountry);
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<office:document-meta"
        " xmlns:office=\"urn:oasis:names:tc:opendocument:xmlns:office:1.0\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\""
        " xmlns:meta=\"urn:oasis:names:tc:opendocument:xmlns:meta:1.0\""
        " office:version=\"1.2\">\n"
        " <office:meta>\n%1"
        "  <meta:generator>md-editor</meta:generator>\n"
        " </office:meta>\n"
        "</office:document-meta>\n")
        .arg(meta);
    return xml.toUtf8();
}

QByteArray odfManifestWithLanguageFiles(const QByteArray &manifest)
{
    const QString entries = QStringLiteral(
        " <manifest:file-entry manifest:media-type=\"text/xml\""
        " manifest:full-path=\"styles.xml\"/>\n"
        " <manifest:file-entry manifest:media-type=\"text/xml\""
        " manifest:full-path=\"meta.xml\"/>\n");
    QString out = QString::fromUtf8(manifest);
    const int pos = out.indexOf(QStringLiteral("</manifest:manifest>"));
    if (pos >= 0)
        out.insert(pos, entries);
    return out.toUtf8();
}

bool writeOdf(const QTextDocument *doc, const QString &path, const Language &language,
              const QString &title, QString *error)
{
    // 1) Qt genera el ODT base (sin idioma) en memoria.
    QByteArray odt;
    {
        QBuffer buffer(&odt);
        buffer.open(QIODevice::WriteOnly);
        QTextDocumentWriter writer(&buffer, "ODF");
        if (!writer.write(doc)) {
            if (error)
                *error = QStringLiteral("QTextDocumentWriter falló al generar el ODF.");
            return false;
        }
    }

    // 2) Reempaqueta añadiendo styles.xml y meta.xml con el idioma, y la entrada
    //    correspondiente en el manifest. mimetype va primero y sin comprimir.
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QBuffer in(&odt);
    in.open(QIODevice::ReadOnly);
    QZipReader reader(&in);
    QZipWriter writer(&file);

    for (const QZipReader::FileInfo &fi : reader.fileInfoList()) {
        if (!fi.isFile)
            continue;
        QByteArray data = reader.fileData(fi.filePath);
        if (fi.filePath == QLatin1String("META-INF/manifest.xml"))
            data = odfManifestWithLanguageFiles(data);
        writer.setCompressionPolicy(fi.filePath == QLatin1String("mimetype")
                                        ? QZipWriter::NeverCompress
                                        : QZipWriter::AutoCompress);
        writer.addFile(fi.filePath, data);
    }
    writer.setCompressionPolicy(QZipWriter::AutoCompress);
    writer.addFile(QStringLiteral("styles.xml"), odfStylesXml(language));
    writer.addFile(QStringLiteral("meta.xml"), odfMetaXml(language, title));
    writer.close();
    file.close();

    if (writer.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete ODF.");
        return false;
    }
    return true;
}

// --------------------------------------------------------------------------
// DOCX (OOXML WordprocessingML)
// --------------------------------------------------------------------------

namespace {

QString xmlEsc(const QString &s)
{
    QString o;
    o.reserve(s.size());
    for (const QChar c : s) {
        switch (c.unicode()) {
        case u'&':  o += QStringLiteral("&amp;");  break;
        case u'<':  o += QStringLiteral("&lt;");   break;
        case u'>':  o += QStringLiteral("&gt;");   break;
        case u'"':  o += QStringLiteral("&quot;"); break;
        case u'\'': o += QStringLiteral("&apos;"); break;
        default:    o += c;
        }
    }
    return o;
}

constexpr qint64 kEmuPerPx = 9525;       // 1 px = 9525 EMU
constexpr qint64 kMaxImageCx = 5731200;  // ancho útil A4 con márgenes de 1"

// Convierte un QTextDocument a OOXML: párrafos, encabezados, formato de carácter
// (negrita/cursiva/subrayado/tachado/código + super/subíndice de las fórmulas),
// enlaces (campo HYPERLINK, sin relaciones), listas, citas, tablas, reglas e
// imágenes embebidas. El estado mutable (las imágenes registradas) vive aquí.
class DocxWriter
{
public:
    DocxWriter(const QTextDocument *doc, QList<DocxImage> *images)
        : m_doc(doc), m_images(images) {}

    QString document(const QString &title);

private:
    QString paragraph(const QString &pPr, const QString &runs) const;
    QString runsForBlock(const QTextBlock &block, bool ignoreBold);
    QString runProps(const QTextCharFormat &cf, bool ignoreBold, bool asLink) const;
    QString makeRun(const QString &text, const QString &rPr) const;
    QString imageRun(const QTextCharFormat &cf);
    QString tableXml(QTextTable *table);

    const QTextDocument *m_doc;
    QList<DocxImage> *m_images;  // nulo = omitir imágenes
    int m_docPrId = 1;           // id de cada <wp:docPr> (debe ser único)
};

QString DocxWriter::paragraph(const QString &pPr, const QString &runs) const
{
    QString o = QStringLiteral("<w:p>");
    if (!pPr.isEmpty())
        o += QStringLiteral("<w:pPr>") + pPr + QStringLiteral("</w:pPr>");
    o += runs + QStringLiteral("</w:p>\n");
    return o;
}

QString DocxWriter::makeRun(const QString &text, const QString &rPr) const
{
    // Los saltos suaves (\n, U+2028) dentro de un fragmento van como <w:br/>.
    static const QRegularExpression breaks(QStringLiteral("[\\n\\x{2028}]"));
    const QStringList parts = text.split(breaks);
    QString inner;
    for (int i = 0; i < parts.size(); ++i) {
        if (i > 0)
            inner += QStringLiteral("<w:br/>");
        inner += QStringLiteral("<w:t xml:space=\"preserve\">") + xmlEsc(parts.at(i))
                 + QStringLiteral("</w:t>");
    }
    return QStringLiteral("<w:r>") + rPr + inner + QStringLiteral("</w:r>");
}

QString DocxWriter::runProps(const QTextCharFormat &cf, bool ignoreBold, bool asLink) const
{
    QString p;
    if (cf.fontFixedPitch())
        p += QStringLiteral("<w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\""
                            " w:cs=\"Courier New\"/>");
    if (asLink)  // estilo de enlace: azul subrayado
        p += QStringLiteral("<w:color w:val=\"0563C1\"/><w:u w:val=\"single\"/>");
    if (!ignoreBold && cf.fontWeight() >= QFont::Bold)
        p += QStringLiteral("<w:b/>");
    if (cf.fontItalic())
        p += QStringLiteral("<w:i/>");
    if (cf.fontUnderline() && !asLink && !cf.isAnchor())
        p += QStringLiteral("<w:u w:val=\"single\"/>");
    if (cf.fontStrikeOut())
        p += QStringLiteral("<w:strike/>");
    const QTextCharFormat::VerticalAlignment va = cf.verticalAlignment();
    if (va == QTextCharFormat::AlignSuperScript)
        p += QStringLiteral("<w:vertAlign w:val=\"superscript\"/>");  // super/subíndice de fórmulas
    else if (va == QTextCharFormat::AlignSubScript)
        p += QStringLiteral("<w:vertAlign w:val=\"subscript\"/>");
    return p.isEmpty() ? QString()
                       : QStringLiteral("<w:rPr>") + p + QStringLiteral("</w:rPr>");
}

QString DocxWriter::imageRun(const QTextCharFormat &cf)
{
    if (!m_images)
        return QString();
    const QString name = cf.toImageFormat().name();
    if (name.isEmpty())
        return QString();

    // Recupera la imagen del documento (el clon de exportación trae la baseUrl, así
    // que las rutas relativas se resuelven desde disco). Si no se obtiene, se omite.
    const QVariant res = m_doc->resource(QTextDocument::ImageResource, QUrl(name));
    QImage img = qvariant_cast<QImage>(res);
    if (img.isNull()) {
        const QPixmap pm = qvariant_cast<QPixmap>(res);
        if (!pm.isNull())
            img = pm.toImage();
    }
    if (img.isNull())
        return QString();

    QByteArray png;
    {
        QBuffer buf(&png);
        buf.open(QIODevice::WriteOnly);
        img.save(&buf, "PNG");
    }
    const int index = m_images->size() + 1;
    m_images->append({QStringLiteral("media/image%1.png").arg(index), png});
    const QString rId = QStringLiteral("rId%1").arg(2 + index);  // 1=styles, 2=numbering

    qint64 cx = static_cast<qint64>(img.width()) * kEmuPerPx;
    qint64 cy = static_cast<qint64>(img.height()) * kEmuPerPx;
    if (cx > kMaxImageCx) {  // escala manteniendo proporción al ancho de página
        cy = cy * kMaxImageCx / cx;
        cx = kMaxImageCx;
    }
    const int id = m_docPrId++;
    return QStringLiteral(
        "<w:r><w:drawing><wp:inline distT=\"0\" distB=\"0\" distL=\"0\" distR=\"0\">"
        "<wp:extent cx=\"%1\" cy=\"%2\"/>"
        "<wp:docPr id=\"%3\" name=\"Picture %3\"/>"
        "<a:graphic><a:graphicData uri=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">"
        "<pic:pic><pic:nvPicPr><pic:cNvPr id=\"%3\" name=\"image%4.png\"/><pic:cNvPicPr/>"
        "</pic:nvPicPr><pic:blipFill><a:blip r:embed=\"%5\"/>"
        "<a:stretch><a:fillRect/></a:stretch></pic:blipFill>"
        "<pic:spPr><a:xfrm><a:off x=\"0\" y=\"0\"/><a:ext cx=\"%1\" cy=\"%2\"/></a:xfrm>"
        "<a:prstGeom prst=\"rect\"><a:avLst/></a:prstGeom></pic:spPr></pic:pic>"
        "</a:graphicData></a:graphic></wp:inline></w:drawing></w:r>")
        .arg(QString::number(cx), QString::number(cy))
        .arg(id).arg(index).arg(rId);
}

QString DocxWriter::runsForBlock(const QTextBlock &block, bool ignoreBold)
{
    QString out;
    for (auto it = block.begin(); it != block.end(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid())
            continue;
        const QTextCharFormat cf = frag.charFormat();
        if (cf.isImageFormat()) {
            out += imageRun(cf);
            continue;
        }
        const QString text = frag.text();
        if (text.isEmpty())
            continue;
        const bool asLink = cf.isAnchor() && !cf.anchorHref().isEmpty();
        const QString run = makeRun(text, runProps(cf, ignoreBold, asLink));
        if (asLink) {
            // Campo HYPERLINK: el destino va en el propio document.xml (sin relación).
            out += QStringLiteral("<w:fldSimple w:instr=\" HYPERLINK &quot;%1&quot; \">")
                       .arg(xmlEsc(cf.anchorHref()))
                   + run + QStringLiteral("</w:fldSimple>");
        } else {
            out += run;
        }
    }
    return out;
}

QString DocxWriter::tableXml(QTextTable *table)
{
    static const QString borders = QStringLiteral(
        "<w:tblBorders>"
        "<w:top w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:left w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:bottom w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:right w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:insideH w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "<w:insideV w:val=\"single\" w:sz=\"4\" w:space=\"0\" w:color=\"auto\"/>"
        "</w:tblBorders>");
    QString out = QStringLiteral("<w:tbl><w:tblPr><w:tblW w:w=\"0\" w:type=\"auto\"/>")
                  + borders + QStringLiteral("</w:tblPr><w:tblGrid>");
    for (int c = 0; c < table->columns(); ++c)
        out += QStringLiteral("<w:gridCol/>");
    out += QStringLiteral("</w:tblGrid>");
    for (int r = 0; r < table->rows(); ++r) {
        out += QStringLiteral("<w:tr>");
        for (int c = 0; c < table->columns(); ++c) {
            const QTextTableCell cell = table->cellAt(r, c);
            const Qt::Alignment a =
                cell.firstCursorPosition().blockFormat().alignment();
            QString jc;
            if (a & Qt::AlignHCenter)
                jc = QStringLiteral("<w:jc w:val=\"center\"/>");
            else if (a & Qt::AlignRight)
                jc = QStringLiteral("<w:jc w:val=\"right\"/>");
            QString runs;
            for (QTextFrame::iterator fit = cell.begin(); !fit.atEnd(); ++fit) {
                const QTextBlock b = fit.currentBlock();
                if (b.isValid())
                    runs += runsForBlock(b, false);
            }
            // Una celda = un párrafo (suficiente para tablas Markdown); siempre debe
            // contener al menos un <w:p>, que paragraph() garantiza.
            out += QStringLiteral("<w:tc><w:tcPr><w:tcW w:w=\"0\" w:type=\"auto\"/></w:tcPr>")
                   + paragraph(jc, runs) + QStringLiteral("</w:tc>");
        }
        out += QStringLiteral("</w:tr>");
    }
    out += QStringLiteral("</w:tbl>\n");
    return out;
}

QString DocxWriter::document(const QString &title)
{
    QString out = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:document"
        " xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\""
        " xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\""
        " xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\""
        " xmlns:a=\"http://schemas.openxmlformats.org/drawingml/2006/main\""
        " xmlns:pic=\"http://schemas.openxmlformats.org/drawingml/2006/picture\">\n"
        "<w:body>\n");

    if (!title.isEmpty())
        out += paragraph(QStringLiteral("<w:pStyle w:val=\"Title\"/>"),
                         makeRun(title, QString()));

    QList<QTextTable *> doneTables;
    for (QTextBlock block = m_doc->begin(); block.isValid(); block = block.next()) {
        const QTextBlockFormat bf = block.blockFormat();

        // Tablas: se emiten una vez, al ver su primer bloque. Tras la tabla hace
        // falta un párrafo (OOXML no admite tabla pegada a otra ni al final).
        if (QTextTable *table = QTextCursor(block).currentTable()) {
            if (!doneTables.contains(table)) {
                doneTables.append(table);
                out += tableXml(table) + QStringLiteral("<w:p/>\n");
            }
            continue;
        }

        // Bloque de código → párrafo con sombreado y fuente monoespaciada.
        if (bf.hasProperty(QTextFormat::BlockCodeFence)) {
            out += paragraph(
                QStringLiteral("<w:shd w:val=\"clear\" w:color=\"auto\" w:fill=\"F2F2F2\"/>"),
                makeRun(block.text(), QStringLiteral(
                    "<w:rPr><w:rFonts w:ascii=\"Courier New\" w:hAnsi=\"Courier New\""
                    " w:cs=\"Courier New\"/></w:rPr>")));
            continue;
        }

        // Regla horizontal → párrafo con borde inferior.
        if (bf.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
            out += paragraph(QStringLiteral(
                "<w:pBdr><w:bottom w:val=\"single\" w:sz=\"6\" w:space=\"1\""
                " w:color=\"auto\"/></w:pBdr>"), QString());
            continue;
        }

        // Listas (viñetas, numeradas y tareas) con numeración real (numbering.xml).
        if (QTextList *list = block.textList()) {
            const int depth = qMax(1, list->format().indent());
            const int ilvl = qMin(8, depth - 1);
            const bool ordered = list->format().style() == QTextListFormat::ListDecimal;
            const QString pPr =
                QStringLiteral("<w:pStyle w:val=\"ListParagraph\"/>"
                               "<w:numPr><w:ilvl w:val=\"%1\"/><w:numId w:val=\"%2\"/></w:numPr>")
                    .arg(ilvl).arg(ordered ? 2 : 1);
            // Las tareas no tienen casilla nativa: se prefija el carácter ☐/☒.
            QString marker;
            if (bf.marker() == QTextBlockFormat::MarkerType::Checked)
                marker = makeRun(QStringLiteral("☒ "), QString());
            else if (bf.marker() == QTextBlockFormat::MarkerType::Unchecked)
                marker = makeRun(QStringLiteral("☐ "), QString());
            out += paragraph(pPr, marker + runsForBlock(block, false));
            continue;
        }

        // Cita en bloque → sangría + borde izquierdo.
        if (bf.intProperty(QTextFormat::BlockQuoteLevel) > 0) {
            out += paragraph(QStringLiteral(
                "<w:ind w:left=\"720\"/>"
                "<w:pBdr><w:left w:val=\"single\" w:sz=\"18\" w:space=\"12\""
                " w:color=\"CCCCCC\"/></w:pBdr>"), runsForBlock(block, false));
            continue;
        }

        // Encabezado o párrafo normal, con alineación.
        const int level = bf.headingLevel();
        QString pPr;
        if (level >= 1 && level <= 6)
            pPr += QStringLiteral("<w:pStyle w:val=\"Heading%1\"/>").arg(level);
        const Qt::Alignment a = bf.alignment();
        if (a & Qt::AlignHCenter)
            pPr += QStringLiteral("<w:jc w:val=\"center\"/>");
        else if (a & Qt::AlignRight)
            pPr += QStringLiteral("<w:jc w:val=\"right\"/>");
        else if (a & Qt::AlignJustify)
            pPr += QStringLiteral("<w:jc w:val=\"both\"/>");
        out += paragraph(pPr, runsForBlock(block, /*ignoreBold=*/level >= 1));
    }

    out += QStringLiteral(
        "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/>"
        "<w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\""
        " w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/></w:sectPr>\n"
        "</w:body>\n</w:document>\n");
    return out;
}

QByteArray docxContentTypesXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
        "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
        "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
        "<Default Extension=\"png\" ContentType=\"image/png\"/>"
        "<Override PartName=\"/word/document.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml\"/>"
        "<Override PartName=\"/word/styles.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml\"/>"
        "<Override PartName=\"/word/numbering.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.wordprocessingml.numbering+xml\"/>"
        "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/>"
        "</Types>\n");
}

QByteArray docxRootRelsXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"word/document.xml\"/>"
        "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" Target=\"docProps/core.xml\"/>"
        "</Relationships>\n");
}

QByteArray docxDocumentRelsXml(const QList<DocxImage> &images)
{
    QString out = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
        "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
        "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/numbering\" Target=\"numbering.xml\"/>");
    for (int i = 0; i < images.size(); ++i)
        out += QStringLiteral(
            "<Relationship Id=\"rId%1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/image\" Target=\"%2\"/>")
            .arg(3 + i).arg(images.at(i).partName);
    out += QStringLiteral("</Relationships>\n");
    return out.toUtf8();
}

QByteArray docxCoreXml(const Language &language, const QString &title)
{
    QString meta;
    if (!title.isEmpty())
        meta += QStringLiteral("<dc:title>%1</dc:title>").arg(xmlEsc(title));
    meta += QStringLiteral("<dc:language>%1-%2</dc:language>")
                .arg(language.odfLang, language.odfCountry);
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<cp:coreProperties"
        " xmlns:cp=\"http://schemas.openxmlformats.org/package/2006/metadata/core-properties\""
        " xmlns:dc=\"http://purl.org/dc/elements/1.1/\">"
        "%1</cp:coreProperties>\n").arg(meta);
    return xml.toUtf8();
}

}  // namespace

QString toDocxDocumentXml(const QTextDocument *doc, const QString &title,
                          QList<DocxImage> *images)
{
    DocxWriter writer(doc, images);
    return writer.document(title);
}

QByteArray docxStylesXml(const Language &language)
{
    const QString lang = language.odfLang + QLatin1Char('-') + language.odfCountry;

    QString styles;
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:default=\"1\" w:styleId=\"Normal\">"
        "<w:name w:val=\"Normal\"/></w:style>");
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:styleId=\"Title\"><w:name w:val=\"Title\"/>"
        "<w:basedOn w:val=\"Normal\"/><w:next w:val=\"Normal\"/>"
        "<w:pPr><w:spacing w:after=\"300\"/><w:contextualSpacing/></w:pPr>"
        "<w:rPr><w:b/><w:sz w:val=\"56\"/><w:szCs w:val=\"56\"/></w:rPr></w:style>");
    styles += QStringLiteral(
        "<w:style w:type=\"paragraph\" w:styleId=\"ListParagraph\">"
        "<w:name w:val=\"List Paragraph\"/><w:basedOn w:val=\"Normal\"/>"
        "<w:pPr><w:contextualSpacing/></w:pPr></w:style>");

    const int sizes[6] = {32, 28, 26, 24, 23, 22};  // medio-puntos
    for (int n = 1; n <= 6; ++n)
        styles += QStringLiteral(
            "<w:style w:type=\"paragraph\" w:styleId=\"Heading%1\">"
            "<w:name w:val=\"heading %1\"/><w:basedOn w:val=\"Normal\"/>"
            "<w:next w:val=\"Normal\"/>"
            "<w:pPr><w:keepNext/><w:spacing w:before=\"240\" w:after=\"60\"/>"
            "<w:outlineLvl w:val=\"%2\"/></w:pPr>"
            "<w:rPr><w:b/><w:color w:val=\"2E74B5\"/><w:sz w:val=\"%3\"/>"
            "<w:szCs w:val=\"%3\"/></w:rPr></w:style>")
            .arg(n).arg(n - 1).arg(sizes[n - 1]);

    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:styles xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:docDefaults><w:rPrDefault><w:rPr>"
        "<w:rFonts w:ascii=\"Calibri\" w:hAnsi=\"Calibri\"/>"
        "<w:sz w:val=\"22\"/><w:szCs w:val=\"22\"/><w:lang w:val=\"%1\"/>"
        "</w:rPr></w:rPrDefault>"
        "<w:pPrDefault><w:pPr><w:spacing w:after=\"160\" w:line=\"259\""
        " w:lineRule=\"auto\"/></w:pPr></w:pPrDefault></w:docDefaults>"
        "%2</w:styles>\n").arg(lang, styles);
    return xml.toUtf8();
}

QByteArray docxNumberingXml()
{
    QString bullet, decimal;
    for (int i = 0; i < 9; ++i) {
        const int left = (i + 1) * 720;
        bullet += QStringLiteral(
            "<w:lvl w:ilvl=\"%1\"><w:start w:val=\"1\"/><w:numFmt w:val=\"bullet\"/>"
            "<w:lvlText w:val=\"•\"/><w:lvlJc w:val=\"left\"/>"
            "<w:pPr><w:ind w:left=\"%2\" w:hanging=\"360\"/></w:pPr></w:lvl>")
            .arg(i).arg(left);
        // El lvlText decimal lleva «%N.» (con % literal): se concatena para que no
        // colisione con los marcadores de QString::arg.
        decimal += QStringLiteral("<w:lvl w:ilvl=\"") + QString::number(i)
            + QStringLiteral("\"><w:start w:val=\"1\"/><w:numFmt w:val=\"decimal\"/>"
                             "<w:lvlText w:val=\"%")
            + QString::number(i + 1) + QStringLiteral(".\"/><w:lvlJc w:val=\"left\"/>"
                                                      "<w:pPr><w:ind w:left=\"")
            + QString::number(left) + QStringLiteral("\" w:hanging=\"360\"/></w:pPr></w:lvl>");
    }
    const QString xml = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
        "<w:numbering xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\">"
        "<w:abstractNum w:abstractNumId=\"0\"><w:multiLevelType w:val=\"hybridMultilevel\"/>%1</w:abstractNum>"
        "<w:abstractNum w:abstractNumId=\"1\"><w:multiLevelType w:val=\"multilevel\"/>%2</w:abstractNum>"
        "<w:num w:numId=\"1\"><w:abstractNumId w:val=\"0\"/></w:num>"
        "<w:num w:numId=\"2\"><w:abstractNumId w:val=\"1\"/></w:num>"
        "</w:numbering>\n").arg(bullet, decimal);
    return xml.toUtf8();
}

bool writeDocx(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error)
{
    QList<DocxImage> images;
    const QByteArray documentXml = toDocxDocumentXml(doc, title, &images).toUtf8();

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QZipWriter zip(&file);
    zip.setCompressionPolicy(QZipWriter::AutoCompress);
    zip.addFile(QStringLiteral("[Content_Types].xml"), docxContentTypesXml());
    zip.addFile(QStringLiteral("_rels/.rels"), docxRootRelsXml());
    zip.addFile(QStringLiteral("docProps/core.xml"), docxCoreXml(language, title));
    zip.addFile(QStringLiteral("word/document.xml"), documentXml);
    zip.addFile(QStringLiteral("word/styles.xml"), docxStylesXml(language));
    zip.addFile(QStringLiteral("word/numbering.xml"), docxNumberingXml());
    zip.addFile(QStringLiteral("word/_rels/document.xml.rels"), docxDocumentRelsXml(images));
    for (const DocxImage &img : images)
        zip.addFile(QStringLiteral("word/") + img.partName, img.data);
    zip.close();
    file.close();

    if (zip.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete DOCX.");
        return false;
    }
    return true;
}

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

QString epubContentXhtml(const QString &bodyInner, const QString &title,
                         const Language &language)
{
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<!DOCTYPE html>\n"
               "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"%1\" lang=\"%1\">\n"
               "<head>\n<meta charset=\"utf-8\"/>\n<title>%2</title>\n"
               "<link rel=\"stylesheet\" type=\"text/css\" href=\"style.css\"/>\n"
               "</head>\n<body>\n%3\n</body>\n</html>\n")
        .arg(language.code, title.toHtmlEscaped(), bodyInner);
}

QByteArray epubContainerXml()
{
    return QByteArrayLiteral(
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<container version=\"1.0\" "
        "xmlns=\"urn:oasis:names:tc:opendocument:xmlns:container\">\n"
        "  <rootfiles>\n"
        "    <rootfile full-path=\"OEBPS/content.opf\" "
        "media-type=\"application/oebps-package+xml\"/>\n"
        "  </rootfiles>\n"
        "</container>\n");
}

QByteArray epubContentOpf(const Language &language, const QString &title,
                          const QStringList &imageHrefs, const QString &uuid,
                          const QString &modified)
{
    QString images;
    int i = 0;
    for (const QString &href : imageHrefs)
        images += QStringLiteral(
                      "    <item id=\"img%1\" href=\"%2\" media-type=\"image/png\"/>\n")
                      .arg(++i)
                      .arg(href);

    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<package xmlns=\"http://www.idpf.org/2007/opf\" version=\"3.0\" "
               "unique-identifier=\"bookid\">\n"
               "  <metadata xmlns:dc=\"http://purl.org/dc/elements/1.1/\">\n"
               "    <dc:identifier id=\"bookid\">urn:uuid:%1</dc:identifier>\n"
               "    <dc:title>%2</dc:title>\n"
               "    <dc:language>%3</dc:language>\n"
               "    <meta property=\"dcterms:modified\">%4</meta>\n"
               "  </metadata>\n"
               "  <manifest>\n"
               "    <item id=\"nav\" href=\"nav.xhtml\" "
               "media-type=\"application/xhtml+xml\" properties=\"nav\"/>\n"
               "    <item id=\"ncx\" href=\"toc.ncx\" "
               "media-type=\"application/x-dtbncx+xml\"/>\n"
               "    <item id=\"css\" href=\"style.css\" media-type=\"text/css\"/>\n"
               "    <item id=\"content\" href=\"content.xhtml\" "
               "media-type=\"application/xhtml+xml\"/>\n"
               "%5"
               "  </manifest>\n"
               "  <spine toc=\"ncx\">\n"
               "    <itemref idref=\"content\"/>\n"
               "  </spine>\n"
               "</package>\n")
        .arg(uuid, title.toHtmlEscaped(), language.code, modified, images)
        .toUtf8();
}

QByteArray epubNavXhtml(const Language &language, const QString &title)
{
    const QString t = title.toHtmlEscaped();
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n<!DOCTYPE html>\n"
               "<html xmlns=\"http://www.w3.org/1999/xhtml\" "
               "xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"%1\" lang=\"%1\">\n"
               "<head><meta charset=\"utf-8\"/><title>%2</title></head>\n"
               "<body>\n<nav epub:type=\"toc\" id=\"toc\">\n<h1>%2</h1>\n"
               "<ol><li><a href=\"content.xhtml\">%2</a></li></ol>\n</nav>\n"
               "</body>\n</html>\n")
        .arg(language.code, t)
        .toUtf8();
}

QByteArray epubTocNcx(const QString &title, const QString &uuid)
{
    const QString t = title.toHtmlEscaped();
    return QStringLiteral(
               "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
               "<ncx xmlns=\"http://www.daisy.org/z3986/2005/ncx/\" version=\"2005-1\">\n"
               "  <head><meta name=\"dtb:uid\" content=\"urn:uuid:%1\"/></head>\n"
               "  <docTitle><text>%2</text></docTitle>\n"
               "  <navMap><navPoint id=\"np1\" playOrder=\"1\">"
               "<navLabel><text>%2</text></navLabel>"
               "<content src=\"content.xhtml\"/></navPoint></navMap>\n"
               "</ncx>\n")
        .arg(uuid, t)
        .toUtf8();
}

QByteArray epubStyleCss()
{
    return QByteArrayLiteral(
        "body { font-family: serif; line-height: 1.5; margin: 1em; }\n"
        "h1, h2, h3, h4 { font-family: sans-serif; line-height: 1.2; }\n"
        "pre, code, tt { font-family: monospace; }\n"
        "pre { background: #f4f4f4; padding: 0.6em; overflow: auto; }\n"
        "blockquote { margin: 1em 0; padding: 0.2em 1em; "
        "border-left: 4px solid #ccc; }\n"
        "table { border-collapse: collapse; }\n"
        "td, th { border: 1px solid #999; padding: 0.3em 0.6em; }\n"
        "img { max-width: 100%; }\n");
}

bool writeEpub(const QTextDocument *doc, const QString &path, const Language &language,
               const QString &title, QString *error)
{
    QString body = htmlBodyToXhtml(doc->toHtml());

    // Localiza las imágenes referenciadas, las embebe como PNG y reescribe su src
    // a una ruta dentro del paquete. Las que no se puedan cargar (p. ej. URLs
    // externas) se dejan tal cual.
    QStringList imageHrefs;
    QList<QPair<QString, QByteArray>> imageFiles;
    QHash<QString, QString> remap;
    static const QRegularExpression imgRe(QStringLiteral("src=\"([^\"]+)\""));
    auto it = imgRe.globalMatch(body);
    while (it.hasNext()) {
        const QString src = it.next().captured(1);
        if (remap.contains(src))
            continue;
        const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(src));
        QImage img = qvariant_cast<QImage>(res);
        if (img.isNull()) {
            const QPixmap pm = qvariant_cast<QPixmap>(res);
            if (!pm.isNull())
                img = pm.toImage();
        }
        if (img.isNull())
            continue;
        QByteArray png;
        {
            QBuffer buf(&png);
            buf.open(QIODevice::WriteOnly);
            img.save(&buf, "PNG");
        }
        const QString href = QStringLiteral("images/image%1.png").arg(imageFiles.size() + 1);
        remap.insert(src, href);
        imageFiles.append({href, png});
        imageHrefs.append(href);
    }
    for (auto i = remap.cbegin(); i != remap.cend(); ++i)
        body.replace(QStringLiteral("src=\"%1\"").arg(i.key()),
                     QStringLiteral("src=\"%1\"").arg(i.value()));

    const QString safeTitle = title.isEmpty() ? QStringLiteral("Documento") : title;
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString modified =
        QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-ddTHH:mm:ss'Z'"));

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = file.errorString();
        return false;
    }

    QZipWriter zip(&file);
    // `mimetype` debe ir primero y SIN comprimir (lo exige la especificación EPUB).
    zip.setCompressionPolicy(QZipWriter::NeverCompress);
    zip.addFile(QStringLiteral("mimetype"), QByteArrayLiteral("application/epub+zip"));
    zip.setCompressionPolicy(QZipWriter::AutoCompress);
    zip.addFile(QStringLiteral("META-INF/container.xml"), epubContainerXml());
    zip.addFile(QStringLiteral("OEBPS/content.opf"),
                epubContentOpf(language, safeTitle, imageHrefs, uuid, modified));
    zip.addFile(QStringLiteral("OEBPS/nav.xhtml"), epubNavXhtml(language, safeTitle));
    zip.addFile(QStringLiteral("OEBPS/toc.ncx"), epubTocNcx(safeTitle, uuid));
    zip.addFile(QStringLiteral("OEBPS/style.css"), epubStyleCss());
    zip.addFile(QStringLiteral("OEBPS/content.xhtml"),
                epubContentXhtml(body, safeTitle, language).toUtf8());
    for (const auto &im : imageFiles)
        zip.addFile(QStringLiteral("OEBPS/") + im.first, im.second);
    zip.close();
    file.close();

    if (zip.status() != QZipWriter::NoError) {
        if (error)
            *error = QStringLiteral("Error al escribir el paquete EPUB.");
        return false;
    }
    return true;
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
