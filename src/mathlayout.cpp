#include "mathlayout.h"

#include <QColor>
#include <QFont>
#include <QFontMetricsF>
#include <QPainter>

#include <algorithm>
#include <cmath>

#include "mathblocks.h"  // commandToUnicode

namespace mdmath {

namespace {

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

// Caja del árbol de maquetación. Según `kind`, `kids` tiene aridad fija:
//   HList  -> ítems en fila.
//   Glyph  -> hoja: `text` con `font`.
//   Frac   -> {numerador, denominador}.
//   Script -> {base, superíndice, subíndice} (presencia en hasSup/hasSub).
//   BigOp  -> {operador, límite-encima, límite-debajo} (hasOver/hasUnder).
// La geometría (w/asc/dsc) la rellena `measure`. `asc`/`dsc` se miden desde el
// baseline (arriba/abajo); height = asc + dsc; `w` es el avance horizontal.
struct Box {
    enum Kind { HList, Glyph, Frac, Script, BigOp };
    Kind kind = HList;
    QString text;
    QFont font;
    QList<Box> kids;
    bool hasSup = false, hasSub = false, hasOver = false, hasUnder = false;
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

Box buildHList(const QString &tex, const QFont &font)
{
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
                atom = glyph(QString(tex.at(i)), font);
                ++i;
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
                } else {
                    // \mathbb{R} y similares: el comando con su grupo puede estar
                    // en la tabla como un único glifo.
                    if (after < n && tex.at(after) == QLatin1Char('{')) {
                        int probe = after;
                        const QString arg = readGroup(tex, probe);
                        const QString combined = cmd + QLatin1Char('{') + arg + QLatin1Char('}');
                        const QString mapped = commandToUnicode(combined);
                        if (mapped != QLatin1Char('\\') + combined) {
                            i = probe;
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
    }
}

// --- Pintado -----------------------------------------------------------------

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
        if (cmd == QLatin1String("frac"))
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
