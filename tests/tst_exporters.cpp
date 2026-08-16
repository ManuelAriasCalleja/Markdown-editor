#include <QtTest>

#include <memory>

#include <QDir>
#include <QFile>
#include <QFont>
#include <QImage>
#include <QTemporaryDir>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextEdit>
#include <QTextImageFormat>
#include <QUrl>
#include <QXmlStreamReader>

#include "codehighlighter.h"
#include "exporters.h"
#include "markdownrender.h"
#include "mathblocks.h"
#include "themespec.h"

// Pruebas de la exportación: el serializador LaTeX puro, la lectura del front
// matter y los XML de idioma del ODF. El empaquetado real del ODT (QZip) se
// comprueba con un smoke test aparte.
class TestExporters : public QObject
{
    Q_OBJECT

private slots:
    void languageLookupNormalizesCode();
    void frontMatterValueReadsKeys();
    void pdfInfoReadsTitleAndAuthor();
    void latexHasBabelAndTitle();
    void latexEscapesSpecialChars();
    void latexConstructs();
    void latexKeepsSuperAndSubscript();
    void latexSurvivesHostileContent();
    void latexKeepsQuoteStructure();
    void latexKeepsOrderedListStart();
    void latexHeadingNotDoublyBold();
    void latexSanitizesHighUnicode();
    void latexKeepsCjkAndAsksForUnicodeEngine();
    void latexPreambleFollowsTheTextNotTheLanguage();
    void cjkScriptsAreDetectedOnePerWritingSystem();
    void latexCompanionCharsDoNotForceTheCjkPreamble();
    void latexSerializesEachBlockOnce();
    void latexReportsDroppedSymbols();
    void latexConvertsMathUnicodeToCommands();
    void latexBundlesImagesSelfContained();
    void odfStylesCarryLanguage();
    void odfMetaCarriesLanguageAndTitle();
    void odfManifestListsExtraFiles();
    void docxDocumentConstructs();
    void docxTitleParagraph();
    void docxStylesCarryLanguage();
    void docxNumberingHasBulletAndDecimal();
    void docxWriteProducesZipPackage();
    void htmlBodyToXhtmlExtractsAndSanitizes();
    void htmlDocumentCarriesLanguageAndTitle();
    void htmlEmbedsImagesSelfContained();
    void epubBuildersAreWellFormedXml();
    void epubLanguageTagsAreBcp47();
    void epubTocIsBuiltFromHeadings();
    void epubNavNestingIsWellFormed();
    void epubStyleCoversTaskCheckboxes();
    void epubContentXhtmlWrapsBody();
    void epubWriteProducesZipPackage();
    void twoDFormulaExpandsForHtmlAndLatex();
    void plainTextFlattensTwoDFormula();
    void codeHighlightingBakedIntoExport();
    void printClampsOversizedImages();
    void printBakesRelativeImageResources();
    void cloneNormalizesFontSizeAwayFromZoom();
    void cloneCodeInheritsBodyFontSize();
};

// ¿`xml` es XML bien formado? (para validar las piezas del EPUB).
static bool isWellFormedXml(const QByteArray &xml)
{
    QXmlStreamReader r(xml);
    while (!r.atEnd())
        r.readNext();
    return !r.hasError();
}

void TestExporters::languageLookupNormalizesCode()
{
    QCOMPARE(mdexport::languageForCode(QStringLiteral("es-ES")).babel,
             QStringLiteral("spanish"));
    QCOMPARE(mdexport::languageForCode(QStringLiteral("de_DE")).babel,
             QStringLiteral("ngerman"));
    // Desconocido → inglés de recurso.
    QCOMPARE(mdexport::languageForCode(QStringLiteral("xx")).code, QStringLiteral("en"));

    // El chino no se puede pedir con `[chinese]{babel}`: no existe `chinese.ldf` y
    // babel aborta con «Unknown option». La vía es `provide=*`, que es lo que el
    // propio error indica. Comprobado compilando con xelatex, no leyendo la
    // documentación: es la única manera de saber si un .tex sirve.
    const mdexport::Language zh = mdexport::languageForCode(QStringLiteral("zh_CN"));
    QCOMPARE(zh.code, QStringLiteral("zh_CN"));
    QCOMPARE(zh.babel, QStringLiteral("provide=*,chinese"));
    QCOMPARE(zh.odfLang, QStringLiteral("zh"));
    QCOMPARE(zh.odfCountry, QStringLiteral("CN"));
    // Cualquier forma de escribirlo lleva a la misma fila (mdlang::canonicalTag).
    QCOMPARE(mdexport::languageForCode(QStringLiteral("zh-Hans")).code, zh.code);
    QCOMPARE(mdexport::languageForCode(QStringLiteral("zh_SG")).code, zh.code);
    // El tradicional todavía no tiene fila: cae al inglés, como cualquier idioma que
    // no esté en la tabla. Es una fila de una línea el día que se quiera.
    QCOMPARE(mdexport::languageForCode(QStringLiteral("zh_TW")).code, QStringLiteral("en"));
}

void TestExporters::frontMatterValueReadsKeys()
{
    const QString fm = QStringLiteral("---\nlang: fr\ntitle: \"Mi Título\"\n---\n");
    QCOMPARE(mdexport::frontMatterValue(fm, QStringLiteral("lang")), QStringLiteral("fr"));
    QCOMPARE(mdexport::frontMatterValue(fm, QStringLiteral("title")),
             QStringLiteral("Mi Título"));
    QVERIFY(mdexport::frontMatterValue(fm, QStringLiteral("author")).isEmpty());
}

void TestExporters::pdfInfoReadsTitleAndAuthor()
{
    const mdexport::PdfInfo info = mdexport::pdfDocumentInfo(
        QStringLiteral("---\ntitle: \"Mi Doc\"\nauthor: Ada Lovelace\n---\n"));
    QCOMPARE(info.title, QStringLiteral("Mi Doc"));
    QCOMPARE(info.creator, QStringLiteral("Ada Lovelace"));

    // Sin `author`, cae a `creator`.
    const mdexport::PdfInfo alt = mdexport::pdfDocumentInfo(
        QStringLiteral("+++\ntitle = T\ncreator = Babbage\n+++\n"));
    QCOMPARE(alt.creator, QStringLiteral("Babbage"));

    // Sin front matter, ambos vacíos.
    const mdexport::PdfInfo none = mdexport::pdfDocumentInfo(QString());
    QVERIFY(none.title.isEmpty() && none.creator.isEmpty());
}

void TestExporters::latexHasBabelAndTitle()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QStringLiteral("T"));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[spanish]{babel}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{document}")));
    QVERIFY(tex.contains(QStringLiteral("\\end{document}")));
    QVERIFY(tex.contains(QStringLiteral("\\title{T}")));
    QVERIFY(tex.contains(QStringLiteral("\\maketitle")));
}

void TestExporters::latexEscapesSpecialChars()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("100% de a & b, costo $5 #1\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("en")), QString());
    QVERIFY(tex.contains(QStringLiteral("100\\% de a \\& b")));
    QVERIFY(tex.contains(QStringLiteral("\\$5")));
    QVERIFY(tex.contains(QStringLiteral("\\#1")));
    // Sin título: no hay \maketitle.
    QVERIFY(!tex.contains(QStringLiteral("\\maketitle")));
}

void TestExporters::latexConstructs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "- uno\n- dos\n\n1. a\n2. b\n\n> cita\n\n`x` y **negrita**\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(tex.contains(QStringLiteral("\\begin{itemize}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{enumerate}")));
    QVERIFY(tex.contains(QStringLiteral("\\begin{quote}")));
    QVERIFY(tex.contains(QStringLiteral("\\texttt{x}")));
    QVERIFY(tex.contains(QStringLiteral("\\textbf{negrita}")));
}

// Cuatro maneras de producir un .tex que NO COMPILA o que enlaza a ninguna parte.
// Las cuatro salieron de exportar y compilar con pdflatex, no de leer el código.
void TestExporters::latexSurvivesHostileContent()
{
    const auto body = [](const QString &markdown) {
        QTextDocument doc;
        doc.setMarkdown(markdown);
        return mdexport::toLatex(&doc, mdexport::languageForCode(QStringLiteral("es")),
                                 QString());
    };

    // 1) Un bloque de código con `\end{verbatim}` dentro cerraba el entorno ahí y
    //    el documento entero dejaba de compilar («\begin{document} ended by
    //    \end{verbatim}»). Con alltt el terminador va escapado y es texto.
    const QString code = body(QStringLiteral("```\nantes\n\\end{verbatim}\ndespués\n```\n"));
    QVERIFY2(code.contains(QStringLiteral("\\begin{alltt}")), qPrintable(code));
    QVERIFY2(!code.contains(QStringLiteral("\\end{verbatim}")), qPrintable(code));
    QVERIFY(code.contains(QStringLiteral("\\textbackslash{}end\\{verbatim\\}")));
    QVERIFY(code.contains(QStringLiteral("antes")) && code.contains(QStringLiteral("después")));

    // 2) Más de cuatro listas anidadas: LaTeX aborta con «Too deeply nested». Se
    //    limita la profundidad; el contenido de los niveles de más se conserva.
    QString deep;
    for (int i = 0; i < 7; ++i)
        deep += QString(qsizetype(i) * 2, QLatin1Char(' '))
                + QStringLiteral("- nivel %1\n").arg(i);
    const QString lists = body(deep);
    QCOMPARE(lists.count(QStringLiteral("\\begin{itemize}")), 4);
    QCOMPARE(lists.count(QStringLiteral("\\end{itemize}")), 4);
    QVERIFY2(lists.contains(QStringLiteral("nivel 6")), qPrintable(lists));

    // 3) Un elemento de lista que empieza por `[` (una bibliografía: `- [1] Knuth`)
    //    lo tomaba LaTeX como argumento opcional de \item: salía «1» de viñeta y
    //    sin corchetes. Un grupo vacío delante lo impide.
    const QString refs = body(QStringLiteral("- [1] Knuth\n- normal\n"));
    QVERIFY2(refs.contains(QStringLiteral("\\item {}[1] Knuth")), qPrintable(refs));
    QVERIFY2(refs.contains(QStringLiteral("\\item normal")), qPrintable(refs));

    // 4) Una `~` en la URL salía como `\textasciitilde{}`, y hyperref enlazaba a
    //    «http://e.com/~{}u»: las llaves acababan DENTRO del enlace del PDF.
    const QString link = body(QStringLiteral("[web](http://e.com/~u/a_b?x=1&y=2#z)\n"));
    QVERIFY2(link.contains(QStringLiteral("\\href{http://e.com/~u/a\\_b?x=1\\&y=2\\#z}")),
             qPrintable(link));
    QVERIFY(!link.contains(QStringLiteral("textasciitilde")));

    // Lo que ni siquiera es válido en una URL (espacios, llaves, no-ASCII) sí se
    // codifica en porcentaje, con el `%` escapado para que no comente la línea.
    QTextDocument raw;
    QTextCursor c(&raw);
    QTextCharFormat anchor;
    anchor.setAnchor(true);
    anchor.setAnchorHref(QStringLiteral("http://e.com/a b/{x}/ñ"));
    c.insertText(QStringLiteral("destino"), anchor);
    const QString odd = mdexport::toLatex(
        &raw, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY2(odd.contains(QStringLiteral(
                 "\\href{http://e.com/a\\%20b/\\%7Bx\\%7D/\\%C3\\%B1}")),
             qPrintable(odd));
}

// Una lista dentro de una cita seguía siendo parte de la cita en el editor, pero
// el .tex la sacaba fuera (cerraba `quote`, ponía la lista y abría otra `quote`),
// y dos párrafos citados se fundían en uno por falta de línea en blanco.
void TestExporters::latexKeepsQuoteStructure()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "> Primero.\n>\n> - uno\n> - dos\n>\n> Segundo.\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());

    QCOMPARE(tex.count(QStringLiteral("\\begin{quote}")), 1);
    QCOMPARE(tex.count(QStringLiteral("\\end{quote}")), 1);
    // La lista queda DENTRO de la cita.
    const int quoteStart = tex.indexOf(QStringLiteral("\\begin{quote}"));
    const int quoteEnd = tex.indexOf(QStringLiteral("\\end{quote}"));
    const int listStart = tex.indexOf(QStringLiteral("\\begin{itemize}"));
    QVERIFY2(listStart > quoteStart && listStart < quoteEnd, qPrintable(tex));
    // Y los dos párrafos citados siguen siendo dos.
    QVERIFY2(tex.contains(QStringLiteral("Primero.\n\n")), qPrintable(tex));

    // Un bloque de código citado también pertenece a la cita.
    QTextDocument withCode;
    withCode.setMarkdown(QStringLiteral("> Mira:\n>\n> ```\n> int x;\n> ```\n>\n> Ya.\n"));
    const QString code = mdexport::toLatex(
        &withCode, mdexport::languageForCode(QStringLiteral("es")), QString());
    QCOMPARE(code.count(QStringLiteral("\\begin{quote}")), 1);
    QVERIFY2(code.indexOf(QStringLiteral("\\begin{alltt}"))
                 < code.indexOf(QStringLiteral("\\end{quote}")),
             qPrintable(code));

    // Y `>>` es una cita dentro de otra, no la misma aplanada.
    QTextDocument nested;
    nested.setMarkdown(QStringLiteral("> uno\n>\n> > dos\n>\n> tres\n"));
    const QString deep = mdexport::toLatex(
        &nested, mdexport::languageForCode(QStringLiteral("es")), QString());
    QCOMPARE(deep.count(QStringLiteral("\\begin{quote}")), 2);
    QCOMPARE(deep.count(QStringLiteral("\\end{quote}")), 2);
}

// Una lista numerada que no empieza en 1 (`5. cinco`) salía renumerada desde 1:
// Qt sí conserva el arranque, pero enumerate cuenta desde 1 si no se le mueve el
// contador del nivel.
void TestExporters::latexKeepsOrderedListStart()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("5. cinco\n6. seis\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY2(tex.contains(QStringLiteral("\\begin{enumerate}\n\\setcounter{enumi}{4}")),
             qPrintable(tex));

    // Empezando en 1 no se toca el contador (el caso normal, sin ruido).
    QTextDocument plain;
    plain.setMarkdown(QStringLiteral("1. uno\n2. dos\n"));
    QVERIFY(!mdexport::toLatex(&plain, mdexport::languageForCode(QStringLiteral("es")),
                               QString())
                 .contains(QStringLiteral("setcounter")));
}

// Los super/subíndices que NO son fórmula (`x^2^`, `H~2~O` de mdsupsub, y las
// referencias de nota al pie) viven en el documento como `verticalAlignment`, sin
// propiedades de math. inlineLatex solo miraba las propiedades de math, así que
// salían a ras de línea: «H2O» en vez de H₂O. En el editor, y en DOCX/HTML/ODF, sí
// se veían elevados; el LaTeX era el único que los perdía.
void TestExporters::latexKeepsSuperAndSubscript()
{
    QTextEdit edit;  // pipeline real: mdsupsub necesita su pasada de render
    mdrender::setMarkdownWithExtensions(
        &edit, QStringLiteral("Agua H~2~O y potencia x^2^ junto a $y_1$.\n"));
    const QString tex = mdexport::toLatex(
        edit.document(), mdexport::languageForCode(QStringLiteral("es")), QString());

    QVERIFY2(tex.contains(QStringLiteral("H\\textsubscript{2}O")), qPrintable(tex));
    QVERIFY2(tex.contains(QStringLiteral("x\\textsuperscript{2}")), qPrintable(tex));
    // Y la fórmula de verdad sigue emitiéndose como matemática, no como texto
    // elevado (sus runs también llevan verticalAlignment: no deben confundirse).
    QVERIFY2(tex.contains(QStringLiteral("$y_1$")), qPrintable(tex));
    QVERIFY(!tex.contains(QStringLiteral("\\textsubscript{1}")));
}

void TestExporters::latexHeadingNotDoublyBold()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Encabezado\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(tex.contains(QStringLiteral("\\section{Encabezado}")));
    QVERIFY(!tex.contains(QStringLiteral("\\section{\\textbf{")));
}

void TestExporters::latexSanitizesHighUnicode()
{
    QTextDocument doc;
    // ✅ (U+2705) rompía pdflatex; un emoji arbitrario (🚀) debe omitirse; la raya
    // — y los acentos deben conservarse.
    doc.setMarkdown(QStringLiteral("✅ hecho — cañón 🚀\n\n```\ncode ✅ x\n```\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(!tex.contains(QChar(0x2705)));        // ✅ ya no aparece crudo
    QVERIFY(!tex.contains(QStringLiteral("🚀")));  // 🚀 omitido (es astral: QChar no lo
                                                  // representa, hay que comparar el QString)
    QVERIFY(tex.contains(QStringLiteral("\\checkmark")));  // ✅ → símbolo LaTeX
    QVERIFY(tex.contains(QString::fromUtf8("cañón")));     // acentos intactos
    QVERIFY(tex.contains(QString::fromUtf8("—")));         // raya intacta
    // En verbatim no caben comandos: el ✅ se omite, no se convierte.
    QVERIFY(tex.contains(QStringLiteral("code  x")));
    // Preámbulo portable entre motores.
    QVERIFY(tex.contains(QStringLiteral("\\usepackage{iftex}")));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage{fontspec}")));
}

// Los ideogramas NO son un símbolo suelto que se pueda descartar: descartarlos era
// exportar un .tex sin el texto del documento, y en silencio. Se conservan, el
// preámbulo se prepara para ellos y toLatex lo cuenta para que se pueda avisar.
void TestExporters::latexKeepsCjkAndAsksForUnicodeEngine()
{
    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(u8"# 标题\n\n中文文本 y texto español.\n\n```\n// 注释\n```\n"));
    mdexport::LatexIssues issues;
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString::fromUtf8(u8"文档"),
        QString(), &issues);

    QVERIFY(issues.needsUnicodeEngine);
    QCOMPARE(issues.droppedSymbols, 0);
    // El texto está: en el cuerpo, en el encabezado, en el título y en el código.
    QVERIFY(tex.contains(QString::fromUtf8(u8"中文文本")));
    QVERIFY(tex.contains(QString::fromUtf8(u8"\\section{标题}")));
    QVERIFY(tex.contains(QString::fromUtf8(u8"\\title{文档}")));
    QVERIFY(tex.contains(QString::fromUtf8(u8"注释")));
    QVERIFY(tex.contains(QString::fromUtf8(u8"texto español")));  // lo latino, intacto
    // Preámbulo: ctex en la rama Unicode y un error legible en la de pdflatex (que
    // no puede componer ideogramas de ninguna manera).
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[UTF8]{ctex}")));
    QVERIFY(tex.contains(QStringLiteral("\\errmessage{")));
    QVERIFY(tex.contains(QStringLiteral("xelatex")));
    // El mensaje de TeX va sin acentos a propósito (lo compone el propio TeX).
    const int err = tex.indexOf(QStringLiteral("\\errmessage{"));
    const QString msg = tex.mid(err, tex.indexOf(QLatin1Char('}'), err) - err);
    for (const QChar c : msg)
        QVERIFY2(c.unicode() < 128, qPrintable(msg));
    // Un documento sin ideogramas no arrastra nada de esto.
    QTextDocument plain;
    plain.setMarkdown(QStringLiteral("Solo texto.\n"));
    mdexport::LatexIssues none;
    const QString latin = mdexport::toLatex(
        &plain, mdexport::languageForCode(QStringLiteral("es")), QString(), QString(), &none);
    QVERIFY(!none.needsUnicodeEngine);
    QVERIFY(!latin.contains(QStringLiteral("ctex")));
    QVERIFY(latin.contains(QStringLiteral("\\usepackage[utf8]{inputenc}")));  // portable
}

// Quién decide el preámbulo: lo que el documento TRAE, no el idioma con el que se
// exporta. Parece que debería ser el idioma —sus rótulos son ideogramas—, y por ahí
// se empezó; pero este serializador no emite ni \tableofcontents ni \caption, así que
// esos rótulos no llegan a aparecer, y forzar el motor Unicode por el idioma le
// quitaba pdflatex a un documento que compila perfectamente con él. Comprobado con
// los dos motores sobre el .tex que sale de aquí.
void TestExporters::latexPreambleFollowsTheTextNotTheLanguage()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Zhongwen\n\nDocumento en chino escrito en pinyin.\n"));
    mdexport::LatexIssues issues;
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("zh_CN")), QString(), QString(), &issues);

    // El idioma sí llega: babel lo lleva con la sintaxis que el chino necesita.
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[provide=*,chinese]{babel}")));
    // Pero el documento sigue siendo portable: sin ideogramas, no hay nada que pedirle
    // a xelatex.
    QVERIFY(!issues.needsUnicodeEngine);
    QVERIFY(!tex.contains(QStringLiteral("ctex")));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[utf8]{inputenc}")));

    // Con un solo ideograma en el cuerpo, el mismo idioma sí arrastra ctex, y va
    // ANTES que babel para que los rótulos los siga fijando el idioma del documento.
    QTextDocument han;
    han.setMarkdown(QString::fromUtf8("# 中文\n"));
    mdexport::LatexIssues hanIssues;
    const QString hanTex = mdexport::toLatex(
        &han, mdexport::languageForCode(QStringLiteral("zh_CN")), QString(), QString(), &hanIssues);
    QVERIFY(hanIssues.needsUnicodeEngine);
    QVERIFY(hanTex.indexOf(QStringLiteral("{ctex}"))
            < hanTex.indexOf(QStringLiteral("{babel}")));
}

// Qué escrituras CJK trae el documento, que es lo que hay que preguntarle luego a
// las fuentes del sistema. Se distinguen las tres porque no se cubren entre sí:
// tener una fuente china instalada no dibuja el hangul de un documento coreano.
void TestExporters::cjkScriptsAreDetectedOnePerWritingSystem()
{
    const auto scriptsOf = [](const QString &markdown) {
        QTextDocument doc;
        doc.setMarkdown(markdown);
        return mdexport::cjkScriptsIn(&doc);
    };

    // El caso de casi todos los documentos: nada que comprobar.
    const mdexport::CjkScripts none = scriptsOf(QStringLiteral("Texto **normal** en español.\n"));
    QVERIFY(!none.any());

    const mdexport::CjkScripts han = scriptsOf(QString::fromUtf8("# 中文\n\nmezclado con español\n"));
    QVERIFY(han.han);
    QVERIFY(!han.kana);
    QVERIFY(!han.hangul);

    // El japonés lleva kana Y kanji: los kanji son ideogramas, así que marca las dos.
    const mdexport::CjkScripts ja = scriptsOf(QString::fromUtf8("日本語のテキスト\n"));
    QVERIFY(ja.kana);
    QVERIFY(ja.han);

    const mdexport::CjkScripts ko = scriptsOf(QString::fromUtf8("한국어 텍스트\n"));
    QVERIFY(ko.hangul);
    QVERIFY(!ko.han);
    QVERIFY(!ko.kana);

    // La puntuación de ancho completo acompaña a las tres y no dice a cuál: no marca
    // escritura por su cuenta. Si lo hiciera, un «，» suelto en un documento en
    // español sacaría un aviso sobre fuentes chinas que no viene a cuento.
    QVERIFY(!scriptsOf(QString::fromUtf8("Texto con una coma ancha， y ya\n")).any());
    // Pero acompañando a un ideograma, lo que manda es el ideograma.
    QVERIFY(scriptsOf(QString::fromUtf8("中文，好\n")).han);
    // El documento nulo no revienta (rutas de exportación sin documento).
    QVERIFY(!mdexport::cjkScriptsIn(nullptr).any());
}

// La misma regla que `cjkScriptsIn` aplica al aviso de fuentes tiene que valer para
// el preámbulo: la puntuación CJK y las formas de ancho completo acompañan a las tres
// escrituras y no son ninguna. Un «，» pegado en un documento en español le cambiaba
// el preámbulo entero —ctex, `\errmessage` en la rama de pdfTeX y un aviso de que hay
// que compilar con xelatex— a un documento sin una sola letra china.
void TestExporters::latexCompanionCharsDoNotForceTheCjkPreamble()
{
    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(u8"Una coma pegada de otro sitio， y ya está.\n"));
    mdexport::LatexIssues issues;
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString(), QString(), &issues);

    QVERIFY(!issues.needsUnicodeEngine);
    QVERIFY(!tex.contains(QStringLiteral("ctex")));
    QVERIFY(tex.contains(QStringLiteral("\\usepackage[utf8]{inputenc}")));  // sigue portable
    // Y el signo no se pierde: se pliega a la forma estrecha, que es la misma coma y
    // esta sí la compone pdflatex.
    QVERIFY(tex.contains(QStringLiteral("otro sitio, y ya")));
    QVERIFY(!tex.contains(QString::fromUtf8(u8"，")));

    // Al plegarse puede caer en un carácter especial de LaTeX: hay que escaparlo, no
    // soltarlo crudo (un `&` suelto es un error de compilación).
    QTextDocument amp;
    amp.setMarkdown(QString::fromUtf8(u8"Uno ＆ dos\n"));
    const QString ampTex =
        mdexport::toLatex(&amp, mdexport::languageForCode(QStringLiteral("es")), QString());
    QVERIFY(ampTex.contains(QStringLiteral("Uno \\& dos")));

    // En un documento que SÍ trae la escritura, esa misma puntuación es texto y se
    // conserva tal cual (ctex la compone).
    QTextDocument zh;
    zh.setMarkdown(QString::fromUtf8(u8"中文，好。\n"));
    mdexport::LatexIssues zhIssues;
    const QString zhTex = mdexport::toLatex(
        &zh, mdexport::languageForCode(QStringLiteral("zh_CN")), QString(), QString(), &zhIssues);
    QVERIFY(zhIssues.needsUnicodeEngine);
    QVERIFY(zhTex.contains(QString::fromUtf8(u8"中文，好。")));

    // La regla no puede depender del ORDEN: la coma va ANTES del primer ideograma y
    // tiene que conservarse igual (por eso la escritura se decide antes de serializar).
    QTextDocument before;
    before.setMarkdown(QString::fromUtf8(u8"，中文\n"));
    const QString beforeTex =
        mdexport::toLatex(&before, mdexport::languageForCode(QStringLiteral("zh_CN")), QString());
    QVERIFY(beforeTex.contains(QString::fromUtf8(u8"，中文")));

    // Y un título en chino con el cuerpo en español también cuenta como escritura.
    QTextDocument es;
    es.setMarkdown(QStringLiteral("Cuerpo en español.\n"));
    mdexport::LatexIssues titleIssues;
    mdexport::toLatex(&es, mdexport::languageForCode(QStringLiteral("es")),
                      QString::fromUtf8(u8"文档"), QString(), &titleIssues);
    QVERIFY(titleIssues.needsUnicodeEngine);
}

// Cada bloque se serializa UNA vez. Los encabezados pasaban dos veces por
// inlineLatex (una para el texto y otra para quitarle la negrita), y eso no es
// gratis: contaba dos veces los símbolos descartados y escribía DOS copias de cada
// imagen traída junto al .tex, la segunda huérfana y sin que nadie la referenciara.
void TestExporters::latexSerializesEachBlockOnce()
{
    QTextDocument doc;
    doc.setMarkdown(QString::fromUtf8(u8"# Titulo 🚀\n"));
    mdexport::LatexIssues issues;
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString(), QString(), &issues);
    QCOMPARE(issues.droppedSymbols, 1);  // un cohete, contado una vez
    QVERIFY(tex.contains(QStringLiteral("\\section{Titulo")));

    // Una imagen dentro de un encabezado se trae una sola vez.
    QTemporaryDir srcDir;
    QVERIFY(srcDir.isValid());
    QImage red(10, 10, QImage::Format_RGB32);
    red.fill(Qt::red);
    QVERIFY(red.save(srcDir.filePath(QStringLiteral("pic.png")), "PNG"));

    QTextDocument withImage;
    withImage.setBaseUrl(QUrl::fromLocalFile(srcDir.path() + QLatin1Char('/')));
    withImage.setMarkdown(QStringLiteral("# Titulo ![a](pic.png)\n"));
    QTemporaryDir outDir;
    QVERIFY(outDir.isValid());
    const QString texPath = outDir.filePath(QStringLiteral("salida.tex"));
    mdexport::toLatex(&withImage, mdexport::languageForCode(QStringLiteral("es")),
                      QString(), texPath);
    QVERIFY(QFile::exists(outDir.filePath(QStringLiteral("salida-img1.png"))));
    QVERIFY(!QFile::exists(outDir.filePath(QStringLiteral("salida-img2.png"))));
}

// Lo que sí se descarta (símbolos y emoji) se sigue descartando —rompería
// pdflatex—, pero ahora se cuenta: antes desaparecía sin que nadie se enterara.
void TestExporters::latexReportsDroppedSymbols()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("Cohete 🚀 y otro 🛰\n\n```\ncode 🚀\n```\n"));
    mdexport::LatexIssues issues;
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString(), QString(), &issues);
    QCOMPARE(issues.droppedSymbols, 3);  // dos en la prosa y uno en el bloque de código
    QVERIFY(!issues.needsUnicodeEngine);
    QVERIFY(!tex.contains(QStringLiteral("🚀")));
    // El ✅ tiene equivalente: se convierte, no se descarta (no cuenta como omitido).
    QTextDocument ok;
    ok.setMarkdown(QStringLiteral("✅ hecho\n"));
    mdexport::LatexIssues none;
    mdexport::toLatex(&ok, mdexport::languageForCode(QStringLiteral("es")), QString(),
                      QString(), &none);
    QCOMPARE(none.droppedSymbols, 0);
}

void TestExporters::latexConvertsMathUnicodeToCommands()
{
    QTextDocument doc;
    // Prosa con matemática en Unicode (subíndices, ellipsis, griego, operadores,
    // conjuntos): cruda rompía la compilación pdflatex. Debe salir en modo
    // matemático.
    doc.setMarkdown(QString::fromUtf8(
        u8"La probabilidad P(X₁,…,Xₙ) sobre φ ∈ 𝒞 con ⊕ y Σ⋃ℝ y 𝟙.\n"));
    const QString tex = mdexport::toLatex(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());
    // El caso concreto del informe: P(X₁,…,Xₙ) → P(X$_{1}$,$\ldots$,X$_{n}$).
    QVERIFY(tex.contains(QStringLiteral("$_{1}$")));
    QVERIFY(tex.contains(QStringLiteral("$_{n}$")));
    QVERIFY(tex.contains(QStringLiteral("$\\ldots$")));
    QVERIFY(tex.contains(QStringLiteral("$\\varphi$")));
    QVERIFY(tex.contains(QStringLiteral("$\\in$")));
    QVERIFY(tex.contains(QStringLiteral("$\\mathcal{C}$")));
    QVERIFY(tex.contains(QStringLiteral("$\\oplus$")));
    QVERIFY(tex.contains(QStringLiteral("$\\Sigma$")));
    QVERIFY(tex.contains(QStringLiteral("$\\bigcup$")));
    QVERIFY(tex.contains(QStringLiteral("$\\mathbb{R}$")));
    QVERIFY(tex.contains(QStringLiteral("$\\mathbf{1}$")));
    // Ya no quedan los caracteres crudos que abortaban pdflatex.
    QVERIFY(!tex.contains(QChar(0x2081)));  // ₁
    QVERIFY(!tex.contains(QChar(0x2099)));  // ₙ
    QVERIFY(!tex.contains(QChar(0x2026)));  // …
    QVERIFY(!tex.contains(QChar(0x03C6)));  // φ
    QVERIFY(!tex.contains(QChar(0x2208)));  // ∈
    QVERIFY(!tex.contains(QChar(0x2295)));  // ⊕
    QVERIFY(!tex.contains(QChar(0x211D)));  // ℝ
    QVERIFY(!tex.contains(QString::fromUtf8(u8"𝒞")));  // astral
    QVERIFY(!tex.contains(QString::fromUtf8(u8"𝟙")));  // astral
}

void TestExporters::latexBundlesImagesSelfContained()
{
    // Carpeta del «.md»: un JPG real en disco + una baseUrl que lo resuelva.
    QTemporaryDir srcDir;
    QVERIFY(srcDir.isValid());
    QImage red(40, 20, QImage::Format_RGB32);
    red.fill(Qt::red);
    const QString jpgSrc = srcDir.filePath(QStringLiteral("foto.jpg"));
    QVERIFY(red.save(jpgSrc, "JPG"));
    QByteArray jpgBytes;
    {
        QFile f(jpgSrc);
        QVERIFY(f.open(QIODevice::ReadOnly));
        jpgBytes = f.readAll();
    }

    QTextDocument doc;
    doc.setBaseUrl(QUrl::fromLocalFile(srcDir.path() + QLatin1Char('/')));
    // SVG inyectado como recurso (no incluible → se rasteriza; sin depender de qsvg).
    // resource() resuelve el nombre contra baseUrl, así que se registra bajo la URL
    // resuelta (en la app real el SVG se carga del disco por esa misma ruta).
    doc.addResource(QTextDocument::ImageResource,
                    doc.baseUrl().resolved(QUrl(QStringLiteral("diagrama.svg"))), red);
    QTextCursor cur(&doc);
    QTextImageFormat svg;
    svg.setName(QStringLiteral("diagrama.svg"));
    cur.insertImage(svg);
    cur.insertText(QStringLiteral("\n"));
    QTextImageFormat jpg;
    jpg.setName(QStringLiteral("foto.jpg"));
    cur.insertImage(jpg);

    // Exporta el .tex a OTRA carpeta, distinta de la del .md/imágenes.
    QTemporaryDir outDir;
    QVERIFY(outDir.isValid());
    const QString texPath = outDir.filePath(QStringLiteral("salida.tex"));
    const QString tex = mdexport::toLatex(&doc, mdexport::Language{}, QString(), texPath);

    // SVG → PNG rasterizado junto al .tex.
    QVERIFY(tex.contains(QStringLiteral("\\includegraphics[max width=\\linewidth]{salida-img1.png}")));
    QVERIFY(QFile::exists(outDir.filePath(QStringLiteral("salida-img1.png"))));
    // JPG incluible → COPIA byte a byte junto al .tex, conservando la extensión.
    QVERIFY(tex.contains(QStringLiteral("\\includegraphics[max width=\\linewidth]{salida-img2.jpg}")));
    const QString copied = outDir.filePath(QStringLiteral("salida-img2.jpg"));
    QVERIFY(QFile::exists(copied));
    QByteArray copiedBytes;
    {
        QFile f(copied);
        QVERIFY(f.open(QIODevice::ReadOnly));
        copiedBytes = f.readAll();
    }
    QCOMPARE(copiedBytes, jpgBytes);  // copia exacta, sin re-encode
    // Ya no quedan referencias a las rutas originales.
    QVERIFY(!tex.contains(QStringLiteral("diagrama.svg")));
    QVERIFY(!tex.contains(QStringLiteral("{foto.jpg}")));

    // Sin ruta de salida (p.ej. otros tests): conducta previa — referencia directa
    // las incluibles y deja un marcador inocuo para las que no.
    const QString noOut = mdexport::toLatex(&doc, mdexport::Language{}, QString());
    QVERIFY(noOut.contains(QStringLiteral("\\texttt{[imagen: diagrama.svg]}")));
    QVERIFY(noOut.contains(QStringLiteral("{foto.jpg}")));
}

void TestExporters::odfStylesCarryLanguage()
{
    const QByteArray xml =
        mdexport::odfStylesXml(mdexport::languageForCode(QStringLiteral("fr")));
    QVERIFY(xml.contains("fo:language=\"fr\""));
    QVERIFY(xml.contains("fo:country=\"FR\""));
    QVERIFY(xml.contains("style:default-style"));
}

void TestExporters::odfMetaCarriesLanguageAndTitle()
{
    const QByteArray xml = mdexport::odfMetaXml(
        mdexport::languageForCode(QStringLiteral("it")), QStringLiteral("Mi Doc"));
    QVERIFY(xml.contains("<dc:language>it-IT</dc:language>"));
    QVERIFY(xml.contains("<dc:title>Mi Doc</dc:title>"));
}

void TestExporters::odfManifestListsExtraFiles()
{
    const QByteArray manifest =
        "<?xml version=\"1.0\"?>\n<manifest:manifest>\n"
        " <manifest:file-entry manifest:full-path=\"content.xml\"/>\n"
        "</manifest:manifest>\n";
    const QByteArray out = mdexport::odfManifestWithLanguageFiles(manifest);
    QVERIFY(out.contains("full-path=\"styles.xml\""));
    QVERIFY(out.contains("full-path=\"meta.xml\""));
    QVERIFY(out.contains("</manifest:manifest>"));
}

void TestExporters::docxDocumentConstructs()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "# Título\n\n**negrita** *cursiva* `codigo`\n\n"
        "[enlace](https://example.com)\n\n"
        "- uno\n- dos\n\n> cita\n\n| a | b |\n|---|---|\n| 1 | 2 |\n"));
    QList<mdexport::DocxHyperlink> links;
    const QString xml = mdexport::toDocxDocumentXml(&doc, QString(), nullptr, &links);

    QVERIFY(xml.contains(QStringLiteral("<w:document")));
    QVERIFY(xml.contains(QStringLiteral("<w:pStyle w:val=\"Heading1\"/>")));
    QVERIFY(xml.contains(QStringLiteral("<w:b/>")));
    QVERIFY(xml.contains(QStringLiteral("<w:i/>")));
    QVERIFY(xml.contains(QStringLiteral("Courier New")));            // código monoespaciado
    QVERIFY(xml.contains(QStringLiteral("<w:hyperlink r:id=")));     // enlace por relación
    QCOMPARE(links.size(), 1);
    QCOMPARE(links.first().target, QStringLiteral("https://example.com"));
    QVERIFY(xml.contains(QStringLiteral("<w:numPr>")));             // lista con numeración real
    QVERIFY(xml.contains(QStringLiteral("<w:tbl>")));               // tabla
    QVERIFY(xml.contains(QStringLiteral("<w:sectPr>")));           // propiedades de sección
}

void TestExporters::docxTitleParagraph()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("cuerpo\n"));
    const QString xml = mdexport::toDocxDocumentXml(&doc, QStringLiteral("Mi Doc"), nullptr);
    QVERIFY(xml.contains(QStringLiteral("<w:pStyle w:val=\"Title\"/>")));
    QVERIFY(xml.contains(QStringLiteral("Mi Doc")));
}

void TestExporters::docxStylesCarryLanguage()
{
    const QByteArray xml =
        mdexport::docxStylesXml(mdexport::languageForCode(QStringLiteral("de")));
    QVERIFY(xml.contains("<w:lang w:val=\"de-DE\"/>"));
    QVERIFY(xml.contains("w:styleId=\"Heading1\""));
    QVERIFY(xml.contains("w:styleId=\"Title\""));
}

void TestExporters::docxNumberingHasBulletAndDecimal()
{
    const QByteArray xml = mdexport::docxNumberingXml();
    QVERIFY(xml.contains("w:numFmt w:val=\"bullet\""));
    QVERIFY(xml.contains("w:numFmt w:val=\"decimal\""));
    QVERIFY(xml.contains("w:numId=\"1\""));
    QVERIFY(xml.contains("w:numId=\"2\""));
}

void TestExporters::docxWriteProducesZipPackage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("salida.docx"));

    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo con **negrita**\n"));
    QString error;
    QVERIFY2(mdexport::writeDocx(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("T"), &error),
             qPrintable(error));

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray head = f.read(2);
    QCOMPARE(head, QByteArray("PK"));  // firma de un paquete ZIP (OOXML)
    QVERIFY(f.size() > 0);
}

void TestExporters::htmlBodyToXhtmlExtractsAndSanitizes()
{
    const QString html = QStringLiteral(
        "<!DOCTYPE HTML><html><head><style>p{}</style></head>"
        "<body style=\"x\"><p>Hola&nbsp;mundo</p><hr></body></html>");
    const QString body = mdexport::htmlBodyToXhtml(html);
    QVERIFY(body.contains(QStringLiteral("<p>Hola")));
    QVERIFY(!body.contains(QStringLiteral("&nbsp;")));   // entidad XML inválida saneada
    QVERIFY(body.contains(QStringLiteral("&#160;")));
    QVERIFY(!body.contains(QStringLiteral("<body")));    // solo el interior
    QVERIFY(body.contains(QStringLiteral("<hr/>")));     // elemento vacío cerrado
}

// El `toHtml()` de Qt no pone nada de esto, y el documento sí lo sabe: sin `lang`
// el lector de pantalla no sabe en qué idioma está el texto, y sin `<title>` el
// navegador rotula la pestaña con el nombre del fichero.
void TestExporters::htmlDocumentCarriesLanguageAndTitle()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo\n"));
    const QString html = mdexport::toHtmlDocument(
        &doc, mdexport::languageForCode(QStringLiteral("fr")), QStringLiteral("Mi & Doc"));

    QVERIFY2(html.contains(QStringLiteral("<html lang=\"fr\">")), qPrintable(html.left(200)));
    // El título se escapa: un `&` crudo dejaría el HTML mal formado.
    QVERIFY2(html.contains(QStringLiteral("<title>Mi &amp; Doc</title>")),
             qPrintable(html.left(300)));
    QVERIFY(html.contains(QStringLiteral("charset=\"utf-8\"")));

    // Sin idioma ni título no se inventa nada (y no se rompe el HTML).
    const QString bare = mdexport::toHtmlDocument(&doc, mdexport::Language{}, QString());
    QVERIFY(bare.contains(QStringLiteral("<html>")));
    QVERIFY(!bare.contains(QStringLiteral("<title>")));
}

// Qt referencia las imágenes por su ruta relativa, así que el .html se veía bien
// donde se exportó y se quedaba SIN NINGUNA imagen en cuanto se movía de carpeta o
// se enviaba por correo. Ahora van embebidas, conservando los bytes originales
// cuando el navegador entiende el formato (reencodearlo todo a PNG infla una foto
// JPEG y convierte un SVG vectorial en un mapa de bits).
void TestExporters::htmlEmbedsImagesSelfContained()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString svgPath = dir.filePath(QStringLiteral("vector.svg"));
    const QByteArray svg =
        "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"8\" height=\"8\">"
        "<rect width=\"8\" height=\"8\" fill=\"red\"/></svg>";
    QFile svgFile(svgPath);
    QVERIFY(svgFile.open(QIODevice::WriteOnly));
    svgFile.write(svg);
    svgFile.close();

    const QString pngPath = dir.filePath(QStringLiteral("mapa.png"));
    QImage bitmap(4, 4, QImage::Format_RGB32);
    bitmap.fill(qRgb(10, 20, 30));
    QVERIFY(bitmap.save(pngPath, "PNG"));

    QTextDocument doc;
    doc.setBaseUrl(QUrl::fromLocalFile(dir.path() + QLatin1Char('/')));
    doc.setMarkdown(QStringLiteral(
        "![a](mapa.png)\n\n![b](vector.svg)\n\n![c](https://example.com/no-existe.png)\n"));

    const QString html = mdexport::toHtmlDocument(
        &doc, mdexport::languageForCode(QStringLiteral("es")), QString());

    // El mapa de bits y el vectorial, embebidos con SU tipo (el SVG sigue siendo SVG).
    QVERIFY2(html.contains(QStringLiteral("src=\"data:image/png;base64,")), qPrintable(html));
    QVERIFY2(html.contains(QStringLiteral("src=\"data:image/svg+xml;base64,")),
             qPrintable(html));
    QVERIFY2(html.contains(QString::fromLatin1(svg.toBase64())),
             "el SVG debe viajar tal cual, no rasterizado");
    // Ninguna ruta local suelta: el fichero ya no depende de su carpeta.
    QVERIFY(!html.contains(QStringLiteral("src=\"mapa.png\"")));
    QVERIFY(!html.contains(QStringLiteral("src=\"vector.svg\"")));
    // Lo que no se puede cargar (remoto) se deja como estaba, no se pierde.
    QVERIFY(html.contains(QStringLiteral("src=\"https://example.com/no-existe.png\"")));
}

void TestExporters::epubBuildersAreWellFormedXml()
{
    const mdexport::Language es = mdexport::languageForCode(QStringLiteral("es"));
    QVERIFY(isWellFormedXml(mdexport::epubContainerXml()));
    const QByteArray opf = mdexport::epubContentOpf(
        es, QStringLiteral("Mi <libro>"), {QStringLiteral("images/image1.png")},
        QStringLiteral("abc-123"), QStringLiteral("2026-01-01T00:00:00Z"));
    QVERIFY(isWellFormedXml(opf));
    QVERIFY(opf.contains("<dc:language>es</dc:language>"));
    QVERIFY(opf.contains("urn:uuid:abc-123"));
    QVERIFY(opf.contains("image1.png"));
    QVERIFY(opf.contains("dcterms:modified"));
    QVERIFY(isWellFormedXml(mdexport::epubNavXhtml(es, QStringLiteral("T"))));
    QVERIFY(isWellFormedXml(mdexport::epubTocNcx(QStringLiteral("T"), QStringLiteral("abc-123"))));
}

// El código de idioma del programa es la etiqueta CANÓNICA («zh_CN»), que separa con
// guion bajo porque así se llaman sus recursos. XML no admite esa forma: `_` no es un
// separador válido de subetiqueta BCP 47, así que `xml:lang="zh_CN"` invalida el libro
// para epubcheck y deja al lector sin idioma (separación silábica y lectura en voz
// alta incluidas). El EPUB es el único formato que emite `code` tal cual —ODF y DOCX
// van por odfLang/odfCountry—, así que la traducción es suya.
void TestExporters::epubLanguageTagsAreBcp47()
{
    const mdexport::Language zh = mdexport::languageForCode(QStringLiteral("zh_CN"));
    QCOMPARE(zh.code, QStringLiteral("zh_CN"));  // la canónica no cambia
    QCOMPARE(zh.bcp47(), QStringLiteral("zh-CN"));

    const QByteArray opf = mdexport::epubContentOpf(
        zh, QStringLiteral("书"), {}, QStringLiteral("abc-123"),
        QStringLiteral("2026-01-01T00:00:00Z"));
    QVERIFY(opf.contains("<dc:language>zh-CN</dc:language>"));
    QVERIFY(!opf.contains("zh_CN"));
    QVERIFY(isWellFormedXml(opf));

    const QString xhtml =
        mdexport::epubContentXhtml(QStringLiteral("<p>x</p>"), QStringLiteral("书"), zh);
    QVERIFY(xhtml.contains(QStringLiteral("xml:lang=\"zh-CN\"")));
    QVERIFY(xhtml.contains(QStringLiteral("lang=\"zh-CN\"")));
    QVERIFY(!xhtml.contains(QStringLiteral("zh_CN")));

    const QByteArray nav = mdexport::epubNavXhtml(zh, QStringLiteral("书"));
    QVERIFY(nav.contains("xml:lang=\"zh-CN\""));
    QVERIFY(!nav.contains("zh_CN"));

    // Los nueve idiomas cuyo código no lleva región salen exactamente igual que antes.
    QCOMPARE(mdexport::languageForCode(QStringLiteral("es")).bcp47(), QStringLiteral("es"));
}

// El libro llegaba al lector con UNA entrada de índice («el documento»), sin
// manera de saltar a un capítulo por muchos encabezados que tuviera. El índice se
// arma ahora con ellos, y las anclas las pone epubAnchorHeadings.
void TestExporters::epubTocIsBuiltFromHeadings()
{
    // `<hr>` empieza por «h» pero no es un encabezado: no debe llevarse un id.
    const QString body = mdexport::epubAnchorHeadings(QStringLiteral(
        "<h1 style=\"x\">Uno</h1>\n<hr />\n<p>t</p>\n<h2>Dos</h2>\n<h6>Seis</h6>"));
    QVERIFY2(body.contains(QStringLiteral("<h1 id=\"sec1\" style=\"x\">")), qPrintable(body));
    QVERIFY2(body.contains(QStringLiteral("<h2 id=\"sec2\">")), qPrintable(body));
    QVERIFY2(body.contains(QStringLiteral("<h6 id=\"sec3\">")), qPrintable(body));
    QVERIFY2(body.contains(QStringLiteral("<hr />")), qPrintable(body));

    const QList<mdexport::EpubTocEntry> toc = {
        {1, QStringLiteral("Uno"), QStringLiteral("sec1")},
        {2, QStringLiteral("Dos"), QStringLiteral("sec2")},
    };
    const QByteArray nav = mdexport::epubNavXhtml(
        mdexport::languageForCode(QStringLiteral("es")), QStringLiteral("T"), toc);
    QVERIFY(isWellFormedXml(nav));
    QVERIFY2(nav.contains("content.xhtml#sec1"), nav.constData());
    QVERIFY2(nav.contains("content.xhtml#sec2"), nav.constData());

    const QByteArray ncx = mdexport::epubTocNcx(QStringLiteral("T"),
                                                QStringLiteral("u"), toc);
    QVERIFY(isWellFormedXml(ncx));
    QVERIFY(ncx.contains("content.xhtml#sec2"));
}

// El anidamiento del índice es donde el XHTML se queda mal formado, y un nav mal
// formado invalida el libro ENTERO. Los casos que lo rompían: saltarse un nivel y
// empezar por debajo del primero.
void TestExporters::epubNavNestingIsWellFormed()
{
    const auto nav = [](const QList<mdexport::EpubTocEntry> &toc) {
        return mdexport::epubNavXhtml(mdexport::languageForCode(QStringLiteral("es")),
                                      QStringLiteral("T"), toc);
    };
    const auto entry = [](int level, int n) {
        return mdexport::EpubTocEntry{level, QStringLiteral("h%1").arg(n),
                                      QStringLiteral("sec%1").arg(n)};
    };

    // h1 → h3 → h2: el salto abre UN nivel (un <ol> sin <li> no es válido).
    const QByteArray jump = nav({entry(1, 1), entry(3, 2), entry(2, 3)});
    QVERIFY2(isWellFormedXml(jump), jump.constData());
    QVERIFY(!jump.contains("<ol>\n<ol>"));

    // Empezando por h3, un h1 posterior no puede subir por encima de la raíz.
    QVERIFY(isWellFormedXml(nav({entry(3, 1), entry(1, 2)})));
    // Bajada de varios niveles de golpe, y subida al final.
    QVERIFY(isWellFormedXml(nav({entry(1, 1), entry(2, 2), entry(3, 3), entry(4, 4),
                                 entry(1, 5)})));
    // Sin encabezados: una entrada al documento, mejor que un índice vacío.
    const QByteArray none = nav({});
    QVERIFY(isWellFormedXml(none));
    QVERIFY(none.contains("content.xhtml"));
}

// Qt marca las tareas con `li.unchecked`/`li.checked` y deja la regla que las pinta
// en el <style> de su <head>… que es justo lo que htmlBodyToXhtml descarta. Sin
// esas reglas en el CSS del libro, una tarea hecha y una pendiente son dos viñetas
// idénticas en el lector.
void TestExporters::epubStyleCoversTaskCheckboxes()
{
    const QByteArray css = mdexport::epubStyleCss();
    QVERIFY2(css.contains("li.unchecked::before"), css.constData());
    QVERIFY2(css.contains("li.checked::before"), css.constData());
    QVERIFY(css.contains("2610") && css.contains("2612"));  // ☐ y ☒
}

void TestExporters::epubContentXhtmlWrapsBody()
{
    const mdexport::Language es = mdexport::languageForCode(QStringLiteral("es"));
    const QString xhtml = mdexport::epubContentXhtml(
        QStringLiteral("<p>cuerpo</p>"), QStringLiteral("Título"), es);
    QVERIFY(isWellFormedXml(xhtml.toUtf8()));
    QVERIFY(xhtml.contains(QStringLiteral("xml:lang=\"es\"")));
    QVERIFY(xhtml.contains(QStringLiteral("<p>cuerpo</p>")));
}

void TestExporters::epubWriteProducesZipPackage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("salida.epub"));

    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Hola\n\nmundo con **negrita**\n"));
    QString error;
    QVERIFY2(mdexport::writeEpub(&doc, path,
                                 mdexport::languageForCode(QStringLiteral("es")),
                                 QStringLiteral("T"), &error),
             qPrintable(error));

    QFile f(path);
    QVERIFY(f.open(QIODevice::ReadOnly));
    const QByteArray all = f.readAll();
    QCOMPARE(all.left(2), QByteArray("PK"));             // firma ZIP
    QVERIFY(all.contains("application/epub+zip"));       // mimetype
    QVERIFY(all.contains("mimetype"));
}

// Una fórmula 2D vive en el documento como un carácter objeto que ningún writer
// sabe pintar. cloneForExport debe expandirla a runs inline para HTML/ODF/PDF/
// DOCX; LaTeX, que usa el documento original, la emite como `$$tex$$` desde la
// propiedad. Verificamos que no se cuela un U+FFFC (objeto sin pintar).
void TestExporters::twoDFormulaExpandsForHtmlAndLatex()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(
        QStringLiteral("Sea $$\\sum_{i=1}^n \\frac{x_i}{2}$$ fin\n")));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    const QString html = flat->toHtml();
    QVERIFY2(!html.contains(QChar(0xFFFC)),
             "el carácter objeto no debe llegar al HTML sin expandir");
    QVERIFY(html.contains(QChar(0x2211)));  // Σ del sumatorio expandido

    const QString latex = mdexport::toLatex(&doc, mdexport::Language{}, QString());
    QVERIFY(latex.contains(QStringLiteral("\\sum_{i=1}^n")));
    QVERIFY(latex.contains(QStringLiteral("\\frac{x_i}{2}")));
}

// exportPlainText() escribe doc->toPlainText() sobre el clon plano. cloneForExport
// debe expandir la fórmula 2D a runs para que su texto aparezca en el .txt, no el
// U+FFFC del carácter objeto sin pintar.
void TestExporters::plainTextFlattensTwoDFormula()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(
        QStringLiteral("Sea $$\\sum_{i=1}^n \\frac{x_i}{2}$$ fin\n")));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    const QString text = flat->toPlainText();
    QVERIFY2(!text.contains(QChar(0xFFFC)),
             "el carácter objeto no debe llegar al texto plano sin expandir");
    QVERIFY(text.contains(QChar(0x2211)));  // Σ del sumatorio expandido a runs
    QVERIFY(text.contains(QStringLiteral("Sea")) && text.contains(QStringLiteral("fin")));
}

// El resaltado de sintaxis (overlay del QSyntaxHighlighter) se hornea en el clon
// de exportación como color de carácter real, para que el código exporte con color
// (HTML/ODF/DOCX/EPUB/PDF). El clon directo no lo lleva.
void TestExporters::codeHighlightingBakedIntoExport()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("```cpp\nint main() { return 42; }\n```\n"));
    CodeBlockHighlighter hl(&doc);
    hl.setSyntaxColors(mdtheme::specFor(mdtheme::ThemeId::Light).syntax);
    hl.rehighlight();

    std::unique_ptr<QTextDocument> plain(doc.clone());
    QVERIFY2(!plain->toHtml().contains(QStringLiteral("color:#")),
             "el clon directo no lleva el color del resaltado (es overlay)");

    std::unique_ptr<QTextDocument> baked(mdexport::cloneForExport(&doc));
    // El azul de keyword del tema claro (#0000ff) del «int»/«return».
    QVERIFY2(baked->toHtml().contains(QStringLiteral("color:#0000ff"), Qt::CaseInsensitive),
             "el resaltado de código debe conservarse en la exportación");
}

// Una imagen más ancha que la página (un gantt apaisado) salía TRUNCADA en el PDF
// y la impresión: la maqueta recorta en el borde en vez de escalar. clampImages-
// ToWidth la encoge al ancho imprimible ANTES de maquetar. Ojo con las unidades:
// la maqueta multiplica por el factor de dpi TODOS los tamaños (también los
// explícitos), así que el tope se fija en unidades de formato — fijarlo en píxeles
// de dispositivo salía re-escalado otra vez y la imagen quedaba GIGANTE (partida
// en dos páginas), que fue el primer intento de arreglo.
void TestExporters::printClampsOversizedImages()
{
    const auto imageFormats = [](QTextDocument *doc) {
        QList<QTextImageFormat> fmts;
        for (QTextBlock b = doc->begin(); b.isValid(); b = b.next())
            for (auto it = b.begin(); it != b.end(); ++it)
                if (it.fragment().charFormat().isImageFormat())
                    fmts << it.fragment().charFormat().toImageFormat();
        return fmts;
    };

    // Sin tamaño explícito: se compara la anchura intrínseca del recurso y solo se
    // fija la anchura (la altura sigue en automático y conserva la proporción sola).
    QTextDocument doc;
    QImage wide(3000, 400, QImage::Format_RGB32);
    wide.fill(qRgb(1, 2, 3));
    doc.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("ancha.png")),
                    QVariant(wide));
    QTextCursor(&doc).insertImage(QStringLiteral("ancha.png"));

    mdexport::clampImagesToWidth(&doc, 600.0, 1.0);
    QList<QTextImageFormat> fmts = imageFormats(&doc);
    QCOMPARE(fmts.size(), 1);
    QCOMPARE(fmts.first().width(), 600.0);
    QVERIFY2(fmts.first().height() <= 0, "la altura debe seguir en automático");

    // Con el factor de dpi de una impresora (1200/96 = 12.5): el tope en unidades
    // de formato es maxWidth/dpiScale, no maxWidth.
    QTextDocument hi;
    hi.addResource(QTextDocument::ImageResource, QUrl(QStringLiteral("ancha.png")),
                   QVariant(wide));
    QTextCursor(&hi).insertImage(QStringLiteral("ancha.png"));
    mdexport::clampImagesToWidth(&hi, 7500.0, 12.5);
    QCOMPARE(imageFormats(&hi).first().width(), 600.0);

    // Tamaño explícito que ya cabe: intocable. Y uno que no cabe se reescala
    // conservando la proporción también en la altura fijada.
    QTextDocument sized;
    QTextImageFormat small;
    small.setName(QStringLiteral("s.png"));
    small.setWidth(500);
    QTextImageFormat big;
    big.setName(QStringLiteral("b.png"));
    big.setWidth(1000);
    big.setHeight(200);
    QTextCursor c(&sized);
    c.insertImage(small);
    c.insertImage(big);
    mdexport::clampImagesToWidth(&sized, 600.0, 1.0);
    fmts = imageFormats(&sized);
    QCOMPARE(fmts.at(0).width(), 500.0);   // cabía: no se toca
    QCOMPARE(fmts.at(1).width(), 600.0);   // 1000x200 -> 600x120
    QCOMPARE(fmts.at(1).height(), 120.0);
}

// `QTextDocument::print()` clona el documento por dentro, y ese clon copia los
// recursos explícitos pero NO la baseUrl ni la caché: una imagen de ruta relativa
// desaparecía de la impresión y del PDF con los números de página desactivados
// (salía el icono de imagen rota). bakeImageResources fija lo resuelto como
// recurso explícito, que el clon sí conserva.
void TestExporters::printBakesRelativeImageResources()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QImage img(20, 10, QImage::Format_RGB32);
    img.fill(qRgb(9, 8, 7));
    QVERIFY(img.save(dir.filePath(QStringLiteral("rel.png")), "PNG"));

    QTextDocument doc;
    doc.setBaseUrl(QUrl::fromLocalFile(dir.path() + QLatin1Char('/')));
    doc.setMarkdown(QStringLiteral("![x](rel.png)\n"));
    QVERIFY(doc.resource(QTextDocument::ImageResource, QUrl(QStringLiteral("rel.png")))
                .isValid());

    // La premisa del fallo: el clon (lo que print() usa por dentro) la pierde.
    std::unique_ptr<QTextDocument> before(doc.clone());
    QVERIFY2(!before->resource(QTextDocument::ImageResource,
                               QUrl(QStringLiteral("rel.png"))).isValid(),
             "si esto falla, Qt ya copia baseUrl/caché al clonar y el bake sobra");

    mdexport::bakeImageResources(&doc);
    std::unique_ptr<QTextDocument> after(doc.clone());
    QVERIFY(after->resource(QTextDocument::ImageResource,
                            QUrl(QStringLiteral("rel.png"))).isValid());
}

// El zoom de interfaz agranda la fuente del editor (y con ella el defaultFont del
// documento). La exportación NO debe heredar ese tamaño de pantalla: cloneForExport
// normaliza el cuerpo a un tamaño estándar, aunque el origen venga inflado.
void TestExporters::cloneNormalizesFontSizeAwayFromZoom()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral("# Título\n\nCuerpo del documento.\n"));
    QFont zoomed = doc.defaultFont();
    zoomed.setPointSizeF(30.0);  // simula un zoom fuerte
    doc.setDefaultFont(zoomed);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    // Independiente del tamaño de pantalla: normalizado a 11 pt, no 30.
    QCOMPARE(flat->defaultFont().pointSizeF(), 11.0);
    // La familia y demás atributos se conservan.
    QCOMPARE(flat->defaultFont().family(), doc.defaultFont().family());
    // Y el HTML resultante no lleva el tamaño inflado.
    QVERIFY(!flat->toHtml().contains(QStringLiteral("font-size:30")));
}

// Qt hornea un tamaño ABSOLUTO en los runs de código (inline y bloque); si no se
// neutraliza, sale con distinto tamaño que el cuerpo tras normalizar el defaultFont.
// cloneForExport quita ese tamaño absoluto para que el código herede el cuerpo.
void TestExporters::cloneCodeInheritsBodyFontSize()
{
    QTextDocument doc;
    doc.setMarkdown(QStringLiteral(
        "Texto con `codigo` en linea.\n\n```\nbloque\n```\n"));
    QFont zoomed = doc.defaultFont();
    zoomed.setPointSizeF(30.0);
    doc.setDefaultFont(zoomed);

    std::unique_ptr<QTextDocument> flat(mdexport::cloneForExport(&doc));
    // Ningún fragmento conserva un tamaño de fuente absoluto: todos heredan el cuerpo.
    for (QTextBlock b = flat->begin(); b.isValid(); b = b.next())
        for (auto it = b.begin(); it != b.end(); ++it)
            if (it.fragment().isValid())
                QVERIFY2(!it.fragment().charFormat().hasProperty(QTextFormat::FontPointSize),
                         qPrintable(QStringLiteral("fragmento con tamaño absoluto: '%1'")
                                        .arg(it.fragment().text())));
    // El HTML no emite el tamaño de pantalla en el código.
    QVERIFY(!flat->toHtml().contains(QStringLiteral("font-size:30")));
}

QTEST_MAIN(TestExporters)
#include "tst_exporters.moc"
