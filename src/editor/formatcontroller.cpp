/// \file
/// \brief Implementación de FormatController: comandos de formato WYSIWYG y estado de sus acciones.

#include "formatcontroller.h"

#include <QAction>
#include <QCoreApplication>
#include <QFont>
#include <QInputDialog>
#include <QTextEdit>
#include <QTextList>

#include "blockconstructs.h"
#include "codehighlighter.h"
#include "outlinepanel.h"  // mdoutline::shiftedLevel

// Los textos visibles de setCodeLanguage conservan el contexto de traducción
// "MainWindow" (QCoreApplication::translate) para no re-hogar las cadenas ya
// traducidas en los .ts al moverlas. Ver la nota de i18n en CLAUDE.md.

FormatController::FormatController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                                   QWidget *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_highlighter(highlighter)
    , m_parent(parent)
{
}

void FormatController::mergeCharFormatOnSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = m_editor->textCursor();
    // Sin selección, se aplica a la palabra bajo el cursor; además se fija el
    // formato de inserción para que el texto que se escriba a continuación
    // herede el formato.
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    m_editor->mergeCurrentCharFormat(format);
    m_editor->setFocus();
    updateActions();
}

void FormatController::toggleCharFormat(
    const std::function<void(QTextCharFormat &, const QTextCharFormat &)> &mutate)
{
    QTextCharFormat fmt;
    mutate(fmt, m_editor->currentCharFormat());
    mergeCharFormatOnSelection(fmt);
}

void FormatController::setHeadingLevel(int level)
{
    // Mutación NUCLEAR (absoluta, sin toggle): fija el bloque como encabezado de
    // `level` (1..6) o como párrafo normal (0). La comparten el toggle applyHeading
    // y promover/degradar (shiftHeading).
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();

    // El nivel de encabezado es lo que exporta toMarkdown() como '#'.
    QTextBlockFormat bf;
    bf.setHeadingLevel(level);
    cursor.mergeBlockFormat(bf);

    // Para el WYSIWYG hay que aplicar también el tamaño y la negrita: usamos
    // FontSizeAdjustment = 4 - nivel, igual que hace setMarkdown() de Qt.
    QTextCharFormat cf;
    cf.setProperty(QTextFormat::FontSizeAdjustment, level > 0 ? 4 - level : 0);
    cf.setFontWeight(level > 0 ? QFont::Bold : QFont::Normal);

    QTextCursor blockCursor = cursor;
    blockCursor.movePosition(QTextCursor::StartOfBlock);
    blockCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    blockCursor.mergeCharFormat(cf);

    cursor.endEditBlock();

    m_editor->mergeCurrentCharFormat(cf);  // para el texto que se escriba luego
    m_editor->setFocus();
    updateActions();
}

void FormatController::applyHeading(int level)
{
    const int current = m_editor->textCursor().blockFormat().headingLevel();
    setHeadingLevel(current == level ? 0 : level);  // volver a pulsar = quitar
}

void FormatController::shiftHeading(int delta)
{
    const int current = m_editor->textCursor().blockFormat().headingLevel();
    const int target = mdoutline::shiftedLevel(current, delta);
    if (target == current)
        return;  // no es encabezado, o ya en el límite (H1 al promover / H6 al degradar)
    setHeadingLevel(target);
}

void FormatController::promoteHeading()
{
    shiftHeading(-1);  // hacia H1 (menos '#')
}

void FormatController::demoteHeading()
{
    shiftHeading(1);  // hacia H6 (más '#')
}

void FormatController::applyList(QTextListFormat::Style style)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList *currentList = cursor.currentList();

    cursor.beginEditBlock();
    if (currentList && currentList->format().style() == style) {
        // Ya es ese tipo de lista: la quitamos (desligamos el bloque). También
        // se quita el marcador de tarea, si lo hubiera, para que no quede un
        // checkbox huérfano fuera de toda lista.
        QTextBlockFormat bf = cursor.blockFormat();
        bf.setObjectIndex(-1);
        bf.setIndent(0);
        bf.setMarker(QTextBlockFormat::MarkerType::NoMarker);
        cursor.setBlockFormat(bf);
    } else {
        QTextListFormat lf;
        lf.setStyle(style);
        cursor.createList(lf);
    }
    cursor.endEditBlock();

    m_editor->setFocus();
    updateActions();
}

void FormatController::toggleBlockquote()
{
    Blockquote().toggle(m_editor);
    updateActions();
}

void FormatController::toggleCodeBlock()
{
    CodeBlock().toggle(m_editor);
    updateActions();
}

void FormatController::toggleTaskItem()
{
    QTextCursor cursor = m_editor->textCursor();
    cursor.beginEditBlock();
    if (!cursor.currentList()) {
        QTextListFormat lf;
        lf.setStyle(QTextListFormat::ListDisc);
        cursor.createList(lf);
    }
    QTextBlockFormat bf = cursor.blockFormat();
    const bool isTask = bf.marker() != QTextBlockFormat::MarkerType::NoMarker;
    bf.setMarker(isTask ? QTextBlockFormat::MarkerType::NoMarker
                        : QTextBlockFormat::MarkerType::Unchecked);
    cursor.setBlockFormat(bf);
    cursor.endEditBlock();
    m_editor->setFocus();
    updateActions();
}

void FormatController::indentList()
{
    changeListIndent(+1);
}

void FormatController::outdentList()
{
    changeListIndent(-1);
}

void FormatController::changeListIndent(int delta)
{
    QTextCursor cursor = m_editor->textCursor();
    QTextList *list = cursor.currentList();
    if (!list)
        return;  // la sangría de lista solo aplica dentro de una lista

    QTextListFormat lf = list->format();
    const int newIndent = qMax(1, lf.indent() + delta);
    if (newIndent == lf.indent())
        return;
    lf.setIndent(newIndent);
    cursor.createList(lf);  // crea/ajusta la (sub)lista al nuevo nivel
    m_editor->setFocus();
    updateActions();
}

void FormatController::setCodeLanguage()
{
    QTextCursor cursor = m_editor->textCursor();
    if (!cursor.blockFormat().hasProperty(QTextFormat::BlockCodeFence)) {
        emit statusMessage(QCoreApplication::translate(
            "MainWindow", "Coloca el cursor dentro de un bloque de código"), 3000);
        return;
    }

    const QString current =
        cursor.blockFormat().stringProperty(QTextFormat::BlockCodeLanguage);

    const QStringList langs = {QString(), QStringLiteral("cpp"),
        QStringLiteral("c"), QStringLiteral("python"),
        QStringLiteral("javascript"), QStringLiteral("typescript"),
        QStringLiteral("java"), QStringLiteral("go"), QStringLiteral("rust"),
        QStringLiteral("bash"), QStringLiteral("json"), QStringLiteral("sql"),
        QStringLiteral("html"), QStringLiteral("css")};
    const int currentIndex = qMax(0, langs.indexOf(current));

    bool ok = false;
    const QString lang = QInputDialog::getItem(
        m_parent, QCoreApplication::translate("MainWindow", "Lenguaje del bloque"),
        QCoreApplication::translate("MainWindow", "Lenguaje (vacío = ninguno):"),
        langs, currentIndex, /*editable=*/true, &ok);
    if (!ok)
        return;

    // Aplica el lenguaje a todos los bloques contiguos del bloque de código.
    QTextDocument *doc = m_editor->document();
    int first = cursor.block().blockNumber();
    int last = first;
    while (first > 0 && doc->findBlockByNumber(first - 1).blockFormat()
                            .hasProperty(QTextFormat::BlockCodeFence))
        --first;
    while (last < doc->blockCount() - 1 && doc->findBlockByNumber(last + 1).blockFormat()
                                               .hasProperty(QTextFormat::BlockCodeFence))
        ++last;

    QTextCursor edit(doc);
    edit.beginEditBlock();
    for (int i = first; i <= last; ++i) {
        QTextCursor bc(doc->findBlockByNumber(i));
        QTextBlockFormat add;
        add.setProperty(QTextFormat::BlockCodeLanguage, lang);
        bc.mergeBlockFormat(add);
    }
    edit.endEditBlock();

    m_highlighter->rehighlight();
    m_editor->setFocus();
}

void FormatController::updateActions()
{
    const QTextCharFormat cf = m_editor->currentCharFormat();
    m_actions.bold->setChecked(cf.fontWeight() >= QFont::Bold);
    m_actions.italic->setChecked(cf.fontItalic());
    m_actions.underline->setChecked(cf.fontUnderline());
    m_actions.strike->setChecked(cf.fontStrikeOut());
    m_actions.code->setChecked(cf.fontFixedPitch());
    m_actions.link->setChecked(cf.isAnchor());

    const QTextCursor cursor = m_editor->textCursor();
    const int level = cursor.blockFormat().headingLevel();
    m_actions.h1->setChecked(level == 1);
    m_actions.h2->setChecked(level == 2);
    m_actions.h3->setChecked(level == 3);
    m_actions.h4->setChecked(level == 4);
    m_actions.h5->setChecked(level == 5);
    m_actions.h6->setChecked(level == 6);

    // En un encabezado, el formato de carácter (negrita, cursiva, etc.) no
    // round-trip-ea a Markdown: el `#` ya implica el peso del título y Qt no
    // serializa cursiva/subrayado dentro de un heading. Para no engañar al
    // usuario (ni dejarle «quitar» la negrita del título), se deshabilitan.
    const bool heading = level > 0;
    for (QAction *a : {m_actions.bold, m_actions.italic, m_actions.underline,
                       m_actions.strike, m_actions.code})
        a->setEnabled(!heading);

    const QTextList *list = cursor.currentList();
    const QTextListFormat::Style style =
        list ? list->format().style() : QTextListFormat::ListStyleUndefined;
    const QTextBlockFormat bf = cursor.blockFormat();
    const bool isTask = list && bf.marker() != QTextBlockFormat::MarkerType::NoMarker;
    // Una lista de tareas es internamente una ListDisc con marcador: marca solo
    // «tareas», no «viñetas». Una viñeta normal (sin marcador) marca solo «viñetas».
    m_actions.bullet->setChecked(style == QTextListFormat::ListDisc && !isTask);
    m_actions.numbered->setChecked(style == QTextListFormat::ListDecimal);
    m_actions.task->setChecked(isTask);

    // La sangría de lista solo tiene efecto dentro de una lista.
    m_actions.indent->setEnabled(list != nullptr);
    m_actions.outdent->setEnabled(list != nullptr);

    m_actions.quote->setChecked(bf.intProperty(QTextFormat::BlockQuoteLevel) > 0);
    m_actions.codeBlock->setChecked(bf.hasProperty(QTextFormat::BlockCodeFence));
    // El lenguaje solo se puede fijar con el cursor dentro de un bloque de código.
    m_actions.lang->setEnabled(bf.hasProperty(QTextFormat::BlockCodeFence));

    emit actionsUpdated();  // p. ej. para refrescar las acciones de tabla
}
