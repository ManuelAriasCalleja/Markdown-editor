#include "mathblocks.h"

#include <QChar>
#include <QHash>
#include <QSet>
#include <QString>

// Motor de parseo TeX -> runs/Unicode, extraido de mathblocks.cpp (era el bloque
// mas grande y autonomo del modulo). Es PURO: convierte una cadena TeX en una
// secuencia de MathRun (texto + QTextCharFormat con vertical-align) o en su
// aproximacion Unicode plana. No toca el QTextDocument ni el Markdown fuente: de
// eso se ocupa mathblocks.cpp. Comparte el namespace mdmath y las declaraciones
// publicas (renderTexAsRuns, texToUnicode, wrapTex) que viven en mathblocks.h.

namespace mdmath {

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

} // namespace

// Convierte un comando TeX (sin barra invertida) a Unicode si está en la
// tabla; si no, devuelve `\nombre` para que el usuario vea el error. Público
// (declarado en mathblocks.h) para que el motor de maquetación 2D (mathlayout)
// reutilice la misma tabla de glifos sin duplicarla.
QString commandToUnicode(const QString &cmd)
{
    const auto it = singleCharCommands().constFind(cmd);
    if (it != singleCharCommands().cend())
        return QString(it.value());
    const auto itm = multiCharCommands().constFind(cmd);
    if (itm != multiCharCommands().cend())
        return itm.value();
    // Nombres de función / operadores que se componen como texto (sin `\`):
    // \lim, \sin, \log… En matemáticas van en redonda, pero aquí basta con el
    // propio nombre (el italic del run es aceptable).
    static const QSet<QString> functionNames = {
        QStringLiteral("lim"),    QStringLiteral("sin"),    QStringLiteral("cos"),
        QStringLiteral("tan"),    QStringLiteral("cot"),    QStringLiteral("sec"),
        QStringLiteral("csc"),    QStringLiteral("sinh"),   QStringLiteral("cosh"),
        QStringLiteral("tanh"),   QStringLiteral("arcsin"), QStringLiteral("arccos"),
        QStringLiteral("arctan"), QStringLiteral("log"),    QStringLiteral("ln"),
        QStringLiteral("exp"),    QStringLiteral("max"),    QStringLiteral("min"),
        QStringLiteral("sup"),    QStringLiteral("inf"),    QStringLiteral("det"),
        QStringLiteral("dim"),    QStringLiteral("gcd"),    QStringLiteral("arg"),
        QStringLiteral("deg"),    QStringLiteral("ker"),    QStringLiteral("mod"),
    };
    if (functionNames.contains(cmd))
        return cmd;
    return QLatin1Char('\\') + cmd;
}

// Vuelca el texto acumulado en `buffer` (si lo hay) como un run con `baseFmt` y lo
// limpia. Sub-paso común de los parsers de renderTexAsRuns.
static void flushBuffer(QList<MathRun> &runs, QString &buffer, const QTextCharFormat &baseFmt)
{
    if (!buffer.isEmpty()) {
        runs.append({buffer, baseFmt});
        buffer.clear();
    }
}

// \frac{num}{den}. `i` entra apuntando al '{' del numerador y sale tras el '}' del
// denominador. Numerador y denominador van como sub-runs aparte para que sus
// super/subíndices internos también se rendericen elevados/bajados. Caso simple
// (un solo carácter en cada uno): usa el FRACTION SLASH (U+2044), que en fuentes
// con soporte se pinta como fracción tipográfica.
static void parseFrac(const QString &tex, int &i, const QTextCharFormat &baseFmt,
                      QList<MathRun> &runs, QString &buffer)
{
    const int n = tex.size();
    const QString num = readGroup(tex, i);
    while (i < n && tex.at(i) == QLatin1Char(' ')) ++i;
    QString den;
    if (i < n && tex.at(i) == QLatin1Char('{'))
        den = readGroup(tex, i);
    if (num.size() == 1 && den.size() == 1
        && !num.at(0).isSpace() && !den.at(0).isSpace()) {
        buffer += num + QChar(0x2044) + den;
    } else {
        buffer += QLatin1Char('(');
        flushBuffer(runs, buffer, baseFmt);
        runs.append(renderTexAsRuns(num, baseFmt));
        buffer += QStringLiteral(")/(");
        flushBuffer(runs, buffer, baseFmt);
        runs.append(renderTexAsRuns(den, baseFmt));
        buffer += QLatin1Char(')');
    }
}

// \sqrt: emite "√" y, si hay grupo en `after`, su argumento recursivo (para que
// `\sqrt{x^2}` muestre el 2 elevado). `i` es el final del comando y `after` la
// primera posición tras los espacios. Devuelve la nueva posición a procesar (sin
// grupo, queda en `i`, dejando los espacios para el bucle, como antes).
static int parseSqrt(const QString &tex, int i, int after, const QTextCharFormat &baseFmt,
                     QList<MathRun> &runs, QString &buffer)
{
    buffer += QChar(0x221A);
    const int n = tex.size();
    if (after < n && tex.at(after) == QLatin1Char('{')) {
        i = after;
        const QString arg = readGroup(tex, i);
        buffer += QLatin1Char('(');
        flushBuffer(runs, buffer, baseFmt);
        runs.append(renderTexAsRuns(arg, baseFmt));
        buffer += QLatin1Char(')');
    }
    return i;
}

// Super/subíndice (`^`/`_`). `i` entra apuntando al argumento (tras el ^/_) y sale
// tras consumirlo. El argumento se aplana a Unicode (texToUnicode recursivo) para
// envolverlo en UN solo run con vertical-align: aplanar aquí es lo que mantiene los
// scripts anidados manejables (el nivel exterior es Qt vertical-align y los
// interiores usan la versión Unicode super/sub).
static void parseScript(const QString &tex, int &i, bool sup,
                        const QTextCharFormat &baseFmt, QList<MathRun> &runs, QString &buffer)
{
    const int n = tex.size();
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
    flushBuffer(runs, buffer, baseFmt);
    QTextCharFormat f = baseFmt;
    f.setVerticalAlignment(sup ? QTextCharFormat::AlignSuperScript
                               : QTextCharFormat::AlignSubScript);
    runs.append({arg, f});
}

QString wrapTex(const QString &tex, bool block)
{
    const QString delim = block ? QStringLiteral("$$") : QStringLiteral("$");
    return delim + tex + delim;
}

QList<MathRun> renderTexAsRuns(const QString &tex, const QTextCharFormat &baseFmt)
{
    QList<MathRun> runs;
    QString buffer;

    int i = 0;
    const int n = tex.size();
    while (i < n) {
        const QChar c = tex.at(i);

        if (c == QLatin1Char('\\')) {
            ++i;
            if (i >= n) { buffer += QLatin1Char('\\'); break; }
            if (tex.at(i) == QLatin1Char('\\')) { buffer += QLatin1Char(' '); ++i; continue; }
            if (!tex.at(i).isLetter()) {
                // Comandos de espaciado: `\,` `\;` `\:` `\ ` → espacio fino; `\!`
                // → nada. El resto (`\$`, `\{`, `\_`…) es el carácter literal.
                const QChar e = tex.at(i);
                ++i;
                if (e == QLatin1Char(',') || e == QLatin1Char(';') || e == QLatin1Char(':')
                    || e == QLatin1Char(' '))
                    buffer += QChar(0x2009);  // thin space
                else if (e != QLatin1Char('!'))
                    buffer += e;
                continue;
            }
            const QString cmd = readCommand(tex, i);

            int after = i;
            while (after < n && tex.at(after) == QLatin1Char(' ')) ++after;

            if (cmd == QLatin1String("frac") && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                parseFrac(tex, i, baseFmt, runs, buffer);
                continue;
            }

            if (cmd == QLatin1String("sqrt")) {
                i = parseSqrt(tex, i, after, baseFmt, runs, buffer);
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
            parseScript(tex, i, sup, baseFmt, runs, buffer);
            continue;
        }

        if (c == QLatin1Char('{') || c == QLatin1Char('}')) { ++i; continue; }

        buffer += c;
        ++i;
    }
    flushBuffer(runs, buffer, baseFmt);
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

} // namespace mdmath
