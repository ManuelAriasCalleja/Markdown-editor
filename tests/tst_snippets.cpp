#include <QtTest>

#include "snippets.h"

using mdsnippet::Snippet;

class TestSnippets : public QObject
{
    Q_OBJECT

private slots:
    void roundTripPreservesNameAndBody();
    void bodyWithNewlinesSurvives();
    void dropsEntriesWithoutName();
    void toleratesEntryWithoutSeparator();
};

void TestSnippets::roundTripPreservesNameAndBody()
{
    const QList<Snippet> in = {
        {QStringLiteral("Firma"), QStringLiteral("Un saludo,\n**Manuel**")},
        {QStringLiteral("Aviso"), QStringLiteral("> [!NOTE]\n> Ojo")},
    };
    const QList<Snippet> out = mdsnippet::deserialize(mdsnippet::serialize(in));
    QCOMPARE(out.size(), 2);
    QCOMPARE(out.at(0).name, in.at(0).name);
    QCOMPARE(out.at(0).body, in.at(0).body);
    QCOMPARE(out.at(1).name, in.at(1).name);
    QCOMPARE(out.at(1).body, in.at(1).body);
}

void TestSnippets::bodyWithNewlinesSurvives()
{
    const QList<Snippet> in = {{QStringLiteral("Tabla"),
                                QStringLiteral("| a | b |\n|---|---|\n| 1 | 2 |")}};
    const QList<Snippet> out = mdsnippet::deserialize(mdsnippet::serialize(in));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out.at(0).body, in.at(0).body);
}

void TestSnippets::dropsEntriesWithoutName()
{
    const QList<Snippet> in = {
        {QStringLiteral("  "), QStringLiteral("sin nombre")},
        {QString(), QStringLiteral("vacío")},
        {QStringLiteral("Válido"), QStringLiteral("ok")},
    };
    const QList<Snippet> out = mdsnippet::deserialize(mdsnippet::serialize(in));
    QCOMPARE(out.size(), 1);
    QCOMPARE(out.at(0).name, QStringLiteral("Válido"));
}

void TestSnippets::toleratesEntryWithoutSeparator()
{
    // Una entrada cruda sin separador: todo es el nombre, cuerpo vacío.
    const QList<Snippet> out = mdsnippet::deserialize({QStringLiteral("SoloNombre")});
    QCOMPARE(out.size(), 1);
    QCOMPARE(out.at(0).name, QStringLiteral("SoloNombre"));
    QVERIFY(out.at(0).body.isEmpty());
}

QTEST_APPLESS_MAIN(TestSnippets)
#include "tst_snippets.moc"
