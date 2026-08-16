/// \file
/// \brief Implementación del cálculo de estadísticas del documento.

#include "docstats.h"

#include <QChar>
#include <QRegularExpression>
#include <QStringList>
#include <QStringView>

namespace {

// ¿Es un carácter de una escritura que no separa las palabras con espacios y en la
// que, por tanto, cada carácter cuenta como una palabra? Ideogramas chinos, kana
// japonés y sílabas hangul. NO entra su puntuación (`，。「」`, U+3000-303F): un signo
// no es una palabra, aquí tampoco.
// (`mdspell` y `mdexport` tienen sus propias copias de rangos parecidos, con otro
// criterio y otro fin; ninguno de los tres puede depender de los otros dos.)
bool isCjkWordChar(char32_t c)
{
    return (c >= 0x3040 && c <= 0x30FF)     // hiragana y katakana
        || (c >= 0x3400 && c <= 0x4DBF)     // ideogramas, extensión A
        || (c >= 0x4E00 && c <= 0x9FFF)     // ideogramas, bloque principal
        || (c >= 0xAC00 && c <= 0xD7AF)     // sílabas hangul
        || (c >= 0xF900 && c <= 0xFAFF)     // ideogramas de compatibilidad
        || (c >= 0xFF66 && c <= 0xFF9F)     // katakana de ancho medio (ｶﾀｶﾅ)
        || (c >= 0xFFA0 && c <= 0xFFDC)     // jamo hangul de ancho medio
        // Los planos 2 y 3 son ideográficos enteros. Sin ellos, un nombre propio
        // escrito con ideogramas de la extensión B (𠮷) no era ni palabra CJK ni
        // palabra latina: no lo contaba nadie y el párrafo entero salía como una.
        || (c >= 0x20000 && c <= 0x3FFFF);
}

// ¿Corta este carácter una palabra latina? Además de los blancos, cualquier cosa de
// las escrituras de arriba y su puntuación: en `混合español` hay una palabra latina,
// no un token de cinco caracteres que ningún recuento sabría explicar.
bool breaksLatinWord(char32_t c)
{
    if (isCjkWordChar(c))
        return true;
    if (c >= 0x3000 && c <= 0x303F)     // puntuación china/japonesa
        return true;
    // Formas de ancho completo: las letras y las cifras (`ＡＢＣ`, `１２３`) son texto
    // latino escrito ancho y cuentan como tal —cortar por ellas las dejaba fuera de
    // los dos recuentos, y «ＡＢＣ» era cero palabras—; su puntuación (`，！`) corta,
    // igual que corta la de toda la vida.
    if (c >= 0xFF01 && c <= 0xFF5E)
        return !QChar::isLetterOrNumber(c - 0xFEE0);
    return (c >= 0xFF5F && c <= 0xFF65)     // paréntesis anchos y puntuación de ancho medio
        || (c >= 0xFFE0 && c <= 0xFFEE);    // símbolos de ancho completo y medio (￥│)
}

}  // namespace

mdstats::DocStats mdstats::analyze(const QString &input, int wordsPerMinute)
{
    if (wordsPerMinute <= 0)
        wordsPerMinute = 200;
    // Un ideograma se lee más deprisa que una palabra: ~300-500 caracteres por
    // minuto frente a ~200 palabras. Se deriva de `wordsPerMinute` en vez de ser una
    // segunda constante para que quien suba la velocidad suba las dos a la vez.
    const int cjkCharsPerMinute = wordsPerMinute * 2;

    // selectedText() y los párrafos de Qt usan U+2029 como separador; lo
    // normalizamos a '\n' para contar líneas y caracteres igual que el editor.
    QString text = input;
    text.replace(QChar(QChar::ParagraphSeparator), QLatin1Char('\n'));

    DocStats st;

    // Palabras: los tramos entre blancos, como siempre, MÁS un recuento aparte de
    // los caracteres de las escrituras sin espacios (que además cortan la palabra
    // latina en la que aparezcan). Para un texto latino el resultado es idéntico al
    // de partir por `\s+`: los tramos entre blancos son los mismos.
    int latinWords = 0;
    int cjkChars = 0;
    bool inWord = false;
    for (const char32_t cp : text.toUcs4()) {
        if (isCjkWordChar(cp))
            ++cjkChars;
        if (QChar::isSpace(cp) || breaksLatinWord(cp)) {
            inWord = false;
            continue;
        }
        if (!inWord) {
            ++latinWords;
            inWord = true;
        }
    }
    st.cjkChars = cjkChars;
    st.words = latinWords + cjkChars;

    // Cuenta por PUNTOS DE CÓDIGO, no por unidades UTF-16: un carácter del plano
    // astral (emoji, alfabetos matemáticos) es un par subrogado y debe contar como
    // uno. Un punto de código = una unidad que no sea un subrogado bajo.
    int chars = 0;
    int nonSpace = 0;
    for (int i = 0; i < text.size(); ++i) {
        const QChar c = text.at(i);
        if (c.isLowSurrogate())
            continue;
        ++chars;
        if (!c.isSpace())
            ++nonSpace;
    }
    st.chars = chars;
    st.charsNoSpaces = nonSpace;

    // Párrafos: líneas con algún carácter no blanco (las vacías no cuentan).
    int paragraphs = 0;
    for (QStringView line : QStringView(text).split(QLatin1Char('\n')))
        if (!line.trimmed().isEmpty())
            ++paragraphs;
    st.paragraphs = paragraphs;

    // Frases: cada grupo consecutivo de signos de fin de frase ( . ! ? … ) cuenta
    // una vez, de modo que «¿…?», «...» o «!?» no inflan el recuento. El chino y el
    // japonés terminan la frase con SUS signos (`。！？`), no con los de la ASCII: sin
    // ellos, un documento entero en chino salía con «Frases: 0».
    int sentences = 0;
    bool inTerminator = false;
    for (const QChar &c : text) {
        const bool term = c == QLatin1Char('.') || c == QLatin1Char('!')
                          || c == QLatin1Char('?') || c == QChar(0x2026)   // …
                          || c == QChar(0x3002)                            // 。
                          || c == QChar(0xFF01) || c == QChar(0xFF1F)      // ！？
                          || c == QChar(0xFF0E) || c == QChar(0xFF61);     // ．｡
        if (term && !inTerminator)
            ++sentences;
        inTerminator = term;
    }
    st.sentences = sentences;

    // Cada escritura a su ritmo; un documento mezclado suma los dos.
    st.readingMinutes = static_cast<double>(latinWords) / wordsPerMinute
                        + static_cast<double>(cjkChars) / cjkCharsPerMinute;
    return st;
}
