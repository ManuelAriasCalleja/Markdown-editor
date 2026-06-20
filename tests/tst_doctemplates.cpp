#include <QtTest>

#include "doctemplates.h"

class TestDocTemplates : public QObject
{
    Q_OBJECT
private slots:
    void hasTemplates();
    void allFieldsNonEmpty();
    void certificateUsesHeadingForCertifico();
    void frontMatterTemplatesAreWellFormed();
};

void TestDocTemplates::hasTemplates()
{
    QCOMPARE(mdtemplate::all().size(), 10);
}

void TestDocTemplates::allFieldsNonEmpty()
{
    const auto templates = mdtemplate::all();
    QSet<QString> names;
    for (const auto &t : templates) {
        QVERIFY(!t.name.isEmpty());
        QVERIFY(!t.body.isEmpty());
        names.insert(t.name);
    }
    QCOMPARE(names.size(), templates.size());  // sin nombres duplicados
}

void TestDocTemplates::certificateUsesHeadingForCertifico()
{
    for (const auto &t : mdtemplate::all()) {
        if (t.name == QStringLiteral("Certificado")) {
            // CERTIFICO va como encabezado (Markdown no expresa tamaño de fuente).
            QVERIFY(t.body.contains(QStringLiteral("# CERTIFICO")));
            return;
        }
    }
    QFAIL("No se encontró la plantilla «Certificado»");
}

void TestDocTemplates::frontMatterTemplatesAreWellFormed()
{
    // Toda plantilla que abra con front matter debe cerrarlo (--- … ---), si no
    // load() lo tomaría por una regla horizontal.
    for (const auto &t : mdtemplate::all()) {
        if (t.body.startsWith(QStringLiteral("---\n"))) {
            const int close = t.body.indexOf(QStringLiteral("\n---\n"));
            QVERIFY2(close > 0, qPrintable(t.name));
        }
    }
}

QTEST_MAIN(TestDocTemplates)
#include "tst_doctemplates.moc"
