#include <QtTest>

#include "spellscan.h"

// Pruebas de la lógica pura del corrector: tokenización de palabras y elección
// de diccionario por idioma.
class TestSpellScan : public QObject
{
    Q_OBJECT

private:
    // Devuelve las palabras de `text` como QStringList, para comparar cómodo.
    static QStringList words(const QString &text)
    {
        QStringList out;
        for (const mdspell::Word &w : mdspell::tokenize(text))
            out << text.mid(w.start, w.length);
        return out;
    }

private slots:
    void tokenizeSplitsOnWhitespaceAndPunctuation();
    void tokenizeKeepsInternalApostrophes();
    void tokenizeTrimsEdgeApostrophes();
    void tokenizeSkipsTokensWithDigits();
    void tokenizeSplitsOnHyphen();
    void tokenizeHandlesUnicodeLetters();
    void tokenizeOffsetsAreCorrect();
    void tokenizeSkipsUnsegmentedScripts();
    void pickDictionaryExactRegion();
    void pickDictionaryFallsBackToBase();
    void pickDictionaryFallsBackToAnyRegion();
    void pickDictionaryReturnsEmptyWhenAbsent();

    void packageNameFollowsDistroConvention();
    void installCommandPerDistroFamily();
    void installCommandEmptyOutsideLinux();
    void dictionaryUrlsCoverInterfaceLanguages();
    void dictionaryUrlsEmptyForUnknownLanguage();
    void languagesWithoutAnyDictionary();
};

void TestSpellScan::tokenizeSplitsOnWhitespaceAndPunctuation()
{
    QCOMPARE(words(QStringLiteral("Hola, mundo cruel.")),
             (QStringList{QStringLiteral("Hola"), QStringLiteral("mundo"), QStringLiteral("cruel")}));
    QCOMPARE(words(QStringLiteral("  ¡Vaya!  ")), (QStringList{QStringLiteral("Vaya")}));
    QCOMPARE(words(QString()), QStringList());
}

void TestSpellScan::tokenizeKeepsInternalApostrophes()
{
    QCOMPARE(words(QStringLiteral("don't can't")),
             (QStringList{QStringLiteral("don't"), QStringLiteral("can't")}));
    // Apóstrofo tipográfico ’ (U+2019).
    QCOMPARE(words(QStringLiteral("l’eau")), (QStringList{QStringLiteral("l’eau")}));
}

void TestSpellScan::tokenizeTrimsEdgeApostrophes()
{
    QCOMPARE(words(QStringLiteral("'tis dogs'")),
             (QStringList{QStringLiteral("tis"), QStringLiteral("dogs")}));
}

void TestSpellScan::tokenizeSkipsTokensWithDigits()
{
    QCOMPARE(words(QStringLiteral("h2o utf8 v1 abc")), (QStringList{QStringLiteral("abc")}));
    QCOMPARE(words(QStringLiteral("123 456")), QStringList());
}

void TestSpellScan::tokenizeSplitsOnHyphen()
{
    QCOMPARE(words(QStringLiteral("bien-venido")),
             (QStringList{QStringLiteral("bien"), QStringLiteral("venido")}));
}

void TestSpellScan::tokenizeHandlesUnicodeLetters()
{
    QCOMPARE(words(QStringLiteral("café señor mañana ÑOÑO")),
             (QStringList{QStringLiteral("café"), QStringLiteral("señor"),
                          QStringLiteral("mañana"), QStringLiteral("ÑOÑO")}));
}

void TestSpellScan::tokenizeOffsetsAreCorrect()
{
    const QString text = QStringLiteral("ab cd");
    const QList<mdspell::Word> ws = mdspell::tokenize(text);
    QCOMPARE(ws.size(), 2);
    QCOMPARE(ws.at(0).start, 0);
    QCOMPARE(ws.at(0).length, 2);
    QCOMPARE(ws.at(1).start, 3);
    QCOMPARE(ws.at(1).length, 2);
}

// Las escrituras sin espacios entre palabras no se tokenizan: para
// `isLetterOrNumber` son letras, así que una frase china entera salía como UNA
// palabra que ningún diccionario reconoce y el párrafo aparecía subrayado de punta
// a punta. Lo latino que haya entremedias sí se sigue corrigiendo.
void TestSpellScan::tokenizeSkipsUnsegmentedScripts()
{
    QCOMPARE(words(QString::fromUtf8(u8"中文文本，混合")), QStringList());
    QCOMPARE(words(QString::fromUtf8(u8"日本語のテキスト")), QStringList());
    QCOMPARE(words(QString::fromUtf8(u8"한국어 텍스트")), QStringList());
    // Mezcla, con y sin espacio: solo salen las palabras latinas.
    QCOMPARE(words(QString::fromUtf8(u8"混合español y 中文")),
             (QStringList{QString::fromUtf8(u8"español"), QStringLiteral("y")}));
    // Y no se ha roto nada de lo de siempre.
    QCOMPARE(words(QStringLiteral("Hola mundo")),
             (QStringList{QStringLiteral("Hola"), QStringLiteral("mundo")}));
}

void TestSpellScan::pickDictionaryExactRegion()
{
    const QStringList avail{QStringLiteral("es"), QStringLiteral("es_ES"), QStringLiteral("en_US")};
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("es-ES"), avail), QStringLiteral("es_ES"));
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("es_ES"), avail), QStringLiteral("es_ES"));
    // Caja indiferente en la entrada.
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("EN-us"), avail), QStringLiteral("en_US"));
}

void TestSpellScan::pickDictionaryFallsBackToBase()
{
    const QStringList avail{QStringLiteral("es"), QStringLiteral("es_ES")};
    // es_MX no está; cae al idioma base "es".
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("es_MX"), avail), QStringLiteral("es"));
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("es"), avail), QStringLiteral("es"));
}

void TestSpellScan::pickDictionaryFallsBackToAnyRegion()
{
    const QStringList avail{QStringLiteral("pt_PT"), QStringLiteral("pt_BR")};
    // No hay "pt" base: se elige una variante regional, de forma determinista.
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("pt"), avail), QStringLiteral("pt_BR"));
}

void TestSpellScan::pickDictionaryReturnsEmptyWhenAbsent()
{
    const QStringList avail{QStringLiteral("es"), QStringLiteral("en_US")};
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("de"), avail), QString());
    QCOMPARE(mdspell::pickDictionary(QStringLiteral("es"), QStringList()), QString());
}

void TestSpellScan::packageNameFollowsDistroConvention()
{
    // Debian, Fedora y Arch nombran igual el paquete: hunspell-<idioma en
    // minúsculas y con guion>, aunque el diccionario se llame es_ES.
    QCOMPARE(mdspell::dictionaryPackage(QStringLiteral("es")), QStringLiteral("hunspell-es"));
    QCOMPARE(mdspell::dictionaryPackage(QStringLiteral("en_US")), QStringLiteral("hunspell-en-us"));
    QCOMPARE(mdspell::dictionaryPackage(QStringLiteral("pt_BR")), QStringLiteral("hunspell-pt-br"));
}

void TestSpellScan::installCommandPerDistroFamily()
{
    // La orden que se le enseña al usuario tiene que ser la de SU distribución:
    // «sudo apt install» en Fedora no sirve de nada.
    QCOMPARE(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("ubuntu")),
             QStringLiteral("sudo apt install hunspell-es"));
    QCOMPARE(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("fedora")),
             QStringLiteral("sudo dnf install hunspell-es"));
    QCOMPARE(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("arch")),
             QStringLiteral("sudo pacman -S hunspell-es"));
    QCOMPARE(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("opensuse-leap")),
             QStringLiteral("sudo zypper install hunspell-es"));
    // Distribución desconocida: apt, que es lo más extendido, en vez de nada.
    QCOMPARE(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("loquesea")),
             QStringLiteral("sudo apt install hunspell-es"));
}

void TestSpellScan::installCommandEmptyOutsideLinux()
{
    // Windows y macOS no tienen repositorio de diccionarios: no hay orden que dar
    // y el llamante ofrece la vía manual (copiar los ficheros en su carpeta).
    QVERIFY(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("windows")).isEmpty());
    QVERIFY(mdspell::dictionaryInstallCommand(QStringLiteral("es"), QStringLiteral("macos")).isEmpty());
}

void TestSpellScan::dictionaryUrlsCoverInterfaceLanguages()
{
    // Los nueve idiomas de la interfaz tienen que poder descargarse desde el
    // programa; son los mismos que empaqueta scripts/fetch-dictionaries.sh.
    const QStringList langs = {QStringLiteral("es"), QStringLiteral("en"), QStringLiteral("de"),
                               QStringLiteral("fr"), QStringLiteral("it"), QStringLiteral("pt"),
                               QStringLiteral("pl"), QStringLiteral("nl"), QStringLiteral("ro")};
    for (const QString &lang : langs) {
        const auto urls = mdspell::dictionaryUrls(lang);
        QVERIFY2(urls.first.endsWith(QStringLiteral(".aff")), qPrintable(lang));
        QVERIFY2(urls.second.endsWith(QStringLiteral(".dic")), qPrintable(lang));
        // El .aff y el .dic tienen que ser del MISMO diccionario: mezclarlos daría
        // un diccionario que carga y corrige mal.
        QCOMPARE(urls.first.chopped(4), urls.second.chopped(4));
    }
    // La variante regional y el idioma a secas llevan al mismo sitio.
    QCOMPARE(mdspell::dictionaryUrls(QStringLiteral("es_ES")),
             mdspell::dictionaryUrls(QStringLiteral("es")));
    QCOMPARE(mdspell::dictionaryUrls(QStringLiteral("pt-BR")),
             mdspell::dictionaryUrls(QStringLiteral("pt")));
}

void TestSpellScan::dictionaryUrlsEmptyForUnknownLanguage()
{
    // Idioma fuera de los nueve: sin descarga (el programa ofrece la vía manual).
    QVERIFY(mdspell::dictionaryUrls(QStringLiteral("ja")).first.isEmpty());
    QVERIFY(mdspell::dictionaryUrls(QString()).first.isEmpty());
}

// «No está instalado» y «no existe» son dos cosas distintas: al primero se le dice
// al usuario cómo instalarlo; al segundo, solo que no lo va a haber. Confundirlos
// mandaba a instalar `hunspell-zh`, que no existe en ninguna distribución.
void TestSpellScan::languagesWithoutAnyDictionary()
{
    QVERIFY(!mdspell::hasHunspellDictionary(QStringLiteral("zh")));
    QVERIFY(!mdspell::hasHunspellDictionary(QStringLiteral("zh_CN")));
    QVERIFY(!mdspell::hasHunspellDictionary(QStringLiteral("zh-TW")));
    QVERIFY(!mdspell::hasHunspellDictionary(QStringLiteral("ja")));
    QVERIFY(!mdspell::hasHunspellDictionary(QStringLiteral("ko_KR")));
    // Los idiomas de la interfaz sí tienen diccionario, y también los que no
    // empaquetamos pero existen (ruso, turco): a esos hay que mandarlos a instalar.
    for (const QString &lang : {QStringLiteral("es"), QStringLiteral("en_US"),
                                QStringLiteral("de"), QStringLiteral("fr"),
                                QStringLiteral("it"), QStringLiteral("pt_BR"),
                                QStringLiteral("pl"), QStringLiteral("nl"),
                                QStringLiteral("ro"), QStringLiteral("ru"),
                                QStringLiteral("tr")})
        QVERIFY2(mdspell::hasHunspellDictionary(lang), qPrintable(lang));
}

QTEST_MAIN(TestSpellScan)
#include "tst_spellscan.moc"