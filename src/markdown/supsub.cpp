/// \file
/// \brief Implementación del super/subíndice de texto `^x^` / `~x~`.

#include "supsub.h"

#include <algorithm>

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include "mathblocks.h"  // mdmath::IsMathProperty (no aplicar sobre fórmulas)

namespace mdsupsub {
namespace {

// Centinelas PUA (distintos de los de math, 0xF8FE/0xF8FF/0xF8FD): abren/cierran el
// interior del super/subíndice mientras el texto pasa por md4c / toMarkdown.
constexpr QChar kSupOpen(0xF8F0);
constexpr QChar kSupClose(0xF8F1);
constexpr QChar kSubOpen(0xF8F2);
constexpr QChar kSubClose(0xF8F3);

// Formato de carácter de la posición `pos` del documento (mirando su fragmento),
// para saber si el tramo cae en código en línea o en una fórmula.
QTextCharFormat formatAt(const QTextDocument *doc, int pos)
{
    const QTextBlock b = doc->findBlock(pos);
    for (auto it = b.begin(); it != b.end(); ++it) {
        const QTextFragment f = it.fragment();
        if (f.isValid() && f.position() <= pos && pos < f.position() + f.length())
            return f.charFormat();
    }
    return {};
}

// Aplica las marcas de super/subíndice a un tramo de TEXTO (nunca código).
QString applyMarks(QString s)
{
    // Superíndice `^x^`: interior no vacío, sin espacios ni `^`.
    static const QRegularExpression supRe(QStringLiteral("\\^([^\\^\\s]+)\\^"));
    s.replace(supRe, QString(kSupOpen) + QStringLiteral("\\1") + QString(kSupClose));
    // Subíndice `~x~`: una sola `~` (ni antes ni después), interior sin espacios ni `~`.
    static const QRegularExpression subRe(QStringLiteral("(?<!~)~([^~\\s]+)~(?!~)"));
    s.replace(subRe, QString(kSubOpen) + QStringLiteral("\\1") + QString(kSubClose));
    return s;
}

// Índice tras la carrera de exactamente `ticks` acentos graves que cierra un span de
// código en línea abierto en `from`, o -1 si no cierra en la línea.
int closingBackticks(const QString &s, int from, int ticks)
{
    int i = from;
    const int n = s.size();
    while (i < n) {
        if (s.at(i) == QLatin1Char('`')) {
            int j = i;
            while (j < n && s.at(j) == QLatin1Char('`'))
                ++j;
            if (j - i == ticks)
                return j;
            i = j;
        } else {
            ++i;
        }
    }
    return -1;
}

// Aplica las marcas fuera de los tramos de código en línea (acentos graves) de una
// línea que no está dentro de un bloque vallado.
QString protectOutsideInlineCode(const QString &line)
{
    QString out;
    int i = 0;
    const int n = line.size();
    while (i < n) {
        if (line.at(i) == QLatin1Char('`')) {
            int j = i;
            while (j < n && line.at(j) == QLatin1Char('`'))
                ++j;
            const int ticks = j - i;
            const int close = closingBackticks(line, j, ticks);
            if (close < 0) {
                // Sin cierre: los acentos graves son texto literal; sigue tras ellos.
                out += line.mid(i, ticks);
                i = j;
                continue;
            }
            out += line.mid(i, close - i);  // el span de código, verbatim
            i = close;
        } else {
            int j = i;
            while (j < n && line.at(j) != QLatin1Char('`'))
                ++j;
            out += applyMarks(line.mid(i, j - i));
            i = j;
        }
    }
    return out;
}

}  // namespace

QString protect(const QString &markdown)
{
    // Aplica el super/subíndice SOLO fuera de código: bloques vallados (``` … ```) y
    // tramos de código en línea (`…`, ``…``) se copian tal cual. Las fórmulas ya
    // vienen envueltas en código en línea por mdmath::protectMath (que corre antes en
    // el pipeline de mdrender), así que esto también las respeta: si no, un `^…^`
    // podría emparejarse a través de la fórmula (p. ej. en `\pi^2}{GM}\,a^3`, el `^`
    // de `\pi` con el de `a`) y corromper su TeX.
    const QStringList lines = markdown.split(QLatin1Char('\n'));
    QString out;
    bool inFence = false;
    QString fenceToken;  // ``` o ~~~ que abrió la valla
    for (int i = 0; i < lines.size(); ++i) {
        if (i)
            out += QLatin1Char('\n');
        const QString &line = lines.at(i);
        const QString trimmed = line.trimmed();
        const bool fenceLine = trimmed.startsWith(QLatin1String("```"))
                               || trimmed.startsWith(QLatin1String("~~~"));
        if (inFence) {
            out += line;  // dentro de la valla: verbatim
            if (fenceLine && trimmed.startsWith(fenceToken))
                inFence = false;
            continue;
        }
        if (fenceLine) {
            inFence = true;
            fenceToken = trimmed.startsWith(QLatin1String("```")) ? QStringLiteral("```")
                                                                  : QStringLiteral("~~~");
            out += line;
            continue;
        }
        out += protectOutsideInlineCode(line);
    }
    return out;
}

void renderInDocument(QTextDocument *doc)
{
    if (!doc)
        return;

    struct Hit
    {
        int start;
        int length;
        QString inner;
        bool sup;
    };
    QList<Hit> hits;
    const QString text = doc->toPlainText();
    const auto scan = [&](const QRegularExpression &re, bool sup) {
        QRegularExpressionMatchIterator it = re.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch m = it.next();
            hits.append({int(m.capturedStart()), int(m.capturedLength()), m.captured(1), sup});
        }
    };
    static const QRegularExpression supRe(QStringLiteral("[\\x{F8F0}]([^\\x{F8F1}]*)[\\x{F8F1}]"));
    static const QRegularExpression subRe(QStringLiteral("[\\x{F8F2}]([^\\x{F8F3}]*)[\\x{F8F3}]"));
    scan(supRe, true);
    scan(subRe, false);

    // De atrás hacia delante: cada reemplazo no invalida las posiciones anteriores.
    std::sort(hits.begin(), hits.end(), [](const Hit &a, const Hit &b) { return a.start > b.start; });

    for (const Hit &h : hits) {
        QTextCursor c(doc);
        c.setPosition(h.start);
        c.setPosition(h.start + h.length, QTextCursor::KeepAnchor);
        // En código (bloque o span) o dentro de una fórmula: NO aplicar formato;
        // restaurar el texto literal `^x^`/`~x~` (así el código se ve intacto y
        // el round-trip lo conserva).
        const bool inCodeBlock =
            doc->findBlock(h.start).blockFormat().hasProperty(QTextFormat::BlockCodeFence);
        const QTextCharFormat inner = formatAt(doc, h.start + 1);  // formato del interior
        const bool inCodeOrMath =
            inCodeBlock || inner.fontFixedPitch() || inner.boolProperty(mdmath::IsMathProperty);
        if (inCodeOrMath) {
            const QChar mark = h.sup ? QLatin1Char('^') : QLatin1Char('~');
            c.insertText(QString(mark) + h.inner + QString(mark));
            continue;
        }
        QTextCharFormat fmt;
        fmt.setVerticalAlignment(h.sup ? QTextCharFormat::AlignSuperScript
                                       : QTextCharFormat::AlignSubScript);
        fmt.setProperty(SupSubProperty, true);
        c.insertText(h.inner, fmt);
    }
}

void replaceWithSentinels(QTextDocument *doc)
{
    if (!doc)
        return;
    struct Run
    {
        int start;
        int end;
        QString text;
        bool sup;
    };
    QList<Run> runs;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment f = it.fragment();
            if (!f.isValid() || !f.charFormat().boolProperty(SupSubProperty))
                continue;
            const bool sup =
                f.charFormat().verticalAlignment() == QTextCharFormat::AlignSuperScript;
            runs.append({f.position(), f.position() + f.length(), f.text(), sup});
        }
    }
    std::sort(runs.begin(), runs.end(), [](const Run &a, const Run &b) { return a.start > b.start; });
    for (const Run &r : runs) {
        QTextCursor c(doc);
        c.setPosition(r.start);
        c.setPosition(r.end, QTextCursor::KeepAnchor);
        const QChar open = r.sup ? kSupOpen : kSubOpen;
        const QChar close = r.sup ? kSupClose : kSubClose;
        // Texto plano: los centinelas no deben quedar dentro de un span que Qt escape.
        c.insertText(QString(open) + r.text + QString(close), QTextCharFormat());
    }
}

QString restoreFromSentinels(const QString &markdown)
{
    QString out = markdown;
    out.replace(kSupOpen, QLatin1Char('^'));
    out.replace(kSupClose, QLatin1Char('^'));
    out.replace(kSubOpen, QLatin1Char('~'));
    out.replace(kSubClose, QLatin1Char('~'));
    return out;
}

}  // namespace mdsupsub
