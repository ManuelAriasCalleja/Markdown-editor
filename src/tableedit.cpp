#include "tableedit.h"

#include "mathblocks.h"

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
    for (int i = 0; i + 1 < lines.size() && table < alignments.size(); ++i) {
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
    const mdmath::MathSentinelTable table = mdmath::replaceMathWithSentinels(clone.get());
    const QString md = injectAlignments(clone->toMarkdown(), columnAlignments(clone.get()));
    return mdmath::restoreMathFromSentinels(md, table);
}

} // namespace mdtable
