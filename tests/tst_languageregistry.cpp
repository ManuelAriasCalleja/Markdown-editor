#include <QtTest>

#include "languageregistry.h"

// Pruebas del registro de lenguajes: el mapeo alias → LangSpec.
class TestLanguageRegistry : public QObject
{
    Q_OBJECT

private slots:
    void cppHasKeywordsAndCComments();
    void javascriptFamily();
    void pythonUsesHashComments();
    void hashFamilyHasNoKeywords();
    void aliasesAreCaseAndSpaceInsensitive();
    void unknownLanguageGetsGenericSpec();
    void emptyLanguageGetsGenericSpec();
};

void TestLanguageRegistry::cppHasKeywordsAndCComments()
{
    const LangSpec s = LanguageRegistry::specFor(QStringLiteral("cpp"));
    QVERIFY(s.keywords.contains(QStringLiteral("class")));
    QVERIFY(s.keywords.contains(QStringLiteral("nullptr")));
    QVERIFY(s.slash);   // //
    QVERIFY(s.block);   // /* */
    QVERIFY(!s.hash);   // sin #
}

void TestLanguageRegistry::javascriptFamily()
{
    // Varios alias comparten la misma familia.
    for (const char *alias : {"js", "javascript", "ts", "typescript", "json"}) {
        const LangSpec s = LanguageRegistry::specFor(QString::fromLatin1(alias));
        QVERIFY2(s.keywords.contains(QStringLiteral("function")), alias);
        QVERIFY2(s.slash && s.block && !s.hash, alias);
    }
}

void TestLanguageRegistry::pythonUsesHashComments()
{
    const LangSpec s = LanguageRegistry::specFor(QStringLiteral("python"));
    QVERIFY(s.keywords.contains(QStringLiteral("def")));
    QVERIFY(s.hash);
    QVERIFY(!s.slash);
    QVERIFY(!s.block);
}

void TestLanguageRegistry::hashFamilyHasNoKeywords()
{
    const LangSpec s = LanguageRegistry::specFor(QStringLiteral("bash"));
    QVERIFY(s.keywords.isEmpty());
    QVERIFY(s.hash);
    QVERIFY(!s.slash);
    QVERIFY(!s.block);
}

void TestLanguageRegistry::aliasesAreCaseAndSpaceInsensitive()
{
    QVERIFY(LanguageRegistry::specFor(QStringLiteral("PY")).keywords.contains(
        QStringLiteral("def")));
    QVERIFY(LanguageRegistry::specFor(QStringLiteral("  Cpp  ")).keywords.contains(
        QStringLiteral("class")));
}

void TestLanguageRegistry::unknownLanguageGetsGenericSpec()
{
    const LangSpec s = LanguageRegistry::specFor(QStringLiteral("brainfuck"));
    QVERIFY(s.keywords.isEmpty());
    // Genérico: resalta los tres estilos de comentario.
    QVERIFY(s.slash);
    QVERIFY(s.hash);
    QVERIFY(s.block);
}

void TestLanguageRegistry::emptyLanguageGetsGenericSpec()
{
    const LangSpec s = LanguageRegistry::specFor(QString());
    QVERIFY(s.keywords.isEmpty());
    QVERIFY(s.slash && s.hash && s.block);
}

QTEST_APPLESS_MAIN(TestLanguageRegistry)
#include "tst_languageregistry.moc"
