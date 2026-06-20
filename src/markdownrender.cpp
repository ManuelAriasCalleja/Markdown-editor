#include "markdownrender.h"

#include <QTextDocument>
#include <QTextEdit>

#include "admonitions.h"
#include "footnotes.h"
#include "mathblocks.h"

QString mdrender::protect(const QString &markdown)
{
    // Notas al pie primero, luego fórmulas (mismo orden que tenía DocumentIo::load).
    return mdmath::protectMath(mdfootnote::protectFootnotes(markdown));
}

void mdrender::renderPasses(QTextDocument *doc)
{
    if (!doc)
        return;
    mdmath::renderMathInDocument(doc);
    mdfootnote::renderFootnotesInDocument(doc);
    mdadmonition::renderAdmonitionsInDocument(doc);
}

void mdrender::setMarkdownWithExtensions(QTextEdit *editor, const QString &markdown)
{
    if (!editor)
        return;
    editor->setMarkdown(protect(markdown));
    renderPasses(editor->document());
}
