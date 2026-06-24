/// \file
/// \brief Implementación del auto-emparejado de paréntesis, corchetes, llaves y comillas invertidas.

#include "autopair.h"

#include <QTextCursor>

namespace mdautopair {

const QList<Pair> &defaultPairs()
{
    static const QList<Pair> pairs = {
        {QLatin1Char('('), QLatin1Char(')')},
        {QLatin1Char('['), QLatin1Char(']')},
        {QLatin1Char('{'), QLatin1Char('}')},
        {QLatin1Char('`'), QLatin1Char('`')},
    };
    return pairs;
}

namespace {
// Carácter inmediatamente a la derecha del cursor (tras la selección si la hay);
// QChar nulo si está al final del bloque.
QChar charAfter(const QTextCursor &cursor)
{
    QTextCursor peek = cursor;
    peek.setPosition(cursor.selectionEnd());
    if (peek.atBlockEnd())
        return QChar();
    peek.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    const QString s = peek.selectedText();
    return s.isEmpty() ? QChar() : s.at(0);
}
} // namespace

bool apply(QTextCursor &cursor, QChar typed, const QList<Pair> &pairs)
{
    const QChar next = charAfter(cursor);
    const bool hasSel = cursor.hasSelection();

    // Type-over: teclear un cierre que ya está justo delante → saltarlo, sin
    // duplicar (p. ej. cerrar un `()` recién auto-cerrado escribiendo `)`).
    if (!hasSel) {
        for (const Pair &p : pairs) {
            if (typed == p.close && next == p.close) {
                cursor.movePosition(QTextCursor::NextCharacter);
                return true;
            }
        }
    }

    for (const Pair &p : pairs) {
        if (typed != p.open)
            continue;
        if (hasSel) {
            const QString sel = cursor.selectedText();
            cursor.insertText(QString(p.open) + sel + p.close);
            const int closePos = cursor.position() - 1;        // antes del cierre
            cursor.setPosition(closePos - sel.size());
            cursor.setPosition(closePos, QTextCursor::KeepAnchor);  // re-selecciona el interior
            return true;
        }
        // Sin selección: auto-cierra solo si lo siguiente no es palabra, para no
        // partir «(texto» que el usuario iba a cerrar a mano más a la derecha.
        if (next.isNull() || !next.isLetterOrNumber()) {
            cursor.insertText(QString(p.open) + p.close);
            cursor.movePosition(QTextCursor::PreviousCharacter);
            return true;
        }
        return false;  // apertura pero seguida de palabra: inserción normal
    }
    return false;
}

} // namespace mdautopair
