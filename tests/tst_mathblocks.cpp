#include <QtTest>

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>

#include <QFont>
#include <memory>

#include "mathblocks.h"
#include "mathlayout.h"
#include "tableedit.h"

// Pruebas del módulo mdmath: localización de fórmulas en el Markdown fuente,
// envoltura/desenvoltura para que sobrevivan a setMarkdown() y traducción a
// Unicode para los formatos que no entienden TeX.
class TestMathBlocks : public QObject
{
    Q_OBJECT

private slots:
    void findFindsInlineAndBlock();
    void findIgnoresInsideInlineCode();
    void findIgnoresInsideFencedCode();
    void findIgnoresEscapedDollars();
    void findIgnoresPricesWithLeadingSpace();
    void protectWrapsAllMath();
    void unprotectIsInverseOfProtect();
    void unprotectLeavesPlainInlineCodeAlone();
    void roundTripThroughQTextDocument();
    void texToUnicodeGreekAndOps();
    void texToUnicodeScripts();
    void texToUnicodeFrac();
    void texToUnicodeFunctionsAndSpacing();
    void texToUnicodeAccentsBinomText();
    void texToUnicodeNewSymbolsDelimsLeftRight();
    void texToUnicodeNotAndStyledAlphabets();
    void replaceMathReplacesDollars();
    void renderConvertsInlineCodeToFormattedFragments();
    void renderIsIdempotent();
    void unrenderRestoresInlineCode();
    void fullRoundTripPreservesTex();
    void findFindsMultilineBlockMath();
    void findFindsMultilineWithContentOnDelimiterLines();
    void findIgnoresMultilineOpenInsideFence();
    void findDiscardsUnclosedMultilineBlock();
    void findDoesNotOpenMultilineOnEmptyDollarBlock();
    void roundTripPreservesMultilineMath();
    void deeplyNestedTexDoesNotOverflowStack();
    void renderFormulaRunsPicksObjectFor2D();
    void unicodeToLatexMapsTechnicalChars();
};

void TestMathBlocks::findFindsInlineAndBlock()
{
    const QString s = QStringLiteral("a $x^2$ b $$E=mc^2$$ c");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 2);
    QCOMPARE(spans[0].block, false);
    QCOMPARE(spans[0].content, QStringLiteral("x^2"));
    QCOMPARE(spans[1].block, true);
    QCOMPARE(spans[1].content, QStringLiteral("E=mc^2"));
}

void TestMathBlocks::findIgnoresInsideInlineCode()
{
    const QString s = QStringLiteral("texto `$x$` más $y$");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 1);
    QCOMPARE(spans[0].content, QStringLiteral("y"));
}

void TestMathBlocks::findIgnoresInsideFencedCode()
{
    const QString s = QStringLiteral("```\n$x$ no es fórmula\n```\n$y$ sí\n");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 1);
    QCOMPARE(spans[0].content, QStringLiteral("y"));
}

void TestMathBlocks::findIgnoresEscapedDollars()
{
    const QString s = QStringLiteral("\\$5 y \\$10 no son fórmulas");
    QCOMPARE(mdmath::findMath(s).size(), 0);
}

void TestMathBlocks::findIgnoresPricesWithLeadingSpace()
{
    // `$ 5 y $10` no debe verse como un span $...$ porque el primer carácter
    // tras el `$` es un espacio.
    const QString s = QStringLiteral("Cuesta $ 5 y $ 10.");
    QCOMPARE(mdmath::findMath(s).size(), 0);
}

void TestMathBlocks::protectWrapsAllMath()
{
    const QString out = mdmath::protectMath(QStringLiteral("a $x_1$ b"));
    QCOMPARE(out, QStringLiteral("a ``$x_1$`` b"));
}

void TestMathBlocks::unprotectIsInverseOfProtect()
{
    const QString original = QStringLiteral("a $x_1$ b $$y^2$$ c");
    const QString protectedMd = mdmath::protectMath(original);
    QCOMPARE(mdmath::unprotectMath(protectedMd), original);
}

void TestMathBlocks::unprotectLeavesPlainInlineCodeAlone()
{
    // Código en línea normal (sin `$` envolventes) no debe tocarse.
    const QString s = QStringLiteral("usa `printf` para imprimir");
    QCOMPARE(mdmath::unprotectMath(s), s);
}

void TestMathBlocks::roundTripThroughQTextDocument()
{
    // El problema raíz: sin protección, Qt convierte `_` en cursiva.
    const QString original = QStringLiteral("masa: $m_0 = 9.1$ kg\n");
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(original));
    const QString roundTripped = mdmath::unprotectMath(mdtable::documentMarkdown(&doc));
    // El TeX debe sobrevivir sin que `_` se convierta en cursiva.
    QVERIFY(roundTripped.contains(QStringLiteral("$m_0 = 9.1$")));
}

void TestMathBlocks::texToUnicodeGreekAndOps()
{
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\alpha + \\beta")),
             QStringLiteral("α + β"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\sum_{i=1}^n a_i")),
             QStringLiteral("∑ᵢ₌₁ⁿ aᵢ"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("x \\leq y \\neq z")),
             QStringLiteral("x ≤ y ≠ z"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\mathbb{R}")),
             QStringLiteral("ℝ"));
}

void TestMathBlocks::texToUnicodeScripts()
{
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("x^2")), QStringLiteral("x²"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("H_2O")), QStringLiteral("H₂O"));
    // Carácter sin equivalente Unicode → fallback `^arg` (sin llaves), más
    // legible que la forma TeX `^{Q}` en la previsualización.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("x^{Q}")), QStringLiteral("x^Q"));
    // `^\infty`: lee el comando entero, lo traduce y aplica el script.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\int_0^\\infty")),
             QStringLiteral("∫₀^∞"));
    // Argumentos compuestos con un comando dentro: -x → ⁻ˣ.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("e^{-x}")),
             QStringLiteral("e⁻ˣ"));
}

void TestMathBlocks::texToUnicodeFunctionsAndSpacing()
{
    // Nombres de función: se componen como texto, sin la barra invertida.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\sin x + \\cos y")),
             QStringLiteral("sin x + cos y"));
    QVERIFY(mdmath::texToUnicode(QStringLiteral("\\lim")).startsWith(QStringLiteral("lim")));
    // Comandos de espaciado: `\,` → espacio fino (U+2009); `\!` → nada.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a\\,b")),
             QStringLiteral("a") + QChar(0x2009) + QStringLiteral("b"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a\\!b")), QStringLiteral("ab"));
}

void TestMathBlocks::texToUnicodeAccentsBinomText()
{
    // Acento: base + carácter combinante (x̂ = x + U+0302).
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\hat{x}")),
             QStringLiteral("x") + QChar(0x0302));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\bar{y}")),
             QStringLiteral("y") + QChar(0x0304));
    // \binom inline → C(n, k); \text → literal.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\binom{n}{k}")),
             QStringLiteral("C(n, k)"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\text{si } x")),
             QStringLiteral("si  x"));
}

void TestMathBlocks::texToUnicodeNewSymbolsDelimsLeftRight()
{
    // Símbolos y delimitadores recién añadidos a la tabla.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\langle x \\rangle")),
             QStringLiteral("⟨ x ⟩"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a \\perp b")),
             QStringLiteral("a ⊥ b"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\bigcup_i A_i")),
             QStringLiteral("⋃ᵢ Aᵢ"));
    // \quad / \qquad → espacios em (el espacio que sigue al nombre del comando
    // se conserva literal, igual que en «\sin x»).
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a\\quad b")),
             QStringLiteral("a") + QChar(0x2003) + QStringLiteral(" b"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a\\qquad b")),
             QStringLiteral("a") + QString(2, QChar(0x2003)) + QStringLiteral(" b"));
    // \left( … \right): el delimitador se emite, la palabra clave no; `.` es nulo.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\left( x \\right)")),
             QStringLiteral("( x )"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\left\\langle x \\right\\rangle")),
             QStringLiteral("⟨ x ⟩"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\left. x \\right|")),
             QStringLiteral(" x |"));
    // \underline → combinante de línea inferior (U+0332), como \overline.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\underline{x}")),
             QStringLiteral("x") + QChar(0x0332));
}

void TestMathBlocks::texToUnicodeNotAndStyledAlphabets()
{
    // \not X → operando + combinante de negación (U+0338).
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("a \\not= b")),
             QStringLiteral("a =") + QChar(0x0338) + QStringLiteral(" b"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("x \\not\\in S")),
             QStringLiteral("x ∈") + QChar(0x0338) + QStringLiteral(" S"));
    // \mathcal: letras script (con sustituciones de «Letterlike Symbols»).
    // L tiene excepción BMP (U+2112); A no (bloque astral U+1D49C).
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\mathcal{L}")),
             QString::fromUcs4(U"ℒ"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\mathcal{A}")),
             QString::fromUcs4(U"\U0001D49C"));
    // \mathfrak: fraktur (R tiene excepción BMP U+211C).
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\mathfrak{R}")),
             QString::fromUcs4(U"ℜ"));
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\mathfrak{a}")),
             QString::fromUcs4(U"\U0001D51E"));
}

void TestMathBlocks::texToUnicodeFrac()
{
    // Fracción de un solo char por lado → fraction slash (U+2044), que las
    // fuentes con soporte renderizan como fracción tipográfica.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\frac{a}{b}")),
             QStringLiteral("a⁄b"));
    // Numeradores/denominadores más largos: paréntesis + barra.
    QCOMPARE(mdmath::texToUnicode(QStringLiteral("\\frac{x+1}{2x}")),
             QStringLiteral("(x+1)/(2x)"));
}

void TestMathBlocks::replaceMathReplacesDollars()
{
    const QString s = QStringLiteral("E = $mc^2$ y \\alpha está en $\\alpha$.");
    const QString out = mdmath::replaceMathWithUnicode(s);
    QCOMPARE(out, QStringLiteral("E = mc² y \\alpha está en α."));
}

// Helper: cuenta cuántos fragmentos con IsMathProperty contiene el documento.
static int countMathFragments(QTextDocument *doc, QString *firstTex = nullptr,
                              bool *firstBlock = nullptr)
{
    int n = 0;
    for (QTextBlock b = doc->begin(); b.isValid(); b = b.next()) {
        for (auto it = b.begin(); it != b.end(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;
            const QTextCharFormat cf = frag.charFormat();
            if (!cf.boolProperty(mdmath::IsMathProperty)) continue;
            if (n == 0) {
                if (firstTex)
                    *firstTex = cf.property(mdmath::MathTexProperty).toString();
                if (firstBlock)
                    *firstBlock = cf.boolProperty(mdmath::MathBlockProperty);
            }
            ++n;
        }
    }
    return n;
}

void TestMathBlocks::renderConvertsInlineCodeToFormattedFragments()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(QStringLiteral("La masa $m_0$ es positiva.\n")));
    mdmath::renderMathInDocument(&doc);

    QString tex;
    bool block = false;
    // `m_0` se rinde como dos runs (base "m" + subíndice "0"): cuenta 2
    // fragmentos, ambos con el mismo MathTex y MathBlock.
    QCOMPARE(countMathFragments(&doc, &tex, &block), 2);
    QCOMPARE(tex, QStringLiteral("m_0"));
    QCOMPARE(block, false);
    // El texto visible del documento ya no contiene `$`.
    QVERIFY(!doc.toPlainText().contains(QLatin1Char('$')));
    // …y aparece la letra base seguida del 0 (con o sin forma de subíndice
    // Unicode: el subíndice real lo aporta Qt con vertical-align).
    QVERIFY(doc.toPlainText().contains(QLatin1Char('m')));
    QVERIFY(doc.toPlainText().contains(QLatin1Char('0')));
}

void TestMathBlocks::renderIsIdempotent()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(QStringLiteral("$\\alpha + \\beta$\n")));
    mdmath::renderMathInDocument(&doc);
    const QString afterOne = doc.toPlainText();
    mdmath::renderMathInDocument(&doc);  // segundo pase no debería tocar nada
    QCOMPARE(doc.toPlainText(), afterOne);
    QCOMPARE(countMathFragments(&doc), 1);
}

void TestMathBlocks::unrenderRestoresInlineCode()
{
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(QStringLiteral("a $x^2$ b\n")));
    mdmath::renderMathInDocument(&doc);
    mdmath::unrenderMathInDocument(&doc);
    QCOMPARE(countMathFragments(&doc), 0);
    QVERIFY(doc.toPlainText().contains(QStringLiteral("$x^2$")));
}

void TestMathBlocks::fullRoundTripPreservesTex()
{
    // Ciclo completo: fuente → protectMath → setMarkdown → renderMath →
    // [usuario edita o no] → documentMarkdown (clon + unrenderMath + toMarkdown
    // + unprotectMath) → debe contener el TeX original intacto.
    const QString original = QStringLiteral("E = $m c^2$ y $$\\sum_{i=1}^n a_i$$\n");
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(original));
    mdmath::renderMathInDocument(&doc);

    // Simula la serialización canónica (igual que mdtable::documentMarkdown).
    // Camino canónico: clone + centinelas + toMarkdown + restore.
    std::unique_ptr<QTextDocument> clone(doc.clone());
    const mdmath::MathSentinelTable table = mdmath::replaceMathWithSentinels(clone.get());
    const QString saved = mdmath::restoreMathFromSentinels(clone->toMarkdown(), table);
    QVERIFY(saved.contains(QStringLiteral("$m c^2$")));
    QVERIFY(saved.contains(QStringLiteral("$$\\sum_{i=1}^n a_i$$")));
}

void TestMathBlocks::findFindsMultilineBlockMath()
{
    const QString s = QStringLiteral("texto\n$$\n\\sum x_i\n$$\nmás\n");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 1);
    QCOMPARE(spans[0].block, true);
    QCOMPARE(spans[0].content, QStringLiteral("\n\\sum x_i\n"));
}

void TestMathBlocks::roundTripPreservesMultilineMath()
{
    // Una fórmula $$...$$ multilínea, tipo Obsidian/Pandoc: tras cargar y
    // serializar, el TeX (incluidos sus saltos) sobrevive.
    const QString original = QStringLiteral("texto\n\n$$\n\\sum_{i=1}^n a_i\n$$\n\nmás\n");
    QTextDocument doc;
    doc.setMarkdown(mdmath::protectMath(original));
    mdmath::renderMathInDocument(&doc);

    std::unique_ptr<QTextDocument> clone(doc.clone());
    const mdmath::MathSentinelTable table = mdmath::replaceMathWithSentinels(clone.get());
    const QString saved = mdmath::restoreMathFromSentinels(clone->toMarkdown(), table);
    // El TeX completo (con saltos internos) debe estar presente entre $$.
    QVERIFY(saved.contains(QStringLiteral("$$\n\\sum_{i=1}^n a_i\n$$")));
}

// El `$$` de apertura/cierre puede llevar TeX en la misma línea (estilo Pandoc),
// no solo `$$` aislado en su propia línea: el contenido capturado debe abarcar
// ambas líneas con su salto interno.
void TestMathBlocks::findFindsMultilineWithContentOnDelimiterLines()
{
    const QString s = QStringLiteral("a\n$$ \\sum_{i=1}^n\nx_i $$\nb\n");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 1);
    QCOMPARE(spans[0].block, true);
    QCOMPARE(spans[0].content, QStringLiteral(" \\sum_{i=1}^n\nx_i "));
}

// Un `$$` que abre dentro de una valla de código no debe iniciar un bloque
// multilínea (la apertura cae en zona de código y se ignora por completo).
void TestMathBlocks::findIgnoresMultilineOpenInsideFence()
{
    const QString s = QStringLiteral("```\n$$\nx\n$$\n```\n");
    QCOMPARE(mdmath::findMath(s).size(), 0);
}

// Un `$$` de apertura que nunca encuentra cierre se descarta (no produce span),
// para no «tragarse» el resto del documento.
void TestMathBlocks::findDiscardsUnclosedMultilineBlock()
{
    const QString s = QStringLiteral("texto\n$$\n\\sum x\nsin cierre\n");
    QCOMPARE(mdmath::findMath(s).size(), 0);
}

void TestMathBlocks::findDoesNotOpenMultilineOnEmptyDollarBlock()
{
    // Regresión: `$$$$` (bloque vacío) NO debe abrir un bloque multilínea que
    // engulla los párrafos siguientes; la fórmula real posterior sí se detecta.
    const QString s = QStringLiteral("$$$$\nhola mundo\n$$x$$\n");
    const QList<mdmath::Span> spans = mdmath::findMath(s);
    QCOMPARE(spans.size(), 1);
    QCOMPARE(spans.first().content, QStringLiteral("x"));
    QVERIFY(spans.first().block);
}

// renderFormulaRuns es el despachador: una fórmula con \frac (o gran operador
// con límites) se inserta como UN carácter objeto 2D; el resto, como runs de
// texto inline. Así la serialización y el export las distinguen.
void TestMathBlocks::renderFormulaRunsPicksObjectFor2D()
{
    const QList<mdmath::MathRun> twoD =
        mdmath::renderFormulaRuns(QStringLiteral("\\frac{a}{b}"), false);
    QCOMPARE(twoD.size(), 1);
    QCOMPARE(twoD[0].text, QString(QChar(QChar::ObjectReplacementCharacter)));
    QCOMPARE(twoD[0].fmt.objectType(), mdmath::MathObjectType);
    QVERIFY(twoD[0].fmt.boolProperty(mdmath::IsMathProperty));
    QCOMPARE(twoD[0].fmt.property(mdmath::MathTexProperty).toString(),
             QStringLiteral("\\frac{a}{b}"));

    const QList<mdmath::MathRun> inl =
        mdmath::renderFormulaRuns(QStringLiteral("x^2 + 1"), false);
    QVERIFY(!inl.isEmpty());
    for (const mdmath::MathRun &r : inl) {
        QVERIFY(r.fmt.objectType() != mdmath::MathObjectType);
        QVERIFY(!r.text.contains(QChar(QChar::ObjectReplacementCharacter)));
    }
}

// El TeX lo escribe/pega el usuario, así que puede anidar grupos sin límite
// (`\frac{\frac{...}}`, `x^{y^{z^{...}}}`, `\sqrt{\sqrt{...}}`). Como el parser
// inline (renderTexAsRuns/texToUnicode) y el maquetador 2D (buildHList, vía
// measureFormula) son recursivos por nivel, sin un tope de profundidad reventaban
// la pila (SIGSEGV). Estas entradas patológicas deben procesarse sin caer; el
// resultado puede ser una aproximación pobre (texto en crudo), pero nunca un crash.
void TestMathBlocks::deeplyNestedTexDoesNotOverflowStack()
{
    const int depth = 6000;  // muy por encima del tope (256) y del antiguo umbral de crash

    const QString fracs = QStringLiteral("\\frac{").repeated(depth)
        + QStringLiteral("x") + QStringLiteral("}").repeated(depth);
    const QString sups = QStringLiteral("x")
        + QStringLiteral("^{").repeated(depth) + QStringLiteral("1")
        + QStringLiteral("}").repeated(depth);
    const QString sqrts = QStringLiteral("\\sqrt{").repeated(depth)
        + QStringLiteral("x") + QStringLiteral("}").repeated(depth);

    // Ruta inline (texparser): si desbordara, el proceso moriría antes del COMPARE.
    QTextCharFormat fmt;
    QVERIFY(!mdmath::renderTexAsRuns(sups, fmt).isEmpty());
    QVERIFY(!mdmath::texToUnicode(fracs).isNull());

    // Ruta 2D (mathlayout vía measureFormula): debe medir sin desbordar.
    const QFont f;
    QVERIFY(mdmath::measureFormula(fracs, f).width() >= 0.0);
    QVERIFY(mdmath::measureFormula(sqrts, f).width() >= 0.0);

    // Despachador completo (el que usa la carga real del documento).
    QVERIFY(!mdmath::renderFormulaRuns(fracs, true).isEmpty());

    // Llegar aquí ya prueba que no hubo SIGSEGV.
    QVERIFY(true);
}

void TestMathBlocks::unicodeToLatexMapsTechnicalChars()
{
    // Super/subíndices → modo matemático.
    QCOMPARE(mdmath::unicodeToLatex(0x2081), QStringLiteral("$_{1}$"));   // ₁
    QCOMPARE(mdmath::unicodeToLatex(0x2099), QStringLiteral("$_{n}$"));   // ₙ
    QCOMPARE(mdmath::unicodeToLatex(0x1D62), QStringLiteral("$_{i}$"));   // ᵢ
    QCOMPARE(mdmath::unicodeToLatex(0x207F), QStringLiteral("$^{n}$"));   // ⁿ
    QCOMPARE(mdmath::unicodeToLatex(0x00B2), QStringLiteral("$^{2}$"));   // ²
    // Ellipsis → \ldots (no \dots), como pide el caso de uso.
    QCOMPARE(mdmath::unicodeToLatex(0x2026), QStringLiteral("$\\ldots$"));
    // Griego: φ es la «rizada» (\varphi en LaTeX), no \phi.
    QCOMPARE(mdmath::unicodeToLatex(0x03C6), QStringLiteral("$\\varphi$"));
    QCOMPARE(mdmath::unicodeToLatex(0x03A3), QStringLiteral("$\\Sigma$"));
    QCOMPARE(mdmath::unicodeToLatex(0x03A0), QStringLiteral("$\\Pi$"));
    // Operadores/relaciones que antes se descartaban.
    QCOMPARE(mdmath::unicodeToLatex(0x2295), QStringLiteral("$\\oplus$"));   // ⊕
    QCOMPARE(mdmath::unicodeToLatex(0x2208), QStringLiteral("$\\in$"));      // ∈
    QCOMPARE(mdmath::unicodeToLatex(0x2297), QStringLiteral("$\\otimes$"));  // ⊗
    QCOMPARE(mdmath::unicodeToLatex(0x22C3), QStringLiteral("$\\bigcup$"));  // ⋃
    QCOMPARE(mdmath::unicodeToLatex(0x2261), QStringLiteral("$\\equiv$"));   // ≡
    QCOMPARE(mdmath::unicodeToLatex(0x2265), QStringLiteral("$\\geq$"));     // ≥
    QCOMPARE(mdmath::unicodeToLatex(0x27C2), QStringLiteral("$\\perp$"));    // ⟂
    // Conjuntos «blackboard» y alfabetos matemáticos (astral y BMP).
    QCOMPARE(mdmath::unicodeToLatex(0x211D), QStringLiteral("$\\mathbb{R}$"));   // ℝ
    QCOMPARE(mdmath::unicodeToLatex(0x1D49E), QStringLiteral("$\\mathcal{C}$")); // 𝒞
    QCOMPARE(mdmath::unicodeToLatex(0x1D7D9), QStringLiteral("$\\mathbf{1}$"));  // 𝟙 (aprox.)
    // El latín-1 y la puntuación corriente NO se matematizan (los compone T1).
    QVERIFY(mdmath::unicodeToLatex(0x00D7).isEmpty());  // ×
    QVERIFY(mdmath::unicodeToLatex(0x00B7).isEmpty());  // · (punto medio)
    QVERIFY(mdmath::unicodeToLatex(0x2014).isEmpty());  // — (raya)
    QVERIFY(mdmath::unicodeToLatex(u'a').isEmpty());
    QVERIFY(mdmath::unicodeToLatex(0x00F1).isEmpty());  // ñ
}

QTEST_MAIN(TestMathBlocks)
#include "tst_mathblocks.moc"
