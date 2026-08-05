#include <QtTest>

#include "langtag.h"

// Pruebas de la etiqueta canónica de idioma: que todas las formas de escribir un
// código lleven al mismo recurso, y que el chino simplificado y el tradicional NO
// acaben en el mismo (que es lo que hacía `code.left(2)`).
class TestLangTag : public QObject
{
    Q_OBJECT

private slots:
    void collapsesRegionForLatinLanguages();
    void separatesSimplifiedFromTraditionalChinese();
    void chineseUsesScriptNotTerritory();
    void emptyAndUnknownStayHarmless();
};

void TestLangTag::collapsesRegionForLatinLanguages()
{
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("es")), QStringLiteral("es"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("es_ES")), QStringLiteral("es"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("es-ES")), QStringLiteral("es"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("ES")), QStringLiteral("es"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("pt_BR")), QStringLiteral("pt"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("en-US")), QStringLiteral("en"));
}

void TestLangTag::separatesSimplifiedFromTraditionalChinese()
{
    // Simplificado y tradicional son dos recursos distintos: manual y traducción
    // propios. Confundirlos daba el manual equivocado al lector.
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh")), QStringLiteral("zh_CN"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh_CN")), QStringLiteral("zh_CN"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh-Hans")), QStringLiteral("zh_CN"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh-Hant")), QStringLiteral("zh_TW"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh_TW")), QStringLiteral("zh_TW"));
    QVERIFY(mdlang::canonicalTag(QStringLiteral("zh_CN"))
            != mdlang::canonicalTag(QStringLiteral("zh_TW")));
}

// El territorio no basta para decidir la escritura, y es el error fácil: Singapur
// escribe en simplificado y Hong Kong en tradicional, y ninguno de los dos se llama
// CN ni TW. Qt deduce el script aunque solo se le dé el territorio.
void TestLangTag::chineseUsesScriptNotTerritory()
{
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh_SG")), QStringLiteral("zh_CN"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh_HK")), QStringLiteral("zh_TW"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh-Hant-HK")), QStringLiteral("zh_TW"));
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("zh-Hans-CN")), QStringLiteral("zh_CN"));
}

void TestLangTag::emptyAndUnknownStayHarmless()
{
    // Vacío = «sin preferencia»: quien llama decide (locale del sistema, inglés…).
    // No puede inventarse un idioma.
    QVERIFY(mdlang::canonicalTag(QString()).isEmpty());
    // Un código que Qt no reconoce se queda en su prefijo, no en el locale «C» ni en
    // el del sistema: garbage in, garbage out, pero nada de otro idioma.
    QCOMPARE(mdlang::canonicalTag(QStringLiteral("qqq_XX")), QStringLiteral("qqq"));
}

QTEST_MAIN(TestLangTag)
#include "tst_langtag.moc"
