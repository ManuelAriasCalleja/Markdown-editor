#include <QtTest>

#include <QFile>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>

#include "helpdialog.h"

// La ayuda son 18 archivos Markdown (dos documentos × nueve idiomas) que nadie
// compila: un enlace del índice que no cae en ningún encabezado, o una sección
// que se añade en español y se olvida en las traducciones, no da ningún error —
// simplemente el usuario pulsa y no pasa nada, o lee un manual incompleto.
// Estas pruebas son el único sitio donde eso se nota.
class TestHelp : public QObject
{
    Q_OBJECT

private slots:
    void slugMatchesTheIndexConvention();
    void everyIndexLinkResolves_data();
    void everyIndexLinkResolves();
    void translationsHaveTheSameSections_data();
    void translationsHaveTheSameSections();

private:
    static QString readHelp(const QString &resource);
    static QStringList headings(const QString &markdown);
    static QStringList indexAnchors(const QString &markdown);
};

// Los nueve idiomas con manual: el español es la base (sufijo vacío).
static const char *const kSuffixes[] = {"", "_en", "_de", "_fr", "_it",
                                        "_pt", "_pl", "_nl", "_ro"};
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
