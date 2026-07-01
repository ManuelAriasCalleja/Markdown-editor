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
    void everyTemplateHasANamedCategory();
    void categoriesInOrderHasNoDuplicates();
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

void TestDocTemplates::everyTemplateHasANamedCategory()
{
    const QList<mdtemplate::Category> order = mdtemplate::categoriesInOrder();
    for (const auto &t : mdtemplate::all()) {
        // La categoría de cada plantilla está en el orden de presentación...
        QVERIFY2(order.contains(t.category), qPrintable(t.name));
        // ...y tiene un nombre traducible no vacío.
        QVERIFY2(!mdtemplate::categoryName(t.category).isEmpty(), qPrintable(t.name));
    }
}

void TestDocTemplates::categoriesInOrderHasNoDuplicates()
{
    const QList<mdtemplate::Category> order = mdtemplate::categoriesInOrder();
    QSet<int> unique;
    for (const mdtemplate::Category c : order)
        unique.insert(static_cast<int>(c));
    QCOMPARE(unique.size(), order.size());  // sin categorías repetidas
    // Todo nombre de categoría es no vacío y único.
    QSet<QString> names;
    for (const mdtemplate::Category c : order)
        names.insert(mdtemplate::categoryName(c));
    QCOMPARE(names.size(), order.size());
}

QTEST_MAIN(TestDocTemplates)
#include "tst_doctemplates.moc"
