#include <QtTest>

#include "codespanfix.h"

// Pruebas de mdcodespan::unescapeInlineCode: revierte el sobre-escapado que Qt
// aplica DENTRO de los code spans en línea (`\ & < * [ !`), sin tocar el texto
// normal ni los bloques de código vallados.
class TestCodeSpanFix : public QObject
{
    Q_OBJECT

private slots:
    void unescapesBackslashInCode();
    void unescapesAmpersandInCode();
    void unescapesAllSixEscapedChars();
    void collapsesDoubledBackslashOnce();
    void leavesPlainTextEscapesUntouched();
    void leavesFencedCodeUntouched();
    void leavesIndentedFenceUntouched();
    void handlesMultipleSpansInOneLine();
    void leavesUnescapedCodeUnchanged();
    void handlesMultiBacktickSpan();
    void unescapesBacktickInMultiBacktickSpan();
    void leavesBlockquotedFenceUntouched();
    void leavesIndentedCodeBlockUntouched();
    void processesListContinuationInlineCode();
    void inlineCodeLineStartingWithBackticks();
    void unclosedBacktickIsLiteral();
    void emptyAndNoCode();
};

void TestCodeSpanFix::unescapesBackslashInCode()
{
    // Lo que emite Qt para un code span cuyo contenido es `a\b`.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("x `a\\\\b` y")),
             QStringLiteral("x `a\\b` y"));
}

void TestCodeSpanFix::unescapesAmpersandInCode()
{
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("`a\\&b`")),
             QStringLiteral("`a&b`"));
}

void TestCodeSpanFix::unescapesAllSixEscapedChars()
{
    // Qt antepone `\` a estos seis: \ & < * [ !
    QCOMPARE(mdcodespan::unescapeInlineCode(
                 QStringLiteral("`\\\\ \\& \\< \\* \\[ \\!`")),
             QStringLiteral("`\\ & < * [ !`"));
}

void TestCodeSpanFix::collapsesDoubledBackslashOnce()
{
    // `\\\\` (cuatro) → `\\` (dos): un solo nivel de des-escape, sin reprocesar.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("`a\\\\\\\\b`")),
             QStringLiteral("`a\\\\b`"));
}

void TestCodeSpanFix::leavesPlainTextEscapesUntouched()
{
    // Fuera de un code span, `\*` es un escape legítimo: no se toca.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("texto \\* y \\& fin")),
             QStringLiteral("texto \\* y \\& fin"));
}

void TestCodeSpanFix::leavesFencedCodeUntouched()
{
    // Dentro de un fence Qt NO escapa; el contenido es literal y no debe alterarse.
    const QString in = QStringLiteral("```\na\\b & c\n```");
    QCOMPARE(mdcodespan::unescapeInlineCode(in), in);
}

void TestCodeSpanFix::leavesIndentedFenceUntouched()
{
    // Qt indenta el fence tras una lista; debe seguir reconociéndose como fence.
    const QString in = QStringLiteral("- item\n\n  ```\n  a\\*b\n  ```");
    QCOMPARE(mdcodespan::unescapeInlineCode(in), in);
}

void TestCodeSpanFix::handlesMultipleSpansInOneLine()
{
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("`a\\\\b` y `c\\&d`")),
             QStringLiteral("`a\\b` y `c&d`"));
}

void TestCodeSpanFix::leavesUnescapedCodeUnchanged()
{
    const QString in = QStringLiteral("usa `printf(x)` aqui");
    QCOMPARE(mdcodespan::unescapeInlineCode(in), in);
}

void TestCodeSpanFix::handlesMultiBacktickSpan()
{
    // Code span con doble backtick (por contener un backtick): se procesa igual.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("``a\\\\b``")),
             QStringLiteral("``a\\b``"));
}

void TestCodeSpanFix::unescapesBacktickInMultiBacktickSpan()
{
    // Qt escapa el backtick interno de un span de doble backtick como `` \` ``.
    // Debe revertirse; si no, la barra se acumula en cada guardado.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("``a\\`b``")),
             QStringLiteral("``a`b``"));
}

void TestCodeSpanFix::leavesBlockquotedFenceUntouched()
{
    // Un fence dentro de una cita (`> ```) es código verbatim: su contenido no se
    // des-escapa (antes el prefijo `>` impedía reconocer el fence).
    const QString in = QStringLiteral("> ```\n> hola \\*\n> ```");
    QCOMPARE(mdcodespan::unescapeInlineCode(in), in);
}

void TestCodeSpanFix::leavesIndentedCodeBlockUntouched()
{
    // Un bloque de código indentado (4 espacios) es verbatim: Qt no escapa ahí,
    // así que no hay que des-escapar (borraría el backslash literal del usuario).
    const QString in = QStringLiteral("parrafo\n\n    codigo `\\*` literal");
    QCOMPARE(mdcodespan::unescapeInlineCode(in), in);
}

void TestCodeSpanFix::processesListContinuationInlineCode()
{
    // Una continuación de lista indentada 2 espacios NO es código indentado: su
    // código en línea sí debe des-escaparse (no debe confundirse con un bloque).
    QCOMPARE(mdcodespan::unescapeInlineCode(
                 QStringLiteral("- item\n  ver `a\\*b` aqui")),
             QStringLiteral("- item\n  ver `a*b` aqui"));
}

void TestCodeSpanFix::inlineCodeLineStartingWithBackticks()
{
    // Una línea que EMPIEZA por `` ``` `` pero es código en línea (lleva más
    // backticks) no es un fence: debe procesarse y no desactivar el resto del
    // documento.
    QCOMPARE(mdcodespan::unescapeInlineCode(
                 QStringLiteral("```a``b``` y `c\\*d` fin")),
             QStringLiteral("```a``b``` y `c*d` fin"));
}

void TestCodeSpanFix::unclosedBacktickIsLiteral()
{
    // Un backtick sin cierre no abre code span: el contenido NO se des-escapa.
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("a ` b \\* c")),
             QStringLiteral("a ` b \\* c"));
}

void TestCodeSpanFix::emptyAndNoCode()
{
    QCOMPARE(mdcodespan::unescapeInlineCode(QString()), QString());
    QCOMPARE(mdcodespan::unescapeInlineCode(QStringLiteral("solo texto plano")),
             QStringLiteral("solo texto plano"));
}

QTEST_APPLESS_MAIN(TestCodeSpanFix)
#include "tst_codespanfix.moc"
