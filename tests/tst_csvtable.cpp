#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextTable>

#include "csvtable.h"

// Pruebas de la detección TSV/CSV y su conversión a tabla Markdown (mdcsvtable).
class TestCsvTable : public QObject
{
    Q_OBJECT
private slots:
    void detectsTabSeparated();
    void detectsCommaWithMultipleRows();
    void prefersTabOverComma();
    void rejectsProseWithCommas();
    void rejectsSingleColumn();
    void rejectsEmpty();
    void padsShortRows();
    void toMarkdownBuildsHeaderAndSeparator();
    void escapesPipeInCells();
    void roundTripsThroughMarkdown();
};

void TestCsvTable::detectsTabSeparated()
{
    const auto d = mdcsvtable::detectDelimited(QStringLiteral("a\tb\tc\n1\t2\t3"));
    QVERIFY(d.ok);
    QCOMPARE(d.rows.size(), 2);
    QCOMPARE(d.rows.at(0), (QStringList{"a", "b", "c"}));
    QCOMPARE(d.rows.at(1), (QStringList{"1", "2", "3"}));
}

void TestCsvTable::detectsCommaWithMultipleRows()
{
    const auto d = mdcsvtable::detectDelimited(QStringLiteral("a,b\n1,2\n3,4"));
    QVERIFY(d.ok);
    QCOMPARE(d.rows.size(), 3);
    QCOMPARE(d.rows.at(2), (QStringList{"3", "4"}));
}

void TestCsvTable::prefersTabOverComma()
{
    // Una celda con coma pero separada por tabuladores: el TAB manda.
    const auto d = mdcsvtable::detectDelimited(QStringLiteral("a\t1,5\nb\t2,5"));
    QVERIFY(d.ok);
    QCOMPARE(d.rows.at(0), (QStringList{"a", "1,5"}));
}

void TestCsvTable::rejectsProseWithCommas()
{
    // Una sola línea con comas es probablemente prosa, no una tabla.
    QVERIFY(!mdcsvtable::detectDelimited(QStringLiteral("uno, dos y tres")).ok);
    // Dos líneas pero solo una con coma: tampoco.
    QVERIFY(!mdcsvtable::detectDelimited(QStringLiteral("uno, dos\nsin coma aquí")).ok);
}

void TestCsvTable::rejectsSingleColumn()
{
    QVERIFY(!mdcsvtable::detectDelimited(QStringLiteral("solo\ntexto\nsin\ndelimitador")).ok);
}

void TestCsvTable::rejectsEmpty()
{
    QVERIFY(!mdcsvtable::detectDelimited(QString()).ok);
    QVERIFY(!mdcsvtable::detectDelimited(QStringLiteral("\n\n")).ok);
}

void TestCsvTable::padsShortRows()
{
    const auto d = mdcsvtable::detectDelimited(QStringLiteral("a\tb\tc\n1\t2"));
    QVERIFY(d.ok);
    QCOMPARE(d.rows.at(1).size(), 3);       // rellenada a 3 columnas
    QCOMPARE(d.rows.at(1).at(2), QString());  // celda vacía añadida
}

void TestCsvTable::toMarkdownBuildsHeaderAndSeparator()
{
    const QString md = mdcsvtable::toMarkdownTable({{"a", "b"}, {"1", "2"}});
    const QStringList lines = md.split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QCOMPARE(lines.at(0), QStringLiteral("| a | b |"));
    QCOMPARE(lines.at(1), QStringLiteral("| --- | --- |"));
    QCOMPARE(lines.at(2), QStringLiteral("| 1 | 2 |"));
}

void TestCsvTable::escapesPipeInCells()
{
    const QString md = mdcsvtable::toMarkdownTable({{"a|b", "c"}});
    QVERIFY(md.contains(QStringLiteral("a\\|b")));  // la barra se escapa, no parte la celda
}

void TestCsvTable::roundTripsThroughMarkdown()
{
    // El Markdown generado debe parsearse de vuelta como una QTextTable real.
    const QString md = mdcsvtable::toMarkdownTable({{"Col A", "Col B"}, {"1", "2"}});
    QTextDocument doc;
    doc.setMarkdown(md);

    QTextTable *table = nullptr;
    for (QTextBlock b = doc.begin(); b != doc.end() && !table; b = b.next())
        table = QTextCursor(b).currentTable();
    QVERIFY(table != nullptr);
    QCOMPARE(table->columns(), 2);
    QCOMPARE(table->rows(), 2);  // cabecera + una fila de datos
}

QTEST_MAIN(TestCsvTable)
#include "tst_csvtable.moc"
