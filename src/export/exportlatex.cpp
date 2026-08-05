/// \file
/// \brief Serializador a LaTeX (.tex): preámbulo portable, escapado, y el cuerpo con
///        las imágenes traídas junto al .tex para que compile esté donde esté.

#include "exporters.h"
#include "exportutil.h"
#include "mathblocks.h"

#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextList>
#include <QTextTable>
#include <QTextTableCell>

namespace mdexport {

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

// isScriptChar vive en exportutil.h/exporters.cpp: la misma tabla de rangos decide
// lo que LaTeX conserva y lo que el PDF necesita comprobar (ver `cjkScriptsIn`).

QString latexEscape(const QString &s, LatexIssues *issues = nullptr)
{
    QString out;
    out.reserve(s.size());
    for (const uint cp : s.toUcs4()) {
        // Antes que nada: una escritura se conserva SIEMPRE, esté por encima o por
        // debajo del corte de símbolos (el jamo hangul cae por debajo).
        if (isScriptChar(cp)) {
            const char32_t c = cp;  // el overload de uint está obsoleto en Qt 6
            out += QString::fromUcs4(&c, 1);
            if (issues)
                issues->needsUnicodeEngine = true;
            continue;
        }
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
        // Símbolo/emoji sin equivalente → se omite (evita el fallo de pdflatex), pero
        // se cuenta: hasta ahora desaparecía sin que el usuario llegara a saberlo.
        else if (issues)
            ++issues->droppedSymbols;
    }
    return out;
}

// Versión para entornos verbatim, donde no caben comandos LaTeX: conserva el
// texto literal y solo descarta los símbolos/emoji altos que romperían pdflatex.
QString codeBlockSanitize(const QString &s, LatexIssues *issues = nullptr)
{
    QString out;
    out.reserve(s.size());
    for (const uint cp : s.toUcs4()) {
        if (isScriptChar(cp)) {  // un comentario en chino dentro del código, p.ej.
            const char32_t c = cp;  // el overload de uint está obsoleto en Qt 6
            out += QString::fromUcs4(&c, 1);
            if (issues)
                issues->needsUnicodeEngine = true;
            continue;
        }
        if (cp >= kHighSymbolStart) {
            if (issues)
                ++issues->droppedSymbols;
            continue;  // símbolo/emoji que pdflatex+T1 no compone
        }
        switch (cp) {
        case u'\\': out += QStringLiteral("\\textbackslash{}"); continue;
        case u'{':  out += QStringLiteral("\\{"); continue;
        case u'}':  out += QStringLiteral("\\}"); continue;
        default:    out += QChar(static_cast<char16_t>(cp));
        }
    }
    return out;
}

// Destino de un `\href`. NO vale `latexEscape`: su `~` → `\textasciitilde{}` deja
// las llaves DENTRO de la URL, y hyperref acaba enlazando a
// «http://e.com/~{}manuel», que no lleva a ninguna parte (las URLs con `~` de
// usuario son de lo más común). hyperref lee el argumento casi literal, así que
// basta con anteponer barra a lo que TeX interpreta, dejar el `~` tal cual y
// codificar en porcentaje lo que ni siquiera es válido en una URL (RFC 3986),
// incluido todo lo que no sea ASCII imprimible.
QString latexUrl(const QString &url)
{
    static const QByteArray unsafe = "{}\\^\"<>|`";
    const QByteArray utf8 = url.toUtf8();
    QString out;
    out.reserve(utf8.size());
    for (const char raw : utf8) {
        const uchar b = static_cast<uchar>(raw);
        if (b == '#' || b == '%' || b == '&' || b == '_' || b == '$') {
            out += QLatin1Char('\\');
            out += QLatin1Char(static_cast<char>(b));
        } else if (b < 0x21 || b > 0x7E || unsafe.contains(static_cast<char>(b))) {
            // El `%` de la codificación va escapado: crudo comentaría la línea.
            out += QStringLiteral("\\%")
                   + QString::number(b, 16).rightJustified(2, QLatin1Char('0')).toUpper();
        } else {
            out += QLatin1Char(static_cast<char>(b));
        }
    }
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
const QStringList kPdflatexFormats = {QStringLiteral("pdf"), QStringLiteral("png"),
                                      QStringLiteral("jpg"), QStringLiteral("jpeg")};

bool pdflatexIncludable(const QString &ext)
{
    return kPdflatexFormats.contains(ext);
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

    // Los formatos que pdflatex incluye nativos viajan con sus bytes; el resto
    // (SVG, GIF, BMP…) se rasteriza a PNG. De eso se encarga imageData.
    const ImageData image = imageData(name, images.doc, kPdflatexFormats);
    if (!image.isNull()) {
        const QString file = writeSidecar(images, image.extension, image.bytes);
        if (!file.isEmpty())
            return incl.arg(latexEscape(file));
    }
    return imagePlaceholder(name);
}

// Texto en línea de un bloque, con el formato de carácter convertido a comandos
// LaTeX (negrita, cursiva, subrayado, tachado, código, enlaces e imágenes).
// `ignoreBold` evita el \textbf en los encabezados (que Qt marca en negrita y en
// LaTeX ya lo son), para no producir \section{\textbf{...}}.
// `issues` recoge lo intraducible del TEXTO del documento (ver LatexIssues). Las
// rutas de imagen no pasan por ahí a propósito: un carácter raro en un nombre de
// fichero es otro problema, y no dice nada de la escritura del documento.
QString inlineLatex(const QTextBlock &block, LatexImages &images, LatexIssues *issues,
                    bool ignoreBold = false)
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
        QString t = latexEscape(frag.text(), issues);
        if (t.isEmpty())
            continue;
        // Super/subíndice que NO viene de una fórmula: los de `x^2^`/`H~2~O`
        // (mdsupsub) y los de las referencias de nota al pie. Sin esto se perdían
        // en silencio —el texto salía a ras de línea, «H2O»—, aunque en el editor
        // y en los demás formatos sí se vean elevados.
        const QTextCharFormat::VerticalAlignment va = cf.verticalAlignment();
        if (va == QTextCharFormat::AlignSuperScript)
            t = QStringLiteral("\\textsuperscript{%1}").arg(t);
        else if (va == QTextCharFormat::AlignSubScript)
            t = QStringLiteral("\\textsubscript{%1}").arg(t);
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
            // El destino va por latexUrl (no latexEscape): un `%` crudo rompería el
            // .tex, un `}{\...}` inyectaría comandos, y `\textasciitilde{}` metería
            // llaves dentro de la propia URL.
            t = QStringLiteral("\\href{%1}{%2}").arg(latexUrl(cf.anchorHref()), t);
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

QString tableLatex(QTextTable *table, LatexImages &images, LatexIssues *issues)
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
                    text += inlineLatex(b, images, issues);
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
static QString latexPreamble(const Language &language, const QString &title,
                             LatexIssues *issues)
{
    // El título se escapa lo PRIMERO: puede traer ideogramas él solo (un documento
    // titulado en chino con el cuerpo en español), y de eso depende el preámbulo.
    const QString escapedTitle = title.isEmpty() ? QString() : latexEscape(title, issues);
    // Lo decide lo que TRAE el documento, no el idioma con el que se exporta: un
    // documento marcado como chino pero escrito en pinyin no necesita nada de esto y
    // se queda con el preámbulo portable. Los rótulos que babel pondría en chino
    // («目录», «表») no llegan a aparecer porque este serializador no emite ni
    // \tableofcontents ni \caption; comprobado compilándolo con los dos motores.
    const bool cjk = issues && issues->needsUnicodeEngine;

    QString out;
    if (cjk) {
        // Con ideogramas el .tex ya no es portable entre motores, así que lo primero
        // que se ve al abrirlo es con qué se compila.
        out += QStringLiteral(
            "% Este documento contiene texto en chino, japonés o coreano.\n"
            "% Compílalo con:  xelatex documento.tex   (o lualatex)\n"
            "% pdflatex NO puede componer esas escrituras.\n");
    }
    out += QStringLiteral("\\documentclass[11pt]{article}\n");
    out += QStringLiteral("\\usepackage{iftex}\n");
    if (cjk) {
        // ctex (sobre xeCJK) elige por sí solo una fuente con ideogramas de las
        // instaladas en el sistema, que es lo que hace que el .tex compile tal cual
        // en Windows, macOS y Linux sin que el usuario configure nada. Va ANTES que
        // babel para que los rótulos («Índice», «Cuadro») los siga fijando el idioma
        // del documento, no ctex.
        // En pdfTeX no hay nada que hacer —ninguna combinación de inputenc/fontenc
        // compone ideogramas—, así que se aborta con una línea que explique el motivo
        // en vez de dejar el críptico «Unicode character not set up for use with
        // LaTeX». Sin acentos: es un mensaje del propio TeX.
        out += QStringLiteral(
            "\\ifPDFTeX\n"
            "  \\errmessage{Este documento tiene texto chino/japones/coreano:"
            " compilalo con xelatex o lualatex, no con pdflatex}\n"
            "\\else\n"
            "  \\usepackage{fontspec}\n"
            "  \\usepackage[UTF8]{ctex}\n"
            "\\fi\n");
    } else {
        out += QStringLiteral("\\ifPDFTeX\n"
                              "  \\usepackage[utf8]{inputenc}\n"
                              "  \\usepackage[T1]{fontenc}\n"
                              "\\else\n"
                              "  \\usepackage{fontspec}\n"
                              "\\fi\n");
    }
    out += QStringLiteral("\\usepackage[%1]{babel}\n").arg(language.babel);
    out += QStringLiteral("\\usepackage{amsmath}\n");
    out += QStringLiteral("\\usepackage{amssymb}\n");
    out += QStringLiteral("\\usepackage[normalem]{ulem}\n");
    out += QStringLiteral("\\usepackage{alltt}\n");  // bloques de código (ver codeBlockSanitize)
    out += QStringLiteral("\\usepackage{graphicx}\n");
    out += QStringLiteral("\\usepackage[export]{adjustbox}\n");  // max width en imágenes
    out += QStringLiteral("\\usepackage{hyperref}\n");
    if (!escapedTitle.isEmpty())
        out += QStringLiteral("\\title{%1}\n\\author{}\n\\date{}\n").arg(escapedTitle);
    out += QStringLiteral("\\begin{document}\n");
    if (!escapedTitle.isEmpty())  // `\maketitle` sin `\title` deja un título vacío
        out += QStringLiteral("\\maketitle\n");
    return out;
}

QString toLatex(const QTextDocument *doc, const Language &language, const QString &title,
                const QString &outputTexPath, LatexIssues *issuesOut)
{
    // Un destino siempre válido evita comprobar el puntero en cada carácter; si el
    // llamante no quiere el parte de incidencias, se tira al salir.
    LatexIssues discarded;
    LatexIssues *issues = issuesOut ? issuesOut : &discarded;

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
    int quoteLevel = 0;     // citas anidadas abiertas (`>`, `>>`…)
    bool inCode = false;
    QString body;

    // LaTeX cuenta `quote` como entorno de lista y no admite más de cuatro
    // anidados («Too deeply nested» es un error FATAL, no un aviso): se limitan
    // ambas profundidades para que el .tex compile siempre. Se pierde la sangría
    // de los niveles de más, nunca el contenido.
    constexpr int kMaxNesting = 4;
    constexpr int kMaxQuoteLevel = 3;

    const auto closeLists = [&] {
        while (!openLists.isEmpty())
            body += QStringLiteral("\\end{%1}\n").arg(openLists.takeLast());
    };
    const auto closeQuote = [&] {
        while (quoteLevel > 0) {
            body += QStringLiteral("\\end{quote}\n");
            --quoteLevel;
        }
    };
    // Abre o cierra citas hasta dejar `want` niveles: `>>` es una cita DENTRO de
    // otra, y aplanarlas a una sola perdía la estructura del original.
    const auto setQuoteLevel = [&](int want) {
        want = qBound(0, want, kMaxQuoteLevel);
        while (quoteLevel > want) {
            body += QStringLiteral("\\end{quote}\n");
            --quoteLevel;
        }
        while (quoteLevel < want) {
            body += QStringLiteral("\\begin{quote}\n");
            ++quoteLevel;
        }
    };
    const auto closeCode = [&] {
        if (inCode) { body += QStringLiteral("\\end{alltt}\n"); inCode = false; }
    };

    QList<QTextTable *> doneTables;
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        const QTextBlockFormat bf = block.blockFormat();

        // Tablas: se emiten una vez, al ver su primer bloque.
        if (QTextTable *table = QTextCursor(block).currentTable()) {
            if (!doneTables.contains(table)) {
                closeLists(); closeQuote(); closeCode();
                doneTables.append(table);
                body += tableLatex(table, images, issues) + QLatin1Char('\n');
            }
            continue;
        }

        // Bloque de código → verbatim (agrupando líneas consecutivas).
        if (bf.hasProperty(QTextFormat::BlockCodeFence)) {
            closeLists();
            // Un bloque de código DENTRO de una cita sigue siendo parte de ella.
            if (bf.intProperty(QTextFormat::BlockQuoteLevel) <= 0)
                closeQuote();
            // `alltt` y no `verbatim`: verbatim es INTERRUMPIBLE —basta que el
            // código contenga la línea `\end{verbatim}`, como cualquier documento que
            // hable de LaTeX, para que el entorno se cierre ahí y el .tex deje de
            // compilar entero—. En alltt solo `\ { }` conservan su significado, y
            // codeBlockSanitize los escapa: el resto es literal pase lo que pase.
            if (!inCode) { body += QStringLiteral("\\begin{alltt}\n"); inCode = true; }
            body += codeBlockSanitize(block.text(), issues) + QLatin1Char('\n');
            continue;
        }
        closeCode();

        // Regla horizontal.
        if (bf.hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth)) {
            closeLists(); closeQuote();
            body += QStringLiteral("\\noindent\\rule{\\linewidth}{0.4pt}\n\n");
            continue;
        }

        const QString text = inlineLatex(block, images, issues);

        // Listas (viñetas, numeradas y tareas) con anidamiento por sangría.
        if (QTextList *list = block.textList()) {
            // Una lista DENTRO de una cita sigue siendo parte de la cita: cerrarla
            // aquí la sacaba del bloque citado y abría otra cita detrás.
            if (bf.intProperty(QTextFormat::BlockQuoteLevel) <= 0)
                closeQuote();
            // El margen de anidamiento lo comparten listas y citas (ver kMaxNesting).
            const int depth =
                qBound(1, list->format().indent(), qMax(1, kMaxNesting - quoteLevel));
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
                // `5. cinco` empieza en cinco. Qt conserva ese número de arranque,
                // pero enumerate siempre cuenta desde 1 salvo que se le mueva el
                // contador del nivel correspondiente (enumi, enumii, enumiii…).
                if (!ordered || list->format().start() == 1)
                    continue;
                static const char *const counters[] = {"enumi", "enumii", "enumiii",
                                                       "enumiv"};
                const int level = qBound(1, int(openLists.size()), kMaxNesting);
                body += QStringLiteral("\\setcounter{%1}{%2}\n")
                            .arg(QLatin1String(counters[level - 1]))
                            .arg(list->format().start() - 1);
            }
            // Tarea: prefijo con casilla (requiere amssymb).
            QString marker;
            if (bf.marker() == QTextBlockFormat::MarkerType::Checked)
                marker = QStringLiteral("$\\boxtimes$ ");
            else if (bf.marker() == QTextBlockFormat::MarkerType::Unchecked)
                marker = QStringLiteral("$\\square$ ");
            const QString content = marker + text;
            // Un contenido que empieza por `[` LaTeX se lo come como argumento
            // opcional de `\item`: el clásico `- [1] Knuth` de una bibliografía
            // salía con «1» de viñeta y sin corchetes. Un grupo vacío lo impide.
            const QString guard =
                content.startsWith(QLatin1Char('[')) ? QStringLiteral("{}") : QString();
            body += QStringLiteral("\\item %1%2\n").arg(guard, content);
            continue;
        }
        closeLists();

        if (text.isEmpty())
            continue;

        // Cita en bloque.
        if (const int level = bf.intProperty(QTextFormat::BlockQuoteLevel); level > 0) {
            setQuoteLevel(level);
            // Línea en blanco entre párrafos: sin ella, dos párrafos citados se
            // fundían en uno solo al componer.
            body += text + QStringLiteral("\n\n");
            continue;
        }
        closeQuote();

        // Encabezado o párrafo normal.
        const int level = bf.headingLevel();
        if (level >= 1)
            body += headingCommand(level,
                                   inlineLatex(block, images, issues, /*ignoreBold=*/true));
        else
            body += text + QStringLiteral("\n\n");
    }
    closeLists(); closeQuote(); closeCode();

    // El preámbulo va el ÚLTIMO a propósito: depende de lo que haya aparecido en el
    // cuerpo (los ideogramas cambian los paquetes y el motor exigido).
    return latexPreamble(language, title, issues) + body + QStringLiteral("\\end{document}\n");
}

}  // namespace mdexport
