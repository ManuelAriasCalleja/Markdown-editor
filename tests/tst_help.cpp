#include <QtTest>

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#include "helpdialog.h"

// La ayuda son 20 archivos Markdown (dos documentos × diez idiomas) que nadie
// compila: un enlace del índice que no cae en ningún encabezado, o una sección
// que se añade en español y se olvida en las traducciones, no da ningún error —
// simplemente el usuario pulsa y no pasa nada, o lee un manual incompleto.
// Estas pruebas son el único sitio donde eso se nota.
class TestHelp : public QObject
{
    Q_OBJECT

private slots:
    void slugMatchesTheIndexConvention();
    void suffixPicksTheManualOfTheLanguage_data();
    void suffixPicksTheManualOfTheLanguage();
    void everyIndexLinkResolves_data();
    void everyIndexLinkResolves();
    void translationsHaveTheSameSections_data();
    void translationsHaveTheSameSections();

private:
    static QString readHelp(const QString &resource);
    static QStringList headings(const QString &markdown);
    static QStringList indexAnchors(const QString &markdown);
};

// Los diez idiomas con manual: el español es la base (sufijo vacío).
static const char *const kSuffixes[] = {"", "_en", "_de", "_fr", "_it",
                                        "_pt", "_pl", "_nl", "_ro", "_zh_CN"};
static const char *const kDocuments[] = {"help-app", "help-markdown"};

QString TestHelp::readHelp(const QString &resource)
{
    QFile f(QStringLiteral(":/help/") + resource + QStringLiteral(".md"));
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    return QString::fromUtf8(f.readAll());
}

// Texto de cada encabezado ATX (`# …`, `## …`), en orden.
QStringList TestHelp::headings(const QString &markdown)
{
    static const QRegularExpression re(QStringLiteral("^(#{1,6})\\s+(.+?)\\s*$"),
                                       QRegularExpression::MultilineOption);
    QStringList out;
    auto it = re.globalMatch(markdown);
    while (it.hasNext())
        out << it.next().captured(2);
    return out;
}

// Destinos internos de los enlaces del índice: `[Texto](#ancla)` -> «ancla».
QStringList TestHelp::indexAnchors(const QString &markdown)
{
    static const QRegularExpression re(QStringLiteral("\\]\\(#([^)]+)\\)"));
    QStringList out;
    auto it = re.globalMatch(markdown);
    while (it.hasNext())
        out << it.next().captured(1);
    return out;
}

void TestHelp::slugMatchesTheIndexConvention()
{
    QCOMPARE(mdhelp::headingSlug(QStringLiteral("Enlaces e imágenes")),
             QStringLiteral("enlaces-e-imagenes"));
    QCOMPARE(mdhelp::headingSlug(QStringLiteral("Snippets (fragmentos reutilizables)")),
             QStringLiteral("snippets-fragmentos-reutilizables"));
    QCOMPARE(mdhelp::headingSlug(QStringLiteral("Extensiones que admite md-editor")),
             QStringLiteral("extensiones-que-admite-md-editor"));
}

// Qué manual abre cada idioma. Antes era un `switch` sobre `QLocale::Language`, que
// no podía distinguir el chino simplificado del tradicional (`QLocale::Chinese` vale
// para los dos) y había que ampliar a mano con cada manual nuevo. Ahora sale de la
// etiqueta canónica y de que el recurso exista, así que este test es lo que vigila
// las dos reglas: el idioma con manual abre el SUYO y el que no lo tiene cae al
// inglés, nunca a un visor vacío.
void TestHelp::suffixPicksTheManualOfTheLanguage_data()
{
    QTest::addColumn<QString>("code");
    QTest::addColumn<QString>("suffix");

    // El español es la base: sin sufijo. Vacío = «sin preferencia», que en el
    // diálogo llega solo si el locale del sistema tampoco dice nada.
    QTest::newRow("es") << QStringLiteral("es") << QString();
    QTest::newRow("es_ES") << QStringLiteral("es_ES") << QString();
    QTest::newRow("es-MX") << QStringLiteral("es-MX") << QString();
    QTest::newRow("vacio") << QString() << QString();

    // Los nueve traducidos, en cualquiera de sus formas.
    QTest::newRow("de") << QStringLiteral("de") << QStringLiteral("_de");
    QTest::newRow("de_AT") << QStringLiteral("de_AT") << QStringLiteral("_de");
    QTest::newRow("en_US") << QStringLiteral("en_US") << QStringLiteral("_en");
    QTest::newRow("fr") << QStringLiteral("fr") << QStringLiteral("_fr");
    QTest::newRow("it") << QStringLiteral("it") << QStringLiteral("_it");
    QTest::newRow("pt-BR") << QStringLiteral("pt-BR") << QStringLiteral("_pt");
    QTest::newRow("pl") << QStringLiteral("pl") << QStringLiteral("_pl");
    QTest::newRow("nl") << QStringLiteral("nl") << QStringLiteral("_nl");
    QTest::newRow("ro") << QStringLiteral("ro") << QStringLiteral("_ro");
    // El chino simplificado ya tiene manual; el sufijo lleva la región porque
    // `zh_TW` es otro recurso, no una variante del mismo (de ahí que las formas de
    // abajo, que canonizan a simplificado, abran las tres el mismo manual).
    QTest::newRow("zh_CN") << QStringLiteral("zh_CN") << QStringLiteral("_zh_CN");
    QTest::newRow("zh") << QStringLiteral("zh") << QStringLiteral("_zh_CN");
    QTest::newRow("zh-Hans") << QStringLiteral("zh-Hans") << QStringLiteral("_zh_CN");
    QTest::newRow("zh_SG") << QStringLiteral("zh_SG") << QStringLiteral("_zh_CN");

    // Sin manual traducido → inglés. El chino tradicional sigue aquí: canoniza a
    // `zh_TW`, para el que no hay `help-app_zh_TW.md`, así que cae al inglés en vez
    // de abrir el simplificado.
    QTest::newRow("zh_TW") << QStringLiteral("zh_TW") << QStringLiteral("_en");
    QTest::newRow("zh_HK") << QStringLiteral("zh_HK") << QStringLiteral("_en");
    QTest::newRow("ja") << QStringLiteral("ja") << QStringLiteral("_en");
    QTest::newRow("desconocido") << QStringLiteral("qqq_XX") << QStringLiteral("_en");
}

void TestHelp::suffixPicksTheManualOfTheLanguage()
{
    QFETCH(QString, code);
    QFETCH(QString, suffix);
    QCOMPARE(mdhelp::helpSuffixForLanguage(code), suffix);
    // El sufijo que devuelva tiene que llevar a un manual que se pueda leer: es la
    // única forma de que el diálogo no se abra en blanco.
    for (const char *doc : kDocuments)
        QVERIFY(!readHelp(QString::fromLatin1(doc) + suffix).isEmpty());
}

void TestHelp::everyIndexLinkResolves_data()
{
    QTest::addColumn<QString>("resource");
    for (const char *doc : kDocuments)
        for (const char *suffix : kSuffixes) {
            const QString res = QString::fromLatin1(doc) + QString::fromLatin1(suffix);
            QTest::newRow(qPrintable(res)) << res;
        }
}

void TestHelp::everyIndexLinkResolves()
{
    QFETCH(QString, resource);
    const QString md = readHelp(resource);
    QVERIFY2(!md.isEmpty(), qPrintable(QStringLiteral("no se pudo leer %1").arg(resource)));

    QSet<QString> slugs;
    for (const QString &h : headings(md))
        slugs.insert(mdhelp::headingSlug(h));

    const QStringList anchors = indexAnchors(md);
    QVERIFY2(!anchors.isEmpty(), "el documento debería tener índice");
    for (const QString &anchor : anchors)
        QVERIFY2(slugs.contains(anchor),
                 qPrintable(QStringLiteral("%1: el índice apunta a «%2», que no es "
                                           "ningún encabezado del documento")
                                .arg(resource, anchor)));
}

void TestHelp::translationsHaveTheSameSections_data()
{
    QTest::addColumn<QString>("document");
    QTest::addColumn<QString>("suffix");
    for (const char *doc : kDocuments)
        for (const char *suffix : kSuffixes) {
            if (*suffix == '\0')
                continue;  // el español es la referencia
            QTest::newRow(qPrintable(QString::fromLatin1(doc) + QString::fromLatin1(suffix)))
                << QString::fromLatin1(doc) << QString::fromLatin1(suffix);
        }
}

void TestHelp::translationsHaveTheSameSections()
{
    QFETCH(QString, document);
    QFETCH(QString, suffix);
    // No se comparan los textos (están traducidos), sino la ESTRUCTURA: mismo
    // número de encabezados y mismo número de entradas de índice. Es lo que se
    // desincroniza al documentar algo nuevo y traducirlo solo a medias.
    const QStringList base = headings(readHelp(document));
    const QStringList other = headings(readHelp(document + suffix));
    QVERIFY2(base.size() == other.size(),
             qPrintable(QStringLiteral("%1%2 tiene %3 encabezados y el español %4: "
                                       "falta traducir alguna sección")
                            .arg(document, suffix)
                            .arg(other.size())
                            .arg(base.size())));
    QCOMPARE(indexAnchors(readHelp(document + suffix)).size(),
             indexAnchors(readHelp(document)).size());
}

QTEST_MAIN(TestHelp)
#include "tst_help.moc"
