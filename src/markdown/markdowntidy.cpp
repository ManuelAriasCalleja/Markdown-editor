/// \file
/// \brief Implementación de la limpieza/normalización del texto Markdown.

#include "markdowntidy.h"

#include <QRegularExpression>
#include <QStringList>

namespace mdtidy {

namespace {

// Quita espacios y tabuladores finales.
QString rstripPlain(const QString &s)
{
    int i = s.size();
    while (i > 0 && (s.at(i - 1) == QLatin1Char(' ') || s.at(i - 1) == QLatin1Char('\t')))
        --i;
    return s.left(i);
}

// Como rstripPlain, pero preserva el salto de línea duro de Markdown: una línea
// con contenido que acaba en 2+ espacios se deja con exactamente dos.
QString rstripKeepHardBreak(const QString &s)
{
    QString stripped = rstripPlain(s);
    if (stripped.isEmpty())
        return QString();
    if (s.size() - stripped.size() >= 2 && s.at(s.size() - 1) == QLatin1Char(' ')
        && s.at(s.size() - 2) == QLatin1Char(' '))
        return stripped + QLatin1String("  ");
    return stripped;
}

} // namespace

QString tidy(const QString &markdown)
{
    QString text = markdown;
    text.replace(QLatin1String("\r\n"), QLatin1String("\n"));
    text.replace(QLatin1Char('\r'), QLatin1Char('\n'));

    static const QRegularExpression fenceRe(QStringLiteral("^[ \\t]*(`{3,}|~{3,})"));
    static const QRegularExpression headingRe(QStringLiteral("^(#{1,6})[ \\t]+(.*)$"));
    static const QRegularExpression bulletRe(QStringLiteral("^([ \\t]*)[*+\\-][ \\t]+(\\S.*)$"));
    static const QRegularExpression thematicRe(
        QStringLiteral("^[ \\t]*([*\\-_])([ \\t]*\\1){2,}[ \\t]*$"));

    QStringList out;
    QChar fenceChar;       // nulo = fuera de un bloque de código cercado
    bool lastBlank = false;

    const QStringList lines = text.split(QLatin1Char('\n'));
    for (const QString &raw : lines) {
        const QRegularExpressionMatch fm = fenceRe.match(raw);
        if (fm.hasMatch()) {
            const QChar c = fm.captured(1).at(0);
            if (fenceChar.isNull()) {
                fenceChar = c;             // abre el fence
            } else if (c == fenceChar) {
                fenceChar = QChar();       // lo cierra (mismo carácter)
            } else {
                out << raw;                // otro carácter dentro del fence: es código
                lastBlank = false;
                continue;
            }
            out << rstripPlain(raw);
            lastBlank = false;
            continue;
        }
        if (!fenceChar.isNull()) {
            out << raw;                    // contenido de código: intacto
            lastBlank = false;
            continue;
        }

        const QString line = rstripKeepHardBreak(raw);
        if (line.isEmpty()) {
            if (lastBlank)
                continue;                  // colapsa líneas en blanco consecutivas
            out << QString();
            lastBlank = true;
            continue;
        }
        lastBlank = false;

        if (thematicRe.match(line).hasMatch()) {
            out << line;                   // regla / separador temático: no tocar
            continue;
        }
        const QRegularExpressionMatch hm = headingRe.match(line);
        if (hm.hasMatch()) {
            out << hm.captured(1) + QLatin1Char(' ') + hm.captured(2);  // un espacio tras #
            continue;
        }
        const QRegularExpressionMatch bm = bulletRe.match(line);
        if (bm.hasMatch()) {
            out << bm.captured(1) + QLatin1String("- ") + bm.captured(2);  // viñeta uniforme
            continue;
        }
        out << line;
    }

    while (!out.isEmpty() && out.first().isEmpty())
        out.removeFirst();
    while (!out.isEmpty() && out.last().isEmpty())
        out.removeLast();
    if (out.isEmpty())
        return QString();
    return out.join(QLatin1Char('\n')) + QLatin1Char('\n');
}

} // namespace mdtidy
