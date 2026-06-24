/// \file
/// \brief Implementación del des-escapado del contenido de los code spans en línea.

#include "codespanfix.h"

#include <QStringList>

namespace mdcodespan {

namespace {

// Posición del primer carácter no blanco (tras la sangría) de la línea.
int firstNonSpace(const QString &line)
{
    int i = 0;
    while (i < line.size()
           && (line.at(i) == QLatin1Char(' ') || line.at(i) == QLatin1Char('\t')))
        ++i;
    return i;
}

// ¿La línea es la apertura/cierre de un fence de código vallado? (≥3 `` ` `` o `~`
// tras una sangría opcional). Devuelve el carácter del fence en `*ch`.
bool isFenceLine(const QString &line, QChar *ch)
{
    int i = firstNonSpace(line);
    if (line.size() - i < 3)
        return false;
    const QChar c = line.at(i);
    if (c != QLatin1Char('`') && c != QLatin1Char('~'))
        return false;
    int run = 0;
    while (i < line.size() && line.at(i) == c) {
        ++i;
        ++run;
    }
    if (run < 3)
        return false;
    if (ch)
        *ch = c;
    return true;
}

// Revierte el escape de Qt en el contenido de UN code span. Cada `\` que Qt emite
// introduce un escape de uno de los seis caracteres; un barrido de izquierda a
// derecha colapsa `\X` → `X` y trata bien `\\` (no reprocesa), preservando el
// contenido literal real del usuario.
QString unescapeContent(const QString &s)
{
    static const QString escaped = QStringLiteral("\\&<*[!");
    QString out;
    out.reserve(s.size());
    for (int i = 0; i < s.size(); ++i) {
        if (s.at(i) == QLatin1Char('\\') && i + 1 < s.size()
            && escaped.contains(s.at(i + 1))) {
            out += s.at(i + 1);
            ++i;
        } else {
            out += s.at(i);
        }
    }
    return out;
}

// Procesa los code spans en línea de UNA línea (que no es un fence). Un code span
// se abre con una secuencia de N backticks y se cierra con la SIGUIENTE secuencia
// de exactamente N backticks (regla de CommonMark).
QString processLine(const QString &line)
{
    QString out;
    const int n = line.size();
    int i = 0;
    while (i < n) {
        if (line.at(i) != QLatin1Char('`')) {
            out += line.at(i);
            ++i;
            continue;
        }
        // Secuencia de apertura.
        const int openStart = i;
        int run = 0;
        while (i < n && line.at(i) == QLatin1Char('`')) {
            ++i;
            ++run;
        }
        // Busca el cierre: una secuencia de EXACTAMENTE `run` backticks.
        int closeStart = -1;
        for (int j = i; j < n;) {
            if (line.at(j) != QLatin1Char('`')) {
                ++j;
                continue;
            }
            int k = j, r = 0;
            while (k < n && line.at(k) == QLatin1Char('`')) {
                ++k;
                ++r;
            }
            if (r == run) {
                closeStart = j;
                break;
            }
            j = k;  // secuencia de otro tamaño: seguir buscando tras ella
        }
        if (closeStart < 0) {
            // Sin cierre: la secuencia de apertura es texto literal.
            out += line.mid(openStart, run);
            continue;
        }
        const QString content = line.mid(i, closeStart - i);
        const QString ticks(run, QLatin1Char('`'));
        out += ticks + unescapeContent(content) + ticks;
        i = closeStart + run;
    }
    return out;
}

}  // namespace

QString unescapeInlineCode(const QString &markdown)
{
    const QStringList lines = markdown.split(QLatin1Char('\n'));
    QStringList out;
    out.reserve(lines.size());

    bool inFence = false;
    QChar fenceCh;
    for (const QString &line : lines) {
        QChar ch;
        if (inFence) {
            // Dentro de un fence todo es literal; solo nos interesa su cierre.
            if (isFenceLine(line, &ch) && ch == fenceCh)
                inFence = false;
            out << line;
            continue;
        }
        if (isFenceLine(line, &ch)) {
            inFence = true;
            fenceCh = ch;
            out << line;
            continue;
        }
        out << processLine(line);
    }
    return out.join(QLatin1Char('\n'));
}

}  // namespace mdcodespan
