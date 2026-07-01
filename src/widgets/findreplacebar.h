#ifndef FINDREPLACEBAR_H
#define FINDREPLACEBAR_H

/// \file
/// \brief Barra inferior de buscar/reemplazar sobre un QTextEdit.

#include <QToolBar>

#include "find.h"  // mdfind::Match (señal highlightMatches)

class QTextEdit;
class QLineEdit;
class QLabel;
class QCheckBox;
class QRegularExpression;
class QTextCursor;

/// \brief Barra inferior de buscar/reemplazar. Encapsula sus propios campos y opera
/// sobre el QTextEdit que se le pasa. No conoce la ventana: reporta resultados
/// por la señal statusMessage() para que quien la aloje los muestre donde quiera.
class FindReplaceBar : public QToolBar
{
    Q_OBJECT

public:
    explicit FindReplaceBar(QTextEdit *editor, QWidget *parent = nullptr);

    /// \brief Cambia el editor sobre el que actúa (p. ej. al pasar a la vista de fuente).
    void setEditor(QTextEdit *editor);

public slots:
    void showFind();      ///< muestra la barra y enfoca el campo de búsqueda
    void showReplace();   ///< como showFind() pero enfoca el campo de reemplazo

signals:
    /// \brief Reporta un mensaje de resultado para mostrar en la barra de estado.
    /// \param text texto del mensaje.
    /// \param timeout milisegundos que permanece visible.
    void statusMessage(const QString &text, int timeout);

    /// \brief Coincidencias actuales para resaltar en el editor (vacío = limpiar).
    /// Quien aloja la barra las inyecta en el editor activo.
    void highlightMatches(const QList<mdfind::Match> &matches);

private:
    friend class TestFind;  // caracterización del contador y de «Reemplazar todo»

    void buildUi();
    bool doFind(bool backward);
    void findNext();
    void findPrev();
    void replaceOne();
    void replaceAll();
    void closeBar();
    // Construye y valida la regex del campo de búsqueda (con la sensibilidad a
    // mayúsculas del checkbox). false y avisa por statusMessage si es inválida.
    bool buildRegex(QRegularExpression *re);
    // ¿La selección de `c` es una coincidencia del término actual (texto o regex)?
    bool selectionMatches(const QTextCursor &c) const;
    // Todas las coincidencias del término actual en el editor (vacío si no hay término).
    QList<mdfind::Match> currentMatches() const;
    // Recalcula las coincidencias, actualiza el contador «N de M» y emite
    // highlightMatches. Se llama al cambiar el término/opciones, buscar o reemplazar.
    void updateMatches();

    QTextEdit *m_editor;
    QLineEdit *m_findEdit = nullptr;
    QLineEdit *m_replaceEdit = nullptr;
    QLabel *m_countLabel = nullptr;
    QCheckBox *m_caseCheck = nullptr;
    QCheckBox *m_wholeWordCheck = nullptr;
    QCheckBox *m_regexCheck = nullptr;
};

#endif // FINDREPLACEBAR_H
