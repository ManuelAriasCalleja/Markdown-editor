#ifndef CODEBLOCK_H
#define CODEBLOCK_H

/// \file
/// \brief Localización del grupo de bloques de un fence de código en el documento
///        (para la etiqueta de lenguaje y el botón copiar del overlay).

#include <QString>

class QTextDocument;

namespace mdcodeblock {

/// Grupo de bloques contiguos que forman un mismo bloque de código (fence). Los
/// bloques de un fence comparten la propiedad QTextFormat::BlockCodeFence; el
/// lenguaje va en BlockCodeLanguage del primero.
struct Group {
    int firstBlock = -1;  ///< número del primer bloque del fence
    int lastBlock = -1;   ///< número del último bloque del fence
    QString language;     ///< lenguaje declarado (vacío = ninguno)
    bool valid = false;   ///< false si `blockNumber` no cae en un fence
};

/// Grupo de fence que contiene al bloque `blockNumber`, o un Group inválido si ese
/// bloque no es de código. Función pura (sobre el documento, sin GUI).
Group groupAt(const QTextDocument *doc, int blockNumber);

/// Texto de código del grupo (los bloques de `first`..`last` unidos por saltos de
/// línea), tal cual para copiar al portapapeles. Función pura.
QString groupText(const QTextDocument *doc, const Group &group);

}  // namespace mdcodeblock

#endif  // CODEBLOCK_H
