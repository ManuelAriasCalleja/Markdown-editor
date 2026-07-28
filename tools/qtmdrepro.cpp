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
#include <QTextDocument>

#include <cstdio>
#include <cstdlib>

namespace {

struct Variante
{
    const char *nombre;
    QString texto;
};

// La cadena completa es la que produce `mdrender::protect()` sobre el documento
// del caso 281 (verificado volcándola carácter a carácter): el cuerpo original
// con la fórmula envuelta en inline-code doble. Se escribe aquí literal, sin
// llamar a `protect`, para no arrastrar el proyecto.
const QString kCompleta = QStringLiteral(
    "---\n"
    "\n"
    "- [ ] **über** ~~niño~~\n"
    "- [ ] **c_d** *dato* c_d\n"
    "\n"
    "!bang [hash#](http://e.com/back\\slash) `a&b` ``$\\sqrt{x}$``\n"
    "\n"
    "---");

QList<Variante> variantes()
{
    return {
        {"completa", kCompleta},
        // El párrafo suelto: los tres constructos sospechosos juntos, sin las
        // listas de tareas ni las reglas temáticas.
        {"parrafo", QStringLiteral(
             "!bang [hash#](http://e.com/back\\slash) `a&b` ``$\\sqrt{x}$``")},
        // Cada constructo del párrafo por separado.
        {"enlace-barra", QStringLiteral("[hash#](http://e.com/back\\slash)")},
        {"enlace-simple", QStringLiteral("[a](http://e.com/x)")},
        {"code-amp", QStringLiteral("`a&b`")},
        {"code-doble-tex", QStringLiteral("``$\\sqrt{x}$``")},
        {"bang", QStringLiteral("!bang")},
        // Las listas de tareas con acentuadas (los únicos no-ASCII del caso).
        {"tareas", QStringLiteral(
             "- [ ] **über** ~~niño~~\n- [ ] **c_d** *dato* c_d")},
        {"tarea-acentos", QStringLiteral("- [ ] **über** ~~niño~~")},
        // Regla temática pegada a una lista de tareas: el arranque del documento.
        {"regla-tareas", QStringLiteral("---\n\n- [ ] **über** ~~niño~~")},
        {"regla", QStringLiteral("---")},
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
