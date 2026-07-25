#include <QtTest>

#include <QDir>

#include "pandocimport.h"

// Pruebas de la parte pura de la importación con Pandoc (mdimport): argumentos,
// patrón de ficheros y orden de instalación. La ejecución del proceso vive en
// MainWindow y no se prueba aquí (depende de que Pandoc esté instalado).
class TestPandocImport : public QObject
{
    Q_OBJECT
private slots:
    void argumentsConvertToGfmFromStdin();
    void argumentsExtractMediaWhenAsked();
    void mediaDirSitsNextToTheDocument();
    void repairsImgTagsIntoMarkdown();
    void repairsFillsEmptyAltText();
    void repairsAwkwardPathsWithAngleBrackets();
    void repairsLeaveOtherHtmlAlone();
    void htmlTableBecomesPipeTable();
    void htmlTableFlattensWhatMarkdownCannotHold();
    void htmlTableKeepsInlineFormatting();
    void htmlTableLeavesUnconvertibleTextAlone();
    void filePatternListsCommonFormats();
    void installCommandMentionsPandoc();
    void availabilityDoesNotCrash();
};

void TestPandocImport::argumentsConvertToGfmFromStdin()
{
    const QStringList args = mdimport::pandocArguments(QStringLiteral("/ruta/doc.docx"));
    QVERIFY(args.contains(QStringLiteral("--to=gfm")));
    QVERIFY(args.contains(QStringLiteral("--wrap=none")));
    // Sin `--standalone`, Pandoc tira los metadatos del documento (el título).
    QVERIFY(args.contains(QStringLiteral("--standalone")));
    // Sin carpeta de medios no se pide extracción.
    QVERIFY(!args.filter(QStringLiteral("--extract-media")).size());
    // La ruta de entrada es el último argumento (Pandoc infiere el formato por ella).
    QCOMPARE(args.last(), QStringLiteral("/ruta/doc.docx"));
}

void TestPandocImport::argumentsExtractMediaWhenAsked()
{
    const QStringList args = mdimport::pandocArguments(QStringLiteral("/ruta/doc.docx"),
                                                       QStringLiteral("/ruta/doc-media"));
    // Un solo argumento con `=`: así la ruta con espacios viaja entera (QProcess no
    // pasa por el shell, pero separarlo en dos argumentos sí confundiría a Pandoc).
    QVERIFY(args.contains(QStringLiteral("--extract-media=/ruta/doc-media")));
    QCOMPARE(args.last(), QStringLiteral("/ruta/doc.docx"));
}

void TestPandocImport::mediaDirSitsNextToTheDocument()
{
    // La ruta se construye sobre QDir::rootPath() («/» en Unix, «C:/» en
    // Windows) porque mediaDirFor normaliza a absoluta: en Windows «/casa» no
    // lo es —le falta la unidad— y QFileInfo le antepone la unidad actual,
    // devolviendo «D:/casa/…» en el runner de CI.
    const QString root = QDir::rootPath();
    QCOMPARE(mdimport::mediaDirFor(root + QStringLiteral("casa/docs/Informe final.docx")),
             root + QStringLiteral("casa/docs/Informe final-media"));
    // Nombre compuesto: solo se quita la última extensión.
    QCOMPARE(mdimport::mediaDirFor(root + QStringLiteral("casa/v1.2.docx")),
             root + QStringLiteral("casa/v1.2-media"));
    QVERIFY(mdimport::mediaDirFor(QString()).isEmpty());
}

void TestPandocImport::repairsImgTagsIntoMarkdown()
{
    // Lo que emite Pandoc cuando la imagen lleva tamaño: GFM no lo sabe expresar y
    // cae a HTML crudo, que el editor (MarkdownNoHTML) mostraría como texto.
    const QString md = mdimport::repairImages(QStringLiteral(
        "Antes\n\n<img src=\"/tmp/doc-media/media/image1.png\" "
        "style=\"width:0.8in;height:0.8in\" />\n\nDespués\n"));
    QCOMPARE(md, QStringLiteral(
        "Antes\n\n![image1](/tmp/doc-media/media/image1.png)\n\nDespués\n"));

    // Con texto alternativo, y con entidades en los atributos.
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("<img src=\"a&amp;b.png\" alt=\"Uno &amp; otro\" />")),
             QStringLiteral("![Uno & otro](a&b.png)"));
    // Los corchetes del alt se escapan (romperían el propio enlace).
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("<img src='x.png' alt='ver [1]'>")),
             QStringLiteral("![ver \\[1\\]](x.png)"));
}

// `QTextDocument::setMarkdown` DESCARTA `![](ruta)`: con el texto alternativo
// vacío no inserta ni la imagen ni nada en su lugar, así que la imagen se pierde
// sin dejar rastro. Se rellena con el nombre del fichero.
void TestPandocImport::repairsFillsEmptyAltText()
{
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("Antes\n\n![](/tmp/doc-media/media/image1.png)\n\nFin\n")),
             QStringLiteral("Antes\n\n![image1](/tmp/doc-media/media/image1.png)\n\nFin\n"));
    // Con título, que se conserva; y con la ruta entre `<...>`.
    QCOMPARE(mdimport::repairImages(QStringLiteral("![](foto.jpg \"Pie\")")),
             QStringLiteral("![foto](foto.jpg \"Pie\")"));
    QCOMPARE(mdimport::repairImages(QStringLiteral("![](<mi foto.jpg>)")),
             QStringLiteral("![mi foto](<mi foto.jpg>)"));
    // Un alt que ya viene puesto no se toca; `![]()` sin destino, tampoco.
    QCOMPARE(mdimport::repairImages(QStringLiteral("![mío](x.png)")),
             QStringLiteral("![mío](x.png)"));
    QCOMPARE(mdimport::repairImages(QStringLiteral("![]()")), QStringLiteral("![]()"));
}

void TestPandocImport::repairsAwkwardPathsWithAngleBrackets()
{
    // Un espacio o un paréntesis cortan la sintaxis `![](...)`: van entre `<...>`.
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("<img src=\"/casa/Mis documentos/x.png\"/>")),
             QStringLiteral("![x](</casa/Mis documentos/x.png>)"));
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("<img src=\"/casa/copia (2)/x.png\"/>")),
             QStringLiteral("![x](</casa/copia (2)/x.png>)"));
    // Y un `<`/`>` en la propia ruta rompería ese envoltorio: se codifica.
    QCOMPARE(mdimport::repairImages(
                 QStringLiteral("<img src=\"/casa/a&lt;b/x.png\"/>")),
             QStringLiteral("![x](/casa/a%3Cb/x.png)"));
}

void TestPandocImport::repairsLeaveOtherHtmlAlone()
{
    // Solo se tocan las imágenes: el resto del HTML crudo que Pandoc pueda emitir
    // (tablas anidadas, por ejemplo) se deja como está.
    const QString tabla = QStringLiteral("<table>\n<tr><td>Celda</td></tr>\n</table>\n");
    QCOMPARE(mdimport::repairImages(tabla), tabla);
    // Un <img> sin src no es rescatable: se deja tal cual en vez de inventarse nada.
    const QString sinSrc = QStringLiteral("texto <img alt=\"roto\"/> más texto");
    QCOMPARE(mdimport::repairImages(sinSrc), sinSrc);
    // Sin ninguna imagen, la cadena vuelve idéntica (sin copias ni reescrituras).
    const QString limpio = QStringLiteral("# Título\n\nPárrafo normal.\n");
    QCOMPARE(mdimport::repairImages(limpio), limpio);
}

// Pandoc cae a HTML crudo cuando GFM no expresa la tabla (celdas combinadas,
// multipárrafo, anidadas). Con MarkdownNoHTML eso se vería como texto literal, así
// que la tabla se perdía entera.
void TestPandocImport::htmlTableBecomesPipeTable()
{
    const QString html = QStringLiteral(
        "Antes\n\n<table>\n<thead>\n<tr><th>Concepto</th><th>Importe</th></tr>\n</thead>\n"
        "<tbody>\n<tr><td>Licencias</td><td>1.200</td></tr>\n</tbody>\n</table>\n\nDespués\n");
    QCOMPARE(mdimport::htmlTablesToMarkdown(html), QStringLiteral(
        "Antes\n\n"
        "| Concepto | Importe |\n"
        "|---|---|\n"
        "| Licencias | 1.200 |\n"
        "\nDespués\n"));
}

void TestPandocImport::htmlTableFlattensWhatMarkdownCannotHold()
{
    // colspan → celdas vacías a la derecha, para que la fila no encoja; y una celda
    // con dos párrafos cabe en una sola línea. Sin <thead>, encabezado vacío (GFM lo
    // exige), igual que emite el propio Pandoc.
    QCOMPARE(mdimport::htmlTablesToMarkdown(QStringLiteral(
                 "<table>\n<tbody>\n"
                 "<tr><td colspan=\"2\">Cabecera combinada</td></tr>\n"
                 "<tr><td><p>Dos</p><p>párrafos</p></td><td>D</td></tr>\n"
                 "</tbody>\n</table>\n")),
             QStringLiteral("|  |  |\n|---|---|\n"
                            "| Cabecera combinada |  |\n| Dos párrafos | D |\n"));

    // Tabla ANIDADA: ningún Markdown la expresa, así que su contenido se aplana
    // dentro de la celda de fuera. Se pierde la anidación, no el texto.
    QCOMPARE(mdimport::htmlTablesToMarkdown(QStringLiteral(
                 "<table>\n<tbody>\n<tr><td><p>Celda externa</p>\n"
                 "<table>\n<tbody>\n<tr><td>Celda anidada</td></tr>\n</tbody>\n</table>"
                 "</td></tr>\n</tbody>\n</table>\n")),
             QStringLiteral("|  |\n|---|\n| Celda externa Celda anidada |\n"));
}

void TestPandocImport::htmlTableKeepsInlineFormatting()
{
    const QString md = mdimport::htmlTablesToMarkdown(QStringLiteral(
        "<table>\n<tbody>\n<tr>"
        "<td><strong>fuerte</strong> y <em>suave</em></td>"
        "<td><code>codigo()</code></td>"
        "<td><a href=\"https://ej.com\">sitio</a></td>"
        "<td>a | b</td>"
        "</tr>\n</tbody>\n</table>\n"));
    QVERIFY2(md.contains(QStringLiteral("**fuerte** y *suave*")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("`codigo()`")), qPrintable(md));
    QVERIFY2(md.contains(QStringLiteral("[sitio](https://ej.com)")), qPrintable(md));
    // Una tubería dentro de una celda cortaría la fila: se escapa.
    QVERIFY2(md.contains(QStringLiteral("a \\| b")), qPrintable(md));
}

void TestPandocImport::htmlTableLeavesUnconvertibleTextAlone()
{
    // Sin tablas, la cadena vuelve idéntica.
    const QString limpio = QStringLiteral("# Título\n\nUn párrafo.\n");
    QCOMPARE(mdimport::htmlTablesToMarkdown(limpio), limpio);
    // Un `<table>` dentro de un bloque de código es TEXTO del usuario: intocable.
    const QString codigo = QStringLiteral(
        "Ejemplo:\n\n```html\n<table>\n<tr><td>x</td></tr>\n</table>\n```\n");
    QCOMPARE(mdimport::htmlTablesToMarkdown(codigo), codigo);
    // HTML truncado (sin cierre): se deja, que es mejor que perderlo.
    const QString roto = QStringLiteral("<table>\n<tr><td>a</td></tr>\n");
    QCOMPARE(mdimport::htmlTablesToMarkdown(roto), roto);
    // Una tabla sin filas no produce nada convertible: tampoco se toca.
    const QString vacia = QStringLiteral("<table>\n</table>\n");
    QCOMPARE(mdimport::htmlTablesToMarkdown(vacia), vacia);
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
