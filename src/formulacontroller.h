#ifndef FORMULACONTROLLER_H
#define FORMULACONTROLLER_H

#include <QObject>
#include <QString>

class MathObject;
class QKeyEvent;
class QPoint;
class QTextEdit;
class QWidget;

// Inserción y edición de fórmulas TeX (`$...$` / `$$...$$`) renderizadas en el
// editor WYSIWYG, y su protección frente a la edición accidental con el teclado.
//
// Las fórmulas se insertan como secuencias de runs (super/subíndices reales de
// Qt) que comparten el TeX en una propiedad, lo que permite reconocerlas como
// grupo (ver mdmath). Las operaciones de teclado/pegado las invoca el filtro de
// eventos de MainWindow. Los mensajes de estado van por señal; los diálogos usan
// `parent` como padre.
class FormulaController : public QObject
{
    Q_OBJECT

public:
    FormulaController(QTextEdit *editor, QWidget *parent);

    // Doble clic sobre una fórmula: si en `viewportPos` (coords del viewport del
    // editor) hay una fórmula, abre el diálogo precargado y la sustituye. Devuelve
    // true si actuó (atendió el clic, aunque el usuario cancele).
    bool editFormulaAt(const QPoint &viewportPos);
    // Intercepta teclas cerca o dentro de una fórmula: bloquea el tecleo de
    // imprimibles dentro (se editan con doble clic) y convierte Backspace/Delete en
    // el borde en borrado atómico del grupo. true si consumió el evento.
    bool handleMathKeyPress(QKeyEvent *event);
    // Si la selección toca o atraviesa fórmulas, la extiende a los grupos enteros
    // (para que un pegado reemplace la fórmula completa, no la parta).
    void guardPasteAgainstMath();

public slots:
    void insertFormula();  // diálogo (Ctrl+Shift+F) e inserción en el cursor

signals:
    void statusMessage(const QString &text, int timeoutMs);

private:
    // Diálogo de fórmula con previsualización en vivo, precargado con
    // `initialTex`/`initialBlock`. Escribe en *tex/*block y devuelve true si se
    // acepta con TeX no vacío.
    bool askFormula(QString *tex, bool *block,
                    const QString &initialTex = QString(), bool initialBlock = false);

    QTextEdit *m_editor = nullptr;
    QWidget *m_parent = nullptr;  // padre del diálogo de fórmula
    MathObject *m_mathObject = nullptr;  // handler 2D registrado en el documento
};

#endif // FORMULACONTROLLER_H
