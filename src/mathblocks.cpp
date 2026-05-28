#include "mathblocks.h"

#include <QChar>
#include <QHash>
#include <QRegularExpression>
#include <QStringList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include <algorithm>

namespace mdmath {

namespace {

// ¿La línea es una valla de bloque de código ``` o ~~~ (al menos tres seguidas
// al inicio, opcionalmente con espacios)?
bool isFenceLine(const QString &line)
{
    const QString s = line.trimmed();
    if (s.size() < 3)
        return false;
    const QChar c = s.at(0);
    if (c != QLatin1Char('`') && c != QLatin1Char('~'))
        return false;
    int n = 0;
    while (n < s.size() && s.at(n) == c)
        ++n;
    return n >= 3;
}

// Devuelve los rangos [a,b) de una línea ocupados por código en línea
// (`...`/``...``…). El número de comillas invertidas que abre debe coincidir
// con el que cierra. Lo usamos para no detectar `$` dentro de código.
QList<QPair<int, int>> inlineCodeRanges(const QString &line)
{
    QList<QPair<int, int>> ranges;
    int i = 0;
    const int n = line.size();
    while (i < n) {
        if (line.at(i) != QLatin1Char('`')) {
            ++i;
            continue;
        }
        int run = 0;
        const int start = i;
        while (i < n && line.at(i) == QLatin1Char('`')) {
            ++run;
            ++i;
        }
        // Buscamos la misma longitud `run` de comillas como cierre.
        int j = i;
        while (j < n) {
            if (line.at(j) == QLatin1Char('`')) {
                int closing = 0;
                const int cstart = j;
                while (j < n && line.at(j) == QLatin1Char('`')) {
                    ++closing;
                    ++j;
                }
                if (closing == run) {
                    ranges.append({start, j});
                    break;
                }
                Q_UNUSED(cstart);
            } else {
                ++j;
            }
        }
        if (j >= n) {
            // No hubo cierre: nada que proteger más allá.
            break;
        }
    }
    return ranges;
}

bool inRange(int pos, const QList<QPair<int, int>> &ranges)
{
    for (const auto &r : ranges)
        if (pos >= r.first && pos < r.second)
            return true;
    return false;
}

// ¿El `$` en la posición `i` está escapado con `\$` (precedido por un número
// impar de barras invertidas)?
bool isEscaped(const QString &s, int i)
{
    int back = 0;
    while (i - 1 - back >= 0 && s.at(i - 1 - back) == QLatin1Char('\\'))
        ++back;
    return (back % 2) == 1;
}

// Busca una fórmula que abre en `line[start]` (que es '$'). Devuelve el span
// (con `start` en absoluto sumando `lineOffset`) y true, o false si no
// reconoce un cierre razonable. `codeRanges` son los rangos de código en línea
// que no se deben cruzar.
bool matchAt(const QString &line, int start, int lineOffset,
             const QList<QPair<int, int>> &codeRanges, Span &out)
{
    const int n = line.size();
    const bool isBlock = (start + 1 < n && line.at(start + 1) == QLatin1Char('$'));
    const int delimLen = isBlock ? 2 : 1;
    const int contentStart = start + delimLen;

    // Inline ($...$): no se permite que el primer carácter sea un espacio (regla
    // habitual de Pandoc, evita falsos positivos con precios "$5 y $10").
    if (!isBlock) {
        if (contentStart >= n)
            return false;
        const QChar c = line.at(contentStart);
        if (c.isSpace())
            return false;
    }

    // Busca un cierre adecuado.
    for (int j = contentStart; j + delimLen - 1 < n; ++j) {
        if (line.at(j) != QLatin1Char('$'))
            continue;
        if (isEscaped(line, j))
            continue;
        if (inRange(j, codeRanges))
            continue;
        if (isBlock) {
            if (j + 1 >= n || line.at(j + 1) != QLatin1Char('$'))
                continue;
        } else {
            // Inline: el cierre no debe ir precedido por espacio, y no
            // puede ser parte de un $$ (evita confundir $...$ con $$..$$).
            if (j > contentStart && line.at(j - 1).isSpace())
                continue;
            if (j + 1 < n && line.at(j + 1) == QLatin1Char('$'))
                continue;
        }
        const int contentLen = j - contentStart;
        if (contentLen <= 0)
            return false;  // $$ vacío o $$ adyacentes no son fórmulas

        out.start = lineOffset + start;
        out.length = (j + delimLen) - start;
        out.block = isBlock;
        out.content = line.mid(contentStart, contentLen);
        return true;
    }
    return false;
}

} // namespace

QList<Span> findMath(const QString &text)
{
    QList<Span> spans;
    const QStringList lines = text.split(QLatin1Char('\n'));
    bool insideFence = false;
    int offset = 0;
    // Si >= 0, hay un `$$...$$` abierto cuya apertura está en la posición
    // absoluta `openStart`. El cierre se busca en líneas sucesivas; el
    // contenido entre medias preserva sus saltos de línea.
    int openStart = -1;
    for (const QString &line : lines) {
        if (openStart >= 0) {
            const QList<QPair<int, int>> ranges = inlineCodeRanges(line);
            int closeAt = -1;
            for (int j = 0; j + 1 < line.size(); ++j) {
                if (line.at(j) != QLatin1Char('$') || line.at(j + 1) != QLatin1Char('$'))
                    continue;
                if (isEscaped(line, j) || inRange(j, ranges))
                    continue;
                closeAt = j;
                break;
            }
            if (closeAt >= 0) {
                Span s;
                s.start = openStart;
                s.length = (offset + closeAt + 2) - openStart;
                s.block = true;
                s.content = text.mid(openStart + 2, s.length - 4);
                spans.append(s);
                openStart = -1;
                // El resto de la línea tras el cierre no se busca de nuevo
                // (caso típico: `$$` en su propia línea). Es una simplificación
                // consciente.
            }
            offset += line.size() + 1;
            continue;
        }
        if (isFenceLine(line)) {
            insideFence = !insideFence;
            offset += line.size() + 1;
            continue;
        }
        if (insideFence) {
            offset += line.size() + 1;
            continue;
        }

        const QList<QPair<int, int>> codeRanges = inlineCodeRanges(line);
        int i = 0;
        while (i < line.size()) {
            if (line.at(i) != QLatin1Char('$') || isEscaped(line, i) || inRange(i, codeRanges)) {
                ++i;
                continue;
            }
            Span s;
            if (matchAt(line, i, offset, codeRanges, s)) {
                spans.append(s);
                // s.start es absoluto (lineOffset + i): saltamos justo detrás
                // del cierre, en coordenadas de línea.
                i = (s.start - offset) + s.length;
                continue;
            }
            // matchAt no encontró cierre en la misma línea. Si es `$$`, lo
            // tratamos como apertura de un bloque multilínea cuyo cierre
            // buscamos en las líneas siguientes.
            if (i + 1 < line.size() && line.at(i + 1) == QLatin1Char('$')) {
                openStart = offset + i;
                break;
            }
            ++i;
        }
        offset += line.size() + 1;
    }
    // Si `openStart` quedó abierto, descartamos: el bloque nunca cerró.
    return spans;
}

// Carácter PUA placeholder para los saltos de línea dentro del contenido de
// una fórmula multilínea. Inline-code de Markdown no puede llevar `\n`, así
// que sustituimos los saltos por este char (que Qt deja como texto opaco) y
// los restauramos al extraer el TeX en renderMathInDocument.
static constexpr QChar kNewlinePlaceholder(0xF8FD);

QString protectMath(const QString &markdown)
{
    const QList<Span> spans = findMath(markdown);
    if (spans.isEmpty())
        return markdown;
    QString out;
    out.reserve(markdown.size() + spans.size() * 4);
    int cur = 0;
    for (const Span &s : spans) {
        out += QStringView{markdown}.mid(cur, s.start - cur);
        const QString delim = s.block ? QStringLiteral("$$") : QStringLiteral("$");
        QString content = s.content;
        // Codifica saltos para que el inline-code resultante quepa en una
        // sola línea de Markdown (requisito para que setMarkdown lo trate
        // como código en línea).
        content.replace(QLatin1Char('\n'), kNewlinePlaceholder);
        out += QLatin1String("``");
        out += delim;
        out += content;
        out += delim;
        out += QLatin1String("``");
        cur = s.start + s.length;
    }
    out += QStringView{markdown}.mid(cur);
    return out;
}

QString unprotectMath(const QString &markdown)
{
    // Patrón: `…` o ``…`` (ya no más, suficiente). El contenido debe empezar y
    // acabar por `$` (uno o dos). El cuerpo puede tener espacios opcionales
    // entre la valla y el primer `$` (Qt los puede introducir si el contenido
    // empezara/acabara por `).
    static const QRegularExpression re(
        QStringLiteral("(`{1,2})[ ]?(\\${1,2})([^`]*?)(\\${1,2})[ ]?\\1"));
    QString out;
    out.reserve(markdown.size());
    int last = 0;
    QRegularExpressionMatchIterator it = re.globalMatch(markdown);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const QString open = m.captured(2);
        const QString close = m.captured(4);
        if (open != close) {
            // No es una fórmula bien formada: déjala como código.
            continue;
        }
        out += QStringView{markdown}.mid(last, m.capturedStart() - last);
        out += open;
        out += m.captured(3);
        out += close;
        last = m.capturedEnd();
    }
    out += QStringView{markdown}.mid(last);
    return out;
}

// ---------------------------------------------------------------------------
// TeX → Unicode (aproximación)
// ---------------------------------------------------------------------------

namespace {

// Comando TeX → carácter Unicode (uno solo). Las letras griegas y los símbolos
// matemáticos más usados. La tabla está acotada a propósito: cuanto más cubre,
// más fácil es producir un Unicode confuso cuando hay matices (\Pi vs \prod).
const QHash<QString, QChar> &singleCharCommands()
{
    static const QHash<QString, QChar> m = {
        // Griego minúsculas
        {QStringLiteral("alpha"),      QChar(0x03B1)},
        {QStringLiteral("beta"),       QChar(0x03B2)},
        {QStringLiteral("gamma"),      QChar(0x03B3)},
        {QStringLiteral("delta"),      QChar(0x03B4)},
        {QStringLiteral("epsilon"),    QChar(0x03B5)},
        {QStringLiteral("varepsilon"), QChar(0x03B5)},
        {QStringLiteral("zeta"),       QChar(0x03B6)},
        {QStringLiteral("eta"),        QChar(0x03B7)},
        {QStringLiteral("theta"),      QChar(0x03B8)},
        {QStringLiteral("vartheta"),   QChar(0x03D1)},
        {QStringLiteral("iota"),       QChar(0x03B9)},
        {QStringLiteral("kappa"),      QChar(0x03BA)},
        {QStringLiteral("lambda"),     QChar(0x03BB)},
        {QStringLiteral("mu"),         QChar(0x03BC)},
        {QStringLiteral("nu"),         QChar(0x03BD)},
        {QStringLiteral("xi"),         QChar(0x03BE)},
        {QStringLiteral("pi"),         QChar(0x03C0)},
        {QStringLiteral("varpi"),      QChar(0x03D6)},
        {QStringLiteral("rho"),        QChar(0x03C1)},
        {QStringLiteral("varrho"),     QChar(0x03F1)},
        {QStringLiteral("sigma"),      QChar(0x03C3)},
        {QStringLiteral("varsigma"),   QChar(0x03C2)},
        {QStringLiteral("tau"),        QChar(0x03C4)},
        {QStringLiteral("upsilon"),    QChar(0x03C5)},
        {QStringLiteral("phi"),        QChar(0x03C6)},
        {QStringLiteral("varphi"),     QChar(0x03D5)},
        {QStringLiteral("chi"),        QChar(0x03C7)},
        {QStringLiteral("psi"),        QChar(0x03C8)},
        {QStringLiteral("omega"),      QChar(0x03C9)},
        // Griego mayúsculas
        {QStringLiteral("Gamma"),   QChar(0x0393)},
        {QStringLiteral("Delta"),   QChar(0x0394)},
        {QStringLiteral("Theta"),   QChar(0x0398)},
        {QStringLiteral("Lambda"),  QChar(0x039B)},
        {QStringLiteral("Xi"),      QChar(0x039E)},
        {QStringLiteral("Pi"),      QChar(0x03A0)},
        {QStringLiteral("Sigma"),   QChar(0x03A3)},
        {QStringLiteral("Upsilon"), QChar(0x03A5)},
        {QStringLiteral("Phi"),     QChar(0x03A6)},
        {QStringLiteral("Psi"),     QChar(0x03A8)},
        {QStringLiteral("Omega"),   QChar(0x03A9)},
        // Operadores y símbolos
        {QStringLiteral("pm"),     QChar(0x00B1)},
        {QStringLiteral("mp"),     QChar(0x2213)},
        {QStringLiteral("times"),  QChar(0x00D7)},
        {QStringLiteral("div"),    QChar(0x00F7)},
        {QStringLiteral("cdot"),   QChar(0x00B7)},
        {QStringLiteral("ast"),    QChar(0x2217)},
        {QStringLiteral("star"),   QChar(0x22C6)},
        {QStringLiteral("circ"),   QChar(0x2218)},
        {QStringLiteral("bullet"), QChar(0x2022)},
        {QStringLiteral("oplus"),  QChar(0x2295)},
        {QStringLiteral("ominus"), QChar(0x2296)},
        {QStringLiteral("otimes"), QChar(0x2297)},
        {QStringLiteral("oslash"), QChar(0x2298)},
        // Relaciones
        {QStringLiteral("leq"),     QChar(0x2264)},
        {QStringLiteral("le"),      QChar(0x2264)},
        {QStringLiteral("geq"),     QChar(0x2265)},
        {QStringLiteral("ge"),      QChar(0x2265)},
        {QStringLiteral("neq"),     QChar(0x2260)},
        {QStringLiteral("ne"),      QChar(0x2260)},
        {QStringLiteral("approx"),  QChar(0x2248)},
        {QStringLiteral("equiv"),   QChar(0x2261)},
        {QStringLiteral("sim"),     QChar(0x223C)},
        {QStringLiteral("simeq"),   QChar(0x2243)},
        {QStringLiteral("cong"),    QChar(0x2245)},
        {QStringLiteral("propto"),  QChar(0x221D)},
        {QStringLiteral("ll"),      QChar(0x226A)},
        {QStringLiteral("gg"),      QChar(0x226B)},
        // Grandes operadores
        {QStringLiteral("sum"),    QChar(0x2211)},
        {QStringLiteral("prod"),   QChar(0x220F)},
        {QStringLiteral("coprod"), QChar(0x2210)},
        {QStringLiteral("int"),    QChar(0x222B)},
        {QStringLiteral("iint"),   QChar(0x222C)},
        {QStringLiteral("iiint"),  QChar(0x222D)},
        {QStringLiteral("oint"),   QChar(0x222E)},
        // Conjuntos
        {QStringLiteral("in"),         QChar(0x2208)},
        {QStringLiteral("notin"),      QChar(0x2209)},
        {QStringLiteral("ni"),         QChar(0x220B)},
        {QStringLiteral("subset"),     QChar(0x2282)},
        {QStringLiteral("supset"),     QChar(0x2283)},
        {QStringLiteral("subseteq"),   QChar(0x2286)},
        {QStringLiteral("supseteq"),   QChar(0x2287)},
        {QStringLiteral("cup"),        QChar(0x222A)},
        {QStringLiteral("cap"),        QChar(0x2229)},
        {QStringLiteral("emptyset"),   QChar(0x2205)},
        {QStringLiteral("varnothing"), QChar(0x2205)},
        // Lógica y conjuntos numéricos
        {QStringLiteral("forall"),  QChar(0x2200)},
        {QStringLiteral("exists"),  QChar(0x2203)},
        {QStringLiteral("nexists"), QChar(0x2204)},
        {QStringLiteral("neg"),     QChar(0x00AC)},
        {QStringLiteral("lnot"),    QChar(0x00AC)},
        {QStringLiteral("land"),    QChar(0x2227)},
        {QStringLiteral("wedge"),   QChar(0x2227)},
        {QStringLiteral("lor"),     QChar(0x2228)},
        {QStringLiteral("vee"),     QChar(0x2228)},
        // Flechas
        {QStringLiteral("to"),                QChar(0x2192)},
        {QStringLiteral("rightarrow"),        QChar(0x2192)},
        {QStringLiteral("leftarrow"),         QChar(0x2190)},
        {QStringLiteral("gets"),              QChar(0x2190)},
        {QStringLiteral("leftrightarrow"),    QChar(0x2194)},
        {QStringLiteral("Rightarrow"),        QChar(0x21D2)},
        {QStringLiteral("Leftarrow"),         QChar(0x21D0)},
        {QStringLiteral("Leftrightarrow"),    QChar(0x21D4)},
        {QStringLiteral("iff"),               QChar(0x21D4)},
        {QStringLiteral("mapsto"),            QChar(0x21A6)},
        {QStringLiteral("uparrow"),           QChar(0x2191)},
        {QStringLiteral("downarrow"),         QChar(0x2193)},
        // Diversos
        {QStringLiteral("infty"),   QChar(0x221E)},
        {QStringLiteral("partial"), QChar(0x2202)},
        {QStringLiteral("nabla"),   QChar(0x2207)},
        {QStringLiteral("hbar"),    QChar(0x210F)},
        {QStringLiteral("ell"),     QChar(0x2113)},
        {QStringLiteral("Re"),      QChar(0x211C)},
        {QStringLiteral("Im"),      QChar(0x2111)},
        {QStringLiteral("aleph"),   QChar(0x2135)},
        {QStringLiteral("prime"),   QChar(0x2032)},
        {QStringLiteral("dots"),    QChar(0x2026)},
        {QStringLiteral("ldots"),   QChar(0x2026)},
        {QStringLiteral("cdots"),   QChar(0x22EF)},
        {QStringLiteral("vdots"),   QChar(0x22EE)},
        {QStringLiteral("ddots"),   QChar(0x22F1)},
        // Conjuntos numéricos (Blackboard bold) — útiles aunque no son
        // mayúsculas latinas estándar.
        {QStringLiteral("mathbb{R}"), QChar(0x211D)},
        {QStringLiteral("mathbb{N}"), QChar(0x2115)},
        {QStringLiteral("mathbb{Z}"), QChar(0x2124)},
        {QStringLiteral("mathbb{Q}"), QChar(0x211A)},
        {QStringLiteral("mathbb{C}"), QChar(0x2102)},
        {QStringLiteral("mathbb{P}"), QChar(0x2119)},
    };
    return m;
}

// Comandos TeX que se traducen a más de un carácter Unicode (raros).
const QHash<QString, QString> &multiCharCommands()
{
    static const QHash<QString, QString> m = {
        {QStringLiteral("sqrt"), QString(QChar(0x221A))},   // se trata aparte si lleva arg
    };
    return m;
}

// Caracteres que tienen forma superíndice en Unicode.
QChar toSuperscript(QChar c)
{
    static const QHash<QChar, QChar> m = {
        {QLatin1Char('0'), QChar(0x2070)}, {QLatin1Char('1'), QChar(0x00B9)},
        {QLatin1Char('2'), QChar(0x00B2)}, {QLatin1Char('3'), QChar(0x00B3)},
        {QLatin1Char('4'), QChar(0x2074)}, {QLatin1Char('5'), QChar(0x2075)},
        {QLatin1Char('6'), QChar(0x2076)}, {QLatin1Char('7'), QChar(0x2077)},
        {QLatin1Char('8'), QChar(0x2078)}, {QLatin1Char('9'), QChar(0x2079)},
        {QLatin1Char('+'), QChar(0x207A)}, {QLatin1Char('-'), QChar(0x207B)},
        {QLatin1Char('='), QChar(0x207C)}, {QLatin1Char('('), QChar(0x207D)},
        {QLatin1Char(')'), QChar(0x207E)}, {QLatin1Char('n'), QChar(0x207F)},
        {QLatin1Char('i'), QChar(0x2071)},
        {QLatin1Char('a'), QChar(0x1D43)}, {QLatin1Char('b'), QChar(0x1D47)},
        {QLatin1Char('c'), QChar(0x1D9C)}, {QLatin1Char('d'), QChar(0x1D48)},
        {QLatin1Char('e'), QChar(0x1D49)}, {QLatin1Char('f'), QChar(0x1DA0)},
        {QLatin1Char('g'), QChar(0x1D4D)}, {QLatin1Char('h'), QChar(0x02B0)},
        {QLatin1Char('j'), QChar(0x02B2)}, {QLatin1Char('k'), QChar(0x1D4F)},
        {QLatin1Char('l'), QChar(0x02E1)}, {QLatin1Char('m'), QChar(0x1D50)},
        {QLatin1Char('o'), QChar(0x1D52)}, {QLatin1Char('p'), QChar(0x1D56)},
        {QLatin1Char('r'), QChar(0x02B3)}, {QLatin1Char('s'), QChar(0x02E2)},
        {QLatin1Char('t'), QChar(0x1D57)}, {QLatin1Char('u'), QChar(0x1D58)},
        {QLatin1Char('v'), QChar(0x1D5B)}, {QLatin1Char('w'), QChar(0x02B7)},
        {QLatin1Char('x'), QChar(0x02E3)}, {QLatin1Char('y'), QChar(0x02B8)},
        {QLatin1Char('z'), QChar(0x1DBB)},
    };
    const auto it = m.constFind(c);
    return it != m.cend() ? it.value() : QChar();
}

// Caracteres con forma subíndice en Unicode (bastantes menos que los súper).
QChar toSubscript(QChar c)
{
    static const QHash<QChar, QChar> m = {
        {QLatin1Char('0'), QChar(0x2080)}, {QLatin1Char('1'), QChar(0x2081)},
        {QLatin1Char('2'), QChar(0x2082)}, {QLatin1Char('3'), QChar(0x2083)},
        {QLatin1Char('4'), QChar(0x2084)}, {QLatin1Char('5'), QChar(0x2085)},
        {QLatin1Char('6'), QChar(0x2086)}, {QLatin1Char('7'), QChar(0x2087)},
        {QLatin1Char('8'), QChar(0x2088)}, {QLatin1Char('9'), QChar(0x2089)},
        {QLatin1Char('+'), QChar(0x208A)}, {QLatin1Char('-'), QChar(0x208B)},
        {QLatin1Char('='), QChar(0x208C)}, {QLatin1Char('('), QChar(0x208D)},
        {QLatin1Char(')'), QChar(0x208E)},
        {QLatin1Char('a'), QChar(0x2090)}, {QLatin1Char('e'), QChar(0x2091)},
        {QLatin1Char('h'), QChar(0x2095)}, {QLatin1Char('i'), QChar(0x1D62)},
        {QLatin1Char('j'), QChar(0x2C7C)}, {QLatin1Char('k'), QChar(0x2096)},
        {QLatin1Char('l'), QChar(0x2097)}, {QLatin1Char('m'), QChar(0x2098)},
        {QLatin1Char('n'), QChar(0x2099)}, {QLatin1Char('o'), QChar(0x2092)},
        {QLatin1Char('p'), QChar(0x209A)}, {QLatin1Char('r'), QChar(0x1D63)},
        {QLatin1Char('s'), QChar(0x209B)}, {QLatin1Char('t'), QChar(0x209C)},
        {QLatin1Char('u'), QChar(0x1D64)}, {QLatin1Char('v'), QChar(0x1D65)},
        {QLatin1Char('x'), QChar(0x2093)},
    };
    const auto it = m.constFind(c);
    return it != m.cend() ? it.value() : QChar();
}

// Lee un identificador TeX a partir de `i` (que apunta justo después de `\`):
// letras consecutivas. Avanza `i`.
QString readCommand(const QString &s, int &i)
{
    const int start = i;
    while (i < s.size() && s.at(i).isLetter())
        ++i;
    return s.mid(start, i - start);
}

// Lee un grupo `{...}` desde `i` (que apunta a `{`), devolviendo el contenido
// sin las llaves y avanzando `i` justo tras la `}` de cierre. Si no hay
// cierre, devuelve el resto.
QString readGroup(const QString &s, int &i)
{
    if (i >= s.size() || s.at(i) != QLatin1Char('{'))
        return QString();
    ++i;  // consume '{'
    const int start = i;
    int depth = 1;
    while (i < s.size() && depth > 0) {
        const QChar c = s.at(i);
        if (c == QLatin1Char('{'))
            ++depth;
        else if (c == QLatin1Char('}'))
            --depth;
        if (depth > 0)
            ++i;
    }
    const QString content = s.mid(start, i - start);
    if (i < s.size() && s.at(i) == QLatin1Char('}'))
        ++i;
    return content;
}

// Aplana un texto en forma de script (super/sub) usando el repertorio Unicode
// (`x²`, `Hᵢ`). Si algún carácter no tiene forma → devuelve el arg precedido de
// `^`/`_`, que es lo más legible cuando no se dispone de formato rico.
QString flattenScriptToUnicode(const QString &arg, bool sup)
{
    QString out;
    out.reserve(arg.size());
    for (const QChar c : arg) {
        const QChar mapped = sup ? toSuperscript(c) : toSubscript(c);
        if (mapped.isNull())
            return QLatin1Char(sup ? '^' : '_') + arg;
        out += mapped;
    }
    return out;
}

// Convierte un comando TeX (sin barra invertida) a Unicode si está en la
// tabla; si no, devuelve `\nombre` para que el usuario vea el error.
QString commandToUnicode(const QString &cmd)
{
    const auto it = singleCharCommands().constFind(cmd);
    if (it != singleCharCommands().cend())
        return QString(it.value());
    const auto itm = multiCharCommands().constFind(cmd);
    if (itm != multiCharCommands().cend())
        return itm.value();
    return QLatin1Char('\\') + cmd;
}

} // namespace

QList<MathRun> renderTexAsRuns(const QString &tex, const QTextCharFormat &baseFmt)
{
    QList<MathRun> runs;
    QString buffer;
    auto flush = [&] {
        if (!buffer.isEmpty()) {
            runs.append({buffer, baseFmt});
            buffer.clear();
        }
    };
    auto scriptFmt = [&](bool sup) {
        QTextCharFormat f = baseFmt;
        f.setVerticalAlignment(sup ? QTextCharFormat::AlignSuperScript
                                   : QTextCharFormat::AlignSubScript);
        return f;
    };

    int i = 0;
    const int n = tex.size();
    while (i < n) {
        const QChar c = tex.at(i);

        if (c == QLatin1Char('\\')) {
            ++i;
            if (i >= n) { buffer += QLatin1Char('\\'); break; }
            if (tex.at(i) == QLatin1Char('\\')) { buffer += QLatin1Char(' '); ++i; continue; }
            if (!tex.at(i).isLetter()) { buffer += tex.at(i); ++i; continue; }
            const QString cmd = readCommand(tex, i);

            int after = i;
            while (after < n && tex.at(after) == QLatin1Char(' ')) ++after;

            // \frac: numerador y denominador como sub-runs aparte para que sus
            // super/subíndices internos también se rendericen elevados/bajados.
            if (cmd == QLatin1String("frac") && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                const QString num = readGroup(tex, i);
                while (i < n && tex.at(i) == QLatin1Char(' ')) ++i;
                QString den;
                if (i < n && tex.at(i) == QLatin1Char('{'))
                    den = readGroup(tex, i);
                // Caso simple: un solo char en num y den → usa el FRACTION
                // SLASH (U+2044), que en fuentes con soporte se pinta como
                // fracción tipográfica entre los dos caracteres.
                if (num.size() == 1 && den.size() == 1
                    && !num.at(0).isSpace() && !den.at(0).isSpace()) {
                    buffer += num + QChar(0x2044) + den;
                } else {
                    buffer += QLatin1Char('(');
                    flush();
                    runs.append(renderTexAsRuns(num, baseFmt));
                    buffer += QStringLiteral(")/(");
                    flush();
                    runs.append(renderTexAsRuns(den, baseFmt));
                    buffer += QLatin1Char(')');
                }
                continue;
            }

            // \sqrt: emite "√" y el argumento (recursivo, para que `√{x^2}` lo
            // muestre con el 2 elevado).
            if (cmd == QLatin1String("sqrt")) {
                buffer += QChar(0x221A);
                if (after < n && tex.at(after) == QLatin1Char('{')) {
                    i = after;
                    const QString arg = readGroup(tex, i);
                    buffer += QLatin1Char('(');
                    flush();
                    runs.append(renderTexAsRuns(arg, baseFmt));
                    buffer += QLatin1Char(')');
                }
                continue;
            }

            // \mathbb{X} y similares: el comando completo (con su arg) puede
            // estar en la tabla; si lo está, ese símbolo va al buffer normal.
            if (after < n && tex.at(after) == QLatin1Char('{')) {
                int probe = after;
                const QString arg = readGroup(tex, probe);
                const QString combined = cmd + QLatin1Char('{') + arg + QLatin1Char('}');
                const auto it = singleCharCommands().constFind(combined);
                if (it != singleCharCommands().cend()) {
                    buffer += it.value();
                    i = probe;
                    continue;
                }
            }

            buffer += commandToUnicode(cmd);
            continue;
        }

        if (c == QLatin1Char('^') || c == QLatin1Char('_')) {
            const bool sup = (c == QLatin1Char('^'));
            ++i;
            if (i >= n) { buffer += c; break; }
            // Lee el argumento del script y lo aplana a Unicode (texToUnicode
            // recursivo) para envolverlo en UN solo run con vertical-align.
            // Aplanar (no recurseAsRuns aquí) es lo que mantiene los scripts
            // anidados manejables: el nivel exterior es Qt vertical-align y los
            // interiores usan la versión Unicode super/sub.
            QString arg;
            if (tex.at(i) == QLatin1Char('{')) {
                arg = texToUnicode(readGroup(tex, i));
            } else if (tex.at(i) == QLatin1Char('\\')) {
                ++i;
                if (i < n && tex.at(i).isLetter()) {
                    arg = commandToUnicode(readCommand(tex, i));
                } else if (i < n) {
                    arg = QString(tex.at(i));
                    ++i;
                } else {
                    arg = QStringLiteral("\\");
                }
            } else {
                arg = QString(tex.at(i));
                ++i;
            }
            flush();
            runs.append({arg, scriptFmt(sup)});
            continue;
        }

        if (c == QLatin1Char('{') || c == QLatin1Char('}')) { ++i; continue; }

        buffer += c;
        ++i;
    }
    flush();
    return runs;
}

QString texToUnicode(const QString &tex)
{
    // Aplanado a una sola cadena: usa renderTexAsRuns y traduce cada run con
    // vertical-align a su forma Unicode super/sub (o el fallback `^x`/`_x` si
    // algún carácter no tiene representación elevada).
    QTextCharFormat empty;
    const QList<MathRun> runs = renderTexAsRuns(tex, empty);
    QString out;
    out.reserve(tex.size());
    for (const MathRun &r : runs) {
        const auto va = r.fmt.verticalAlignment();
        if (va == QTextCharFormat::AlignSuperScript)
            out += flattenScriptToUnicode(r.text, true);
        else if (va == QTextCharFormat::AlignSubScript)
            out += flattenScriptToUnicode(r.text, false);
        else
            out += r.text;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Render dentro del QTextDocument (Nivel 1)
// ---------------------------------------------------------------------------

QTextCharFormat mathCharFormat(const QString &tex, bool block)
{
    QTextCharFormat fmt;
    fmt.setProperty(IsMathProperty, true);
    fmt.setProperty(MathTexProperty, tex);
    fmt.setProperty(MathBlockProperty, block);
    // Cursiva como señal visual de que es una fórmula (las matemáticas se
    // componen en cursiva por convención tipográfica). Sin monoespaciado: el
    // texto visible es Unicode, no inline-code.
    fmt.setFontItalic(true);
    return fmt;
}

namespace {

// Helper: ¿el texto de un fragmento de inline-code es una fórmula con la
// forma `$tex$` o `$$tex$$`? Devuelve también el TeX original (con los
// saltos de línea restaurados desde el placeholder PUA).
bool inlineCodeIsMath(const QString &text, QString &tex, bool &isBlock)
{
    if (text.size() < 3 || !text.startsWith(QLatin1Char('$')) || !text.endsWith(QLatin1Char('$')))
        return false;
    if (text.startsWith(QStringLiteral("$$")) && text.endsWith(QStringLiteral("$$"))) {
        if (text.size() < 5) return false;
        isBlock = true;
        tex = text.mid(2, text.size() - 4);
    } else {
        isBlock = false;
        tex = text.mid(1, text.size() - 2);
    }
    tex.replace(kNewlinePlaceholder, QLatin1Char('\n'));
    return !tex.isEmpty();
}

// Una sustitución pendiente sobre el documento. La lista se aplica en orden
// descendente por `start` para no invalidar las posiciones de las siguientes.
// Un Repl puede emitir varios runs consecutivos (una fórmula con super/sub se
// rinde como varios fragmentos contiguos compartiendo MathTex).
struct Repl {
    int start;
    int end;
    QList<MathRun> runs;
};

void applyReplacements(QTextDocument *doc, QList<Repl> repls)
{
    std::sort(repls.begin(), repls.end(),
              [](const Repl &a, const Repl &b) { return a.start > b.start; });
    QTextCursor c(doc);
    c.beginEditBlock();
    for (const Repl &r : repls) {
        c.setPosition(r.start);
        c.setPosition(r.end, QTextCursor::KeepAnchor);
        c.removeSelectedText();
        for (const MathRun &run : r.runs)
            c.insertText(run.text, run.fmt);
    }
    c.endEditBlock();
}

// Recorre los grupos de fragmentos consecutivos con IsMathProperty que
// comparten MathTex. `callback` recibe (start, end, tex, isBlock) en
// coordenadas absolutas del documento.
template <typename Fn>
void forEachMathGroup(QTextDocument *doc, Fn callback)
{
    int gStart = -1;
    int gEnd = -1;
    QString gTex;
    bool gBlock = false;
    auto flush = [&] {
        if (gStart >= 0) {
            callback(gStart, gEnd, gTex, gBlock);
            gStart = -1;
        }
    };
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid())
                continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.boolProperty(IsMathProperty)) {
                flush();
                continue;
            }
            const QString tex = cf.property(MathTexProperty).toString();
            const bool isBlock = cf.boolProperty(MathBlockProperty);
            const int fs = frag.position();
            const int fe = fs + frag.length();
            // Agrupa si es contiguo al grupo abierto y comparte tex/isBlock.
            if (gStart >= 0 && gEnd == fs && gTex == tex && gBlock == isBlock) {
                gEnd = fe;
            } else {
                flush();
                gStart = fs; gEnd = fe; gTex = tex; gBlock = isBlock;
            }
        }
        flush();  // cambio de bloque cierra el grupo
    }
    flush();
}

} // namespace

void renderMathInDocument(QTextDocument *doc)
{
    QList<Repl> repls;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid())
                continue;
            const QTextCharFormat cf = frag.charFormat();
            // Si ya está renderizado, no se toca (idempotencia).
            if (cf.boolProperty(IsMathProperty))
                continue;
            if (!cf.fontFixedPitch())
                continue;
            QString tex;
            bool isBlock = false;
            if (!inlineCodeIsMath(frag.text(), tex, isBlock))
                continue;
            const QTextCharFormat base = mathCharFormat(tex, isBlock);
            repls.append({frag.position(),
                          frag.position() + frag.length(),
                          renderTexAsRuns(tex, base)});
        }
    }
    applyReplacements(doc, repls);
}

// Caracteres centinela en el área de uso privado de Unicode. Qt los trata como
// texto opaco al serializar a Markdown (no los escapa), así que sirven como
// marcador estable que sobrevive a toMarkdown sin alteraciones.
static constexpr QChar kSentinelOpen(0xF8FE);
static constexpr QChar kSentinelClose(0xF8FF);

QList<QPair<int, int>> mathGroupBounds(QTextDocument *doc)
{
    QList<QPair<int, int>> result;
    forEachMathGroup(doc, [&](int s, int e, const QString &, bool) {
        result.append({s, e});
    });
    return result;
}

MathSentinelTable replaceMathWithSentinels(QTextDocument *doc)
{
    MathSentinelTable table;
    QList<Repl> repls;
    // Una sentinela por GRUPO (fórmula entera), no por fragmento — una fórmula
    // con super/sub vive como varios fragmentos contiguos.
    forEachMathGroup(doc, [&](int start, int end, const QString &tex, bool isBlock) {
        const int index = table.entries.size();
        table.entries.append({tex, isBlock});
        const QString sentinel = kSentinelOpen + QString::number(index) + kSentinelClose;
        // Sin formato: texto plano. Así no acaba dentro de un span de
        // negrita/cursiva/código que Qt pudiera escapar.
        QTextCharFormat plain;
        repls.append({start, end, {{sentinel, plain}}});
    });
    applyReplacements(doc, repls);
    return table;
}

QString restoreMathFromSentinels(const QString &markdown,
                                 const MathSentinelTable &table)
{
    if (table.entries.isEmpty())
        return markdown;
    static const QRegularExpression re(
        QStringLiteral("[\\x{F8FE}]([0-9]+)[\\x{F8FF}]"));
    QString out;
    out.reserve(markdown.size());
    int last = 0;
    QRegularExpressionMatchIterator it = re.globalMatch(markdown);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        const int idx = m.captured(1).toInt();
        if (idx < 0 || idx >= table.entries.size())
            continue;
        out += QStringView{markdown}.mid(last, m.capturedStart() - last);
        const auto &entry = table.entries.at(idx);
        const QString delim = entry.second ? QStringLiteral("$$") : QStringLiteral("$");
        out += delim + entry.first + delim;
        last = m.capturedEnd();
    }
    out += QStringView{markdown}.mid(last);
    return out;
}

void unrenderMathInDocument(QTextDocument *doc)
{
    QList<Repl> repls;
    forEachMathGroup(doc, [&](int start, int end, const QString &tex, bool isBlock) {
        const QString delim = isBlock ? QStringLiteral("$$") : QStringLiteral("$");
        QTextCharFormat code;
        code.setFontFixedPitch(true);
        repls.append({start, end, {{delim + tex + delim, code}}});
    });
    applyReplacements(doc, repls);
}

QString replaceMathWithUnicode(const QString &markdown)
{
    const QList<Span> spans = findMath(markdown);
    if (spans.isEmpty())
        return markdown;
    QString out;
    out.reserve(markdown.size());
    int cur = 0;
    for (const Span &s : spans) {
        out += QStringView{markdown}.mid(cur, s.start - cur);
        out += texToUnicode(s.content);
        cur = s.start + s.length;
    }
    out += QStringView{markdown}.mid(cur);
    return out;
}

} // namespace mdmath
