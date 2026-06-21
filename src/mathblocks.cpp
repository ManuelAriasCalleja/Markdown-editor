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
        QString content = s.content;
        // Codifica saltos para que el inline-code resultante quepa en una
        // sola línea de Markdown (requisito para que setMarkdown lo trate
        // como código en línea).
        content.replace(QLatin1Char('\n'), kNewlinePlaceholder);
        out += QLatin1String("``");
        out += wrapTex(content, s.block);
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

QTextCharFormat mathObjectFormat(const QString &tex, bool block)
{
    QTextCharFormat fmt;
    fmt.setProperty(IsMathProperty, true);
    fmt.setProperty(MathTexProperty, tex);
    fmt.setProperty(MathBlockProperty, block);
    fmt.setObjectType(MathObjectType);
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
        out += wrapTex(entry.first, entry.second);
        last = m.capturedEnd();
    }
    out += QStringView{markdown}.mid(last);
    return out;
}

void unrenderMathInDocument(QTextDocument *doc)
{
    QList<Repl> repls;
    forEachMathGroup(doc, [&](int start, int end, const QString &tex, bool isBlock) {
        QTextCharFormat code;
        code.setFontFixedPitch(true);
        repls.append({start, end, {{wrapTex(tex, isBlock), code}}});
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
