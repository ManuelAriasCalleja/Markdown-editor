#include <QtTest>

#include <QTextDocument>

// Red de seguridad sobre la conversión que sustenta abrir/guardar: el editor
// hace setMarkdown() al abrir y toMarkdown() al guardar. Estas pruebas
// comprueban que un ciclo Markdown → documento → Markdown:
//   1) conserva el contenido y los marcadores esenciales, y
//   2) es idempotente (un segundo ciclo no cambia el resultado),
// que es la invariante robusta sin depender de la normalización exacta de Qt.
class TestMarkdownRoundtrip : public QObject
{
    Q_OBJECT

private slots:
    void bold();
    void italic();
    void heading();
    void bulletList();
    void blockquote();
    void codeBlock();
    void inlineCode();
    void highlightMark();

private:
    static QString roundtrip(const QString &markdown)
    {
        QTextDocument doc;
        doc.setMarkdown(markdown);
        return doc.toMarkdown().trimmed();
    }

    // Comprueba idempotencia y supervivencia del contenido; opcionalmente, que
    // aparezca un marcador concreto.
    void check(const QString &input, const QString &mustContain,
               const QString &marker = QString())
    {
        const QString once = roundtrip(input);
        QVERIFY2(once.contains(mustContain),
                 qPrintable(QStringLiteral("'%1' no contiene '%2'")
                                .arg(once, mustContain)));
        if (!marker.isEmpty())
            QVERIFY2(once.contains(marker),
                     qPrintable(QStringLiteral("'%1' no contiene el marcador '%2'")
                                    .arg(once, marker)));
        // Idempotente: un segundo ciclo no debe cambiar nada.
        QCOMPARE(roundtrip(once), once);
    }
};

void TestMarkdownRoundtrip::bold()
{
    check(QStringLiteral("texto **negrita** aqui"),
          QStringLiteral("negrita"), QStringLiteral("**"));
}

void TestMarkdownRoundtrip::italic()
{
    check(QStringLiteral("texto *cursiva* aqui"),
          QStringLiteral("cursiva"), QStringLiteral("*"));
}

void TestMarkdownRoundtrip::heading()
{
    check(QStringLiteral("# Titulo principal"),
          QStringLiteral("Titulo principal"), QStringLiteral("#"));
}

void TestMarkdownRoundtrip::bulletList()
{
    // No fijamos el carácter de viñeta (Qt puede normalizarlo): solo contenido
    // e idempotencia.
    check(QStringLiteral("- uno\n- dos\n- tres"), QStringLiteral("dos"));
}

void TestMarkdownRoundtrip::blockquote()
{
    check(QStringLiteral("> una cita"),
          QStringLiteral("una cita"), QStringLiteral(">"));
}

void TestMarkdownRoundtrip::codeBlock()
{
    check(QStringLiteral("```\nint x = 0;\n```"),
          QStringLiteral("int x = 0;"), QStringLiteral("```"));
}

void TestMarkdownRoundtrip::inlineCode()
{
    check(QStringLiteral("usa `printf` aqui"),
          QStringLiteral("printf"), QStringLiteral("`"));
}

void TestMarkdownRoundtrip::highlightMark()
{
    // `==` no es delimitador en md4c: la marca viaja como texto literal, con sus
    // delimitadores intactos (el resaltado es solo presentación, aparte).
    check(QStringLiteral("un ==texto resaltado== aqui"),
          QStringLiteral("texto resaltado"), QStringLiteral("=="));
}

QTEST_MAIN(TestMarkdownRoundtrip)
#include "tst_markdownroundtrip.moc"
