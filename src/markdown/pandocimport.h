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
/// los párrafos en líneas (mejor para el editor visual). Función pura.
QStringList pandocArguments(const QString &inputPath);

/// Patrón de extensiones de archivo que Pandoc sabe importar (para el filtro del
/// diálogo de abrir), p. ej. "*.docx *.odt *.rtf …". Solo datos; el rótulo
/// traducible del filtro lo pone quien abre el diálogo. Función pura.
QString pandocFilePattern();

/// Orden de instalación de Pandoc según el sistema en ejecución (QSysInfo, sin
/// `#ifdef`), para sugerirla cuando no está instalado. Función pura.
QString pandocInstallCommand();

}  // namespace mdimport

#endif  // PANDOCIMPORT_H
