#include <QtTest>

#include "spellchecker.h"

// Pruebas del motor (Hunspell). Dependen de que haya diccionarios del sistema
// instalados; si no, se saltan (QSKIP) para no romper en entornos sin ellos.
class TestSpellChecker : public QObject
{
    Q_OBJECT

private:
    // Carga el primer idioma disponible que empiece por `prefix`, o salta.
    static QString anyLanguage(SpellChecker &c, const QString &prefix)
    {
        for (const QString &l : SpellChecker::availableLanguages())
            if (l.startsWith(prefix) && c.setLanguage(l))
                return l;
        return QString();
    }

private slots:
    void unavailableWhenNoLanguageLoaded();
    void checksEnglishWords();
    void suggestsForMisspelling();
    void ignoreAndPersonalMakeWordCorrect();
};

void TestSpellChecker::unavailableWhenNoLanguageLoaded()
{
    SpellChecker c;
    QVERIFY(!c.isAvailable());
    // Sin diccionario, no marca nada como erróneo.
    QVERIFY(c.isCorrect(QStringLiteral("xyzzyqwerty")));
    QVERIFY(c.suggestions(QStringLiteral("xyzzy")).isEmpty());
    // Idioma inexistente: queda sin cargar.
    QVERIFY(!c.setLanguage(QStringLiteral("zz")));
    QVERIFY(!c.isAvailable());
}

void TestSpellChecker::checksEnglishWords()
{
    SpellChecker c;
    const QString lang = anyLanguage(c, QStringLiteral("en"));
    if (lang.isEmpty())
        QSKIP("No hay diccionario en_* instalado");
    QVERIFY(c.isAvailable());
    QVERIFY(c.isCorrect(QStringLiteral("house")));
    QVERIFY(c.isCorrect(QStringLiteral("Hello")));
    QVERIFY(!c.isCorrect(QStringLiteral("housse")));   // errata
    QVERIFY(!c.isCorrect(QStringLiteral("teh")));
}

void TestSpellChecker::suggestsForMisspelling()
{
    SpellChecker c;
    if (anyLanguage(c, QStringLiteral("en")).isEmpty())
        QSKIP("No hay diccionario en_* instalado");
    const QStringList sug = c.suggestions(QStringLiteral("housse"));
    QVERIFY(!sug.isEmpty());
    QVERIFY(sug.contains(QStringLiteral("house")));
}

void TestSpellChecker::ignoreAndPersonalMakeWordCorrect()
{
    SpellChecker c;
    if (anyLanguage(c, QStringLiteral("en")).isEmpty())
        QSKIP("No hay diccionario en_* instalado");
    const QString w = QStringLiteral("zzqquux");
    QVERIFY(!c.isCorrect(w));
    c.ignoreWord(w);
    QVERIFY(c.isCorrect(w));

    const QString p = QStringLiteral("Anthropic");
    c.addToPersonal(p);
    QVERIFY(c.isCorrect(p));
    QVERIFY(c.personalWords().contains(p));
}

QTEST_MAIN(TestSpellChecker)
#include "tst_spellchecker.moc"
