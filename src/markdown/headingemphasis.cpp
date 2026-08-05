/// \file
/// \brief Implementación del énfasis dentro de los encabezados.

#include "headingemphasis.h"

#include <algorithm>

#include <QFont>
#include <QList>
#include <QPair>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

namespace mdheademph {
namespace {

// Centinelas PUA, distintos de los de las fórmulas (0xF8FD-0xF8FF) y los del
// super/subíndice (0xF8F0-0xF8F3). Son texto opaco: toMarkdown no los escapa.
constexpr QChar kBoldOpen(0xF8E0);
constexpr QChar kBoldClose(0xF8E1);
constexpr QChar kItalicOpen(0xF8E2);
constexpr QChar kItalicClose(0xF8E3);
constexpr QChar kStrikeOpen(0xF8E4);
constexpr QChar kStrikeClose(0xF8E5);

// Los fragmentos que no son texto (una imagen, el carácter objeto de una fórmula 2D)
// no llevan énfasis que emitir.
bool isTextFragment(const QTextCharFormat &cf)
{
    return !cf.isImageFormat() && cf.objectType() == QTextFormat::NoObject;
}

// ¿La negrita de este run viene del `**` del fuente? Solo vale recién importado: el
// run estructural del encabezado trae el ajuste de tamaño del nivel, y el span de
// énfasis no. Después de repairHeadingRuns todos lo llevan y ya no se distinguen.
bool isImportedBoldRun(const QTextCharFormat &cf)
{
    return cf.fontWeight() >= QFont::Bold
           && !cf.hasProperty(QTextFormat::FontSizeAdjustment);
}

struct Style
{
    bool bold = false;
    bool italic = false;
    bool strike = false;

    bool isPlain() const { return !bold && !italic && !strike; }
    bool operator==(const Style &o) const
    {
        return bold == o.bold && italic == o.italic && strike == o.strike;
    }
};

Style styleOf(const QTextCharFormat &cf)
{
    Style s;
    // La negrita se toma de la marca de carga, no del peso: todo el encabezado está en
    // negrita. Se exige además que siga en negrita, para no emitir `**` sobre un run
    // al que el usuario se la haya quitado.
    s.bold = cf.boolProperty(ExplicitBoldProperty) && cf.fontWeight() >= QFont::Bold;
    s.italic = cf.fontItalic();
    s.strike = cf.fontStrikeOut();
    return s;
}

struct Run
{
    int start;
    int end;
    Style style;
    QString text;
};

// Encoge el run hasta su núcleo sin espacios: `*x *` no es énfasis para un lector de
// Markdown (la marca de cierre tiene que ir pegada al texto).
void trimToCore(Run &r)
{
    int lead = 0;
    while (lead < r.text.size() && r.text.at(lead).isSpace())
        ++lead;
    int trail = 0;
    while (trail < r.text.size() - lead && r.text.at(r.text.size() - 1 - trail).isSpace())
        ++trail;
    r.start += lead;
    r.end -= trail;
}

}  // namespace

void markExplicitBold(QTextDocument *doc)
{
    if (!doc)
        return;
    // Se recogen los tramos antes de tocar nada: cambiar el formato parte y fusiona
    // fragmentos, y mutar mientras se recorre el bloque invalidaría el iterador.
    QList<QPair<int, int>> spans;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().headingLevel() < 1)
            continue;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || frag.length() == 0)
                continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!isTextFragment(cf) || !isImportedBoldRun(cf))
                continue;
            spans.append({frag.position(), frag.position() + frag.length()});
        }
    }
    if (spans.isEmpty())
        return;
    QTextCharFormat mark;
    mark.setProperty(ExplicitBoldProperty, true);
    QTextCursor c(doc);
    c.beginEditBlock();
    for (const auto &s : spans) {
        c.setPosition(s.first);
        c.setPosition(s.second, QTextCursor::KeepAnchor);
        c.mergeCharFormat(mark);
    }
    c.endEditBlock();
}

void replaceWithSentinels(QTextDocument *doc)
{
    if (!doc)
        return;
    QList<Run> runs;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        if (b.blockFormat().headingLevel() < 1)
            continue;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || frag.length() == 0)
                continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!isTextFragment(cf))
                continue;
            const Style s = styleOf(cf);
            if (s.isPlain())
                continue;
            // Contiguo y del mismo énfasis: un solo par de marcas para los dos (un
            // `**enlace****y más**` no lo lee como negrita ningún parser).
            if (!runs.isEmpty() && runs.last().end == frag.position()
                && runs.last().style == s) {
                runs.last().end += frag.length();
                runs.last().text += frag.text();
                continue;
            }
            runs.append({frag.position(), frag.position() + frag.length(), s, frag.text()});
        }
    }

    for (Run &r : runs)
        trimToCore(r);
    runs.erase(std::remove_if(runs.begin(), runs.end(),
                              [](const Run &r) { return r.start >= r.end; }),
               runs.end());
    if (runs.isEmpty())
        return;

    // De atrás hacia delante: insertar no invalida las posiciones anteriores.
    std::sort(runs.begin(), runs.end(),
              [](const Run &a, const Run &b) { return a.start > b.start; });
    QTextCursor c(doc);
    c.beginEditBlock();
    for (const Run &r : runs) {
        // Anidamiento fijo: tachado fuera, luego negrita, cursiva dentro.
        QString open;
        QString close;
        if (r.style.strike)
            open += kStrikeOpen;
        if (r.style.bold)
            open += kBoldOpen;
        if (r.style.italic) {
            open += kItalicOpen;
            close += kItalicClose;
        }
        if (r.style.bold)
            close += kBoldClose;
        if (r.style.strike)
            close += kStrikeClose;
        // En formato plano: dentro de un span de código Qt escaparía las marcas.
        c.setPosition(r.end);
        c.insertText(close, QTextCharFormat());
        c.setPosition(r.start);
        c.insertText(open, QTextCharFormat());
    }
    c.endEditBlock();
}

QString restoreFromSentinels(const QString &markdown)
{
    QString out = markdown;
    out.replace(kBoldOpen, QLatin1String("**"));
    out.replace(kBoldClose, QLatin1String("**"));
    out.replace(kItalicOpen, QLatin1String("*"));
    out.replace(kItalicClose, QLatin1String("*"));
    out.replace(kStrikeOpen, QLatin1String("~~"));
    out.replace(kStrikeClose, QLatin1String("~~"));
    return out;
}

}  // namespace mdheademph
