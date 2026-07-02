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

// Sangría de la línea en columnas (el tabulador salta a múltiplo de 4). Sirve para
// detectar bloques de código indentados (≥4 columnas).
int leadingIndent(const QString &line)
{
    int col = 0;
    for (int i = 0; i < line.size(); ++i) {
        const QChar c = line.at(i);
        if (c == QLatin1Char(' '))
            ++col;
        else if (c == QLatin1Char('\t'))
            col += 4 - (col % 4);
        else
            break;
    }
    return col;
}

// Longitud del prefijo de cita (`>` con un espacio opcional, posiblemente anidado)
// al principio de la línea. Un fence o code span dentro de una cita conserva ese
// prefijo en cada línea; hay que ignorarlo para reconocer el fence.
int quotePrefixLen(const QString &line)
{
    int i = 0;
    while (true) {
        int j = i;
        while (j < line.size()
               && (line.at(j) == QLatin1Char(' ') || line.at(j) == QLatin1Char('\t')))
            ++j;
        if (j < line.size() && line.at(j) == QLatin1Char('>')) {
            ++j;
            if (j < line.size() && line.at(j) == QLatin1Char(' '))
                ++j;  // un único espacio tras `>`
            i = j;
        } else {
            break;
        }
    }
    return i;
}

// Análisis de una posible línea de fence (ya sin el prefijo de cita).
struct Fence {
    bool ok = false;       // ≥3 `` ` `` o `~` tras una sangría opcional
    QChar ch;              // carácter del fence
    int len = 0;           // longitud del run de apertura/cierre
    bool bare = false;     // tras el run solo hay espacios (requisito de un CIERRE)
    bool backtickAfter = false;  // hay backticks tras el run (invalida un fence de `` ` ``)
};

Fence fenceOf(const QString &line)
{
    Fence f;
    int i = firstNonSpace(line);
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

// ¿Abre un fence vallado? Un fence de backticks NO puede llevar más backticks en su
// info string (CommonMark): si los hay, la línea es en realidad código en línea que
// empieza por `` ``` ``, no un fence.
bool opensFence(const Fence &f)
{
    return f.ok && !(f.ch == QLatin1Char('`') && f.backtickAfter);
}

// ¿Cierra el fence abierto (mismo carácter, run ≥ el de apertura y sin info string)?
bool closesFence(const Fence &f, QChar openCh, int openLen)
{
    return f.ok && f.ch == openCh && f.len >= openLen && f.bare;
}

// Revierte el escape de Qt en el contenido de UN code span. Cada `\` que Qt emite
// introduce un escape de uno de estos caracteres; un barrido de izquierda a
// derecha colapsa `\X` → `X` y trata bien `\\` (no reprocesa), preservando el
// contenido literal real del usuario. Incluye el backtick, que Qt también escapa
// dentro de los spans multi-backtick (`` \` ``); sin él, la barra se acumulaba en
// cada guardado.
QString unescapeContent(const QString &s)
{
    static const QString escaped = QStringLiteral("\\&<*[!`");
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
    int fenceLen = 0;
    bool inIndentedCode = false;
    bool prevBlank = true;  // el inicio del documento cuenta como «tras un blanco»

    for (const QString &line : lines) {
        const QString body = line.mid(quotePrefixLen(line));  // sin el prefijo de cita
        const bool blank = body.trimmed().isEmpty();

        if (inFence) {
            // Dentro de un fence todo es literal; solo nos interesa su cierre.
            if (closesFence(fenceOf(body), fenceCh, fenceLen))
                inFence = false;
            out << line;
            prevBlank = blank;
            continue;
        }

        // Bloque de código indentado (≥4 columnas tras un blanco): contenido
        // verbatim. Qt NO escapa dentro de él, así que des-escapar borraría
        // backslashes legítimos del usuario. Continúa sobre líneas en blanco y
        // termina en la primera línea no blanca con <4 de sangría.
        const int indent = leadingIndent(body);
        if (inIndentedCode) {
            if (blank || indent >= 4) {
                out << line;
                prevBlank = blank;
                continue;
            }
            inIndentedCode = false;  // fin del bloque: sigue el tratamiento normal
        } else if (!blank && indent >= 4 && prevBlank) {
            inIndentedCode = true;
            out << line;
            prevBlank = blank;
            continue;
        }

        const Fence f = fenceOf(body);
        if (opensFence(f)) {
            inFence = true;
            fenceCh = f.ch;
            fenceLen = f.len;
            out << line;
            prevBlank = blank;
            continue;
        }
        // La cita (`>`, espacios) no lleva backticks, así que procesar la línea
        // entera es seguro y conserva el prefijo tal cual.
        out << processLine(line);
        prevBlank = blank;
    }
    return out.join(QLatin1Char('\n'));
}

}  // namespace mdcodespan
