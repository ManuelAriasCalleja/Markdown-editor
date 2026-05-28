#include "exporters.h"

#include "mathblocks.h"
#include "tableedit.h"

#include <QBuffer>
#include <QFile>
#include <QFont>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QTextFragment>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

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
        if (cp < kHighSymbolStart)
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

// Texto en línea de un bloque, con el formato de carácter convertido a comandos
// LaTeX (negrita, cursiva, subrayado, tachado, código, enlaces e imágenes).
// `ignoreBold` evita el \textbf en los encabezados (que Qt marca en negrita y en
// LaTeX ya lo son), para no producir \section{\textbf{...}}.
QString inlineLatex(const QTextBlock &block, bool ignoreBold = false)
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
            const QString delim = isBlock ? QStringLiteral("$$") : QStringLiteral("$");
            out += delim + tex + delim;
            lastMathTex = tex;
            continue;
        }
        lastMathTex.clear();
        if (cf.isImageFormat()) {
            out += QStringLiteral("\\includegraphics[max width=\\linewidth]{%1}")
                       .arg(cf.toImageFormat().name());
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
            t = QStringLiteral("\\href{%1}{%2}").arg(cf.anchorHref(), t);
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

QString tableLatex(QTextTable *table)
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
                    text += inlineLatex(b);
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

QString toLatex(const QTextDocument *doc, const Language &language, const QString &title)
{
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
                body += tableLatex(table) + QLatin1Char('\n');
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

        const QString text = inlineLatex(block);

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
            body += headingCommand(level, inlineLatex(block, /*ignoreBold=*/true));
        else
            body += text + QStringLiteral("\n\n");
    }
    closeLists(); closeQuote(); closeCode();

    QString out;
    out += QStringLiteral("\\documentclass[11pt]{article}\n");
    // Preámbulo portable entre motores: pdfLaTeX usa inputenc/T1; LuaLaTeX y
    // XeLaTeX usan fontspec (Unicode nativo). Así el .tex compila con cualquiera.
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
    out += body;
    out += QStringLiteral("\\end{document}\n");
    return out;
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

QTextDocument *cloneForExport(const QTextDocument *src)
{
    // Clon directo: preserva fragmentos, formatos y vertical-align (lo que
    // hace que HTML/PDF/ODF muestren los super/subíndices reales). Después
    // limpiamos las propiedades custom de math: son internas del editor y no
    // las entiende ningún writer; quitarlas mantiene el .html/.odt exportado
    // sin atributos privados sueltos.
    QTextDocument *out = src->clone();
    QTextCursor c(out);
    for (QTextBlock b = out->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid())
                continue;
            QTextCharFormat cf = frag.charFormat();
            if (!cf.boolProperty(mdmath::IsMathProperty))
                continue;
            cf.clearProperty(mdmath::IsMathProperty);
            cf.clearProperty(mdmath::MathTexProperty);
            cf.clearProperty(mdmath::MathBlockProperty);
            c.setPosition(frag.position());
            c.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
            c.setCharFormat(cf);
        }
    }
    return out;
}

} // namespace mdexport
