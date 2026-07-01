/// \file
/// \brief Implementación de la decodificación de HTML para la importación.

#include "htmlimport.h"

#include <QByteArray>
#include <QRegularExpression>
#include <QStringConverter>
#include <QStringDecoder>

namespace mdimport {

QString charsetOf(const QByteArray &bytes)
{
    // El charset se declara en el <head>: basta mirar el principio. Lo leemos como
    // Latin-1 (las etiquetas meta son ASCII) para localizar la declaración.
    const QString head = QString::fromLatin1(bytes.left(2048)).toLower();
    // Cubre a la vez `<meta charset="X">` y
    // `<meta http-equiv="Content-Type" content="…; charset=X">` (ambos llevan charset=X).
    static const QRegularExpression re(
        QStringLiteral(R"(<meta[^>]*charset\s*=\s*["']?\s*([a-z0-9._\-]+))"));
    const QRegularExpressionMatch m = re.match(head);
    return m.hasMatch() ? m.captured(1) : QString();
}

QString decodeHtml(const QByteArray &bytes)
{
    // 1. BOM (UTF-8/UTF-16/UTF-32): mando fiable, gana sobre lo declarado.
    if (const auto enc = QStringConverter::encodingForData(bytes)) {
        QStringDecoder dec(*enc);
        return dec.decode(bytes);
    }
    // 2. Juego declarado en <meta>, si Qt lo sabe decodificar.
    const QString name = charsetOf(bytes);
    if (!name.isEmpty()) {
        if (const auto enc = QStringConverter::encodingForName(name.toUtf8().constData())) {
            QStringDecoder dec(*enc);
            return dec.decode(bytes);
        }
    }
    // 3. Por defecto UTF-8 (lo habitual en la web actual).
    QStringDecoder dec(QStringConverter::Utf8);
    return dec.decode(bytes);
}

}  // namespace mdimport
