// Reproductor mínimo del cuelgue de `QTextDocument::setMarkdown` en Windows.
//
// NO enlaza `md-editor-core`: solo Qt. Ese es todo el objetivo — el caso 281 de
// `tst_roundtripfuzz` muere en Windows con 0xC0000005 dentro de Qt
// (`QTextMarkdownImporter::import` → `QtPrivate::convertToUtf8` →
// `QUtf8::convertFromUnicode`), en la PRIMERA pasada, así que la cadena que lo
// mata es exactamente la que el editor le pasa a `setMarkdown` y nada de este
// proyecto interviene después. Si este programa revienta, el fallo es de Qt y
// hay que reportarlo aguas arriba; si no revienta, la culpa vuelve a casa.
//
// Se ejecuta UNA variante por proceso (`qtmdrepro <n>`), porque un fallo mata el
// proceso y se llevaría por delante el resto de la batería: el script de CI las
// recorre en bucle y anota el código de salida de cada una. Las variantes van de
// la cadena completa a fragmentos de un solo constructo, para acotar el
// disparador sin depender de un depurador.
//
// Salida: una línea por variante en stdout, sin buffer, antes y después del
// `setMarkdown`, de modo que un crash deje claro en cuál murió.

#include <QApplication>
#include <QString>
#include <QStringList>
#include <QTextDocument>

#include <cstdio>
#include <cstdlib>

namespace {

struct Variante
{
    const char *nombre;
    QString texto;
};

// Las piezas del documento del caso 281, tal cual salen de `mdrender::protect()`
// (verificado volcando la cadena carácter a carácter): el cuerpo original con la
// fórmula envuelta en inline-code doble. Se escriben aquí literales, sin llamar a
// `protect`, para no arrastrar el proyecto.
const QString kRegla = QStringLiteral("---");
const QString kTarea1 = QStringLiteral("- [ ] **über** ~~niño~~");
const QString kTarea2 = QStringLiteral("- [ ] **c_d** *dato* c_d");
const QString kParrafo =
    QStringLiteral("!bang [hash#](http://e.com/back\\slash) `a&b` ``$\\sqrt{x}$``");

// Los bloques se separan por línea en blanco, como en el documento original.
QString arma(const QStringList &bloques)
{
    return bloques.join(QStringLiteral("\n\n"));
}

const QString kTareas = kTarea1 + QLatin1Char('\n') + kTarea2;
const QString kCompleta = arma({kRegla, kTareas, kParrafo, kRegla});

QList<Variante> variantes()
{
    // Segunda ronda. La primera dejó dos hechos: con Qt PELADO la cadena
    // completa revienta en Windows (luego es un fallo de Qt, no del editor), y
    // NINGÚN fragmento aislado lo hace —ni el párrafo, ni las tareas, ni el
    // enlace con `\`, ni el code span con `&`, ni el inline-code con TeX—, así
    // que hace falta el documento entero. Toca bisecarlo.
    //
    // Hipótesis de trabajo, que es lo que ordena esta lista: el fallo está en
    // `convertToUtf8` según la traza, y los ÚNICOS caracteres no-ASCII del
    // documento son la `ü` y la `ñ` de la primera tarea. Si el importador
    // Markdown de Qt mezcla desplazamientos en BYTES de UTF-8 con índices en
    // UTF-16 —clase de fallo conocida—, hacen falta las dos cosas a la vez: un
    // carácter multibyte que descuadre la cuenta y documento suficiente por
    // delante para que el índice torcido acabe fuera del buffer. Eso explicaría
    // por qué los fragmentos cortos con acentos pasan y el documento no.
    // `completa-ascii` es la variante que decide: misma estructura exacta, solo
    // que `ü`→`u` y `ñ`→`n`. Si esa pasa, la hipótesis se sostiene.
    const QString tarea1Ascii = QStringLiteral("- [ ] **uber** ~~nino~~");
    const QString tareasAscii = tarea1Ascii + QLatin1Char('\n') + kTarea2;

    return {
        {"completa", kCompleta},  // control: tiene que caer
        {"completa-ascii", arma({kRegla, tareasAscii, kParrafo, kRegla})},

        // Qué bloques hacen falta. Se quita uno cada vez, dejando el resto igual.
        {"sin-regla-inicial", arma({kTareas, kParrafo, kRegla})},
        {"sin-regla-final", arma({kRegla, kTareas, kParrafo})},
        {"sin-reglas", arma({kTareas, kParrafo})},
        {"sin-tarea-2", arma({kRegla, kTarea1, kParrafo, kRegla})},
        {"sin-tareas", arma({kRegla, kParrafo, kRegla})},
        {"sin-parrafo", arma({kRegla, kTareas, kRegla})},

        // Qué parte del párrafo hace falta, con el resto del documento intacto.
        {"parrafo-solo-bang", arma({kRegla, kTareas, QStringLiteral("!bang"), kRegla})},
        {"parrafo-sin-tex", arma({kRegla, kTareas,
                                  QStringLiteral("!bang [hash#](http://e.com/back\\slash) `a&b`"),
                                  kRegla})},
        {"parrafo-sin-enlace", arma({kRegla, kTareas,
                                     QStringLiteral("!bang `a&b` ``$\\sqrt{x}$``"), kRegla})},
    };
}

}  // namespace

int main(int argc, char **argv)
{
    // QApplication (no QCoreApplication): `setMarkdown` acaba en la maquetación
    // de texto, que necesita la base de datos de fuentes.
    QApplication app(argc, argv);

    const QList<Variante> v = variantes();

    // Sin argumento: lista las variantes y su número, para que el bucle del
    // script sepa cuántas hay sin tenerlas duplicadas en dos sitios.
    if (argc < 2) {
        std::printf("%lld\n", qint64(v.size()));
        for (qsizetype i = 0; i < v.size(); ++i)
            std::printf("%lld %s\n", qint64(i), v[i].nombre);
        return 0;
    }

    const int n = std::atoi(argv[1]);
    if (n < 0 || n >= v.size()) {
        std::fprintf(stderr, "variante fuera de rango: %d\n", n);
        return 2;
    }

    // Las mismas features que usa el editor de verdad (`mdrender::kMarkdownFeatures`).
    // El segundo argumento permite probar sin NoHTML, por si el bicho está ahí.
    QTextDocument::MarkdownFeatures features =
        QTextDocument::MarkdownFeatures(QTextDocument::MarkdownDialectGitHub
                                        | QTextDocument::MarkdownNoHTML);
    if (argc > 2 && QString::fromLocal8Bit(argv[2]) == QLatin1String("github"))
        features = QTextDocument::MarkdownDialectGitHub;

    std::printf("variante %d (%s): %lld caracteres, antes de setMarkdown\n",
                n, v[n].nombre, qint64(v[n].texto.size()));
    std::fflush(stdout);

    QTextDocument doc;
    doc.setMarkdown(v[n].texto, features);

    std::printf("variante %d (%s): OK, %d bloques\n", n, v[n].nombre, doc.blockCount());
    std::fflush(stdout);
    return 0;
}
