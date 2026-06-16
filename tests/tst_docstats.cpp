#include <QtTest>

#include "docstats.h"

// Pruebas de la estadística pura del documento (mdstats::analyze) que alimenta la
// barra de estado y el diálogo de estadísticas.
class TestDocStats : public QObject
{
    Q_OBJECT

private slots:
    void emptyTextIsAllZero();
    void countsWordsAndChars();
    void charsNoSpacesExcludesWhitespace();
    void countsParagraphs();
    void countsSentencesCollapsingTerminators();
    void readingTimeScalesWithWords();
    void invalidWpmFallsBackToDefault();
    void normalizesParagraphSeparator();
};

void TestDocStats::emptyTextIsAllZero()
{
    const mdstats::DocStats st = mdstats::analyze(QString());
    QCOMPARE(st.words, 0);
    QCOMPARE(st.chars, 0);
    QCOMPARE(st.charsNoSpaces, 0);
    QCOMPARE(st.paragraphs, 0);
    QCOMPARE(st.sentences, 0);
    QCOMPARE(st.readingMinutes, 0.0);
}

void TestDocStats::countsWordsAndChars()
{
    const mdstats::DocStats st = mdstats::analyze(QStringLiteral("hola mundo feliz"));
    QCOMPARE(st.words, 3);
    QCOMPARE(st.chars, 16);  // incluye los dos espacios
}

void TestDocStats::charsNoSpacesExcludesWhitespace()
{
    const mdstats::DocStats st = mdstats::analyze(QStringLiteral("a b\tc\nd"));
    QCOMPARE(st.chars, 7);
    QCOMPARE(st.charsNoSpaces, 4);  // a b c d -> 4 sin blancos
    QCOMPARE(st.words, 4);
}

void TestDocStats::countsParagraphs()
{
    // Tres líneas con texto, una vacía en medio (no cuenta).
    const mdstats::DocStats st =
        mdstats::analyze(QStringLiteral("uno\n\ndos\ntres"));
    QCOMPARE(st.paragraphs, 3);
}

void TestDocStats::countsSentencesCollapsingTerminators()
{
    const mdstats::DocStats st =
        mdstats::analyze(QStringLiteral("Hola. ¿Qué tal? Bien... gracias!"));
    // «.», «?», «...» (un grupo) y «!» -> 4 frases.
    QCOMPARE(st.sentences, 4);
}

void TestDocStats::readingTimeScalesWithWords()
{
    // 400 palabras a 200 ppm = 2 minutos exactos.
    const QString text = QStringLiteral("palabra ").repeated(400).trimmed();
    const mdstats::DocStats st = mdstats::analyze(text, 200);
    QCOMPARE(st.words, 400);
    QCOMPARE(st.readingMinutes, 2.0);
}

void TestDocStats::invalidWpmFallsBackToDefault()
{
    const QString text = QStringLiteral("palabra ").repeated(200).trimmed();
    const mdstats::DocStats a = mdstats::analyze(text, 0);
    const mdstats::DocStats b = mdstats::analyze(text, 200);
    QCOMPARE(a.readingMinutes, b.readingMinutes);  // 0 -> 200 por defecto
    QCOMPARE(a.readingMinutes, 1.0);
}

void TestDocStats::normalizesParagraphSeparator()
{
    // U+2029 (separador de párrafo de Qt) cuenta como salto de línea: dos
    // párrafos, no uno con un carácter raro en medio.
    QString text = QStringLiteral("uno");
    text += QChar(QChar::ParagraphSeparator);
    text += QStringLiteral("dos");
    const mdstats::DocStats st = mdstats::analyze(text);
    QCOMPARE(st.paragraphs, 2);
    QCOMPARE(st.words, 2);
}

QTEST_MAIN(TestDocStats)
#include "tst_docstats.moc"
