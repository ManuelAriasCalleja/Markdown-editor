#ifndef SPELLCONTROLLER_H
#define SPELLCONTROLLER_H

/// \file
/// \brief Integración del corrector ortográfico en el editor WYSIWYG (idioma y menú contextual).

#include <QObject>
#include <QSet>
#include <QString>

#include "spellchecker.h"

class CodeBlockHighlighter;
class DictionaryInstaller;
class DocumentIo;
class QContextMenuEvent;
class QTextEdit;

/// Integración del corrector ortográfico en el editor WYSIWYG. Posee el motor
/// (SpellChecker), el interruptor activado/desactivado y la lógica de qué idioma
/// cargar y del menú contextual de sugerencias. Se lo extrajo de MainWindow para
/// no inflar el «God object»; el resaltado lo hace el CodeBlockHighlighter, al que
/// este controlador le pasa el motor.
///
/// El idioma se deduce del documento (front matter `lang` › ajuste › locale) salvo
/// que haya un override manual (Ver → Idioma de corrección). Si el corrector está
/// activo pero falta el diccionario, emite `statusMessage` (degrada en silencio si
/// no, y el usuario no sabría por qué no subraya).
class SpellController : public QObject
{
    Q_OBJECT

public:
    SpellController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                    DocumentIo *documentIo, QObject *parent = nullptr);

    /// \brief Activa/desactiva el corrector (persiste el ajuste y recarga/limpia).
    void setEnabled(bool on);
    /// \brief Fija el idioma de corrección (basename de diccionario); vacío = automático.
    void setLanguageOverride(const QString &code);
    /// (Re)carga el diccionario del idioma actual y rehace el resaltado. Llamar al
    /// arrancar y en cada documentLoaded (el front matter puede cambiar el idioma).
    void applyLanguage();
    /// Menú contextual del editor: sobre una errata, antepone sugerencias +
    /// «añadir al diccionario»/«ignorar». Devuelve true (siempre lo atiende).
    bool showContextMenu(QContextMenuEvent *event);

    /// \brief Nombre legible de un diccionario (basename como "en_US") para los menús.
    static QString languageLabel(const QString &code);

signals:
    /// \brief Solicita mostrar un mensaje en la barra de estado durante `timeoutMs` ms.
    void statusMessage(const QString &text, int timeoutMs);

private:
    /// Avisa (una vez por idioma y sesión) de que falta el diccionario, con las
    /// instrucciones de instalación de la plataforma. No hace nada si el usuario
    /// pidió no volver a avisar.
    void warnMissingDictionary(const QString &code);
    /// El diálogo en sí (se llama diferido: llega en mitad de la carga del
    /// documento y un modal ahí sale mal colocado).
    void showMissingDictionaryDialog(const QString &code);
    /// Descarga el diccionario a la carpeta del usuario y recarga el idioma.
    void downloadDictionary(const QString &code);

    QTextEdit *m_editor = nullptr;
    CodeBlockHighlighter *m_highlighter = nullptr;
    DocumentIo *m_documentIo = nullptr;
    SpellChecker m_checker;       // motor; lo consume el highlighter
    bool m_enabled = true;        // interruptor del corrector (Ver → ...)
    QSet<QString> m_warnedLanguages;  // idiomas ya avisados en esta sesión
    DictionaryInstaller *m_installer = nullptr;  // perezoso: solo si se descarga
};

#endif // SPELLCONTROLLER_H
