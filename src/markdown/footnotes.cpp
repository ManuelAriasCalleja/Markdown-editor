/// \file
/// \brief Implementación de las notas al pie: detección, protección de las
///        definiciones y renderizado de las referencias.

#include "footnotes.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "mathblocks.h"

namespace {

// `[^id]` con id sin espacios ni `]`. El id admite letras, dígitos, `-` y `_`,
// que es lo que aceptan las implementaciones habituales de footnotes.
const QRegularExpression &tokenRe()
{
    static const QRegularExpression re(QStringLiteral(R"(\[\^([^\]\s]+)\])"));
    return re;
}

// Centinela que sustituye temporalmente al `:` de una definición durante la
// carga (PUA; distinto de los de math en F8FD..F8FF).
constexpr QChar kColonSentinel(0xF8FB);

}  // namespace

QList<mdfootnote::Footnote> mdfootnote::references(const QString &text)
{
    QList<Footnote> out;
    auto it = tokenRe().globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        // Una definición es `[^id]:`; su rótulo no es una referencia.
        const int after = m.capturedEnd();
        if (after < text.size() && text.at(after) == QLatin1Char(':'))
            continue;
        out.append({int(m.capturedStart()), int(m.capturedLength()), m.captured(1)});
    }
    return out;
}

QList<mdfootnote::Footnote> mdfootnote::definitions(const QString &text)
{
    // `[^id]:` al principio de línea, con hasta 3 espacios de sangría.
    static const QRegularExpression defRe(
        QStringLiteral(R"((?:^|\n)[ ]{0,3}(\[\^([^\]\s]+)\]):)"));
    QList<Footnote> out;
    auto it = defRe.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        // El grupo 1 es el rótulo `[^id]` (sin los `:`); longitud +1 para incluir
        // los dos puntos en el token de la definición.
        out.append({int(m.capturedStart(1)), int(m.capturedLength(1)) + 1, m.captured(2)});
    }
    return out;
}

QString mdfootnote::protectFootnotes(const QString &markdown)
{
    const QList<Footnote> defs = definitions(markdown);
    if (defs.isEmpty())
        return markdown;
    QString out = markdown;
    for (const Footnote &d : defs) {
        // El token de la definición incluye el `:` final (su último carácter).
        const int colonPos = d.start + d.length - 1;
        if (colonPos >= 0 && colonPos < out.size() && out.at(colonPos) == QLatin1Char(':'))
            out[colonPos] = kColonSentinel;
    }
    return out;
}

QString mdfootnote::nextId(const QString &markdown)
{
    int maxId = 0;
    auto it = tokenRe().globalMatch(markdown);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        bool ok = false;
        const int n = m.captured(1).toInt(&ok);
        if (ok && n > maxId)
            maxId = n;
    }
    return QString::number(maxId + 1);
}

QTextCharFormat mdfootnote::refFormat(const QString &id)
{
    QTextCharFormat f;
    f.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
    f.setProperty(IsFootnoteRefProperty, true);
    f.setProperty(FootnoteIdProperty, id);
    return f;
}

void mdfootnote::renderFootnotesInDocument(QTextDocument *doc)
{
    if (!doc)
        return;
    // Restaura los `:` de las definiciones que protectFootnotes() sustituyó por el
    // centinela antes de setMarkdown(). Se hace primero para que el texto de los
    // bloques quede ya canónico al buscar referencias.
    for (QTextCursor c = doc->find(QString(kColonSentinel)); !c.isNull();
         c = doc->find(QString(kColonSentinel), c))
        c.insertText(QStringLiteral(":"));

    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        const QString text = block.text();
        const QList<Footnote> refs = references(text);
        if (refs.isEmpty())
            continue;
        const int base = block.position();
        for (const Footnote &r : refs) {
            // No tocar referencias dentro de código o de una fórmula: se mira el
            // formato del primer carácter del token.
            QTextCursor probe(doc);
            probe.setPosition(base + r.start + 1);
            const QTextCharFormat at = probe.charFormat();
            if (at.fontFixedPitch() || at.boolProperty(mdmath::IsMathProperty))
                continue;
            if (at.boolProperty(IsFootnoteRefProperty))
                continue;  // ya renderizada (idempotencia)

            QTextCursor cur(doc);
            cur.setPosition(base + r.start);
            cur.setPosition(base + r.start + r.length, QTextCursor::KeepAnchor);
            cur.mergeCharFormat(refFormat(r.id));
        }
    }
}

int mdfootnote::definitionBlockNumber(const QTextDocument *doc, const QString &id)
{
    if (!doc)
        return -1;
    for (QTextBlock block = doc->begin(); block != doc->end(); block = block.next()) {
        const QList<Footnote> defs = definitions(block.text());
        for (const Footnote &d : defs)
            if (d.id == id)
                return block.blockNumber();
    }
    return -1;
}
