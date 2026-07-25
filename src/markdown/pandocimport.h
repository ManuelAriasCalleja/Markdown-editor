#ifndef PANDOCIMPORT_H
#define PANDOCIMPORT_H

/// \file
/// \brief Importación de otros formatos (DOCX, ODT, RTF, LaTeX, reST…) mediante la
///        herramienta externa Pandoc, si está instalada. Mismo enfoque que los
///        diagramas: sin dependencia enlazada (solo se ejecuta por QProcess), con
///        degradación elegante cuando falta. La ejecución del proceso vive en
///        MainWindow; aquí, solo la lógica pura y testeable.

#include <QString>
#include <QStringList>

namespace mdimport {

/// ¿Está `pandoc` en el PATH? (búsqueda del ejecutable, sin lanzarlo).
bool pandocAvailable();

/// Argumentos para convertir `inputPath` a Markdown GitHub por la salida estándar:
/// Pandoc infiere el formato de entrada por la extensión. `--wrap=none` evita partir
/// los párrafos en líneas (mejor para el editor visual). Si `mediaDir` no está
/// vacío se añade `--extract-media`: sin él, las imágenes incrustadas en el .docx
/// (o .odt, .epub…) se PIERDEN —Pandoc deja una referencia a una ruta que solo
/// existe dentro del paquete—. Función pura.
QStringList pandocArguments(const QString &inputPath, const QString &mediaDir = QString());

/// Carpeta donde extraer las imágenes de `inputPath`: `<nombre>-media`, junto al
/// documento de origen, que es donde el usuario espera encontrarlas (y donde
/// guardará el Markdown). "" si `inputPath` está vacío. Función pura.
QString mediaDirFor(const QString &inputPath);

/// Deja las imágenes de la salida de Pandoc en la forma que el editor sabe cargar.
/// Son dos arreglos y los dos evitan que la imagen SE PIERDA:
///   - `<img src="…" style="…">` → `![alt](ruta)`. Pandoc cae a HTML crudo cuando
///     la imagen lleva tamaño (GFM no lo expresa), y el editor carga con
///     `MarkdownNoHTML`: esa etiqueta se vería como texto literal.
///   - texto alternativo vacío → el nombre del fichero. `QTextDocument::setMarkdown`
///     **descarta** `![](ruta)`: sin alt no inserta nada, ni imagen ni texto.
/// Las rutas con espacios o paréntesis se envuelven en `<...>`. Función pura.
QString repairImages(const QString &markdown);

/// Convierte a tabla Markdown los bloques `<table>` que Pandoc emite cuando GFM no
/// sabe expresar la tabla: celdas combinadas (`colspan`/`rowspan`), celdas con
/// varios párrafos o listas, y tablas ANIDADAS. Con `MarkdownNoHTML` ese HTML se
/// vería como texto literal, así que la tabla se perdía entera.
///
/// La conversión APLANA lo que Markdown no tiene: una celda con varios bloques (o
/// con una tabla dentro) queda como una sola línea con sus textos separados por
/// espacios, y un `colspan` se reparte en celdas vacías a la derecha. Se conserva el
/// formato en línea (negrita, cursiva, código, enlaces). Si el bloque no se puede
/// parsear se deja intacto, que es mejor que perderlo. Función pura.
QString htmlTablesToMarkdown(const QString &markdown);

/// Patrón de extensiones de archivo que Pandoc sabe importar (para el filtro del
/// diálogo de abrir), p. ej. "*.docx *.odt *.rtf …". Solo datos; el rótulo
/// traducible del filtro lo pone quien abre el diálogo. Función pura.
QString pandocFilePattern();

/// Orden de instalación de Pandoc según el sistema en ejecución (QSysInfo, sin
/// `#ifdef`), para sugerirla cuando no está instalado. Función pura.
QString pandocInstallCommand();

}  // namespace mdimport

#endif  // PANDOCIMPORT_H
