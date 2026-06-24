#ifndef BLOCKCONSTRUCTS_H
#define BLOCKCONSTRUCTS_H

#include <QString>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextDocumentFragment>

class QTextEdit;
class QTextCursor;

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

    // Formato de bloque canónico del constructo (cita o valla). toggle() lo
    // re-aplica a cada bloque insertado: insertFragment fusiona el primer bloque
    // del fragmento con el actual y descarta su formato de bloque, así que sin
    // esto la primera línea perdería la valla/cita (y un párrafo vacío no se
    // convertiría en nada).
    virtual QTextBlockFormat blockFormat() const = 0;
    // Formato de carácter por defecto del bloque (p. ej. monoespaciado para el
    // código), para que el texto que se teclee en un bloque vacío salga bien.
    virtual QTextCharFormat blockCharFormat() const { return {}; }
};

// Cita / blockquote: `> texto` (conserva el formato en línea).
class Blockquote : public BlockConstruct
{
public:
    bool contains(const QTextBlockFormat &bf) const override;

protected:
    QTextDocumentFragment buildReplacement(const QTextCursor &selection,
                                           bool removing) const override;
    QTextBlockFormat blockFormat() const override;
};

// Bloque de código con vallas ```. El contenido es literal (texto plano).
class CodeBlock : public BlockConstruct
{
public:
    bool contains(const QTextBlockFormat &bf) const override;

protected:
    QTextDocumentFragment buildReplacement(const QTextCursor &selection,
                                           bool removing) const override;
    QTextBlockFormat blockFormat() const override;
    QTextCharFormat blockCharFormat() const override;
};

#endif // BLOCKCONSTRUCTS_H
