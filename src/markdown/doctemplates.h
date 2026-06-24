#ifndef DOCTEMPLATES_H
#define DOCTEMPLATES_H

/// \file
/// \brief Catálogo de plantillas de documento para «Nuevo desde plantilla».

#include <QList>
#include <QString>

/// Catálogo de plantillas de documento para «Archivo → Nuevo desde plantilla».
/// Cada plantilla es un esqueleto Markdown con marcadores «[…]» que el usuario
/// rellena. Los textos pasan por tr() (contexto "MainWindow"), así que se traducen
/// con el resto de la interfaz; la fecha no se rellena automáticamente (el usuario
/// edita el marcador «[fecha]»).
namespace mdtemplate {

/// Una plantilla de documento: rótulo de menú y cuerpo Markdown inicial.
struct DocTemplate {
    QString name;  ///< etiqueta del submenú (traducida)
    QString body;  ///< contenido Markdown inicial (traducido)
};

/// Las plantillas disponibles, ya en el idioma vigente. Se construye al vuelo para
/// que recoja el traductor activo (la interfaz cambia de idioma sin reiniciar).
QList<DocTemplate> all();

}  // namespace mdtemplate

#endif  // DOCTEMPLATES_H
