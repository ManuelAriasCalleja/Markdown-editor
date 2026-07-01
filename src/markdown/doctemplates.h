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

/// Categoría profesional de una plantilla. Agrupa el menú «Nuevo desde plantilla» en
/// submenús; una categoría sin plantillas no se muestra. El orden de presentación lo
/// da categoriesInOrder().
enum class Category {
    Personal,     ///< Personal y general (nota, carta, tareas, certificado…)
    Programming,  ///< Programación (README, CHANGELOG…)
    Academic,     ///< Académico / investigación
    Teaching,     ///< Docencia (examen, práctica…)
    Business,     ///< Empresa / negocios (acta, informe…)
    Legal,        ///< Derecho
    Writing,      ///< Escritura (blog…)
};

/// Una plantilla de documento: rótulo de menú, cuerpo Markdown inicial y categoría.
struct DocTemplate {
    QString name;       ///< etiqueta del submenú (traducida)
    QString body;       ///< contenido Markdown inicial (traducido)
    Category category;  ///< submenú en el que aparece
};

/// Las plantillas disponibles, ya en el idioma vigente. Se construye al vuelo para
/// que recoja el traductor activo (la interfaz cambia de idioma sin reiniciar).
QList<DocTemplate> all();

/// Nombre traducido de una categoría (para el rótulo del submenú).
QString categoryName(Category category);

/// Las categorías en el orden en que deben aparecer en el menú.
QList<Category> categoriesInOrder();

/// Documento de bienvenida (traducido) que se muestra en el primer arranque. No es
/// una plantilla del menú: es un texto introductorio de un solo uso.
QString welcomeDocument();

}  // namespace mdtemplate

#endif  // DOCTEMPLATES_H
