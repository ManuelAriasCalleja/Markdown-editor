#include <QtTest>

#include <QTextDocument>

#include "admonitions.h"
#include "tableedit.h"

class TestAdmonitions : public QObject
{
    Q_OBJECT
private slots:
    void detectsAllTypes();
    void rejectsNonMarkers();
    void caseInsensitiveAndSpaces();
    void detectsMarkerAsFirstLineOfBlock();
    void unescapeOnlyOnQuoteLines();
    void roundTripsThroughDocumentMarkdown();
    void skeletonHasMarkerAndBlankQuoteLine();
};

void TestAdmonitions::detectsAllTypes()
{
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!NOTE]")), QStringLiteral("NOTE"));
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!TIP]")), QStringLiteral("TIP"));
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!IMPORTANT]")), QStringLiteral("IMPORTANT"));
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!WARNING]")), QStringLiteral("WARNING"));
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!CAUTION]")), QStringLiteral("CAUTION"));
    QCOMPARE(mdadmonition::types().size(), 5);
}

void TestAdmonitions::rejectsNonMarkers()
{
    QVERIFY(mdadmonition::markerKeyword(QString()).isEmpty());
    QVERIFY(mdadmonition::markerKeyword(QStringLiteral("texto normal")).isEmpty());
    QVERIFY(mdadmonition::markerKeyword(QStringLiteral("[!UNKNOWN]")).isEmpty());
    QVERIFY(mdadmonition::markerKeyword(QStringLiteral("[NOTE]")).isEmpty());
    QVERIFY(mdadmonition::markerKeyword(QStringLiteral("[!NOTE] con texto detrás")).isEmpty());
}

void TestAdmonitions::caseInsensitiveAndSpaces()
{
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("[!note]")), QStringLiteral("NOTE"));
    QCOMPARE(mdadmonition::markerKeyword(QStringLiteral("  [!Warning]  ")), QStringLiteral("WARNING"));
}

void TestAdmonitions::detectsMarkerAsFirstLineOfBlock()
{
    // Dentro de un QTextBlock, los saltos internos son U+2028/U+2029.
    const QString block = QStringLiteral("[!NOTE]") + QChar(QChar::LineSeparator)
                          + QStringLiteral("contenido");
    QCOMPARE(mdadmonition::markerKeyword(block), QStringLiteral("NOTE"));
}

void TestAdmonitions::unescapeOnlyOnQuoteLines()
{
    // En línea de cita: se quita la barra.
    QCOMPARE(mdadmonition::unescapeMarkers(QStringLiteral("> \\[!NOTE]")),
             QStringLiteral("> [!NOTE]"));
    QCOMPARE(mdadmonition::unescapeMarkers(QStringLiteral("> \\[!warning]\n> hola")),
             QStringLiteral("> [!warning]\n> hola"));
    // Fuera de una cita: no se toca (podría ser texto literal a propósito).
    QCOMPARE(mdadmonition::unescapeMarkers(QStringLiteral("texto \\[!NOTE] suelto")),
             QStringLiteral("texto \\[!NOTE] suelto"));
}

void TestAdmonitions::roundTripsThroughDocumentMarkdown()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("> [!NOTE]\n>\n> Cuerpo del aviso.\n"));
    mdadmonition::renderAdmonitionsInDocument(&doc);  // el estilo no debe romper nada
    const QString md = mdtable::documentMarkdown(&doc);
    // El marcador sobrevive SIN escape (apto para GitHub).
    QVERIFY(md.contains(QStringLiteral("> [!NOTE]")));
    QVERIFY(!md.contains(QStringLiteral("\\[!NOTE]")));
    QVERIFY(md.contains(QStringLiteral("Cuerpo del aviso.")));
}

void TestAdmonitions::skeletonHasMarkerAndBlankQuoteLine()
{
    const QString s = mdadmonition::skeleton(QStringLiteral("TIP"));
    QVERIFY(s.startsWith(QStringLiteral("> [!TIP]\n")));
    // Línea de cita en blanco para separar marcador y contenido en bloques.
    QVERIFY(s.contains(QStringLiteral("\n>\n")));
}

QTEST_MAIN(TestAdmonitions)
#include "tst_admonitions.moc"
