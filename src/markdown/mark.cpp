/// \file
/// \brief Implementación de la localización de marcas `==texto==`.

#include "mark.h"

#include <QRegularExpression>

namespace mdmark {

QList<Span> spansIn(const QString &text)
{
    QList<Span> spans;
    // `==` + interior no vacío que no empieza ni acaba en `=` (evita comerse
    // secuencias de `===`) + `==`. No codicioso.
    static const QRegularExpression re(QStringLiteral("==(?=[^=])(.+?)(?<=[^=])=="));
    QRegularExpressionMatchIterator it = re.globalMatch(text);
    while (it.hasNext()) {
        const QRegularExpressionMatch m = it.next();
        spans.append({int(m.capturedStart()), int(m.capturedLength())});
    }
    return spans;
}

}  // namespace mdmark
