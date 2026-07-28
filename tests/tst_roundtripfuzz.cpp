#include <QtTest>

#include <QString>
#include <QStringList>
#include <QTextDocument>
#include <QTextEdit>

#include <cstdio>

#include "markdownrender.h"
#include "tableedit.h"

// Fuzzing del round-trip Markdown. Genera documentos Markdown variados y adversarios
// con un PRNG DETERMINISTA (semillas fijas → cualquier fallo es reproducible) y los
// pasa, DOS veces, por la ruta REAL de abrir/guardar de la app:
//   body → mdrender::setMarkdownWithExtensions(editor) → mdtable::documentMarkdown
//
// Invariante comprobada: **no crashea**. Mete miles de combinaciones —fórmulas,
// tablas, notas al pie, admoniciones, listas anidadas, código, y caracteres que
// estresan el escapado (`* _ [ ] \ & | # < >` …)— por todo el pipeline de
// carga/render/serialización, buscando accesos inválidos, asserts o cuelgues. La
// doble pasada ejercita también la re-carga de la salida ya serializada.
//
// **Dónde corre.** No está registrado en CTest (ver el comentario de CMakeLists):
// lo ejecuta a mano quien quiera y, en CI, el job `fuzz`, que lo invoca directo
// bajo ASan/UBSan y no puede bloquear el workflow. Ojo con la tentación de
// meterlo en el job de sanitizers: ese corre `ctest`, que no lo ve, así que
// durante un tiempo los comentarios afirmaron que se fuzzeaba bajo sanitizers
// cuando en realidad no se hacía nunca.
//
// **Por qué NO se comprueba convergencia/idempotencia aquí.** Sería lo natural en un
// round-trip, pero `QTextDocument::toMarkdown` de Qt NO es idempotente para
// contenido no trivial, por motivos ajenos a esta app (verificado aislando cada
// caso):
//   - parte las líneas a ~80 columnas, y el corte puede caer dentro de un `*…*`,
//     que al re-parsear se rompe (`*\nuber*` → `\* uber*`);
//   - escapa `\` y `&` DENTRO de un code span pero los re-lee literales, así que se
//     duplican en cada guardado (`` `a\b` `` → `` `a\\b` `` → …);
//   - un fence justo tras una lista se indenta como continuación perezosa;
//   - escapa varios caracteres especiales de forma no estable.
// La maquinaria PROPIA de la app (sentinelas de fórmulas, alineación de tablas,
// notas al pie, admoniciones) SÍ converge en aislamiento; su idempotencia la vigila
// `tst_markdownroundtrip` con casos curados. Aquí el valor es la robustez ante
// entrada arbitraria, no la igualdad byte a byte de la salida de Qt.
class TestRoundtripFuzz : public QObject
{
    Q_OBJECT

private slots:
    void survivesAdversarialContent();

private:
    // --- PRNG determinista (xorshift32) ---
    quint32 m_state = 1;
    void seed(quint32 s) { m_state = s ? s : 1; }
    quint32 next()
    {
        quint32 x = m_state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        return m_state = x;
    }
    int pick(int n) { return int(next() % quint32(n)); }          // [0, n)
    int range(int lo, int hi) { return lo + pick(hi - lo + 1); }  // [lo, hi]
    template <class T> const T &choice(const QList<T> &v) { return v[pick(v.size())]; }

    QString word();
    QString inlineToken();
    QString sentence(int minTok, int maxTok);
    QString block();
    QString document();
};

QString TestRoundtripFuzz::word()
{
    static const QList<QString> words = {
        QStringLiteral("alfa"), QStringLiteral("beta"), QStringLiteral("café"),
        QStringLiteral("niño"), QStringLiteral("über"), QStringLiteral("dato"),
        QStringLiteral("x123"), QStringLiteral("fin"),
        // caracteres que estresan el escapado Markdown de Qt. (No `<...>`: Qt lo
        // trata como HTML en línea y se traga el contenido —pérdida de datos, ajena
        // a la app—, lo que es inyección HTML, no estrés de round-trip Markdown.)
        QStringLiteral("a*b"), QStringLiteral("c_d"), QStringLiteral("e[f]"),
        QStringLiteral("g|h"), QStringLiteral("hash#"), QStringLiteral("back\\slash"),
        QStringLiteral("a&b"), QStringLiteral("1.5"),
        QStringLiteral("x)y"), QStringLiteral("q~r"), QStringLiteral("!bang"),
    };
    return choice(words);
}

QString TestRoundtripFuzz::inlineToken()
{
    static const QList<QString> tex = {
        QStringLiteral("x^2"), QStringLiteral("a_i"), QStringLiteral("\\alpha"),
        QStringLiteral("\\frac{a}{b}"), QStringLiteral("\\sqrt{x}"),
        QStringLiteral("\\sum_{i=1}^{n} i"),
    };
    switch (pick(7)) {
    case 0: return QStringLiteral("**") + word() + QStringLiteral("**");
    case 1: return QStringLiteral("*") + word() + QStringLiteral("*");
    case 2: return QStringLiteral("`") + word() + QStringLiteral("`");
    case 3: return QStringLiteral("[") + word() + QStringLiteral("](http://e.com/")
                   + word() + QStringLiteral(")");
    case 4: return QStringLiteral("~~") + word() + QStringLiteral("~~");
    case 5: return QStringLiteral("$") + choice(tex) + QStringLiteral("$");
    default: return word();
    }
}

QString TestRoundtripFuzz::sentence(int minTok, int maxTok)
{
    QStringList toks;
    const int n = range(minTok, maxTok);
    for (int i = 0; i < n; ++i)
        toks << inlineToken();
    return toks.join(QLatin1Char(' '));
}

QString TestRoundtripFuzz::block()
{
    static const QList<QString> admon = {
        QStringLiteral("NOTE"), QStringLiteral("TIP"), QStringLiteral("WARNING"),
        QStringLiteral("IMPORTANT"), QStringLiteral("CAUTION"),
    };
    static const QList<QString> texBlocks = {
        QStringLiteral("\\sum_{i=1}^{n} \\frac{1}{i^2}"),
        QStringLiteral("E = mc^2"), QStringLiteral("\\frac{\\pi^2}{6}"),
        QStringLiteral("\\begin{matrix} a & b \\\\ c & d \\end{matrix}"),
    };
    switch (pick(13)) {
    case 0:  // encabezado
        return QString(QLatin1Char('#')).repeated(range(1, 6))
               + QLatin1Char(' ') + sentence(1, 4);
    case 1:  // párrafo
        return sentence(3, 8);
    case 2: {  // viñetas
        QStringList items;
        for (int i = 0, n = range(1, 4); i < n; ++i)
            items << QStringLiteral("- ") + sentence(1, 3);
        return items.join(QLatin1Char('\n'));
    }
    case 3: {  // lista numerada
        QStringList items;
        for (int i = 0, n = range(1, 4); i < n; ++i)
            items << QString::number(i + 1) + QStringLiteral(". ") + sentence(1, 3);
        return items.join(QLatin1Char('\n'));
    }
    case 4: {  // tareas
        QStringList items;
        for (int i = 0, n = range(1, 3); i < n; ++i)
            items << (pick(2) ? QStringLiteral("- [x] ") : QStringLiteral("- [ ] "))
                         + sentence(1, 3);
        return items.join(QLatin1Char('\n'));
    }
    case 5:  // cita
        return QStringLiteral("> ") + sentence(2, 5);
    case 6:  // admonición
        return QStringLiteral("> [!") + choice(admon) + QStringLiteral("]\n>\n> ")
               + sentence(2, 5);
    case 7: {  // bloque de código
        const QStringList langs = {QString(), QStringLiteral("cpp"),
                                   QStringLiteral("python"), QStringLiteral("js")};
        QStringList lines;
        for (int i = 0, n = range(1, 3); i < n; ++i)
            lines << word() + QLatin1Char(' ') + word();
        return QStringLiteral("```") + choice(langs) + QLatin1Char('\n')
               + lines.join(QLatin1Char('\n')) + QStringLiteral("\n```");
    }
    case 8: {  // tabla
        const int cols = range(2, 4);
        const QStringList aligns = {QStringLiteral("---"), QStringLiteral(":--"),
                                    QStringLiteral(":-:"), QStringLiteral("--:")};
        QStringList header, delim;
        for (int c = 0; c < cols; ++c) {
            header << word();
            delim << choice(aligns);
        }
        QString t = QStringLiteral("| ") + header.join(QStringLiteral(" | "))
                    + QStringLiteral(" |\n| ") + delim.join(QStringLiteral(" | "))
                    + QStringLiteral(" |");
        for (int r = 0, n = range(1, 3); r < n; ++r) {
            QStringList cells;
            for (int c = 0; c < cols; ++c)
                cells << word();
            t += QStringLiteral("\n| ") + cells.join(QStringLiteral(" | ")) + QStringLiteral(" |");
        }
        return t;
    }
    case 9:  // regla horizontal
        return pick(2) ? QStringLiteral("---") : QStringLiteral("***");
    case 10:  // nota al pie (referencia + definición)
        return sentence(2, 4) + QStringLiteral("[^1]\n\n[^1]: ") + sentence(1, 3);
    case 11:  // fórmula de bloque
        return QStringLiteral("$$") + choice(texBlocks) + QStringLiteral("$$");
    default:  // párrafo largo (estresa el line-wrapping de toMarkdown)
        return sentence(6, 12);
    }
}

QString TestRoundtripFuzz::document()
{
    QStringList blocks;
    for (int i = 0, n = range(1, 6); i < n; ++i)
        blocks << block();
    return blocks.join(QStringLiteral("\n\n"));
}

void TestRoundtripFuzz::survivesAdversarialContent()
{
    // Si un caso mata el proceso, el fallo no deja rastro de CUÁL era: la salida
    // se pierde con el crash y las semillas no se ven. Dos válvulas por entorno,
    // que no cambian nada en una ejecución normal:
    //   MD_FUZZ_TRACE=1  → escribe el índice de cada caso en stderr sin buffer,
    //                      así el último impreso ES el que revienta.
    //   MD_FUZZ_ONLY=N   → ejecuta solo el caso N y vuelca su documento, para
    //                      reproducirlo aislado (el PRNG es determinista, así
    //                      que N genera lo mismo en cualquier sistema).
    const bool trace = !qEnvironmentVariableIsEmpty("MD_FUZZ_TRACE");
    const int only = qEnvironmentVariableIntValue("MD_FUZZ_ONLY");

    constexpr int kCases = 2000;
    for (int i = 1; i <= kCases; ++i) {
        if (only > 0 && i != only)
            continue;
        seed(quint32(i) * 2654435761u);  // dispersa las semillas (Knuth)
        const QString body = document();
        if (trace) {
            std::fprintf(stderr, "caso %d (%lld caracteres)\n", i, qint64(body.size()));
            std::fflush(stderr);
        }
        if (only > 0) {
            std::fprintf(stderr, "--- documento del caso %d ---\n%s\n--- fin ---\n",
                         i, qPrintable(body));
            std::fflush(stderr);
        }

        // Editor NUEVO por caso: cada documento renderiza fórmulas (que registran
        // un handler de objeto en su layout), notas al pie, etc. Reutilizar un solo
        // QTextEdit para miles de setMarkdown acumula ese estado por el documento y
        // puede degenerar en entornos con otro build de Qt; un editor por caso deja
        // cada uno independiente (que además es lo correcto para fuzzear).
        QTextEdit ed;
        ed.setAcceptRichText(true);

        // Con MD_FUZZ_ONLY, vuelca la entrada EXACTA de cada pasada escapando
        // todo lo que no sea ASCII imprimible. La segunda pasada recibe la
        // salida ya serializada, que lleva dentro los centinelas de la PUA de
        // las fórmulas: si lo que mata al proceso es esa, hay que verlo carácter
        // a carácter, no en texto renderizado.
        const auto dump = [only](const char *etiqueta, const QString &s) {
            if (only <= 0)
                return;
            QString esc;
            for (const QChar c : s) {
                if (c.unicode() < 0x20 || c.unicode() > 0x7e)
                    esc += QStringLiteral("\\u%1").arg(c.unicode(), 4, 16, QLatin1Char('0'));
                else
                    esc += c;
            }
            std::fprintf(stderr, "%s (%lld caracteres): %s\n", etiqueta,
                         qint64(s.size()), qPrintable(esc));
            std::fflush(stderr);
        };

        // Doble pasada por el pipeline real. El objetivo es que no haya SIGSEGV,
        // assert ni error de sanitizer en ninguna de las etapas (proteger →
        // setMarkdown → pasadas de render → serialización con sentinelas).
        if (trace) {
            std::fprintf(stderr, "  caso %d pasada 1\n", i);
            std::fflush(stderr);
        }
        dump("  entrada pasada 1", body);
        mdrender::setMarkdownWithExtensions(&ed, body);
        const QString once = mdtable::documentMarkdown(ed.document());
        if (trace) {
            std::fprintf(stderr, "  caso %d pasada 2\n", i);
            std::fflush(stderr);
        }
        dump("  entrada pasada 2", once);
        mdrender::setMarkdownWithExtensions(&ed, once);
        const QString twice = mdtable::documentMarkdown(ed.document());
        Q_UNUSED(twice);
    }

    // Llegar aquí (completar los kCases sin morir) ES la prueba: el valor está en
    // que no haya habido un crash/assert/error de sanitizer en ninguna de las
    // combinaciones. No se comprueba el contenido exacto (toMarkdown de Qt no es
    // idempotente y los documentos solo-regla serializan a vacío legítimamente).
    QVERIFY(true);
}

QTEST_MAIN(TestRoundtripFuzz)
#include "tst_roundtripfuzz.moc"
