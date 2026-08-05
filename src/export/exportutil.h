#ifndef EXPORTUTIL_H
#define EXPORTUTIL_H

/// \file
/// \brief Piezas compartidas por los serializadores de exportación. Cabecera
///        **interna**: la usan entre sí los `export*.cpp`, no los consumidores
///        (que solo incluyen `exporters.h`).

#include <QByteArray>
#include <QString>
#include <QStringList>

class QTextDocument;
class QUrl;

namespace mdexport {

/// \brief Una imagen del documento ya resuelta a bytes, lista para meterla en el
/// paquete (EPUB), en un fichero al lado (LaTeX) o en un `data:` URI (HTML).
struct ImageData {
    QByteArray bytes;   ///< contenido: los originales, o un PNG rasterizado
    QString extension;  ///< "png", "jpg", "svg"… (en minúsculas)
    QString mediaType;  ///< "image/png", "image/svg+xml"…

    bool isNull() const { return bytes.isEmpty(); }
};

/// \brief ¿El punto de código pertenece a una ESCRITURA CJK (no a un símbolo suelto)?
///
/// Ideogramas chinos, kana japonés, hangul coreano, bopomofo y la puntuación y las
/// formas de ancho completo que los acompañan. La distinción es la clave de dos
/// arreglos distintos, y por eso la tabla de rangos vive en un solo sitio: un emoji
/// que no se puede componer se descarta y no pasa nada, pero descartar una escritura
/// entera es borrar el documento. En LaTeX, lo que caiga aquí se emite y obliga a
/// xelatex (`latexEscape`); en el PDF, obliga a comprobar que hay fuente que lo
/// dibuje (`cjkScriptsIn`).
bool isScriptChar(uint cp);

/// \brief Ruta local legible del recurso `name` resuelta contra `baseUrl` (las rutas
/// relativas del Markdown), o "" si es remota o no existe.
QString localFileFor(const QString &name, const QUrl &baseUrl);

/// \brief Media type de una extensión de imagen ("svg" → "image/svg+xml"). Lo que no
/// reconoce cae a "image/png", que es a lo que se rasteriza (ver `imageData`).
QString imageMediaType(const QString &extension);

/// \brief Recupera del documento la imagen `name`.
///
/// Si su formato está en `keepFormats` y el fichero se puede leer del disco,
/// devuelve los **bytes originales**: reencodearlo todo a PNG conserva el aspecto
/// pero infla una foto JPEG y convierte un SVG vectorial en un mapa de bits. Si no
/// (formato que el destino no admite, fichero ilegible, imagen remota), la
/// **rasteriza a PNG** con `doc->resource()`, que resuelve la ruta relativa por la
/// baseUrl y rasteriza el SVG con el mismo plugin que ya lo pinta en el editor.
/// Con `keepFormats` vacío siempre rasteriza (es lo que necesita el DOCX, que solo
/// declara PNG en sus tipos de contenido).
///
/// Devuelve un `ImageData` nulo si no hay manera de obtenerla.
ImageData imageData(const QString &name, const QTextDocument *doc,
                    const QStringList &keepFormats);

/// \brief Escapa para XML y descarta lo que XML 1.0 no admite como carácter: los de
/// control C0 (salvo tabulador, salto de línea y retorno), U+FFFE/U+FFFF y los
/// suplentes sueltos. No es un adorno: basta uno de esos —pegado desde otra
/// aplicación, o leído de un .md con basura binaria— para dejar el XML mal formado,
/// y entonces Word (o el lector de EPUB) rechaza el fichero entero sin decir por qué.
QString xmlEsc(const QString &s);

}  // namespace mdexport

#endif  // EXPORTUTIL_H
