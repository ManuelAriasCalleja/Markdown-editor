/// \file
/// \brief Implementación del pipeline de carga de Markdown (protección y pasadas
///        de render de fórmulas, notas al pie y admoniciones).

#include "markdownrender.h"

#include <QFileInfo>
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

QString mdrender::imageAltFallback(const QString &destination)
{
    QString path = destination;
    if (path.startsWith(QLatin1Char('<')) && path.endsWith(QLatin1Char('>')))
        path = path.mid(1, path.size() - 2);
    const QString name = QFileInfo(path).completeBaseName();
    return name.isEmpty() ? QStringLiteral("imagen") : name;
}

QString mdrender::imageMarkdown(const QString &destination, const QString &alt)
{
    QString dest = destination;
    // Un `<`/`>` dentro de la propia ruta rompería el envoltorio `<...>`: se codifica.
    if (dest.contains(QLatin1Char('<')) || dest.contains(QLatin1Char('>'))) {
        dest.replace(QLatin1String("<"), QLatin1String("%3C"));
        dest.replace(QLatin1String(">"), QLatin1String("%3E"));
    }
    if (dest.contains(QLatin1Char(' ')) || dest.contains(QLatin1Char('('))
        || dest.contains(QLatin1Char(')')))
        dest = QLatin1Char('<') + dest + QLatin1Char('>');

    QString text = alt.isEmpty() ? imageAltFallback(destination) : alt;
    text.replace(QLatin1String("["), QLatin1String("\\["));  // cerrarían el rótulo
    text.replace(QLatin1String("]"), QLatin1String("\\]"));
    return QStringLiteral("![%1](%2)").arg(text, dest);
}
