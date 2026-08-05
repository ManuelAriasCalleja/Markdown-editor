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
    void countsCjkCharactersAsWords();
    void cjkReadingTimeUsesItsOwnPace();
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

// Chino, japonés y coreano no separan las palabras con espacios: partir por
// espacios contaba un párrafo entero como UNA palabra. Cada ideograma, kana o
// sílaba hangul cuenta como una palabra; su puntuación, no.
void TestDocStats::countsCjkCharactersAsWords()
{
    const mdstats::DocStats zh = mdstats::analyze(QString::fromUtf8(u8"中文文本"));
    QCOMPARE(zh.words, 4);
    QCOMPARE(zh.cjkChars, 4);

    // La puntuación no es palabra (como no lo es la latina).
    QCOMPARE(mdstats::analyze(QString::fromUtf8(u8"中文，文本。")).words, 4);

    // Mezcla sin espacio: dos ideogramas + una palabra latina.
    const mdstats::DocStats mix = mdstats::analyze(QString::fromUtf8(u8"混合español"));
    QCOMPARE(mix.words, 3);
    QCOMPARE(mix.cjkChars, 2);

    // Kana y hangul cuentan igual.
    QCOMPARE(mdstats::analyze(QString::fromUtf8(u8"テキスト")).words, 4);
    QCOMPARE(mdstats::analyze(QString::fromUtf8(u8"텍스트")).words, 3);

    // Y el texto latino sigue contándose exactamente igual que antes.
    const mdstats::DocStats es = mdstats::analyze(QStringLiteral("hola mundo feliz"));
    QCOMPARE(es.words, 3);
    QCOMPARE(es.cjkChars, 0);
}

// Los ideogramas se leen más deprisa que las palabras, así que el tiempo no puede
// salir de dividir el total por la misma velocidad.
void TestDocStats::cjkReadingTimeUsesItsOwnPace()
{
    // 400 ideogramas a 400 caracteres/minuto (200 ppm x 2) = 1 minuto.
    const QString han = QString::fromUtf8(u8"字").repeated(400);
    const mdstats::DocStats zh = mdstats::analyze(han, 200);
    QCOMPARE(zh.words, 400);
    QCOMPARE(zh.readingMinutes, 1.0);
    // Las mismas 400 palabras latinas, el doble de tiempo.
    QCOMPARE(mdstats::analyze(QStringLiteral("palabra ").repeated(400).trimmed(), 200)
                 .readingMinutes, 2.0);
    // Mezclado: se suman los dos ritmos (200 palabras + 400 ideogramas = 1 + 1).
    const mdstats::DocStats mix = mdstats::analyze(
        QStringLiteral("palabra ").repeated(200) + han, 200);
    QCOMPARE(mix.readingMinutes, 2.0);
}

QTEST_MAIN(TestDocStats)
#include "tst_docstats.moc"
