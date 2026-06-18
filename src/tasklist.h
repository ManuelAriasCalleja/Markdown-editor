#ifndef TASKLIST_H
#define TASKLIST_H

#include <QPoint>

class QTextEdit;

// Casillas de tarea interactivas (`- [ ]` / `- [x]`). Qt renderiza los ítems de
// tarea como bloques con un marcador (`QTextBlockFormat::marker()`), y los
// serializa de vuelta a Markdown sin ayuda: aquí solo añadimos el gesto de
// marcar/desmarcar con un clic sobre la casilla del editor WYSIWYG.
//
// La lógica de geometría (mapear el punto del visor a un bloque y decidir si el
// clic cae sobre la casilla, a la izquierda del texto del ítem) vive aquí, fuera
// de MainWindow, para poder probarla con un QTextEdit aislado.
namespace mdtask {

// ¿El punto `viewportPos` (coordenadas del viewport del editor) cae sobre la
// casilla de un ítem de tarea? Sirve para la pista del cursor al pasar por
// encima, sin modificar el documento.
bool isCheckboxAt(QTextEdit *editor, const QPoint &viewportPos);

// Si `viewportPos` cae sobre la casilla de un ítem de tarea, alterna su estado
// (marcado ↔ desmarcado) con una edición deshacible y devuelve true. Si el
// bloque no es una tarea o el clic cae sobre el texto del ítem, no hace nada y
// devuelve false (para que el clic siga su curso normal: colocar el cursor).
bool toggleCheckboxAt(QTextEdit *editor, const QPoint &viewportPos);

}  // namespace mdtask

#endif  // TASKLIST_H
