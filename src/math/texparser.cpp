/// \file
/// \brief Motor de parseo TeX→runs/Unicode: tablas de glifos, super/subíndices, fracciones y alfabetos matemáticos.

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

// Tope de profundidad de recursión del parser. TeX arbitrario (tecleado o pegado)
// puede anidar grupos sin límite (`\frac{\frac{...}}`, `x^{y^{z^{...}}}`), y como
// renderTexAsRuns/parseFrac/parseSqrt/parseScript/texToUnicode se llaman entre sí
// por cada nivel, sin tope reventaban la pila (SIGSEGV) con suficientes niveles.
// 256 es holgadísimo para cualquier fórmula real y corta de raíz el desbordamiento.
// Al excederlo, renderTexAsRuns devuelve el TeX restante como texto literal.
constexpr int kMaxTexDepth = 256;

// Contador de profundidad con RAII. thread_local por si en el futuro se parsea
// fuera del hilo de la GUI; hoy el parseo es monohilo.
namespace {
thread_local int g_texDepth = 0;

struct TexDepthGuard {
    TexDepthGuard() { ++g_texDepth; }
    ~TexDepthGuard() { --g_texDepth; }
    bool overflow() const { return g_texDepth > kMaxTexDepth; }
};
}  // namespace

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
        {QStringLiteral("dagger"),  QChar(0x2020)},
        {QStringLiteral("ddagger"), QChar(0x2021)},
        {QStringLiteral("wp"),      QChar(0x2118)},
        {QStringLiteral("therefore"), QChar(0x2234)},
        {QStringLiteral("because"),   QChar(0x2235)},
        // Delimitadores (los que no son un carácter ASCII directo). Los
        // autoescalables \left/\right se tratan aparte en el parser.
        {QStringLiteral("langle"),  QChar(0x27E8)},
        {QStringLiteral("rangle"),  QChar(0x27E9)},
        {QStringLiteral("lceil"),   QChar(0x2308)},
        {QStringLiteral("rceil"),   QChar(0x2309)},
        {QStringLiteral("lfloor"),  QChar(0x230A)},
        {QStringLiteral("rfloor"),  QChar(0x230B)},
        {QStringLiteral("Vert"),    QChar(0x2016)},
        {QStringLiteral("vert"),    QChar(0x007C)},
        {QStringLiteral("mid"),     QChar(0x2223)},
        {QStringLiteral("nmid"),    QChar(0x2224)},
        // Más relaciones y operadores de conjuntos/orden
        {QStringLiteral("perp"),       QChar(0x22A5)},
        {QStringLiteral("parallel"),   QChar(0x2225)},
        {QStringLiteral("angle"),      QChar(0x2220)},
        {QStringLiteral("setminus"),   QChar(0x2216)},
        {QStringLiteral("triangle"),   QChar(0x25B3)},
        {QStringLiteral("prec"),       QChar(0x227A)},
        {QStringLiteral("succ"),       QChar(0x227B)},
        {QStringLiteral("preceq"),     QChar(0x227C)},
        {QStringLiteral("succeq"),     QChar(0x227D)},
        {QStringLiteral("subsetneq"),  QChar(0x228A)},
        {QStringLiteral("supsetneq"),  QChar(0x228B)},
        {QStringLiteral("sqsubseteq"), QChar(0x2291)},
        {QStringLiteral("sqsupseteq"), QChar(0x2292)},
        {QStringLiteral("sqcup"),      QChar(0x2294)},
        {QStringLiteral("sqcap"),      QChar(0x2293)},
        {QStringLiteral("vdash"),      QChar(0x22A2)},
        {QStringLiteral("dashv"),      QChar(0x22A3)},
        {QStringLiteral("models"),     QChar(0x22A8)},
        {QStringLiteral("top"),        QChar(0x22A4)},
        {QStringLiteral("bot"),        QChar(0x22A5)},
        {QStringLiteral("asymp"),      QChar(0x224D)},
        {QStringLiteral("doteq"),      QChar(0x2250)},
        {QStringLiteral("odot"),       QChar(0x2299)},
        {QStringLiteral("nleq"),       QChar(0x2270)},
        {QStringLiteral("ngeq"),       QChar(0x2271)},
        {QStringLiteral("nless"),      QChar(0x226E)},
        {QStringLiteral("ngtr"),       QChar(0x226F)},
        {QStringLiteral("nsubseteq"),  QChar(0x2288)},
        {QStringLiteral("nsupseteq"),  QChar(0x2289)},
        // Grandes operadores adicionales (en \sum/\int conviven con isBigOp del
        // motor 2D; aquí dan su glifo inline).
        {QStringLiteral("bigcup"),     QChar(0x22C3)},
        {QStringLiteral("bigcap"),     QChar(0x22C2)},
        {QStringLiteral("bigvee"),     QChar(0x22C1)},
        {QStringLiteral("bigwedge"),   QChar(0x22C0)},
        {QStringLiteral("bigoplus"),   QChar(0x2A01)},
        {QStringLiteral("bigotimes"),  QChar(0x2A02)},
        {QStringLiteral("bigodot"),    QChar(0x2A00)},
        {QStringLiteral("bigsqcup"),   QChar(0x2A06)},
        // Más flechas
        {QStringLiteral("Uparrow"),            QChar(0x21D1)},
        {QStringLiteral("Downarrow"),          QChar(0x21D3)},
        {QStringLiteral("updownarrow"),        QChar(0x2195)},
        {QStringLiteral("Updownarrow"),        QChar(0x21D5)},
        {QStringLiteral("hookrightarrow"),     QChar(0x21AA)},
        {QStringLiteral("hookleftarrow"),      QChar(0x21A9)},
        {QStringLiteral("longrightarrow"),     QChar(0x27F6)},
        {QStringLiteral("longleftarrow"),      QChar(0x27F5)},
        {QStringLiteral("longleftrightarrow"), QChar(0x27F7)},
        {QStringLiteral("Longrightarrow"),     QChar(0x27F9)},
        {QStringLiteral("Longleftarrow"),      QChar(0x27F8)},
        {QStringLiteral("Longleftrightarrow"), QChar(0x27FA)},
        {QStringLiteral("rightharpoonup"),     QChar(0x21C0)},
        {QStringLiteral("leftharpoonup"),      QChar(0x21BC)},
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
        {QStringLiteral("quad"),  QString(QChar(0x2003))},              // espacio em
        {QStringLiteral("qquad"), QString(2, QChar(0x2003))},          // doble em
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

QChar accentCombiningChar(const QString &cmd)
{
    static const QHash<QString, QChar> m = {
        {QStringLiteral("hat"),       QChar(0x0302)}, {QStringLiteral("widehat"),   QChar(0x0302)},
        {QStringLiteral("bar"),       QChar(0x0304)}, {QStringLiteral("overline"),  QChar(0x0304)},
        {QStringLiteral("underline"), QChar(0x0332)},
        {QStringLiteral("tilde"),     QChar(0x0303)}, {QStringLiteral("widetilde"), QChar(0x0303)},
        {QStringLiteral("vec"),       QChar(0x20D7)},
        {QStringLiteral("dot"),       QChar(0x0307)}, {QStringLiteral("ddot"),      QChar(0x0308)},
        {QStringLiteral("acute"),     QChar(0x0301)}, {QStringLiteral("grave"),     QChar(0x0300)},
        {QStringLiteral("check"),     QChar(0x030C)}, {QStringLiteral("breve"),     QChar(0x0306)},
    };
    return m.value(cmd, QChar());
}

QString readTokenAsUnicode(const QString &tex, int &i)
{
    const int n = tex.size();
    if (i >= n)
        return QString();
    if (tex.at(i) == QLatin1Char('\\')) {
        ++i;
        if (i < n && tex.at(i).isLetter())
            return commandToUnicode(readCommand(tex, i));
        if (i < n) {
            const QChar e = tex.at(i);
            ++i;
            return QString(e);   // \{ \} \| \/ y demás delimitadores con escape
        }
        return QStringLiteral("\\");
    }
    const QChar e = tex.at(i);
    ++i;
    return QString(e);
}

bool isStyledAlphabetCommand(const QString &cmd)
{
    return cmd == QLatin1String("mathcal") || cmd == QLatin1String("mathscr")
           || cmd == QLatin1String("mathfrak");
}

namespace {
// Letra latina A-Z/a-z en su variante matemática Unicode (script o fraktur). El
// bloque astral de letras matemáticas (U+1D49C…) tiene «huecos»: varias letras se
// reservaron antes en «Letterlike Symbols» (BMP), así que hay que sustituirlas o
// el glifo sale en blanco. Las no latinas se devuelven sin tocar.
QString styledLetter(QChar c, bool fraktur)
{
    auto ucs4 = [](char32_t cp) { return QString::fromUcs4(&cp, 1); };
    const ushort u = c.unicode();
    if (fraktur) {
        static const QHash<ushort, char32_t> upperExc = {
            {'C', 0x212D}, {'H', 0x210C}, {'I', 0x2111}, {'R', 0x211C}, {'Z', 0x2128},
        };
        if (u >= 'A' && u <= 'Z')
            return ucs4(upperExc.value(u, 0x1D504 + (u - 'A')));
        if (u >= 'a' && u <= 'z')
            return ucs4(0x1D51E + (u - 'a'));
        return QString(c);
    }
    // Script / caligráfica (mathcal, mathscr).
    static const QHash<ushort, char32_t> upperExc = {
        {'B', 0x212C}, {'E', 0x2130}, {'F', 0x2131}, {'H', 0x210B}, {'I', 0x2110},
        {'L', 0x2112}, {'M', 0x2133}, {'R', 0x211B},
    };
    static const QHash<ushort, char32_t> lowerExc = {
        {'e', 0x212F}, {'g', 0x210A}, {'o', 0x2134},
    };
    if (u >= 'A' && u <= 'Z')
        return ucs4(upperExc.value(u, 0x1D49C + (u - 'A')));
    if (u >= 'a' && u <= 'z')
        return ucs4(lowerExc.value(u, 0x1D4B6 + (u - 'a')));
    return QString(c);
}
} // namespace

QString styledMathAlphabet(const QString &cmd, const QString &arg)
{
    const bool fraktur = (cmd == QLatin1String("mathfrak"));
    QString out;
    out.reserve(arg.size());
    for (const QChar c : arg)
        out += styledLetter(c, fraktur);
    return out;
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
    const TexDepthGuard depth;

    QList<MathRun> runs;
    QString buffer;

    // Anidamiento patológico: en vez de seguir recurriendo (y desbordar la pila),
    // se deja el resto del TeX como texto literal. La fórmula se verá «en crudo»
    // pero la app no cae.
    if (depth.overflow()) {
        runs.append({tex, baseFmt});
        return runs;
    }

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

            // Acento: base + carácter combinante (x̂). Aplana el argumento.
            const QChar accent = accentCombiningChar(cmd);
            if (!accent.isNull() && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                buffer += texToUnicode(readGroup(tex, i)) + accent;
                continue;
            }

            // \binom{n}{k}: inline no se puede apilar, así que se aproxima como
            // C(n, k) (el 2D sí lo apila; LaTeX emite \binom nativo).
            if (cmd == QLatin1String("binom") && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                const QString a = readGroup(tex, i);
                while (i < n && tex.at(i) == QLatin1Char(' ')) ++i;
                QString b;
                if (i < n && tex.at(i) == QLatin1Char('{')) b = readGroup(tex, i);
                buffer += QStringLiteral("C(") + texToUnicode(a) + QStringLiteral(", ")
                          + texToUnicode(b) + QLatin1Char(')');
                continue;
            }

            // \left( … \right): delimitadores autoescalables. Inline no se
            // escalan, así que basta con emitir el delimitador y descartar la
            // palabra clave (antes salía el literal «\left»). `.` = delim. nulo.
            if (cmd == QLatin1String("left") || cmd == QLatin1String("right")) {
                int j = after;
                if (j < n && tex.at(j) == QLatin1Char('.'))
                    ++j;                                  // delimitador nulo: nada
                else
                    buffer += readTokenAsUnicode(tex, j);
                i = j;
                continue;
            }

            // \not X: negación genérica (combinante U+0338 sobre el operando).
            if (cmd == QLatin1String("not")) {
                int j = after;
                buffer += readTokenAsUnicode(tex, j) + QChar(0x0338);
                i = j;
                continue;
            }

            // \mathcal/\mathscr/\mathfrak{...}: cada letra a su variante
            // matemática Unicode (script/fraktur).
            if (isStyledAlphabetCommand(cmd) && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                buffer += styledMathAlphabet(cmd, readGroup(tex, i));
                continue;
            }

            // \text{...} y familia de fuentes: el argumento se emite literal
            // (texto en redonda). \mathbb{X} cae más abajo (tabla de glifos).
            if ((cmd == QLatin1String("text") || cmd == QLatin1String("mathrm")
                 || cmd == QLatin1String("mathbf") || cmd == QLatin1String("mathit")
                 || cmd == QLatin1String("mathsf") || cmd == QLatin1String("mathtt")
                 || cmd == QLatin1String("operatorname"))
                && after < n && tex.at(after) == QLatin1Char('{')) {
                i = after;
                buffer += readGroup(tex, i);
                continue;
            }

            // \mathbb{X} y similares: las entradas combinadas de la tabla son todas
            // de UN solo carácter (`\mathbb{R}`), así que solo miramos el caso `{X}`
            // sin leer todo el grupo. Leerlo (readGroup) para CADA comando con `{`
            // era O(n²): con TeX hostil como `\a{\a{\a{…` colgaba la interfaz.
            if (after + 2 < n && tex.at(after) == QLatin1Char('{')
                && tex.at(after + 2) == QLatin1Char('}')) {
                const QString combined =
                    cmd + QLatin1Char('{') + tex.at(after + 1) + QLatin1Char('}');
                const auto it = singleCharCommands().constFind(combined);
                if (it != singleCharCommands().cend()) {
                    buffer += it.value();
                    i = after + 3;  // pasa `{X}`
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
