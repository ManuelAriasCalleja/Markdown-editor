#ifndef SPELLCONTROLLER_H
#define SPELLCONTROLLER_H

#include <QObject>
#include <QString>

#include "spellchecker.h"

class CodeBlockHighlighter;
class DocumentIo;
class QContextMenuEvent;
class QTextEdit;

// Integración del corrector ortográfico en el editor WYSIWYG. Posee el motor
// (SpellChecker), el interruptor activado/desactivado y la lógica de qué idioma
// cargar y del menú contextual de sugerencias. Se lo extrajo de MainWindow para
// no inflar el «God object»; el resaltado lo hace el CodeBlockHighlighter, al que
// este controlador le pasa el motor.
//
// El idioma se deduce del documento (front matter `lang` › ajuste › locale) salvo
// que haya un override manual (Ver → Idioma de corrección). Si el corrector está
// activo pero falta el diccionario, emite `statusMessage` (degrada en silencio si
// no, y el usuario no sabría por qué no subraya).
class SpellController : public QObject
{
    Q_OBJECT

public:
    SpellController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                    DocumentIo *documentIo, QObject *parent = nullptr);

    bool isEnabled() const { return m_enabled; }
    // Activa/desactiva el corrector (persiste el ajuste y recarga/limpia).
    void setEnabled(bool on);
    // Fija el idioma de corrección (basename de diccionario); vacío = automático.
    void setLanguageOverride(const QString &code);
    // (Re)carga el diccionario del idioma actual y rehace el resaltado. Llamar al
    // arrancar y en cada documentLoaded (el front matter puede cambiar el idioma).
    void applyLanguage();
    // Menú contextual del editor: sobre una errata, antepone sugerencias +
    // «añadir al diccionario»/«ignorar». Devuelve true (siempre lo atiende).
    bool showContextMenu(QContextMenuEvent *event);

    // Nombre legible de un diccionario (basename como "en_US") para los menús.
    static QString languageLabel(const QString &code);

signals:
    void statusMessage(const QString &text, int timeoutMs);

private:
    QTextEdit *m_editor = nullptr;
    CodeBlockHighlighter *m_highlighter = nullptr;
    DocumentIo *m_documentIo = nullptr;
    SpellChecker m_checker;       // motor; lo consume el highlighter
    bool m_enabled = true;        // interruptor del corrector (Ver → ...)
};

#endif // SPELLCONTROLLER_H
