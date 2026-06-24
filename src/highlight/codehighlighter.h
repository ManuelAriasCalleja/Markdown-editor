#ifndef CODEHIGHLIGHTER_H
#define CODEHIGHLIGHTER_H

/// \file
/// \brief Resaltador de sintaxis para bloques de código, fragmentos de fórmula y ortografía.

#include "languageregistry.h"
#include "themespec.h"

#include <QColor>
#include <QHash>
#include <QList>
#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class SpellChecker;

/// \brief Resaltador de sintaxis que actúa sobre dos cosas del documento:
///   1) Bloques de código (los que tienen la propiedad QTextFormat::BlockCodeFence).
///   2) Fragmentos de fórmula (los marcados con mdmath::IsMathProperty), a los
///      que aplica el color matemático del tema. Una sola instancia los pinta
///      porque QTextDocument solo admite un QSyntaxHighlighter por documento.
///
/// Los formatos de un QSyntaxHighlighter son de presentación (capa de la
/// maqueta), así que el resaltado no se guarda en el Markdown: toMarkdown()
/// sigue produciendo el código (y el TeX) tal cual.
///
/// El lenguaje de cada bloque se lee de QTextFormat::BlockCodeLanguage (lo
/// rellena setMarkdown a partir de ```cpp, ```python, etc.).
class CodeBlockHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit CodeBlockHighlighter(QTextDocument *parent);

    /// \brief Ajusta la paleta de colores del resaltado (la provee el tema) y vuelve a
    /// resaltar.
    void setSyntaxColors(const mdtheme::SyntaxColors &colors);

    /// \brief Motor de corrección ortográfica (propiedad del llamador). Si es nullptr o
    /// no tiene diccionario cargado, no se subraya nada. Tras cambiarlo (o cambiar
    /// de idioma/diccionario), el llamador debe llamar a `rehighlight()`.
    void setSpellChecker(SpellChecker *checker);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    // Reglas compiladas (cacheadas) para un lenguaje, según su LangSpec.
    const QList<Rule> &rulesFor(const QString &language, const LangSpec &spec);
    void highlightMultilineComments(const QString &text);
    // Aplica el color matemático a las posiciones del bloque ocupadas por
    // fragmentos con IsMathProperty.
    void highlightMathFragments();
    // Subraya las palabras mal escritas del bloque (solo en prosa), saltando
    // fragmentos de código en línea, fórmulas y enlaces.
    void highlightSpelling();

    QColor m_keywordColor;
    QColor m_stringColor;
    QColor m_commentColor;
    QColor m_numberColor;
    QColor m_mathColor;
    QColor m_misspellColor{0xd0, 0x30, 0x30};  // rojo del subrayado ortográfico

    SpellChecker *m_spell = nullptr;
    QHash<QString, QList<Rule>> m_cache;  // reglas compiladas por lenguaje
};

#endif // CODEHIGHLIGHTER_H
