/// \file
/// \brief Implementación del pipeline de carga de Markdown (protección y pasadas
///        de render de fórmulas, notas al pie y admoniciones).

#include "markdownrender.h"

#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>

#include "admonitions.h"
#include "footnotes.h"
#include "mathblocks.h"
#include "supsub.h"

QString mdrender::protect(const QString &markdown)
{
    // Notas al pie primero, luego fórmulas; el super/subíndice al final (sus `^`/`~`
    // no interfieren con `$`/`[^id]`, y así sus centinelas ya no se ven como delimitadores).
    return mdsupsub::protect(mdmath::protectMath(mdfootnote::protectFootnotes(markdown)));
}

void mdrender::renderPasses(QTextDocument *doc)
{
    if (!doc)
        return;
    // Las tres pasadas reformatean fragmentos por todo el documento. Agruparlas en
    // un único edit block evita que cada una provoque un re-trazado intermedio
    // (parpadeo visible al cargar un archivo grande) y las fusiona en un solo undo.
    QTextCursor cursor(doc);
    cursor.beginEditBlock();
    mdmath::renderMathInDocument(doc);
    mdfootnote::renderFootnotesInDocument(doc);
    mdadmonition::renderAdmonitionsInDocument(doc);
    mdsupsub::renderInDocument(doc);
    cursor.endEditBlock();
}

void mdrender::setMarkdownWithExtensions(QTextEdit *editor, const QString &markdown)
{
    if (!editor)
        return;
    // document()->setMarkdown (no editor->setMarkdown) para poder fijar las
    // features (NoHTML); el QTextEdit refleja el documento igual.
    editor->document()->setMarkdown(protect(markdown), kMarkdownFeatures);
    renderPasses(editor->document());
}
