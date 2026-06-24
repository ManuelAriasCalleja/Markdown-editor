/// \file
/// \brief Implementación del cálculo de estadísticas del documento.

#include "docstats.h"

#include <QChar>
#include <QRegularExpression>
#include <QStringList>
#include <QStringView>

mdstats::DocStats mdstats::analyze(const QString &input, int wordsPerMinute)
{
    if (wordsPerMinute <= 0)
        wordsPerMinute = 200;

    // selectedText() y los párrafos de Qt usan U+2029 como separador; lo
    // normalizamos a '\n' para contar líneas y caracteres igual que el editor.
    QString text = input;
    text.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));

    DocStats st;
    st.chars = static_cast<int>(text.size());

    static const QRegularExpression whitespace(QStringLiteral("\\s+"));
    st.words = static_cast<int>(text.split(whitespace, Qt::SkipEmptyParts).size());

    int nonSpace = 0;
    for (const QChar &c : text)
        if (!c.isSpace())
            ++nonSpace;
    st.charsNoSpaces = nonSpace;

    // Párrafos: líneas con algún carácter no blanco (las vacías no cuentan).
    int paragraphs = 0;
    for (QStringView line : QStringView(text).split(QLatin1Char('\n')))
        if (!line.trimmed().isEmpty())
            ++paragraphs;
    st.paragraphs = paragraphs;

    // Frases: cada grupo consecutivo de signos de fin de frase ( . ! ? … ) cuenta
    // una vez, de modo que «¿…?», «...» o «!?» no inflan el recuento.
    int sentences = 0;
    bool inTerminator = false;
    for (const QChar &c : text) {
        const bool term = c == QLatin1Char('.') || c == QLatin1Char('!')
                          || c == QLatin1Char('?') || c == QChar(0x2026);  // …
        if (term && !inTerminator)
            ++sentences;
        inTerminator = term;
    }
    st.sentences = sentences;

    st.readingMinutes = static_cast<double>(st.words) / wordsPerMinute;
    return st;
}
