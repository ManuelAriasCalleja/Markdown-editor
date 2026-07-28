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
    // Bisección terminada (tres rondas de CI). Conclusiones, que son las que
    // dejan esta lista reducida a lo que sigue aportando algo:
    //   • Con Qt PELADO —esto no enlaza md-editor-core— el documento revienta en
    //     Windows con 0xC0000005: el fallo es de Qt, no del editor.
    //   • El caso mínimo son SEIS caracteres: `---\n\n---`.
    //   • Solo con el marcador `---`: `***` y `- - -` pasan, y el mixto también.
    //     Es la forma ambigua (regla temática / subrayado setext / delimitador
    //     de front matter) la que lo dispara.
    //   • Solo con exactamente dos: con tres reglas pasa, y `---\n---` pegadas
    //     también.
    //   • MarkdownNoHTML es indiferente, y los acentos no pintan nada
    //     (`completa-ascii` caía igual), pese a que la traza pasa por
    //     `convertToUtf8`.
    //   • Un `\n` final lo esquiva, tanto en el mínimo como en el documento real.
    //     Ese es el rodeo que aplica `mdrender::protect()`.
    //
    // Lo que queda aquí es la evidencia mínima para el reporte aguas arriba y,
    // sobre todo, el CENTINELA: cuando `minimo` deje de caer en el CI de
    // Windows, Qt lo habrá arreglado y se podrá retirar el rodeo de `protect()`
    // (y este programa con él).
    return {
        {"minimo", QStringLiteral("---\n\n---")},           // debe CAER en Windows
        {"minimo-nl", QStringLiteral("---\n\n---\n")},      // el rodeo: debe pasar
        {"completa", kCompleta},                            // debe CAER en Windows
        {"completa-nl", kCompleta + QLatin1Char('\n')},     // el rodeo: debe pasar
        {"asteriscos", QStringLiteral("***\n\nx\n\n***")},  // otro marcador: pasa
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
