#include <QtTest>

#include "usertemplate.h"

// Pruebas de la (de)serialización de las plantillas de usuario (mdusertemplate).
class TestUserTemplate : public QObject
{
    Q_OBJECT
private slots:
    void roundTripPreservesFields();
    void bodyWithNewlinesSurvives();
    void emptyNameDropped();
    void invalidCategoryFallsBackToPersonal();
};

void TestUserTemplate::roundTripPreservesFields()
{
    const QList<mdusertemplate::UserTemplate> in = {
        {QStringLiteral("Contrato"), QStringLiteral("# Contrato\n"), mdtemplate::Category::Legal},
        {QStringLiteral("ADR"), QStringLiteral("# Decisión\n"), mdtemplate::Category::Programming},
    };
    const QList<mdusertemplate::UserTemplate> out =
        mdusertemplate::deserialize(mdusertemplate::serialize(in));
    QCOMPARE(out.size(), 2);
    QCOMPARE(out[0].name, QStringLiteral("Contrato"));
    QCOMPARE(out[0].body, QStringLiteral("# Contrato\n"));
    QCOMPARE(out[0].category, mdtemplate::Category::Legal);
    QCOMPARE(out[1].category, mdtemplate::Category::Programming);
}

void TestUserTemplate::bodyWithNewlinesSurvives()
{
    const QString body = QStringLiteral("Línea 1\n\nLínea 3\n- viñeta\n");
    const QList<mdusertemplate::UserTemplate> in = {
        {QStringLiteral("X"), body, mdtemplate::Category::Personal}};
    const auto out = mdusertemplate::deserialize(mdusertemplate::serialize(in));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].body, body);
}

void TestUserTemplate::emptyNameDropped()
{
    const QList<mdusertemplate::UserTemplate> in = {
        {QStringLiteral("  "), QStringLiteral("cuerpo"), mdtemplate::Category::Personal},
        {QStringLiteral("Buena"), QStringLiteral("cuerpo"), mdtemplate::Category::Personal}};
    const auto out = mdusertemplate::deserialize(mdusertemplate::serialize(in));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].name, QStringLiteral("Buena"));
}

void TestUserTemplate::invalidCategoryFallsBackToPersonal()
{
    // Una entrada con categoría fuera de rango (999) debe caer a Personal, no romper.
    const QChar sep(0x1F);
    const QStringList stored = {QStringLiteral("Nombre") + sep + QStringLiteral("999") + sep
                               + QStringLiteral("cuerpo")};
    const auto out = mdusertemplate::deserialize(stored);
    QCOMPARE(out.size(), 1);
    QCOMPARE(out[0].category, mdtemplate::Category::Personal);
    QCOMPARE(out[0].body, QStringLiteral("cuerpo"));
}

QTEST_MAIN(TestUserTemplate)
#include "tst_usertemplate.moc"
