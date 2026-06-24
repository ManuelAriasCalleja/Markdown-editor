/// \file
/// \brief Implementación del controlador del corrector: idioma, resaltado y menú de sugerencias.

#include "spellcontroller.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QFont>
#include <QLocale>
#include <QMenu>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>

#include <memory>

#include "appsettings.h"
#include "codehighlighter.h"
#include "documentio.h"
#include "exporters.h"
#include "mathblocks.h"
#include "spellscan.h"

SpellController::SpellController(QTextEdit *editor, CodeBlockHighlighter *highlighter,
                                DocumentIo *documentIo, QObject *parent)
    : QObject(parent)
    , m_editor(editor)
    , m_highlighter(highlighter)
    , m_documentIo(documentIo)
{
    m_enabled = AppSettings::spellCheck();
    m_highlighter->setSpellChecker(&m_checker);  // el resaltado consume el motor
}

QString SpellController::languageLabel(const QString &code)
{
    const QLocale loc(code);
    QString name = loc.nativeLanguageName();
    if (name.isEmpty())
        return code;
    name[0] = name.at(0).toUpper();
    if (code.contains(QLatin1Char('_')) && !loc.nativeTerritoryName().isEmpty())
        return QStringLiteral("%1 (%2)").arg(name, loc.nativeTerritoryName());
    return name;
}

void SpellController::setEnabled(bool on)
{
    m_enabled = on;
    AppSettings::setSpellCheck(on);
    applyLanguage();  // carga/descarga el diccionario y rehace el resaltado
}

void SpellController::setLanguageOverride(const QString &code)
{
    AppSettings::setSpellLanguage(code);
    applyLanguage();
}

void SpellController::applyLanguage()
{
    // Desactivado: descarga el diccionario (sin subrayado y sin huella de
    // memoria) y rehace el resaltado para limpiar las erratas marcadas.
    if (!m_enabled) {
        m_checker.setLanguage(QString());
        m_highlighter->rehighlight();
        return;
    }
    // Idioma: override manual (Ver → Idioma de corrección) si lo hay; si no,
    // automático, igual que la exportación: front matter › ajuste › locale.
    QString code = AppSettings::spellLanguage();
    if (code.isEmpty()) {
        const QString fm = m_documentIo->frontMatter();
        code = mdexport::frontMatterValue(fm, QStringLiteral("lang"));
        if (code.isEmpty())
            code = mdexport::frontMatterValue(fm, QStringLiteral("language"));
        if (code.isEmpty())
            code = AppSettings::language();
        if (code.isEmpty())
            code = QLocale::system().name();  // p. ej. "es_ES"
    }

    m_checker.setPersonalWords(AppSettings::personalDictionary());
    m_checker.setLanguage(code);
    m_highlighter->rehighlight();  // re-subraya con el diccionario nuevo

    // Aviso visible SOLO si el problema está presente: el corrector está
    // activado pero no se cargó diccionario para el idioma pedido (degrada en
    // silencio, así que sin esto el usuario no sabría por qué no subraya).
    if (!m_checker.isAvailable()) {
        emit statusMessage(
            QCoreApplication::translate("MainWindow",
                "Sin diccionario de corrección para «%1»: instálalo (Hunspell) o "
                "desactiva el corrector en «Ver».").arg(languageLabel(code)),
            8000);
    }
}

bool SpellController::showContextMenu(QContextMenuEvent *event)
{
    std::unique_ptr<QMenu> menu(m_editor->createStandardContextMenu());

    // Palabra bajo el clic (misma tokenización que el subrayado).
    const QTextCursor cursor = m_editor->cursorForPosition(event->pos());
    const QTextBlock block = cursor.block();
    const QString blockText = block.text();
    const int posInBlock = cursor.position() - block.position();
    int wordStart = -1;
    int wordLen = 0;
    for (const mdspell::Word &w : mdspell::tokenize(blockText)) {
        if (posInBlock >= w.start && posInBlock <= w.start + w.length) {
            wordStart = w.start;
            wordLen = w.length;
            break;
        }
    }

    if (wordStart >= 0 && m_checker.isAvailable()) {
        const QString word = blockText.mid(wordStart, wordLen);
        const int absStart = block.position() + wordStart;
        // Saltar código en línea / fórmula / enlace, igual que el subrayado.
        QTextCursor fc(m_editor->document());
        fc.setPosition(absStart + 1);
        const QTextCharFormat cf = fc.charFormat();
        const bool skip = cf.fontFixedPitch()
                          || cf.boolProperty(mdmath::IsMathProperty) || cf.isAnchor();
        if (!skip && !m_checker.isCorrect(word)) {
            // Insertamos todo ANTES de la primera acción estándar (cortar/copiar…).
            QAction *before = menu->actions().value(0);
            const QStringList suggestions = m_checker.suggestions(word);
            if (suggestions.isEmpty()) {
                QAction *none = new QAction(QCoreApplication::translate("MainWindow", "(sin sugerencias)"), menu.get());
                none->setEnabled(false);
                menu->insertAction(before, none);
            } else {
                for (const QString &s : suggestions.mid(0, 8)) {  // hasta 8 sugerencias
                    QAction *act = new QAction(s, menu.get());
                    QFont f = act->font();
                    f.setBold(true);
                    act->setFont(f);
                    connect(act, &QAction::triggered, this, [this, absStart, wordLen, s] {
                        QTextCursor c(m_editor->document());
                        c.setPosition(absStart);
                        c.setPosition(absStart + wordLen, QTextCursor::KeepAnchor);
                        c.insertText(s);
                    });
                    menu->insertAction(before, act);
                }
            }
            QAction *addAct =
                new QAction(QCoreApplication::translate("MainWindow", "Añadir «%1» al diccionario").arg(word), menu.get());
            connect(addAct, &QAction::triggered, this, [this, word] {
                m_checker.addToPersonal(word);
                AppSettings::setPersonalDictionary(m_checker.personalWords());
                m_highlighter->rehighlight();
            });
            QAction *ignoreAct = new QAction(QCoreApplication::translate("MainWindow", "Ignorar «%1»").arg(word), menu.get());
            connect(ignoreAct, &QAction::triggered, this, [this, word] {
                m_checker.ignoreWord(word);
                m_highlighter->rehighlight();
            });
            menu->insertAction(before, addAct);
            menu->insertAction(before, ignoreAct);
            menu->insertSeparator(before);
        }
    }

    menu->exec(event->globalPos());
    return true;
}
