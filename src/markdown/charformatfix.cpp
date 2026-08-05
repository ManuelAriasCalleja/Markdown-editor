/// \file
/// \brief Implementación de los arreglos del formato de carácter del importador de Qt.

#include "charformatfix.h"

#include <QFont>
#include <QList>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include "headingemphasis.h"  // marcar el énfasis antes de reponer la negrita

namespace {

// Un cambio de formato pendiente. Se recogen todos antes de aplicarlos: cambiar el
// formato de un fragmento fusiona/parte fragmentos, así que mutar mientras se
// recorre el bloque invalidaría el iterador.
struct FormatEdit
{
    int start;
    int end;
    QTextCharFormat fmt;
};

// Los fragmentos que no son texto (una imagen, el carácter objeto de una fórmula
// 2D) no tienen letra que ajustar: se dejan tal cual.
bool isTextFragment(const QTextCharFormat &cf)
{
    return !cf.isImageFormat() && cf.objectType() == QTextFormat::NoObject;
}

// ¿Es un run de código? Los dos sabores llevan el tamaño clavado, pero Qt los marca
// distinto: el span en línea con `fontFixedPitch`, y el BLOQUE de código solo con la
// familia monospace (sin fixed pitch), de modo que hay que reconocerlo por el
// bloque. Se usa BlockCodeLanguage, no BlockCodeFence, porque el vallado (```) y el
// indentado (cuatro espacios) son código igual y solo el primero trae la valla.
bool isCodeRun(const QTextBlock &block, const QTextCharFormat &cf)
{
    return cf.fontFixedPitch()
           || block.blockFormat().hasProperty(QTextFormat::BlockCodeLanguage);
}

void applyEdits(QTextDocument *doc, const QList<FormatEdit> &edits)
{
    if (edits.isEmpty())
        return;
    QTextCursor c(doc);
    c.beginEditBlock();
    for (const FormatEdit &e : edits) {
        c.setPosition(e.start);
        c.setPosition(e.end, QTextCursor::KeepAnchor);
        c.setCharFormat(e.fmt);
    }
    c.endEditBlock();
}

}  // namespace

void mdcharfix::repairHeadingRuns(QTextDocument *doc)
{
    if (!doc)
        return;
    // El énfasis del fuente (`**negrita**`) hay que anotarlo AQUÍ, antes de reponer la
    // negrita estructural del encabezado: después ya no se distingue una de otra y se
    // perdería al guardar. Va dentro de esta función, y no en el pipeline, para que el
    // orden no dependa de que nadie lo reordene (ver mdheademph).
    mdheademph::markExplicitBold(doc);
    QList<FormatEdit> edits;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        const int level = b.blockFormat().headingLevel();
        if (level < 1)
            continue;
        // El mismo paso relativo que usa el importador de Qt: h1 → +3 … h6 → −2.
        const int adjustment = 4 - level;
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || frag.length() == 0)
                continue;
            QTextCharFormat cf = frag.charFormat();
            if (!isTextFragment(cf))
                continue;
            const bool sizeOk = cf.hasProperty(QTextFormat::FontSizeAdjustment)
                                && cf.intProperty(QTextFormat::FontSizeAdjustment) == adjustment
                                && !cf.hasProperty(QTextFormat::FontPointSize);
            const bool weightOk = cf.fontWeight() >= QFont::Bold;
            if (sizeOk && weightOk)
                continue;  // ya está bien: no ensuciar el documento
            // El tamaño del encabezado es relativo (un paso sobre el cuerpo); un
            // tamaño absoluto en el fragmento lo anularía, así que se retira.
            cf.setProperty(QTextFormat::FontSizeAdjustment, adjustment);
            cf.clearProperty(QTextFormat::FontPointSize);
            cf.setFontWeight(QFont::Bold);
            edits.append({frag.position(), frag.position() + frag.length(), cf});
        }
    }
    applyEdits(doc, edits);
}

void mdcharfix::unpinCodeFontSize(QTextDocument *doc)
{
    if (!doc)
        return;
    QList<FormatEdit> edits;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid() || frag.length() == 0)
                continue;
            QTextCharFormat cf = frag.charFormat();
            if (!isTextFragment(cf) || !isCodeRun(b, cf)
                || !cf.hasProperty(QTextFormat::FontPointSize))
                continue;
            cf.clearProperty(QTextFormat::FontPointSize);
            edits.append({frag.position(), frag.position() + frag.length(), cf});
        }
    }
    applyEdits(doc, edits);
}

void mdcharfix::apply(QTextDocument *doc)
{
    // Primero el código (quita el tamaño absoluto de TODOS los runs de código) y
    // luego los encabezados, que además reponen el paso relativo del nivel.
    unpinCodeFontSize(doc);
    repairHeadingRuns(doc);
}
