#ifndef EPUBIMPORT_H
#define EPUBIMPORT_H

/// \file
/// \brief Importación de libros EPUB: un .epub es un ZIP de XHTML (más el paquete
///        OPF y META-INF/container.xml). Se sigue container → OPF → lomo (spine) y
///        cada capítulo XHTML se convierte a Markdown con mdrichpaste::htmlToMarkdown.

#include <QString>
#include <QStringList>

class QByteArray;

namespace mdimport {

/// Ruta del paquete OPF dentro del EPUB, leída de `META-INF/container.xml` (el
/// primer `<rootfile full-path=…>`), o cadena vacía. Función pura (testeable).
QString epubOpfPath(const QByteArray &containerXml);

/// Hrefs de los documentos XHTML del lomo (`<spine>`), en orden de lectura,
/// resolviendo el manifiesto (`id`→`href`) y quedándose solo con los XHTML. Los
/// href son relativos al OPF y vienen ya des-escapados de porcentaje. Función pura.
QStringList epubSpineHrefs(const QByteArray &opfXml);

/// Convierte los bytes de un `.epub` a Markdown: lee el ZIP, sigue container → OPF →
/// lomo, convierte cada capítulo XHTML con mdrichpaste::htmlToMarkdown y los
/// concatena en orden. Devuelve cadena vacía si no es un EPUB válido o no tiene
/// contenido. Integración (usa el QZip privado de Qt), pero testeable en memoria.
QString epubToMarkdown(const QByteArray &epubBytes);

}  // namespace mdimport

#endif  // EPUBIMPORT_H
