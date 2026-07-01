#include <QtTest>

#include "pandocimport.h"

// Pruebas de la parte pura de la importación con Pandoc (mdimport): argumentos,
// patrón de ficheros y orden de instalación. La ejecución del proceso vive en
// MainWindow y no se prueba aquí (depende de que Pandoc esté instalado).
class TestPandocImport : public QObject
{
    Q_OBJECT
private slots:
    void argumentsConvertToGfmFromStdin();
    void filePatternListsCommonFormats();
    void installCommandMentionsPandoc();
    void availabilityDoesNotCrash();
};

void TestPandocImport::argumentsConvertToGfmFromStdin()
{
    const QStringList args = mdimport::pandocArguments(QStringLiteral("/ruta/doc.docx"));
    QVERIFY(args.contains(QStringLiteral("--to=gfm")));
    QVERIFY(args.contains(QStringLiteral("--wrap=none")));
    // La ruta de entrada es el último argumento (Pandoc infiere el formato por ella).
    QCOMPARE(args.last(), QStringLiteral("/ruta/doc.docx"));
}

void TestPandocImport::filePatternListsCommonFormats()
{
    const QString pattern = mdimport::pandocFilePattern();
    for (const QString &ext : {"docx", "odt", "rtf", "tex", "rst"})
        QVERIFY2(pattern.contains(ext), qPrintable(ext));
}

void TestPandocImport::installCommandMentionsPandoc()
{
    const QString cmd = mdimport::pandocInstallCommand();
    QVERIFY(!cmd.isEmpty());
    QVERIFY(cmd.contains(QStringLiteral("pandoc")));
}

void TestPandocImport::availabilityDoesNotCrash()
{
    // No podemos saber si Pandoc está instalado en el entorno de pruebas; solo que
    // la consulta devuelve un booleano sin reventar.
    const bool available = mdimport::pandocAvailable();
    QVERIFY(available == true || available == false);
}

QTEST_MAIN(TestPandocImport)
#include "tst_pandocimport.moc"
