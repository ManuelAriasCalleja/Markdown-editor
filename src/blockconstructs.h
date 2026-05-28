#ifndef BLOCKCONSTRUCTS_H
#define BLOCKCONSTRUCTS_H

#include <QString>
#include <QTextDocumentFragment>

class QTextEdit;
class QTextCursor;
class QTextBlockFormat;

// Transformaciones de texto puras (sin estado ni GUI) que sustentan los
// constructos de bloque. Se exponen aparte para poder probarlas de forma
// aislada.
namespace mdblock {

// Antepone "> " a cada línea del Markdown dado.
QString addBlockquoteMarkers(const QString &markdown);
// Quita el "> " (o ">") inicial de cada línea.
QString removeBlockquoteMarkers(const QString &markdown);
// Envuelve el texto plano en una valla de código ```.
QString fenceCode(const QString &plainText);

} // namespace mdblock

// Constructo de bloque Markdown que se activa/desactiva sobre la selección
// (cita o bloque de código). El algoritmo de alternado es común (Template
// Method en toggle()); cada subclase aporta cómo se reconoce el bloque y cómo
// se reconstruye el fragmento (Strategy).
class BlockConstruct
{
public:
    virtual ~BlockConstruct() = default;

    // Alterna el constructo sobre la selección actual del editor.
    void toggle(QTextEdit *editor) const;

    // ¿El bloque pertenece a este constructo? (público porque la expansión de
    // la selección lo consulta en bloques vecinos).
    virtual bool contains(const QTextBlockFormat &bf) const = 0;

protected:
    // Construye el fragmento que sustituirá a la selección (ya expandida a
    // bloques completos). `removing` indica si se está quitando el constructo.
    virtual QTextDocumentFragment buildReplacement(const QTextCursor &selection,
                                                   bool removing) const = 0;
};

// Cita / blockquote: `> texto` (conserva el formato en línea).
class Blockquote : public BlockConstruct
{
public:
    bool contains(const QTextBlockFormat &bf) const override;

protected:
    QTextDocumentFragment buildReplacement(const QTextCursor &selection,
                                           bool removing) const override;
};

// Bloque de código con vallas ```. El contenido es literal (texto plano).
class CodeBlock : public BlockConstruct
{
public:
    bool contains(const QTextBlockFormat &bf) const override;

protected:
    QTextDocumentFragment buildReplacement(const QTextCursor &selection,
                                           bool removing) const override;
};

#endif // BLOCKCONSTRUCTS_H
