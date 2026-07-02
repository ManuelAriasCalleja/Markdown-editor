/// \file
/// \brief Implementación de la serialización canónica del documento: alineación
///        de tablas, fórmulas y admoniciones reinyectadas sobre `toMarkdown`.

#include "tableedit.h"

#include "admonitions.h"
#include "codespanfix.h"
#include "diagramdoc.h"
#include "markdownrender.h"
#include "mathblocks.h"
#include "supsub.h"

#include <QStringList>

#include <memory>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>
#include <QTextTableCell>

namespace mdtable {

namespace {

// ¿La línea es una separadora de tabla? (solo '|', '-', ':' y espacios, y con al
// menos un guion). Es lo que distingue la 2ª fila de una tabla de una de datos.
bool isSeparatorLine(const QString &line)
{
    const QString s = line.trimmed();
    if (!s.startsWith(QLatin1Char('|')) || !s.endsWith(QLatin1Char('|')))
        return false;
    bool hasDash = false;
    for (const QChar c : s) {
        if (c == QLatin1Char('-'))
            hasDash = true;
        else if (c != QLatin1Char('|') && c != QLatin1Char(':') && c != QLatin1Char(' '))
            return false;
    }
    return hasDash;
}

bool isPipeRow(const QString &line)
{
    const QString s = line.trimmed();
    return s.length() > 1 && s.startsWith(QLatin1Char('|')) && s.endsWith(QLatin1Char('|'));
}

// Análisis de una posible línea de fence de código vallado, para no confundir una
// tabla de EJEMPLO dentro de un bloque de código con una tabla real.
struct FenceLine {
    bool ok = false;
    QChar ch;
    int len = 0;
    bool bare = false;           // tras el run solo hay espacios (requisito de cierre)
    bool backtickAfter = false;  // backticks tras el run (invalida un fence de `` ` ``)
};

FenceLine fenceOf(const QString &line)
{
    FenceLine f;
    int i = 0;
    while (i < line.size()
           && (line.at(i) == QLatin1Char(' ') || line.at(i) == QLatin1Char('\t')))
        ++i;
    if (line.size() - i < 3)
        return f;
    const QChar c = line.at(i);
    if (c != QLatin1Char('`') && c != QLatin1Char('~'))
        return f;
    int run = 0;
    while (i < line.size() && line.at(i) == c) {
        ++i;
        ++run;
    }
    if (run < 3)
        return f;
    bool onlySpaces = true;
    bool hasBacktick = false;
    for (int j = i; j < line.size(); ++j) {
        const QChar x = line.at(j);
        if (x != QLatin1Char(' ') && x != QLatin1Char('\t'))
            onlySpaces = false;
        if (x == QLatin1Char('`'))
            hasBacktick = true;
    }
    f.ok = true;
    f.ch = c;
    f.len = run;
    f.bare = onlySpaces;
    f.backtickAfter = hasBacktick;
    return f;
}

QString alignmentToken(Qt::Alignment a)
{
    if (a & Qt::AlignHCenter)
        return QStringLiteral(":-:");
    if (a & Qt::AlignRight)
        return QStringLiteral("--:");
    return QStringLiteral("---");  // izquierda / por defecto
}

} // namespace

QList<QList<Qt::Alignment>> columnAlignments(const QTextDocument *doc)
{
    QList<QList<Qt::Alignment>> result;
    QList<const QTextTable *> seen;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        QTextTable *table = QTextCursor(b).currentTable();
        if (!table || seen.contains(table))
            continue;
        seen.append(table);

        QList<Qt::Alignment> cols;
        cols.reserve(table->columns());
        // La alineación fiable está en las celdas de datos (Qt la fija ahí al
        // leer el Markdown); si solo hay cabecera, se usa esa fila.
        const int row = table->rows() > 1 ? 1 : 0;
        for (int c = 0; c < table->columns(); ++c) {
            const Qt::Alignment a =
                table->cellAt(row, c).firstCursorPosition().blockFormat().alignment();
            cols.append(a & Qt::AlignHorizontal_Mask);
        }
        result.append(cols);
    }
    return result;
}

QString injectAlignments(const QString &md,
                         const QList<QList<Qt::Alignment>> &alignments)
{
    if (alignments.isEmpty())
        return md;

    QStringList lines = md.split(QLatin1Char('\n'));
    int table = 0;
    QChar fenceChar;  // nulo = fuera de un bloque de código vallado
    int fenceLen = 0;
    for (int i = 0; i + 1 < lines.size() && table < alignments.size(); ++i) {
        // Salta el contenido de los fences: una tabla de ejemplo dentro de un
        // bloque de código no es una tabla real y no debe consumir una entrada de
        // alineación ni reescribirse.
        const FenceLine f = fenceOf(lines[i]);
        if (f.ok) {
            if (fenceChar.isNull()) {
                if (!(f.ch == QLatin1Char('`') && f.backtickAfter)) {
                    fenceChar = f.ch;  // abre (un fence de `` ` `` no lleva backticks tras el run)
                    fenceLen = f.len;
                }
            } else if (f.ch == fenceChar && f.len >= fenceLen && f.bare) {
                fenceChar = QChar();  // cierra
                fenceLen = 0;
            }
            continue;
        }
        if (!fenceChar.isNull())
            continue;  // dentro de un fence: intacto
        // Cabecera de tabla = fila de tubos seguida de una separadora.
        if (!isPipeRow(lines[i]) || !isSeparatorLine(lines[i + 1]))
            continue;

        const QList<Qt::Alignment> &cols = alignments[table];
        // El nº de columnas de la separadora = celdas entre tubos extremos.
        const int sepCols = lines[i + 1].trimmed().count(QLatin1Char('|')) - 1;
        if (sepCols == cols.size()) {
            QString sep = QStringLiteral("|");
            for (const Qt::Alignment a : cols)
                sep += alignmentToken(a) + QLatin1Char('|');
            lines[i + 1] = sep;
        }
        ++table;
        ++i;  // ya tratada la separadora: saltarla
    }
    return lines.join(QLatin1Char('\n'));
}

QString documentMarkdown(const QTextDocument *doc)
{
    // Las fórmulas se almacenan en el documento como fragmentos «renderizados»:
    // texto visible en Unicode con el TeX en una propiedad del char-format.
    // toMarkdown() no entiende esa propiedad. Antes de serializar sustituimos
    // cada fórmula por un centinela en el área de uso privado de Unicode
    // (texto opaco que Qt no escapa). Tras toMarkdown reinyectamos `$tex$`
    // desde la tabla. Así sobreviven íntegros `\`, `_` y `*` dentro del TeX,
    // cosa que el camino antiguo (inline-code) no garantizaba.
    std::unique_ptr<QTextDocument> clone(doc->clone());
    // Las imágenes de previsualización de diagramas son presentación: fuera del
    // clon antes de serializar (no aparecen en el Markdown ni cuentan para
    // «modificado», porque isModified compara la salida de esta función).
    mddiagram::removePreviewBlocks(clone.get());
    const mdmath::MathSentinelTable table = mdmath::replaceMathWithSentinels(clone.get());
    // Super/subíndice de texto: también a centinelas PUA antes de serializar (su
    // formato de vertical-align lo perdería toMarkdown, que no lo sabe emitir).
    mdsupsub::replaceWithSentinels(clone.get());
    const QString md = injectAlignments(clone->toMarkdown(mdrender::kMarkdownFeatures),
                                        columnAlignments(clone.get()));
    // Reinyecta fórmulas, super/sub y deshace el escape `> \[!NOTE]` de las admoniciones.
    const QString restored = mdadmonition::unescapeMarkers(
        mdsupsub::restoreFromSentinels(mdmath::restoreMathFromSentinels(md, table)));
    // Deshace el sobre-escapado de Qt dentro de los code spans en línea (`\`, `&`…
    // se duplicarían en cada guardado si no; ver mdcodespan).
    return mdcodespan::unescapeInlineCode(restored);
}

} // namespace mdtable
