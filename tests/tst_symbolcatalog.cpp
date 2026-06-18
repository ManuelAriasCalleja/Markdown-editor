#include <QtTest>

#include <QSet>

#include "symbolcatalog.h"

// El catálogo de símbolos especiales es datos puros: comprobamos que está bien
// formado (categorías con nombre, con símbolos de un solo punto de código, sin
// duplicados dentro de una categoría).
class TestSymbolCatalog : public QObject
{
    Q_OBJECT
private slots:
    void hasCategories();
    void everyCategoryIsWellFormed();
    void noDuplicatesWithinACategory();
};

void TestSymbolCatalog::hasCategories()
{
    QVERIFY(mdsymbols::categories().size() >= 5);
}

void TestSymbolCatalog::everyCategoryIsWellFormed()
{
    for (const mdsymbols::Category &c : mdsymbols::categories()) {
        QVERIFY2(!c.name.isEmpty(), "categoría sin nombre");
        QVERIFY2(!c.symbols.isEmpty(), qPrintable(QStringLiteral("categoría vacía: %1").arg(c.name)));
        for (const QString &s : c.symbols) {
            QVERIFY2(!s.isEmpty(), "símbolo vacío");
            // Un punto de código (1 QChar, o 2 si es par subrogado).
            QVERIFY2(s.size() == 1 || (s.size() == 2 && s.at(0).isHighSurrogate()),
                     qPrintable(QStringLiteral("símbolo con más de un carácter: %1").arg(s)));
        }
    }
}

void TestSymbolCatalog::noDuplicatesWithinACategory()
{
    for (const mdsymbols::Category &c : mdsymbols::categories()) {
        const QSet<QString> unique(c.symbols.begin(), c.symbols.end());
        QCOMPARE(unique.size(), c.symbols.size());
    }
}

QTEST_MAIN(TestSymbolCatalog)
#include "tst_symbolcatalog.moc"
