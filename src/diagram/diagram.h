#ifndef DIAGRAM_H
#define DIAGRAM_H

#include <QString>

// Lógica PURA de los diagramas (Mermaid/PlantUML): identificar qué bloques de
// código son diagramas a partir de su lenguaje. El render real (ejecutar la
// herramienta externa) vive en `DiagramRenderer`; aquí solo hay clasificación,
// para poder probarlo aislado (`tst_diagram`).
namespace mddiagram {

// Motor de diagrama de un bloque de código, según su etiqueta de lenguaje.
enum class Kind {
    None,      // no es un diagrama
    Mermaid,   // ```mermaid
    PlantUml,  // ```plantuml / ```puml / ```uml
};

// Tipo de diagrama para la etiqueta de lenguaje de un bloque ```lang (sin
// distinguir mayúsculas; se ignoran espacios). `None` si no es un diagrama.
Kind kindForLanguage(const QString &lang);

// ¿Esta etiqueta de lenguaje corresponde a un diagrama renderizable?
inline bool isDiagramLanguage(const QString &lang)
{
    return kindForLanguage(lang) != Kind::None;
}

} // namespace mddiagram

#endif // DIAGRAM_H
