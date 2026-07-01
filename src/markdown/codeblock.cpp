/// \file
/// \brief Implementación de la localización del grupo de bloques de código.

#include "codeblock.h"

#include <QStringList>
#include <QTextBlock>
#include <QTextDocument>

namespace mdcodeblock {

namespace {
bool isFence(const QTextBlock &b)
{
    return b.isValid() && b.blockFormat().hasProperty(QTextFormat::BlockCodeFence);
}
}  // namespace

Group groupAt(const QTextDocument *doc, int blockNumber)
{
    Group g;
    if (!doc)
        return g;
    const QTextBlock block = doc->findBlockByNumber(blockNumber);
    if (!isFence(block))
        return g;

    int first = blockNumber;
    int last = blockNumber;
    while (first > 0 && isFence(doc->findBlockByNumber(first - 1)))
        --first;
    while (last < doc->blockCount() - 1 && isFence(doc->findBlockByNumber(last + 1)))
        ++last;

    g.firstBlock = first;
    g.lastBlock = last;
    g.language =
        doc->findBlockByNumber(first).blockFormat().stringProperty(QTextFormat::BlockCodeLanguage);
    g.valid = true;
    return g;
}

QString groupText(const QTextDocument *doc, const Group &group)
{
    if (!doc || !group.valid)
        return {};
    QStringList lines;
    for (int i = group.firstBlock; i <= group.lastBlock; ++i)
        lines << doc->findBlockByNumber(i).text();
    return lines.join(QLatin1Char('\n'));
}

}  // namespace mdcodeblock
