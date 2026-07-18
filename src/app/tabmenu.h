#ifndef TABMENU_H
#define TABMENU_H

/// \file
/// \brief Datos puros del menú contextual de una pestaña (abrir carpeta / copiar
/// nombre / copiar ruta), derivados de la ruta del documento.

#include <QString>

/// Lógica pura (sin GUI) del menú contextual de la barra de pestañas. La
/// construcción del `QMenu` y las acciones (portapapeles, `QDesktopServices`)
/// viven en `MainWindow::showTabContextMenu`; aquí solo se deriva, de la ruta del
/// documento, qué mostrar. Se prueba en tst_tabmenu.
namespace tabmenu {

/// Datos del menú contextual de una pestaña, derivados de la ruta del documento.
/// Si la pestaña aún no tiene archivo en disco («Sin título»), `hasFile` es false
/// y las tres acciones deben mostrarse deshabilitadas (no hay nada que abrir ni
/// copiar).
struct FileInfo {
    bool hasFile = false;      ///< false si el documento no está guardado todavía.
    QString fileName;          ///< Nombre del archivo (para copiar).
    QString fullPath;          ///< Ruta absoluta completa (para copiar).
    QString containingFolder;  ///< Carpeta contenedora (para abrir en el gestor).
};

/// Deriva de `path` (la ruta del documento de la pestaña, posiblemente vacía) los
/// datos del menú contextual. Función pura.
FileInfo infoForPath(const QString &path);

}  // namespace tabmenu

#endif  // TABMENU_H
