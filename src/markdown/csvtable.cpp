/// \file
/// \brief Implementación de la detección TSV/CSV y su conversión a tabla Markdown.

#include "csvtable.h"

#include <algorithm>

namespace mdcsvtable {

namespace {

// Escapa lo que rompería una celda de tabla Markdown: la barra vertical.
QString escapeCell(const QString &cell)
{
    QString out = cell;
    out.replace(QLatin1Char('|'), QStringLiteral("\\|"));
    return out;
}

// «| a | b |» a partir de una lista de celdas ya escapadas/normalizadas.
QString rowLine(const QStringList &cells)
{
    QString line = QStringLiteral("|");
    for (const QString &c : cells)
        line += QLatin1Char(' ') + escapeCell(c) + QStringLiteral(" |");
    return line;
}

}  // namespace

Delimited detectDelimited(const QString &text)
{
    Delimited out;

    QString norm = text;
    norm.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    norm.replace(QLatin1Char('\r'), QLatin1Char('\n'));
    QStringList lines = norm.split(QLatin1Char('\n'));
    while (!lines.isEmpty() && lines.constLast().isEmpty())
        lines.removeLast();  // saltos finales
    while (!lines.isEmpty() && lines.constFirst().isEmpty())
        lines.removeFirst();
    if (lines.isEmpty())
        return out;

    const bool hasTab =
        std::any_of(lines.cbegin(), lines.cend(),
                    [](const QString &l) { return l.contains(QLatin1Char('\t')); });
    const bool hasComma =
        std::any_of(lines.cbegin(), lines.cend(),
                    [](const QString &l) { return l.contains(QLatin1Char(',')); });

    QChar delim;
    if (hasTab)
        delim = QLatin1Char('\t');
    else if (hasComma)
        delim = QLatin1Char(',');
    else
        return out;  // sin delimitador: no es una tabla

    // Con coma (no TAB), conservador: exige ≥2 líneas y que TODAS lleven coma, para
    // no convertir una frase con comas en una tabla de una fila.
    if (delim == QLatin1Char(',')) {
        if (lines.size() < 2)
            return out;
        for (const QString &l : lines)
            if (!l.contains(QLatin1Char(',')))
                return out;
    }

    QList<QStringList> rows;
    int maxCols = 0;
    for (const QString &l : lines) {
        QStringList cells = l.split(delim);
        for (QString &c : cells)
            c = c.trimmed();
        maxCols = std::max(maxCols, static_cast<int>(cells.size()));
        rows << cells;
    }
    if (maxCols < 2)
        return out;  // una sola columna: no es tabla

    for (QStringList &r : rows)  // rellena las filas cortas
        while (r.size() < maxCols)
            r << QString();

    out.rows = rows;
    out.ok = true;
    return out;
}

QString toMarkdownTable(const QList<QStringList> &rows)
{
    if (rows.isEmpty())
        return QString();

    const int cols = rows.constFirst().size();
    QString md = rowLine(rows.constFirst()) + QLatin1Char('\n');

    QStringList sep;
    sep.reserve(cols);
    for (int i = 0; i < cols; ++i)
        sep << QStringLiteral("---");
    md += rowLine(sep) + QLatin1Char('\n');

    for (int i = 1; i < rows.size(); ++i)
        md += rowLine(rows.at(i)) + QLatin1Char('\n');
    return md;
}

}  // namespace mdcsvtable
