/// \file
/// \brief Punto de entrada de la aplicación: arranque de QApplication, i18n y ciclo de vida de la ventana.

#include "mainwindow.h"

#include "appsettings.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QStyleFactory>
#include <QTimer>
#include <QTranslator>

#include <functional>
#include <memory>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    // Estilo Fusion en todas las plataformas: respeta por completo la paleta de
    // la app, incluidos los menús. Con el estilo nativo (sobre todo en Windows),
    // los menús los pinta el sistema operativo y el tema claro/oscuro de la app
    // no les afecta, dejándolos ilegibles cuando difieren del tema del SO.
    if (QStyle *fusion = QStyleFactory::create(QStringLiteral("Fusion")))
        app.setStyle(fusion);
    app.setApplicationName(QStringLiteral("md-editor"));
    app.setApplicationDisplayName(QStringLiteral("md-editor"));
    app.setOrganizationName(QStringLiteral("md-editor"));  // para QSettings (tema)
    // Asocia la ventana con su .desktop (Wayland usa este nombre para el icono).
    app.setDesktopFileName(QStringLiteral("md-editor"));

    // --- Internacionalización ------------------------------------------------
    // El idioma se toma del ajuste guardado (menú Ver → Idioma); vacío = el del
    // sistema. Las traducciones (.qm) van empotradas en :/i18n.
    //
    // El idioma de origen es el español. Respaldo:
    //   • Selección manual de un idioma sin .qm (p. ej. «Español») → textos fuente.
    //   • Modo automático y el idioma del sistema no tiene traducción → inglés,
    //     salvo que el sistema esté en español (entonces, textos fuente).
    //
    // Los traductores viven en la pila de main() durante toda la vida de la app
    // para poder intercambiarlos en caliente: cambiar de idioma los retira y
    // vuelve a cargar (applyTranslators) y recrea la ventana (ver más abajo).
    QTranslator appTranslator;
    QTranslator qtTranslator;
    auto applyTranslators = [&] {
        app.removeTranslator(&appTranslator);
        app.removeTranslator(&qtTranslator);

        const QString langPref = AppSettings::language();
        QLocale locale = langPref.isEmpty() ? QLocale::system() : QLocale(langPref);

        bool loaded = appTranslator.load(locale, QStringLiteral("md-editor"),
                                         QStringLiteral("_"), QStringLiteral(":/i18n"));
        if (!loaded && langPref.isEmpty() && locale.language() != QLocale::Spanish) {
            // Automático con idioma del sistema no soportado: inglés como respaldo.
            locale = QLocale(QLocale::English);
            loaded = appTranslator.load(locale, QStringLiteral("md-editor"),
                                        QStringLiteral("_"), QStringLiteral(":/i18n"));
        }
        if (loaded)
            app.installTranslator(&appTranslator);

        // Traducciones de Qt (diálogos estándar) para el idioma resuelto.
        if (qtTranslator.load(locale, QStringLiteral("qtbase"), QStringLiteral("_"),
                              QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
            app.installTranslator(&qtTranslator);
    };
    applyTranslators();
    // -------------------------------------------------------------------------

    // Icono de la aplicación (varias resoluciones). En X11 alimenta
    // _NET_WM_ICON, que es lo que muestra el conmutador alt+tab y la barra de
    // tareas; los gestores eligen la resolución que mejor les venga.
    QIcon icon;
    for (const int size : {16, 24, 32, 48, 64, 128, 256})
        icon.addFile(QStringLiteral(":/icons/md-editor-%1.png").arg(size));
    app.setWindowIcon(icon);

    // Ventana principal recreable. La construcción de la UI fija todos los textos
    // con tr() una sola vez, así que la forma robusta de cambiar el idioma sin un
    // retranslateUi() a mano es recrear la ventana: `spawn` la construye, conecta
    // languageChangeRequested (que reintercambia traductores y vuelve a llamarse,
    // reabriendo el documento) y difiere el arranque de sesión.
    std::unique_ptr<MainWindow> window;
    std::function<void(const QString &, bool)> spawn;

    spawn = [&](const QString &openPath, bool relaunch) {
        window = std::make_unique<MainWindow>();
        MainWindow *w = window.get();

        QObject::connect(w, &MainWindow::languageChangeRequested, &app,
                         [&](const QString &reopenPath) {
            // Diferido: no se puede destruir la ventana mientras se procesa una
            // señal suya. Para cuando corra, la pila ya se ha desenrollado.
            QTimer::singleShot(0, &app, [&, reopenPath] {
                applyTranslators();      // carga los .qm del idioma nuevo
                spawn(reopenPath, true); // recrea la ventana (rehace los tr())
            });
        });

        w->show();
        // El arranque se difiere al bucle de eventos: al construirse, QTextEdit
        // hace un ajuste de trazado interno y abrir en mitad de él provocaría un
        // diálogo espurio. En el primer arranque decide la sesión (línea de
        // comandos > borrador > último documento); en una recreación por idioma
        // solo reabre el documento que la ventana anterior tenía abierto.
        QTimer::singleShot(0, w, [w, openPath, relaunch] {
            if (relaunch)
                w->relaunchSession(openPath);
            else
                w->startSession(openPath);
        });
    };

    const QString cmdLineFile = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();
    spawn(cmdLineFile, false);

    return app.exec();
}
