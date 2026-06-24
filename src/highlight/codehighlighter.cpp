/// \file
/// \brief Implementación del resaltador de código, fórmulas y ortografía (CodeBlockHighlighter).

#include "codehighlighter.h"

#include "mathblocks.h"
#include "spellchecker.h"
#include "spellscan.h"

#include <QFont>
#include <QTextBlock>
#include <QTextFragment>

CodeBlockHighlighter::CodeBlockHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // Colores iniciales del tema claro; el tema real los reajusta al aplicarse.
    setSyntaxColors(mdtheme::specFor(mdtheme::ThemeId::Light).syntax);
}

void CodeBlockHighlighter::setSpellChecker(SpellChecker *checker)
{
    m_spell = checker;
}

void CodeBlockHighlighter::setSyntaxColors(const mdtheme::SyntaxColors &colors)
{
    m_keywordColor = colors.keyword;
    m_stringColor = colors.string;
    m_commentColor = colors.comment;
    m_numberColor = colors.number;
    m_mathColor = colors.math;
    m_cache.clear();  // los colores de las reglas cambian con el tema
    rehighlight();
}

const QList<CodeBlockHighlighter::Rule> &
CodeBlockHighlighter::rulesFor(const QString &language, const LangSpec &spec)
{
    const QString key = language.trimmed().toLower();
    const auto cached = m_cache.constFind(key);
    if (cached != m_cache.constEnd())
        return cached.value();

    QList<Rule> rules;

    if (!spec.keywords.isEmpty()) {
        QTextCharFormat kw;
        kw.setForeground(m_keywordColor);
        kw.setFontWeight(QFont::Bold);
        rules.append({QRegularExpression(
                          QStringLiteral("\\b(%1)\\b").arg(spec.keywords.join('|'))),
                      kw});
    }

    QTextCharFormat number;
    number.setForeground(m_numberColor);
    rules.append({QRegularExpression(QStringLiteral("\\b[0-9][0-9._a-fA-FxXeE]*\\b")),
                  number});

    QTextCharFormat string;
    string.setForeground(m_stringColor);
    rules.append({QRegularExpression(QStringLiteral("\"([^\"\\\\]|\\\\.)*\"")), string});
    rules.append({QRegularExpression(QStringLiteral("'([^'\\\\]|\\\\.)*'")), string});

    QTextCharFormat comment;
    comment.setForeground(m_commentColor);
    comment.setFontItalic(true);
    if (spec.slash)
        rules.append({QRegularExpression(QStringLiteral("//[^\n]*")), comment});
    if (spec.hash)
        rules.append({QRegularExpression(QStringLiteral("#[^\n]*")), comment});

    return *m_cache.insert(key, rules);
}

void CodeBlockHighlighter::highlightBlock(const QString &text)
{
    const QTextBlockFormat bf = currentBlock().blockFormat();

    // Fuera de bloques de código: pasamos a colorear fragmentos de fórmula
    // (si los hay) y nos vamos. El estado de bloque vuelve a -1.
    if (!bf.hasProperty(QTextFormat::BlockCodeFence)) {
        setCurrentBlockState(-1);
        highlightMathFragments();
        highlightSpelling();
        return;
    }

    const QString language = bf.stringProperty(QTextFormat::BlockCodeLanguage);
    const LangSpec spec = LanguageRegistry::specFor(language);
    const QList<Rule> &rules = rulesFor(language, spec);

    for (const Rule &rule : rules) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            const QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // Comentarios /* */ (pueden abarcar varias líneas dentro del bloque).
    if (spec.block)
        highlightMultilineComments(text);
    else
        setCurrentBlockState(0);
}

void CodeBlockHighlighter::highlightMathFragments()
{
    if (!m_mathColor.isValid())
        return;
    const QTextBlock block = currentBlock();
    const int basePos = block.position();
    QTextCharFormat fmt;
    fmt.setForeground(m_mathColor);
    for (auto it = block.begin(); it != block.end(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid())
            continue;
        if (!frag.charFormat().boolProperty(mdmath::IsMathProperty))
            continue;
        // setFormat solo merge-a el foreground: la cursiva, vertical-align y
        // demás del fragmento se conservan.
        setFormat(frag.position() - basePos, frag.length(), fmt);
    }
}

void CodeBlockHighlighter::highlightSpelling()
{
    if (!m_spell || !m_spell->isAvailable())
        return;  // sin diccionario no se subraya nada (coste cero)
    const QTextBlock block = currentBlock();
    const int basePos = block.position();
    QTextCharFormat underline;
    underline.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
    underline.setUnderlineColor(m_misspellColor);
    // Recorremos fragmentos para saltar lo que no es prosa: código en línea
    // (monoespaciado), fórmulas (IsMathProperty) y enlaces (anchor). El subrayado
    // es de la capa de presentación: no toca el Markdown.
    for (auto it = block.begin(); it != block.end(); ++it) {
        const QTextFragment frag = it.fragment();
        if (!frag.isValid())
            continue;
        const QTextCharFormat cf = frag.charFormat();
        if (cf.fontFixedPitch() || cf.boolProperty(mdmath::IsMathProperty) || cf.isAnchor())
            continue;
        const QString text = frag.text();
        const int offset = frag.position() - basePos;
        for (const mdspell::Word &w : mdspell::tokenize(text)) {
            if (!m_spell->isCorrect(text.mid(w.start, w.length)))
                setFormat(offset + w.start, w.length, underline);
        }
    }
}

void CodeBlockHighlighter::highlightMultilineComments(const QString &text)
{
    static const QRegularExpression startExpr(QStringLiteral("/\\*"));
    static const QRegularExpression endExpr(QStringLiteral("\\*/"));

    QTextCharFormat comment;
    comment.setForeground(m_commentColor);
    comment.setFontItalic(true);

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(startExpr);

    while (startIndex >= 0) {
        const QRegularExpressionMatch endMatch = endExpr.match(text, startIndex);
        const int endIndex = endMatch.capturedStart();
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);  // el comentario sigue en el bloque siguiente
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, comment);
        startIndex = text.indexOf(startExpr, startIndex + commentLength);
    }
}
