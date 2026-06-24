#ifndef MATHBLOCKS_H
#define MATHBLOCKS_H

/// \file
/// \brief Soporte de fórmulas TeX en el documento: protección, render a runs/objeto 2D y serialización fiel.

#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QTextFormat>

class QTextDocument;

/// \brief Soporte de fórmulas TeX en el documento. El problema raíz es que
/// QTextDocument::setMarkdown() no entiende `$...$`/`$$...$$` y reinterpreta los
/// caracteres especiales (p. ej. `_` como cursiva) dentro de la fórmula, lo que
/// rompe el round-trip. La solución sin dependencias externas es envolver cada
/// fórmula en una valla de inline-code antes de pasársela a Qt (`$x_1$` →
/// `` `$x_1$` ``) y desenvolverla al serializar.
///
/// El módulo es puro (sin GUI ni estado): solo manipula cadenas.
///
/// Las fórmulas inline (`$...$`) deben abrir y cerrar en la misma línea; las de
/// bloque (`$$...$$`) sí pueden cruzar varias líneas (estilo Pandoc/Obsidian):
/// `findMath` las rastrea con un estado de apertura y `protectMath` codifica sus
/// saltos internos en un placeholder PUA para que quepan en el inline-code.
namespace mdmath {

/// \brief Una fórmula encontrada en el Markdown fuente.
struct Span {
    int start;        ///< índice del primer '$' en el texto
    int length;       ///< longitud total, delimitadores incluidos
    bool block;       ///< true si era $$...$$, false si $...$
    QString content;  ///< TeX sin delimitadores (puede ir vacío)
};

/// \brief Encuentra todas las fórmulas inline ($...$) y bloque ($$...$$) en `text`,
/// saltando regiones donde Markdown no las interpretaría (bloques de código con
/// vallas ``` y spans de código en línea con `).
QList<Span> findMath(const QString &text);

/// \brief Envuelve cada fórmula de `markdown` en una valla de inline-code de dobles
/// comillas invertidas: `$x$` → `` ``$x$`` ``. Así setMarkdown() la conserva
/// verbatim como código en línea, sin reinterpretar `_`/`*`/`[`.
QString protectMath(const QString &markdown);

/// \brief Inversa de protectMath: en la salida de toMarkdown(), quita las comillas
/// invertidas a los spans cuyo contenido tiene forma `$...$` o `$$...$$`. El
/// resto del Markdown se respeta tal cual.
/// \note Sigue exportada por compatibilidad con el primer diseño; la
/// serialización canónica de documentos usa centinelas
/// (replaceMathWithSentinels/restoreMathFromSentinels) porque
/// QTextDocument::toMarkdown() escapa los `\` dentro del inline-code.
QString unprotectMath(const QString &markdown);

/// \brief Traduce una expresión TeX a una aproximación en Unicode legible. Cubre lo
/// más común con tablas estáticas: letras griegas, símbolos/operadores
/// frecuentes, super/subíndices (los caracteres que el repertorio Unicode sí
/// tiene) y `\frac{a}{b}` → "(a)/(b)". Lo que no sepa traducir lo deja literal.
/// \note No es un render TeX: para fórmulas complicadas el resultado será aproximado.
QString texToUnicode(const QString &tex);

/// \brief Sustituye en `markdown` cada fórmula encontrada por su versión Unicode (los
/// delimitadores `$` no se emiten). Se usa al exportar a HTML/ODF, formatos que
/// no tienen un equivalente a TeX. LaTeX, en cambio, emite las fórmulas tal
/// cual (no usa esta función).
QString replaceMathWithUnicode(const QString &markdown);

/// \brief Marca «esto es una fórmula» en el QTextCharFormat de un fragmento renderizado.
constexpr int IsMathProperty    = QTextFormat::UserProperty + 1;
/// \brief Guarda el TeX original del fragmento de fórmula (para serializar y reeditar).
constexpr int MathTexProperty   = QTextFormat::UserProperty + 2;
/// \brief Marca si la fórmula del fragmento era de bloque ($$...$$).
constexpr int MathBlockProperty = QTextFormat::UserProperty + 3;

/// \brief Tipo de objeto de texto custom para las fórmulas que se pintan en 2D real
/// (fracciones apiladas, grandes operadores con límites). Una de esas fórmulas
/// vive como UN carácter ObjectReplacementCharacter cuyo char-format lleva las
/// propiedades de math de arriba + `setObjectType(MathObjectType)`. Lo dibuja un
/// QTextObjectInterface registrado en la GUI (MathObject); en mdmath solo se
/// inserta el carácter (sin GUI). Las fórmulas que NO necesitan 2D siguen como
/// secuencia de runs (mathCharFormat + renderTexAsRuns).
constexpr int MathObjectType = QTextFormat::UserObject + 1;

/// \brief Comando TeX (sin la `\`) a su carácter Unicode, o `\cmd` si no está en la
/// tabla. Lo implementa texparser.cpp; lo expone para que mathlayout reutilice
/// la tabla de glifos (griego, operadores) sin duplicarla.
QString commandToUnicode(const QString &cmd);

/// \brief Carácter Unicode combinante del acento `cmd` (`hat`→◌̂, `bar`→◌̄, `vec`→◌⃗,
/// `tilde`/`dot`/`ddot`/`acute`/`grave`/`check`/`breve`), o QChar nulo si no es
/// un acento. Se pone DETRÁS del carácter base para que se superponga. Compartido
/// por el render inline (texparser) y el 2D (mathlayout).
QChar accentCombiningChar(const QString &cmd);

/// \brief Lee un único token TeX desde `i` (un comando `\x` o un carácter) y lo devuelve
/// como Unicode; `i` avanza tras el token. Lo usan `\left`/`\right` (su
/// delimitador) y `\not` (el operando negado). Compartido inline/2D.
QString readTokenAsUnicode(const QString &tex, int &i);

/// \brief ¿Es un comando de alfabeto matemático con argumento (\mathcal/\mathscr/
/// \mathfrak)? El render lo trata como el bloque \text pero mapeando cada letra.
bool isStyledAlphabetCommand(const QString &cmd);

/// \brief Convierte cada letra latina de `arg` a su variante matemática Unicode según
/// `cmd` (script para \mathcal/\mathscr, fraktur para \mathfrak); el resto de
/// caracteres pasa sin cambios. Compartido por el render inline y el 2D.
QString styledMathAlphabet(const QString &cmd, const QString &arg);

/// \brief Char-format del carácter ObjectReplacementCharacter de una fórmula 2D: las
/// tres propiedades de math + objectType = MathObjectType.
QTextCharFormat mathObjectFormat(const QString &tex, bool block);

/// \brief Char-format base para los fragmentos de una fórmula: cursiva (señal
/// visual) + las tres propiedades de math. Cada «run» de la fórmula (texto
/// normal, súper, subíndice) lo extiende añadiendo su alineación vertical.
QTextCharFormat mathCharFormat(const QString &tex, bool block);

/// \brief Un trozo (run) de la renderización de una fórmula en el QTextDocument.
/// Una fórmula es una secuencia consecutiva de runs que comparten el mismo
/// MathTex; los runs de súper/subíndice llevan AlignSuperScript/AlignSubScript
/// para que Qt los pinte elevados/bajados y a menor tamaño, igual que un
/// editor tipográfico hace con «x²». Esto rinde cualquier carácter, incluidos
/// los que no tienen forma en el repertorio Unicode de scripts (∞, letras
/// griegas, comandos compuestos...).
struct MathRun {
    QString text;
    QTextCharFormat fmt;
};

/// \brief Convierte una expresión TeX en una secuencia de runs, partiendo del
/// `baseFmt` (que debe traer IsMath/MathTex/MathBlock). Los argumentos de
/// `^`/`_` se aplanan internamente con `texToUnicode` antes de envolverse en un
/// único run con vertical-align — bastante para super/sub a un nivel; los
/// scripts anidados se aproximan combinando glifos Unicode dentro del run
/// elevado.
QList<MathRun> renderTexAsRuns(const QString &tex, const QTextCharFormat &baseFmt);

/// \brief Representación de una fórmula para insertar en el documento: punto único que
/// elige entre 2D y runs. Si `needsTwoDLayout(tex)` (fracciones, grandes
/// operadores con límites), devuelve UN run con el carácter objeto
/// (`mathObjectFormat`, lo pinta el QTextObjectInterface MathObject); si no, los
/// runs inline de `renderTexAsRuns`. Lo usan la carga (`renderMathInDocument`) y
/// la inserción/edición interactiva (FormulaController).
QList<MathRun> renderFormulaRuns(const QString &tex, bool block);

/// \brief Envuelve una expresión TeX en sus delimitadores: `$tex$` (inline) o `$$tex$$`
/// (bloque). Centraliza la convención de delimitadores del editor.
QString wrapTex(const QString &tex, bool block);

/// \brief Busca en el documento los fragmentos de inline-code cuyo contenido tiene
/// forma `$tex$` o `$$tex$$` (los que dejó `protectMath` + `setMarkdown` al
/// cargar) y los sustituye por fragmentos «renderizados»: texto Unicode visible
/// + mathCharFormat con el TeX en propiedad. Es idempotente: si vuelve a
/// llamarse no toca los que ya están renderizados.
void renderMathInDocument(QTextDocument *doc);

/// \brief Inversa de renderMathInDocument: encuentra los fragmentos marcados con
/// IsMathProperty y los reemplaza por inline-code de texto `$tex$`/`$$tex$$`.
/// Se usa sobre un clon del documento antes de serializar (toMarkdown no sabe
/// nada de las propiedades custom; sí sabe emitir inline-code).
void unrenderMathInDocument(QTextDocument *doc);

/// \brief Devuelve los límites [start, end) de cada grupo de fragmentos consecutivos
/// con IsMathProperty del documento (una fórmula completa = un grupo, aunque
/// se haya renderizado como varios runs). Útil para que la UI pueda expandir
/// una selección al grupo entero antes de operaciones destructivas (paste,
/// delete) y no romper la fórmula a mitad.
QList<QPair<int, int>> mathGroupBounds(QTextDocument *doc);

/// \brief Tabla de fórmulas extraídas en el orden en el que aparecen en el documento:
/// cada entrada lleva el TeX y si es bloque ($$...$$) o inline ($...$).
struct MathSentinelTable {
    QList<QPair<QString, bool>> entries;
};

/// \brief En `doc` (que el llamador clonó), sustituye cada fragmento de fórmula por un
/// par de caracteres centinela del área de uso privado (PUA) Unicode + el índice
/// de la entrada en decimal. Los centinelas son texto plano sin formato: Qt no
/// los escapa al pasar por toMarkdown(), a diferencia de los `\` dentro de
/// inline-code.
/// \return La tabla que permite reconstruir luego los `$tex$`.
MathSentinelTable replaceMathWithSentinels(QTextDocument *doc);

/// \brief Sustituye en `markdown` cada centinela `<PUA-OPEN><n><PUA-CLOSE>` por la
/// fórmula original `$tex$` o `$$tex$$` que indique la tabla.
QString restoreMathFromSentinels(const QString &markdown,
                                 const MathSentinelTable &table);

} // namespace mdmath

#endif // MATHBLOCKS_H
