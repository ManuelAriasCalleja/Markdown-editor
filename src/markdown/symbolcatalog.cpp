#include "symbolcatalog.h"

#include <QCoreApplication>

namespace {

// Parte una cadena en sus puntos de código (un QString por símbolo), respetando
// los pares subrogados por si algún símbolo viviera fuera del BMP.
QStringList codePoints(const QString &s)
{
    QStringList out;
    for (int i = 0; i < s.size();) {
        if (s.at(i).isHighSurrogate() && i + 1 < s.size()) {
            out << s.mid(i, 2);
            i += 2;
        } else {
            out << QString(s.at(i));
            ++i;
        }
    }
    return out;
}

}  // namespace

QList<mdsymbols::Category> mdsymbols::categories()
{
    // Los nombres usan QCoreApplication::translate inline (no envueltos en una
    // función): lupdate solo extrae las llamadas con literales en el propio sitio.
    return {
        {QCoreApplication::translate("SymbolPicker", "Matemáticos"),
         codePoints(QStringLiteral(
             "≈≠≤≥±∓×÷⋅∗∞∝∂∇√∛∑∏∫∮∈∉∋⊂⊃⊆⊇∪∩∅∀∃∄¬∧∨⊕⊗≡≅∼°′″∠⊥∥⇒⇔"))},
        {QCoreApplication::translate("SymbolPicker", "Griego"),
         codePoints(QStringLiteral(
             "αβγδεζηθικλμνξοπρστυφχψωΓΔΘΛΞΠΣΦΨΩ"))},
        {QCoreApplication::translate("SymbolPicker", "Flechas"),
         codePoints(QStringLiteral(
             "←→↑↓↔↕⇐⇒⇑⇓⇔↦↤↩↪↺↻⇄⇆⤴⤵➜➤"))},
        {QCoreApplication::translate("SymbolPicker", "Moneda"),
         codePoints(QStringLiteral("€$£¥¢₿₹₽₩₪₫₴₺฿¤"))},
        {QCoreApplication::translate("SymbolPicker", "Puntuación y tipografía"),
         codePoints(QStringLiteral(
             "—–‒…«»“”‘’‹›„‚†‡§¶•·‰※‽¡¿©®™№⁂"))},
        {QCoreApplication::translate("SymbolPicker", "Astronomía"),
         codePoints(QStringLiteral("☉☽☾★☆☄✦✧⊕☿♀♁♂♃♄⛢♅♆☀"))},
        {QCoreApplication::translate("SymbolPicker", "Marcas y viñetas"),
         codePoints(QStringLiteral(
             "✓✔✗✘☑☒☐♥♦♣♠‣◦▪▫■□◆◇●○⚠⚡☢☣⚙⌘⌥⇧⏎"))},
        {QCoreApplication::translate("SymbolPicker", "Fracciones y superíndices"),
         codePoints(QStringLiteral(
             "½⅓⅔¼¾⅕⅙⅛⅜⅝⅞⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻ⁿ₀₁₂₃₄₅₆₇₈₉₊₋"))},
    };
}
