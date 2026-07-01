#ifndef CODEBLOCKOVERLAY_H
#define CODEBLOCKOVERLAY_H

/// \file
/// \brief Overlay flotante sobre un bloque de código: etiqueta de lenguaje (clic para
///        cambiarlo) y botón de copiar. Aparece al pasar el ratón por el bloque.

#include <QWidget>

class QToolButton;

/// Widget pequeño que se superpone en la esquina superior derecha de un bloque de
/// código. Muestra el lenguaje (un botón plano: al pulsarlo se pide cambiarlo) y un
/// botón para copiar el código. No toca el documento: es solo presentación.
class CodeBlockOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit CodeBlockOverlay(QWidget *parent = nullptr);

    /// Fija el texto del lenguaje (vacío → «texto»).
    void setLanguage(const QString &language);

signals:
    void copyRequested();      ///< se pulsó el botón de copiar
    void languageRequested();  ///< se pulsó la etiqueta de lenguaje

private:
    QToolButton *m_langButton = nullptr;
    QToolButton *m_copyButton = nullptr;
};

#endif  // CODEBLOCKOVERLAY_H
