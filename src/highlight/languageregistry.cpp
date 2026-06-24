/// \file
/// \brief Implementación del registro de lenguajes: tabla alias -> LangSpec.

#include "languageregistry.h"

#include <QHash>

namespace {

// Palabras clave de la familia C (válidas en buena medida para C/C++, Java,
// C#, Go, Rust, Swift, PHP...).
const QStringList cKeywords = {
    "auto", "bool", "break", "case", "catch", "char", "class", "const",
    "constexpr", "continue", "default", "delete", "do", "double", "else",
    "enum", "explicit", "export", "extern", "false", "final", "float",
    "for", "friend", "func", "go", "goto", "if", "import", "inline", "int",
    "interface", "let", "long", "namespace", "new", "noexcept", "nullptr",
    "operator", "override", "package", "private", "protected", "public",
    "return", "short", "signed", "sizeof", "static", "struct", "switch",
    "template", "this", "throw", "true", "try", "typedef", "typename",
    "union", "unsigned", "using", "var", "virtual", "void", "volatile",
    "while"};
const QStringList jsKeywords = {
    "async", "await", "break", "case", "catch", "class", "const",
    "continue", "debugger", "default", "delete", "do", "else", "export",
    "extends", "false", "finally", "for", "from", "function", "if",
    "import", "in", "instanceof", "let", "new", "null", "of", "return",
    "super", "switch", "this", "throw", "true", "try", "typeof",
    "undefined", "var", "void", "while", "yield"};
const QStringList pyKeywords = {
    "and", "as", "assert", "async", "await", "break", "class", "continue",
    "def", "del", "elif", "else", "except", "False", "finally", "for",
    "from", "global", "if", "import", "in", "is", "lambda", "None",
    "nonlocal", "not", "or", "pass", "raise", "return", "True", "try",
    "while", "with", "yield"};

// Familias de sintaxis. Varios alias comparten familia (y por tanto spec).
enum class Family { C, Js, Python, Hash, Generic };

// Alias de lenguaje → familia. Lo que no esté aquí cae en Generic.
const QHash<QString, Family> &aliasTable()
{
    static const QHash<QString, Family> table = {
        // Familia C y afines (comentarios // y /* */).
        {"cpp", Family::C}, {"c", Family::C}, {"c++", Family::C},
        {"cc", Family::C}, {"cxx", Family::C}, {"h", Family::C},
        {"hpp", Family::C}, {"java", Family::C}, {"cs", Family::C},
        {"csharp", Family::C}, {"go", Family::C}, {"golang", Family::C},
        {"rust", Family::C}, {"rs", Family::C}, {"swift", Family::C},
        {"kotlin", Family::C}, {"kt", Family::C}, {"php", Family::C},
        {"scala", Family::C}, {"dart", Family::C},
        // Familia JavaScript/TypeScript/JSON.
        {"js", Family::Js}, {"javascript", Family::Js}, {"ts", Family::Js},
        {"typescript", Family::Js}, {"jsx", Family::Js}, {"tsx", Family::Js},
        {"json", Family::Js},
        // Python.
        {"python", Family::Python}, {"py", Family::Python},
        // Lenguajes con comentarios # (sin lista de palabras clave).
        {"bash", Family::Hash}, {"sh", Family::Hash}, {"shell", Family::Hash},
        {"zsh", Family::Hash}, {"ruby", Family::Hash}, {"rb", Family::Hash},
        {"yaml", Family::Hash}, {"yml", Family::Hash}, {"toml", Family::Hash},
        {"r", Family::Hash}, {"perl", Family::Hash}, {"pl", Family::Hash},
        {"makefile", Family::Hash}, {"ini", Family::Hash}, {"conf", Family::Hash},
    };
    return table;
}

} // namespace

LangSpec LanguageRegistry::specFor(const QString &language)
{
    const QString key = language.trimmed().toLower();
    // LangSpec: {keywords, slash, hash, block}.
    switch (aliasTable().value(key, Family::Generic)) {
    case Family::C:
        return {cKeywords, true, false, true};
    case Family::Js:
        return {jsKeywords, true, false, true};
    case Family::Python:
        return {pyKeywords, false, true, false};
    case Family::Hash:
        return {{}, false, true, false};
    case Family::Generic:
        break;
    }
    // Desconocido: sin palabras clave, pero resaltamos cadenas, números y los
    // tres estilos de comentario habituales para que algo se vea.
    return {{}, true, true, true};
}
