/// \file
/// \brief Implementación del controlador del corrector: idioma, resaltado y menú de sugerencias.

#include "spellcontroller.h"

#include <QAction>
#include <QCheckBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDir>
#include <QFont>
#include <QLocale>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSysInfo>
#include <QTimer>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextEdit>

#include <memory>

#include "appsettings.h"
#include "dictionaryinstaller.h"
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
        // Y, además del mensaje fugaz de la barra de estado, un aviso que se pueda
        // leer y que diga QUÉ hacer. Solo una vez por idioma y sesión: al abrir
        // varios documentos en el mismo idioma sería insufrible.
        warnMissingDictionary(code);
    }
}

void SpellController::warnMissingDictionary(const QString &code)
{
    if (!AppSettings::spellMissingWarning() || m_warnedLanguages.contains(code))
        return;
    // Arranques automatizados (pruebas, paso de humo del empaquetado): el aviso es
    // una ventana que ahí no hay quien cierre, y además roba los atajos de la
    // principal mientras esté abierta. Se calla; el mensaje de la barra de estado
    // sigue saliendo.
    if (qEnvironmentVariableIsSet("MD_EDITOR_NO_POPUPS"))
        return;
    m_warnedLanguages.insert(code);

    // Diferido: esto se dispara al cargar un documento, y un diálogo modal en
    // mitad del trazado inicial del editor sale mal colocado (misma razón por la
    // que main.cpp difiere startSession).
    QTimer::singleShot(0, this, [this, code] { showMissingDictionaryDialog(code); });
}

void SpellController::showMissingDictionaryDialog(const QString &code)
{
    // NO se usa exec(): este aviso lo dispara la carga de un documento, y un
    // `exec()` ahí bloquea el hilo dentro de la ruta de apertura —en las pruebas
    // automáticas eso es un cuelgue (dos tests se quedaron colgados en el CI, sin
    // nadie que pulsara nada) y en la aplicación real congela la apertura de la
    // sesión—. Con `open()` el diálogo es modal para su ventana pero asíncrono: se
    // muestra y el programa sigue; la respuesta se atiende en `finished`. Y NO
    // modal para su ventana, sino suelto: es un aviso, no una pregunta que haya
    // que contestar para seguir editando, y un modal de ventana secuestra el
    // teclado de la principal (con él, un atajo tan normal como F6 dejaba de
    // llegar al editor).
    const QString label = languageLabel(code);
    auto *box = new QMessageBox(m_editor ? m_editor->window() : nullptr);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowModality(Qt::NonModal);
    // Y sin robar el foco: aparece mientras el usuario ya está escribiendo, así que
    // no debe quedarse con el teclado (además de ser molesto, dejaba sin efecto los
    // atajos de la ventana principal mientras estuviera abierto).
    box->setAttribute(Qt::WA_ShowWithoutActivating);
    box->setIcon(QMessageBox::Information);
    box->setWindowTitle(QCoreApplication::translate("MainWindow", "Corrección ortográfica"));

    if (!SpellChecker::isEngineAvailable()) {
        // Ni motor: decirle que instale un diccionario sería mandarlo a un callejón
        // sin salida, porque no habría nada que lo leyera.
        box->setText(QCoreApplication::translate("MainWindow",
            "Esta versión del programa se compiló sin corrector ortográfico."));
        box->setInformativeText(QCoreApplication::translate("MainWindow",
            "No se subrayarán las faltas. Si la has compilado tú, instala Hunspell "
            "(libhunspell-dev, brew install hunspell o vcpkg) y vuelve a compilar."));
        box->setStandardButtons(QMessageBox::Ok);
        box->show();
        return;
    }

    box->setText(QCoreApplication::translate("MainWindow",
        "No hay diccionario de %1, así que la corrección está desactivada en este "
        "documento.").arg(label));

    // Instrucciones para instalarlo a mano: en Linux, el paquete de la
    // distribución; en Windows y macOS no hay repositorio, así que se indica la
    // carpeta (que además es donde deja el diccionario el botón de descargar).
    const QString command =
        mdspell::dictionaryInstallCommand(code, QSysInfo::productType());
    QString how;
    if (!command.isEmpty()) {
        how = QCoreApplication::translate("MainWindow",
                  "Puedes instalarlo con tu gestor de paquetes:\n\n    %1").arg(command);
    } else {
        how = QCoreApplication::translate("MainWindow",
                  "Puedes instalarlo copiando los archivos .aff y .dic del idioma en "
                  "esta carpeta:\n\n    %1").arg(QDir::toNativeSeparators(
                      SpellChecker::userDictionaryDir()));
    }
    box->setInformativeText(how);

    // Descarga directa: no pide contraseña, no depende del gestor de paquetes y
    // vale en las tres plataformas. Solo para los idiomas de los que se conoce el
    // origen (los nueve de la interfaz).
    QPushButton *install = nullptr;
    if (DictionaryInstaller::canInstall(code)) {
        install = box->addButton(QCoreApplication::translate("MainWindow",
                      "Descargar e instalar"), QMessageBox::AcceptRole);
    }
    box->addButton(QMessageBox::Close);
    auto *dontAsk = new QCheckBox(
        QCoreApplication::translate("MainWindow", "No volver a avisar"), box);
    box->setCheckBox(dontAsk);

    connect(box, &QMessageBox::finished, this, [this, box, install, dontAsk, code] {
        if (dontAsk->isChecked())
            AppSettings::setSpellMissingWarning(false);
        if (install && box->clickedButton() == install)
            downloadDictionary(code);
    });
    box->show();
}

void SpellController::downloadDictionary(const QString &code)
{
    if (!m_installer) {
        m_installer = new DictionaryInstaller(this);
        connect(m_installer, &DictionaryInstaller::finished, this,
                [this](bool ok, const QString &error) {
                    if (ok) {
                        emit statusMessage(QCoreApplication::translate("MainWindow",
                            "Diccionario instalado."), 5000);
                        applyLanguage();  // recargar: ahora sí está
                    } else {
                        emit statusMessage(QCoreApplication::translate("MainWindow",
                            "No se pudo descargar el diccionario: %1").arg(error), 8000);
                    }
                });
    }
    emit statusMessage(QCoreApplication::translate("MainWindow",
        "Descargando el diccionario…"), 0);
    m_installer->install(code);
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
