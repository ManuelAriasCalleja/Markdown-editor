#ifndef FORMATCONTROLLER_H
#define FORMATCONTROLLER_H

/// \file
/// \brief Comandos de formato del editor WYSIWYG (marcas, encabezados, listas, sangría) y estado de sus acciones.

#include <QObject>
#include <QTextListFormat>

#include <functional>

class QAction;
class QTextCharFormat;
class QTextEdit;
class QWidget;
class CodeBlockHighlighter;

/// Comandos de formato del editor WYSIWYG: negrita/cursiva/…, encabezados, listas
/// (viñetas, numeradas, tareas), citas y bloques de código, lenguaje del bloque y
/// sangría. Y la sincronización de las acciones de la barra/menú con el formato
/// bajo el cursor (updateActions).
///
/// Aplica formatos de Qt que round-trip-ean limpiamente a Markdown; las citas y
/// bloques de código se reescriben con BlockConstructs. No posee las QAction (las
/// comparten menú y barra de botones): se le pasan con setActions para poder
/// reflejar su estado. Los mensajes de estado van por señal.
class FormatController : public QObject
{
    Q_OBJECT

public:
    /// Acciones de formato que updateActions mantiene marcadas/habilitadas según el
    /// contexto. Las crea MainWindow (las comparten el menú Formato y la barra).
    struct Actions {
        QAction *bold = nullptr;
        QAction *italic = nullptr;
        QAction *underline = nullptr;
        QAction *strike = nullptr;
        QAction *code = nullptr;
        QAction *link = nullptr;
        QAction *quote = nullptr;
        QAction *codeBlock = nullptr;
        QAction *lang = nullptr;
        QAction *h1 = nullptr;
        QAction *h2 = nullptr;
        QAction *h3 = nullptr;
        QAction *h4 = nullptr;
        QAction *h5 = nullptr;
        QAction *h6 = nullptr;
        QAction *bullet = nullptr;
        QAction *numbered = nullptr;
        QAction *task = nullptr;
        QAction *indent = nullptr;
        QAction *outdent = nullptr;
    };

    FormatController(QTextEdit *editor, CodeBlockHighlighter *highlighter, QWidget *parent);

    /// \brief Fija las acciones cuyo estado refresca updateActions.
    void setActions(const Actions &actions) { m_actions = actions; }

    /// Aplica un toggle de formato de carácter: `mutate` recibe el formato a
    /// rellenar y el formato actual bajo el cursor (Strategy de los botones
    /// negrita/cursiva/tachado/código). Público porque lo invocan las lambdas de
    /// las acciones que crea MainWindow.
    void toggleCharFormat(
        const std::function<void(QTextCharFormat &, const QTextCharFormat &)> &mutate);

    /// Mantiene las acciones reflejando el formato bajo el cursor. Emite
    /// actionsUpdated al terminar (para encadenar el refresco de otras acciones
    /// contextuales, p. ej. las de tabla).
    void updateActions();

public slots:
    void applyHeading(int level);                  // fija/quita H`level` (toggle)
    void promoteHeading();                          // sube el encabezado del cursor un nivel (hacia H1)
    void demoteHeading();                           // baja el encabezado del cursor un nivel (hacia H6)
    void insertHighlight();                         // envuelve la selección en `==marca==`
    void toggleSuperscript();                       // super/subíndice de texto (^x^ / ~x~)
    void toggleSubscript();
    void applyList(QTextListFormat::Style style);   // fija/quita lista del estilo (toggle)
    void toggleBlockquote();
    void toggleCodeBlock();
    void toggleTaskItem();
    void setCodeLanguage();
    void indentList();
    void outdentList();

signals:
    /// Mensaje para la barra de estado de la ventana.
    void statusMessage(const QString &text, int timeoutMs);
    /// Se refrescó el estado de las acciones de formato (encadena otras contextuales).
    void actionsUpdated();

private:
    // Fija el bloque actual como encabezado de `level` (1..6) o párrafo (0). Núcleo
    // absoluto (sin toggle) que comparten applyHeading y shiftHeading.
    void setHeadingLevel(int level);
    // Promover (delta −1) / degradar (delta +1) el encabezado del cursor, acotado a
    // [1,6]; no hace nada si el bloque no es un encabezado o ya está en el límite.
    void shiftHeading(int delta);
    // Aplica/quita el super/subíndice de texto (`va`) sobre la selección, marcándolo
    // con SupSubProperty para que la serialización lo reinyecte como `^x^` / `~x~`.
    void toggleVerticalAlign(QTextCharFormat::VerticalAlignment va);
    void mergeCharFormatOnSelection(const QTextCharFormat &format);
    void changeListIndent(int delta);

    QTextEdit *m_editor = nullptr;
    CodeBlockHighlighter *m_highlighter = nullptr;
    QWidget *m_parent = nullptr;     // padre del diálogo de lenguaje
    Actions m_actions;
};

#endif // FORMATCONTROLLER_H
