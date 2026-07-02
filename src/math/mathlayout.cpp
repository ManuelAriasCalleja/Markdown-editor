/// \file
/// \brief Implementación del motor de maquetación 2D: árbol de cajas, medición y pintado de fórmulas.

#include "mathlayout.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>

#include <algorithm>
#include <cmath>

#include "mathblocks.h"  // commandToUnicode

namespace mdmath {

namespace {

// Tope de profundidad de recursión del maquetador 2D. buildHList se llama a sí
// mismo por cada grupo/sub-fórmula (\frac, \sqrt, matrices, scripts), así que TeX
// muy anidado desbordaba la pila (SIGSEGV). 256 es de sobra para fórmulas reales;
// al excederlo, buildHList devuelve el TeX restante como un glifo de texto literal.
constexpr int kMaxLayoutDepth = 256;
thread_local int g_layoutDepth = 0;

struct LayoutDepthGuard {
    LayoutDepthGuard() { ++g_layoutDepth; }
    ~LayoutDepthGuard() { --g_layoutDepth; }
    bool overflow() const { return g_layoutDepth > kMaxLayoutDepth; }
};

// Conjunto de «grandes operadores» que en estilo display llevan sus límites
// (`_`/`^`) apilados encima/debajo en vez de a la derecha.
bool isBigOp(const QString &cmd)
{
    static const QStringList ops = {
        QStringLiteral("sum"),   QStringLiteral("prod"),  QStringLiteral("coprod"),
        QStringLiteral("int"),   QStringLiteral("iint"),  QStringLiteral("iiint"),
        QStringLiteral("oint"),  QStringLiteral("bigcup"),QStringLiteral("bigcap"),
    };
    return ops.contains(cmd);
}

// Escala una fuente por `factor` respetando si usa puntos o píxeles, con un
// suelo para que los scripts profundos no desaparezcan.
QFont scaleFont(QFont f, qreal factor)
{
    if (f.pointSizeF() > 0)
        f.setPointSizeF(std::max(4.0, f.pointSizeF() * factor));
    else if (f.pixelSize() > 0)
        f.setPixelSize(std::max(5, int(std::lround(f.pixelSize() * factor))));
    return f;
}

QFont scriptFont(const QFont &f) { return scaleFont(f, 0.72); }
QFont bigOpFont(const QFont &f)  { return scaleFont(f, 1.55); }

// Métricas derivadas (todas relativas al tamaño de la fuente del nivel) usadas
// por igual en medición y pintado para que casen.
qreal axisHeight(const QFont &f) { return QFontMetricsF(f).xHeight() * 0.5; }
qreal fracGap(const QFont &f)    { return QFontMetricsF(f).xHeight() * 0.35; }
qreal fracPad(const QFont &f)    { return std::max(1.0, QFontMetricsF(f).averageCharWidth() * 0.35); }
qreal barThickness(const QFont &f) { return std::max(1.0, QFontMetricsF(f).ascent() * 0.06); }
qreal scriptRaise(const QFont &f){ return QFontMetricsF(f).ascent() * 0.50; }
qreal scriptDrop(const QFont &f) { return QFontMetricsF(f).ascent() * 0.28; }
qreal bigOpGap(const QFont &f)   { return QFontMetricsF(f).xHeight() * 0.45; }
qreal sqrtGap(const QFont &f)    { return std::max(1.0, barThickness(f) * 2.0); }   // vínculo↔radicando
qreal sqrtSignW(const QFont &f)  { return std::max(6.0, QFontMetricsF(f).averageCharWidth() * 0.9); }
qreal matrixColGap(const QFont &f){ return QFontMetricsF(f).averageCharWidth() * 1.1; }
qreal matrixRowGap(const QFont &f){ return QFontMetricsF(f).xHeight() * 0.7; }
qreal delimWidth(const QFont &f) { return std::max(3.0, QFontMetricsF(f).averageCharWidth() * 0.5); }
qreal delimPad(const QFont &f)   { return QFontMetricsF(f).averageCharWidth() * 0.25; }

// Delimitadores de matriz.
enum Delim { DelimNone = 0, DelimParen, DelimBracket, DelimBrace, DelimBar };

// Caja del árbol de maquetación. Según `kind`, `kids` tiene aridad fija:
//   HList  -> ítems en fila.
//   Glyph  -> hoja: `text` con `font`.
//   Frac   -> {numerador, denominador}.
//   Script -> {base, superíndice, subíndice} (presencia en hasSup/hasSub).
//   BigOp  -> {operador, límite-encima, límite-debajo} (hasOver/hasUnder).
//   Sqrt   -> {radicando, índice} (índice opcional, en hasIndex).
//   Matrix -> rows*cols celdas en orden por filas; `rows`/`cols`/`delim`.
// La geometría (w/asc/dsc) la rellena `measure`. `asc`/`dsc` se miden desde el
// baseline (arriba/abajo); height = asc + dsc; `w` es el avance horizontal.
struct Box {
    enum Kind { HList, Glyph, Frac, Script, BigOp, Sqrt, Matrix, Binom };
    Kind kind = HList;
    QString text;
    QFont font;
    QList<Box> kids;
    bool hasSup = false, hasSub = false, hasOver = false, hasUnder = false;
    bool hasIndex = false;            // Sqrt
    int rows = 0, cols = 0, delim = DelimNone;  // Matrix / Binom
    bool leftAlign = false;           // Matrix: celdas a la izquierda (cases)
    bool braceLeftOnly = false;       // Matrix: solo delimitador izquierdo (cases)
    qreal w = 0, asc = 0, dsc = 0;
    qreal height() const { return asc + dsc; }
};

Box glyph(const QString &text, const QFont &font)
{
    Box b;
    b.kind = Box::Glyph;
    b.text = text;
    b.font = font;
    return b;
}

// --- Parseo TeX -> árbol de cajas --------------------------------------------

QString readCommand(const QString &s, int &i)
{
    const int start = i;
    while (i < s.size() && s.at(i).isLetter())
        ++i;
    return s.mid(start, i - start);
}

QString readGroup(const QString &s, int &i)
{
    if (i >= s.size() || s.at(i) != QLatin1Char('{'))
        return QString();
    ++i;
    const int start = i;
    int depth = 1;
    while (i < s.size() && depth > 0) {
        const QChar c = s.at(i);
        if (c == QLatin1Char('{')) ++depth;
        else if (c == QLatin1Char('}')) --depth;
        if (depth > 0) ++i;
    }
    const QString content = s.mid(start, i - start);
    if (i < s.size() && s.at(i) == QLatin1Char('}'))
        ++i;
    return content;
}

// Lee el argumento de un `^`/`_`: un grupo `{...}`, un comando `\cmd`/`\x` o un
// único carácter. Devuelve la subcadena TeX (con la `\` si la había) para que
// el llamador la re-parsee recursivamente.
QString readScriptArg(const QString &s, int &i)
{
    const int n = s.size();
    if (i >= n)
        return QString();
    const QChar c = s.at(i);
    if (c == QLatin1Char('{'))
        return readGroup(s, i);
    if (c == QLatin1Char('\\')) {
        const int start = i;
        ++i;
        if (i < n && s.at(i).isLetter())
            readCommand(s, i);
        else if (i < n)
            ++i;
        return s.mid(start, i - start);
    }
    ++i;
    return QString(c);
}

Box buildHList(const QString &tex, const QFont &font);

// Tras construir un átomo, absorbe los `^`/`_` que le sigan. Si el átomo es un
// gran operador, los coloca como límites encima/debajo; si no, como super/
// subíndice a la derecha.
Box attachScripts(const QString &tex, int &i, Box base, const QFont &font)
{
    const int n = tex.size();
    Box sup, sub;
    bool hs = false, hb = false;
    while (i < n && (tex.at(i) == QLatin1Char('^') || tex.at(i) == QLatin1Char('_'))) {
        const bool up = tex.at(i) == QLatin1Char('^');
        ++i;
        const QString arg = readScriptArg(tex, i);
        Box s = buildHList(arg, scriptFont(font));
        if (up) { sup = s; hs = true; }
        else    { sub = s; hb = true; }
    }
    if (!hs && !hb)
        return base;
    if (base.kind == Box::BigOp) {
        if (hs) { base.kids[1] = sup; base.hasOver = true; }
        if (hb) { base.kids[2] = sub; base.hasUnder = true; }
        return base;
    }
    Box sc;
    sc.kind = Box::Script;
    sc.font = font;
    sc.kids = { base, hs ? sup : glyph(QString(), scriptFont(font)),
                      hb ? sub : glyph(QString(), scriptFont(font)) };
    sc.hasSup = hs;
    sc.hasSub = hb;
    return sc;
}

Box makeBigOp(const QString &cmd, const QFont &font)
{
    Box b;
    b.kind = Box::BigOp;
    b.font = font;
    b.kids = { glyph(commandToUnicode(cmd), bigOpFont(font)),
               glyph(QString(), scriptFont(font)),
               glyph(QString(), scriptFont(font)) };
    return b;
}

// `\sqrt[índice]{radicando}`. `after` apunta tras `sqrt` (saltados espacios);
// `i` sale tras el grupo consumido. El índice es opcional (`\sqrt[3]{x}`).
Box makeSqrt(const QString &tex, int after, int &i, const QFont &font)
{
    const int n = tex.size();
    int k = after;
    Box index;
    bool hasIdx = false;
    if (k < n && tex.at(k) == QLatin1Char('[')) {
        const int start = ++k;
        int depth = 1;
        while (k < n && depth > 0) {
            const QChar ch = tex.at(k);
            if (ch == QLatin1Char('[')) ++depth;
            else if (ch == QLatin1Char(']')) --depth;
            if (depth > 0) ++k;
        }
        index = buildHList(tex.mid(start, k - start), scriptFont(font));
        hasIdx = true;
        if (k < n && tex.at(k) == QLatin1Char(']')) ++k;
        while (k < n && tex.at(k) == QLatin1Char(' ')) ++k;
    }
    QString radTex;
    if (k < n && tex.at(k) == QLatin1Char('{')) {
        i = k;
        radTex = readGroup(tex, i);
    } else if (k < n) {
        // Sin llaves: el radicando es el siguiente token (`\sqrt 2`, `\sqrt\pi`).
        i = k;
        if (tex.at(k) == QLatin1Char('\\')) {
            const int s = i;
            ++i;
            if (i < n && tex.at(i).isLetter()) readCommand(tex, i);
            else if (i < n) ++i;
            radTex = tex.mid(s, i - s);
        } else {
            radTex = QString(tex.at(k));
            i = k + 1;
        }
    } else {
        i = k;
    }
    Box b;
    b.kind = Box::Sqrt;
    b.font = font;
    b.kids = { buildHList(radTex, font),
               hasIdx ? index : glyph(QString(), scriptFont(font)) };
    b.hasIndex = hasIdx;
    return b;
}

int matrixDelim(const QString &env)
{
    if (env == QLatin1String("pmatrix")) return DelimParen;
    if (env == QLatin1String("bmatrix")) return DelimBracket;
    if (env == QLatin1String("Bmatrix")) return DelimBrace;
    if (env == QLatin1String("vmatrix") || env == QLatin1String("Vmatrix")) return DelimBar;
    return DelimNone;  // matrix y entornos desconocidos: sin delimitador
}

// `\begin{env} celda & celda \\ … \end{env}`. `after` apunta al `{env}`; `i`
// sale tras el `\end{env}`. Construye una rejilla de celdas.
Box makeMatrix(const QString &tex, int after, int &i, const QFont &font)
{
    const int n = tex.size();
    i = after;
    const QString env = readGroup(tex, i);
    const QString endTok = QStringLiteral("\\end{") + env + QLatin1Char('}');
    const int endPos = tex.indexOf(endTok, i);
    const QString body = (endPos >= 0) ? tex.mid(i, endPos - i) : tex.mid(i);
    i = (endPos >= 0) ? endPos + endTok.size() : n;

    Box b;
    b.kind = Box::Matrix;
    b.font = font;
    if (env == QLatin1String("cases")) {
        // Sistema de ecuaciones: llave izquierda y celdas alineadas a la izquierda.
        b.delim = DelimBrace;
        b.braceLeftOnly = true;
        b.leftAlign = true;
    } else {
        b.delim = matrixDelim(env);
    }
    QList<QList<Box>> grid;
    int cols = 0;
    for (const QString &rowStr : body.split(QStringLiteral("\\\\"))) {
        if (rowStr.trimmed().isEmpty())
            continue;  // fila vacía (p. ej. tras el último `\\`)
        QList<Box> cells;
        for (const QString &cellStr : rowStr.split(QLatin1Char('&')))
            cells.append(buildHList(cellStr.trimmed(), font));
        cols = std::max(cols, int(cells.size()));
        grid.append(cells);
    }
    b.rows = int(grid.size());
    b.cols = cols;
    for (const QList<Box> &row : grid) {
        for (int cc = 0; cc < cols; ++cc)
            b.kids.append(cc < row.size() ? row[cc] : glyph(QString(), font));
    }
    return b;
}

// Anchos de columna y asc/dsc de cada fila de una matriz ya medida.
void matrixMetrics(const Box &b, QList<qreal> &colW, QList<qreal> &rowAsc, QList<qreal> &rowDsc)
{
    colW = QList<qreal>(b.cols, 0.0);
    rowAsc = QList<qreal>(b.rows, 0.0);
    rowDsc = QList<qreal>(b.rows, 0.0);
    for (int r = 0; r < b.rows; ++r) {
        for (int cc = 0; cc < b.cols; ++cc) {
            const Box &cell = b.kids[r * b.cols + cc];
            colW[cc] = std::max(colW[cc], cell.w);
            rowAsc[r] = std::max(rowAsc[r], cell.asc);
            rowDsc[r] = std::max(rowDsc[r], cell.dsc);
        }
    }
}

Box buildHList(const QString &tex, const QFont &font)
{
    const LayoutDepthGuard depth;

    // Anidamiento patológico: en vez de seguir recurriendo (y desbordar la pila),
    // se cae a un glifo de texto con el TeX en crudo. Se verá mal pero no crashea.
    if (depth.overflow())
        return glyph(tex, font);

    Box list;
    list.kind = Box::HList;
    list.font = font;
    const int n = tex.size();
    int i = 0;
    while (i < n) {
        const QChar c = tex.at(i);
        if (c == QLatin1Char('{') || c == QLatin1Char('}')) { ++i; continue; }
        if (c == QLatin1Char('^') || c == QLatin1Char('_')) {
            // Script sin base (raro): cuélgalo de un glifo vacío.
            Box base = glyph(QString(), font);
            list.kids.append(attachScripts(tex, i, base, font));
            continue;
        }
        Box atom;
        if (c == QLatin1Char('\\')) {
            ++i;
            if (i >= n) {
                atom = glyph(QStringLiteral("\\"), font);
            } else if (!tex.at(i).isLetter()) {
                // Espaciado `\,` `\;` `\:` `\ ` → espacio fino; `\!` → nada.
                const QChar e = tex.at(i);
                ++i;
                if (e == QLatin1Char(',') || e == QLatin1Char(';') || e == QLatin1Char(':')
                    || e == QLatin1Char(' '))
                    atom = glyph(QString(QChar(0x2009)), font);
                else if (e == QLatin1Char('!'))
                    continue;
                else
                    atom = glyph(QString(e), font);
            } else {
                const QString cmd = readCommand(tex, i);
                int after = i;
                while (after < n && tex.at(after) == QLatin1Char(' ')) ++after;
                if (cmd == QLatin1String("frac") && after < n && tex.at(after) == QLatin1Char('{')) {
                    i = after;
                    const QString num = readGroup(tex, i);
                    while (i < n && tex.at(i) == QLatin1Char(' ')) ++i;
                    QString den;
                    if (i < n && tex.at(i) == QLatin1Char('{'))
                        den = readGroup(tex, i);
                    atom.kind = Box::Frac;
                    atom.font = font;
                    atom.kids = { buildHList(num, font), buildHList(den, font) };
                } else if (isBigOp(cmd)) {
                    atom = makeBigOp(cmd, font);
                } else if (cmd == QLatin1String("sqrt")) {
                    atom = makeSqrt(tex, after, i, font);
                } else if (cmd == QLatin1String("begin")) {
                    atom = makeMatrix(tex, after, i, font);
                } else if (cmd == QLatin1String("binom") && after < n && tex.at(after) == QLatin1Char('{')) {
                    i = after;
                    const QString a = readGroup(tex, i);
                    while (i < n && tex.at(i) == QLatin1Char(' ')) ++i;
                    QString bb;
                    if (i < n && tex.at(i) == QLatin1Char('{')) bb = readGroup(tex, i);
                    atom.kind = Box::Binom;
                    atom.font = font;
                    atom.kids = { buildHList(a, font), buildHList(bb, font) };
                } else if (!accentCombiningChar(cmd).isNull() && after < n
                           && tex.at(after) == QLatin1Char('{')) {
                    // Acento: aplana el argumento y le añade el carácter combinante.
                    i = after;
                    const QString arg = readGroup(tex, i);
                    atom = glyph(texToUnicode(arg) + QString(accentCombiningChar(cmd)), font);
                } else if (cmd == QLatin1String("left") || cmd == QLatin1String("right")) {
                    // Delimitador autoescalable: aquí no se estira (limitación),
                    // pero al menos se pinta el delimitador en vez del literal.
                    int j = after;
                    if (j < n && tex.at(j) == QLatin1Char('.')) {
                        i = j + 1;                 // delimitador nulo
                        atom = glyph(QString(), font);
                    } else {
                        atom = glyph(readTokenAsUnicode(tex, j), font);
                        i = j;
                    }
                } else if (cmd == QLatin1String("not")) {
                    int j = after;
                    atom = glyph(readTokenAsUnicode(tex, j) + QChar(0x0338), font);
                    i = j;
                } else if (isStyledAlphabetCommand(cmd) && after < n
                           && tex.at(after) == QLatin1Char('{')) {
                    i = after;
                    atom = glyph(styledMathAlphabet(cmd, readGroup(tex, i)), font);
                } else if ((cmd == QLatin1String("text") || cmd == QLatin1String("mathrm")
                            || cmd == QLatin1String("mathbf") || cmd == QLatin1String("mathit")
                            || cmd == QLatin1String("mathsf") || cmd == QLatin1String("mathtt")
                            || cmd == QLatin1String("operatorname"))
                           && after < n && tex.at(after) == QLatin1Char('{')) {
                    // Texto literal (redonda); el argumento no se parsea como math.
                    i = after;
                    atom = glyph(readGroup(tex, i), font);
                } else {
                    // \mathbb{R} y similares: las entradas combinadas de la tabla
                    // son de UN carácter, así que solo miramos `{X}` sin leer todo
                    // el grupo (readGroup por cada comando con `{` era O(n²) y
                    // colgaba con TeX hostil como `\a{\a{…`).
                    if (after + 2 < n && tex.at(after) == QLatin1Char('{')
                        && tex.at(after + 2) == QLatin1Char('}')) {
                        const QString combined =
                            cmd + QLatin1Char('{') + tex.at(after + 1) + QLatin1Char('}');
                        const QString mapped = commandToUnicode(combined);
                        if (mapped != QLatin1Char('\\') + combined) {
                            i = after + 3;  // pasa `{X}`
                            atom = glyph(mapped, font);
                            list.kids.append(attachScripts(tex, i, atom, font));
                            continue;
                        }
                    }
                    atom = glyph(commandToUnicode(cmd), font);
                }
            }
        } else {
            atom = glyph(QString(c), font);
            ++i;
        }
        list.kids.append(attachScripts(tex, i, atom, font));
    }
    return list;
}

// --- Medición ----------------------------------------------------------------

void measure(Box &b)
{
    switch (b.kind) {
    case Box::Glyph: {
        const QFontMetricsF fm(b.font);
        b.w = b.text.isEmpty() ? 0.0 : fm.horizontalAdvance(b.text);
        b.asc = fm.ascent();
        b.dsc = fm.descent();
        break;
    }
    case Box::HList: {
        b.w = b.asc = b.dsc = 0;
        for (Box &k : b.kids) {
            measure(k);
            b.w += k.w;
            b.asc = std::max(b.asc, k.asc);
            b.dsc = std::max(b.dsc, k.dsc);
        }
        if (b.kids.isEmpty()) {
            const QFontMetricsF fm(b.font);
            b.asc = fm.ascent() * 0.5;  // alto mínimo para huecos
        }
        break;
    }
    case Box::Frac: {
        Box &num = b.kids[0];
        Box &den = b.kids[1];
        measure(num);
        measure(den);
        const qreal axis = axisHeight(b.font);
        const qreal gap = fracGap(b.font);
        const qreal pad = fracPad(b.font);
        b.asc = num.height() + gap + axis;
        b.dsc = std::max(0.0, den.height() + gap - axis);
        b.w = std::max(num.w, den.w) + 2 * pad;
        break;
    }
    case Box::Script: {
        Box &base = b.kids[0];
        measure(base);
        b.asc = base.asc;
        b.dsc = base.dsc;
        qreal sw = 0;
        if (b.hasSup) {
            measure(b.kids[1]);
            b.asc = std::max(b.asc, scriptRaise(b.font) + b.kids[1].asc);
            sw = std::max(sw, b.kids[1].w);
        }
        if (b.hasSub) {
            measure(b.kids[2]);
            b.dsc = std::max(b.dsc, scriptDrop(b.font) + b.kids[2].dsc);
            sw = std::max(sw, b.kids[2].w);
        }
        b.w = base.w + sw;
        break;
    }
    case Box::BigOp: {
        Box &op = b.kids[0];
        measure(op);
        const qreal gap = bigOpGap(b.font);
        b.asc = op.asc;
        b.dsc = op.dsc;
        b.w = op.w;
        if (b.hasOver) {
            measure(b.kids[1]);
            b.asc = op.asc + gap + b.kids[1].height();
            b.w = std::max(b.w, b.kids[1].w);
        }
        if (b.hasUnder) {
            measure(b.kids[2]);
            b.dsc = op.dsc + gap + b.kids[2].height();
            b.w = std::max(b.w, b.kids[2].w);
        }
        break;
    }
    case Box::Sqrt: {
        Box &rad = b.kids[0];
        measure(rad);
        const qreal t = barThickness(b.font);
        const qreal gap = sqrtGap(b.font);
        qreal idxW = 0;
        if (b.hasIndex) {
            measure(b.kids[1]);
            idxW = b.kids[1].w;
        }
        // El índice se aloja en el codo del radical (dentro de su altura), así
        // que no aumenta el ascenso.
        b.asc = rad.asc + gap + t;
        b.dsc = rad.dsc;
        b.w = idxW + sqrtSignW(b.font) + rad.w + fracPad(b.font);
        break;
    }
    case Box::Matrix: {
        for (Box &cell : b.kids)
            measure(cell);
        QList<qreal> colW, rowAsc, rowDsc;
        matrixMetrics(b, colW, rowAsc, rowDsc);
        qreal totalW = 0;
        for (qreal w : colW) totalW += w;
        totalW += matrixColGap(b.font) * std::max(0, b.cols - 1);
        qreal totalH = 0;
        for (int r = 0; r < b.rows; ++r) totalH += rowAsc[r] + rowDsc[r];
        totalH += matrixRowGap(b.font) * std::max(0, b.rows - 1);
        const qreal dW = (b.delim != DelimNone) ? (delimWidth(b.font) + delimPad(b.font)) : 0;
        const int sides = b.braceLeftOnly ? 1 : 2;  // cases: solo la llave izquierda
        b.w = totalW + sides * dW;
        const qreal axis = axisHeight(b.font);
        b.asc = totalH / 2 + axis;
        b.dsc = std::max(0.0, totalH / 2 - axis);
        break;
    }
    case Box::Binom: {
        Box &top = b.kids[0];
        Box &bot = b.kids[1];
        measure(top);
        measure(bot);
        const qreal axis = axisHeight(b.font);
        const qreal gap = fracGap(b.font);
        const qreal dW = delimWidth(b.font) + delimPad(b.font);
        b.asc = top.height() + gap + axis;
        b.dsc = std::max(0.0, bot.height() + gap - axis);
        b.w = std::max(top.w, bot.w) + 2 * dW;
        break;
    }
    }
}

// --- Pintado -----------------------------------------------------------------

// Dibuja un delimitador de matriz (paréntesis/corchete/llave/barra) de altura
// `h` desde `top`, en la columna [x, x+delimWidth]. `left` elige la orientación.
void paintDelim(QPainter *p, int delim, bool left, qreal x, qreal top, qreal h, const QFont &font)
{
    const qreal w = delimWidth(font);
    QPen pen = p->pen();
    pen.setWidthF(barThickness(font));
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::MiterJoin);
    p->save();
    p->setPen(pen);
    p->setBrush(Qt::NoBrush);
    const qreal bottom = top + h;
    const qreal mid = top + h / 2;
    if (delim == DelimParen) {
        QPainterPath path;
        if (left) { path.moveTo(x + w, top); path.quadTo(x - w * 0.2, mid, x + w, bottom); }
        else      { path.moveTo(x, top);     path.quadTo(x + w * 1.2, mid, x, bottom); }
        p->drawPath(path);
    } else if (delim == DelimBar) {
        p->drawLine(QPointF(x + w / 2, top), QPointF(x + w / 2, bottom));
    } else if (delim == DelimBrace) {
        // Llave curva con cúspide hacia afuera en el centro.
        const qreal cx = x + w * 0.5;
        const qreal cusp = left ? x : x + w;
        const qreal arm = left ? x + w : x;
        QPainterPath path;
        path.moveTo(arm, top);
        path.quadTo(cx, top, cx, top + h * 0.25);
        path.quadTo(cx, mid, cusp, mid);
        path.quadTo(cx, mid, cx, top + h * 0.75);
        path.quadTo(cx, bottom, arm, bottom);
        p->drawPath(path);
    } else {  // corchete: tres tramos en U
        QPolygonF poly = left
            ? QPolygonF({QPointF(x + w, top), QPointF(x, top), QPointF(x, bottom), QPointF(x + w, bottom)})
            : QPolygonF({QPointF(x, top), QPointF(x + w, top), QPointF(x + w, bottom), QPointF(x, bottom)});
        p->drawPolyline(poly);
    }
    p->restore();
}

void paint(const Box &b, QPainter *p, qreal x, qreal baseline)
{
    switch (b.kind) {
    case Box::Glyph:
        if (!b.text.isEmpty()) {
            p->setFont(b.font);
            p->drawText(QPointF(x, baseline), b.text);
        }
        break;
    case Box::HList: {
        qreal cx = x;
        for (const Box &k : b.kids) {
            paint(k, p, cx, baseline);
            cx += k.w;
        }
        break;
    }
    case Box::Frac: {
        const Box &num = b.kids[0];
        const Box &den = b.kids[1];
        const qreal axis = axisHeight(b.font);
        const qreal gap = fracGap(b.font);
        const qreal pad = fracPad(b.font);
        const qreal barY = baseline - axis;
        paint(num, p, x + (b.w - num.w) / 2, barY - gap - num.dsc);
        paint(den, p, x + (b.w - den.w) / 2, barY + gap + den.asc);
        QPen pen = p->pen();
        pen.setWidthF(barThickness(b.font));
        p->save();
        p->setPen(pen);
        p->drawLine(QPointF(x + pad, barY), QPointF(x + b.w - pad, barY));
        p->restore();
        break;
    }
    case Box::Script: {
        const Box &base = b.kids[0];
        paint(base, p, x, baseline);
        if (b.hasSup)
            paint(b.kids[1], p, x + base.w, baseline - scriptRaise(b.font));
        if (b.hasSub)
            paint(b.kids[2], p, x + base.w, baseline + scriptDrop(b.font));
        break;
    }
    case Box::BigOp: {
        const Box &op = b.kids[0];
        const qreal gap = bigOpGap(b.font);
        paint(op, p, x + (b.w - op.w) / 2, baseline);
        if (b.hasOver) {
            const Box &over = b.kids[1];
            paint(over, p, x + (b.w - over.w) / 2, baseline - op.asc - gap - over.dsc);
        }
        if (b.hasUnder) {
            const Box &under = b.kids[2];
            paint(under, p, x + (b.w - under.w) / 2, baseline + op.dsc + gap + under.asc);
        }
        break;
    }
    case Box::Sqrt: {
        const Box &rad = b.kids[0];
        const qreal t = barThickness(b.font);
        const qreal gap = sqrtGap(b.font);
        const qreal sw = sqrtSignW(b.font);
        const qreal idxW = b.hasIndex ? b.kids[1].w : 0;
        const qreal vinY = baseline - rad.asc - gap;     // y del vínculo (sobre el radicando)
        const qreal bottom = baseline + rad.dsc;          // punto más bajo del radical
        const qreal x0 = x + idxW;
        const qreal radX = x0 + sw + fracPad(b.font) * 0.5;
        QPen pen = p->pen();
        pen.setWidthF(t);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::MiterJoin);
        p->save();
        p->setPen(pen);
        // Radical vectorial: arranque medio → punta inferior → sube al vínculo →
        // vínculo horizontal sobre el radicando. Escala con la altura del radicando.
        p->drawPolyline(QPolygonF({
            QPointF(x0, vinY + (bottom - vinY) * 0.6),
            QPointF(x0 + sw * 0.5, bottom),
            QPointF(x0 + sw, vinY),
            QPointF(radX + rad.w + fracPad(b.font) * 0.5, vinY),
        }));
        p->restore();
        paint(rad, p, radX, baseline);
        if (b.hasIndex)  // en el codo: bajo el vínculo, dentro de la altura del radical
            paint(b.kids[1], p, x, vinY + (bottom - vinY) * 0.30 - b.kids[1].dsc);
        break;
    }
    case Box::Matrix: {
        QList<qreal> colW, rowAsc, rowDsc;
        matrixMetrics(b, colW, rowAsc, rowDsc);
        qreal totalH = 0;
        for (int r = 0; r < b.rows; ++r) totalH += rowAsc[r] + rowDsc[r];
        totalH += matrixRowGap(b.font) * std::max(0, b.rows - 1);
        const qreal gridTop = baseline - axisHeight(b.font) - totalH / 2;
        const qreal dW = (b.delim != DelimNone) ? (delimWidth(b.font) + delimPad(b.font)) : 0;
        if (b.delim != DelimNone) {
            paintDelim(p, b.delim, true, x, gridTop, totalH, b.font);
            if (!b.braceLeftOnly)  // cases: solo la llave izquierda
                paintDelim(p, b.delim, false, x + b.w - delimWidth(b.font), gridTop, totalH, b.font);
        }
        qreal cx = x + dW;
        for (int cc = 0; cc < b.cols; ++cc) {
            qreal cy = gridTop;
            for (int r = 0; r < b.rows; ++r) {
                const Box &cell = b.kids[r * b.cols + cc];
                // cases alinea a la izquierda; las matrices centran en la columna.
                const qreal cellX = b.leftAlign ? cx : cx + (colW[cc] - cell.w) / 2;
                paint(cell, p, cellX, cy + rowAsc[r]);
                cy += rowAsc[r] + rowDsc[r] + matrixRowGap(b.font);
            }
            cx += colW[cc] + matrixColGap(b.font);
        }
        break;
    }
    case Box::Binom: {
        const Box &top = b.kids[0];
        const Box &bot = b.kids[1];
        const qreal axis = axisHeight(b.font);
        const qreal gap = fracGap(b.font);
        const qreal centerY = baseline - axis;
        paint(top, p, x + (b.w - top.w) / 2, centerY - gap - top.dsc);
        paint(bot, p, x + (b.w - bot.w) / 2, centerY + gap + bot.asc);
        paintDelim(p, DelimParen, true, x, baseline - b.asc, b.height(), b.font);
        paintDelim(p, DelimParen, false, x + b.w - delimWidth(b.font), baseline - b.asc, b.height(), b.font);
        break;
    }
    }
}

// Margen lateral/vertical para que las cursivas y barras no se recorten.
constexpr qreal kMargin = 2.0;

} // namespace

bool needsTwoDLayout(const QString &tex)
{
    const int n = tex.size();
    int i = 0;
    while (i < n) {
        if (tex.at(i) != QLatin1Char('\\')) { ++i; continue; }
        ++i;
        if (i >= n) break;
        if (!tex.at(i).isLetter()) { ++i; continue; }
        const QString cmd = readCommand(tex, i);
        if (cmd == QLatin1String("frac") || cmd == QLatin1String("sqrt")
            || cmd == QLatin1String("begin") || cmd == QLatin1String("binom"))
            return true;
        if (isBigOp(cmd)) {
            int after = i;
            while (after < n && tex.at(after) == QLatin1Char(' ')) ++after;
            if (after < n && (tex.at(after) == QLatin1Char('_') || tex.at(after) == QLatin1Char('^')))
                return true;
        }
    }
    return false;
}

QSizeF measureFormula(const QString &tex, const QFont &baseFont)
{
    Box root = buildHList(tex, baseFont);
    measure(root);
    return QSizeF(root.w + 2 * kMargin, root.asc + root.dsc + 2 * kMargin);
}

void paintFormula(QPainter *painter, const QPointF &topLeft, const QString &tex,
                  const QFont &baseFont, const QColor &color)
{
    Box root = buildHList(tex, baseFont);
    measure(root);
    painter->save();
    QPen pen = painter->pen();
    pen.setColor(color);
    painter->setPen(pen);
    paint(root, painter, topLeft.x() + kMargin, topLeft.y() + kMargin + root.asc);
    painter->restore();
}

} // namespace mdmath
