/// \file
/// \brief Punto de entrada de la aplicación: arranque de QApplication, i18n y ciclo de vida de la ventana.

#include "mainwindow.h"

#include "appsettings.h"
#include "langtag.h"
#include "singleinstance.h"

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

    // --- Instancias coordinadas ----------------------------------------------
    // Si ya hay un editor en marcha (p. ej. al abrir otro .md desde el explorador),
    // las rutas se entregan a la ÚLTIMA instancia creada, que las abre en pestañas, y
    // este proceso sale sin crear ninguna ventana. `--new-window` (lo usa «Abrir en una
    // nueva ventana») salta esa entrega: la instancia nueva se anota la última.
    const SingleInstance::LaunchArgs launch =
        SingleInstance::parseArguments(app.arguments().mid(1));
    if (!launch.newWindow
        && SingleInstance::sendToLatest({SingleInstance::kOpen, launch.paths}))
        return 0;
    SingleInstance singleInstance;
    // Si falla, simplemente no recibirá archivos de otras ejecuciones.
    singleInstance.listen(SingleInstance::instanceName(QCoreApplication::applicationPid()));
    // -------------------------------------------------------------------------

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
        if (!loaded) {
            // Qt busca el .qm recorriendo las uiLanguages del locale, y para algunas no
            // llega nunca al idioma a secas: `zh_SG` prueba «zh-Hans-SG» y «zh-SG», y se
            // queda sin traducción aunque el .qm del chino simplificado esté ahí (en
            // Singapur se escribe en simplificado). La etiqueta canónica es justo la que
            // sabe eso, así que se reintenta con ella por nombre de archivo, sin volver a
            // pasar por el algoritmo de uiLanguages. No se toca `locale`: de él dependen
            // el formato de números y las traducciones de Qt, y ahí sí manda la región.
            const QString tag = mdlang::canonicalTag(locale.name());
            if (!tag.isEmpty())
                loaded = appTranslator.load(QStringLiteral("md-editor_") + tag,
                                            QStringLiteral(":/i18n"));
        }
        if (!loaded && langPref.isEmpty() && locale.language() != QLocale::Spanish) {
            // Automático con idioma del sistema no soportado: inglés como respaldo.
            locale = QLocale(QLocale::English);
            loaded = appTranslator.load(locale, QStringLiteral("md-editor"),
                                        QStringLiteral("_"), QStringLiteral(":/i18n"));
        }
        if (loaded)
            app.installTranslator(&appTranslator);

        // El formato de números (separador de miles, decimales) sigue al idioma de
        // la interfaz, no al del sistema: así el contador de la barra de estado y las
        // estadísticas agrupan las cifras (12.345) según el idioma elegido. QLocale()
        // por defecto era el del sistema; lo fijamos al `locale` ya resuelto.
        QLocale::setDefault(locale);

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

    // Peticiones de otras ejecuciones. Hasta que la sesión de la ventana actual
    // termina de arrancar (o mientras se recrea por un cambio de idioma) se
    // encolan: abrir antes pisaría la lógica de arranque, o iría a una ventana
    // que está a punto de destruirse.
    bool sessionReady = false;
    QStringList pendingPaths;
    bool pendingRaise = false;
    auto flushPending = [&] {
        if (!sessionReady || !window)
            return;
        if (pendingPaths.isEmpty() && !pendingRaise)
            return;
        const QStringList paths = pendingPaths;
        pendingPaths.clear();
        pendingRaise = false;
        window->openExternalPaths(paths);
    };
    QObject::connect(&singleInstance, &SingleInstance::messageReceived, &app,
                     [&](const SingleInstance::Message &message) {
        if (message.command == SingleInstance::kOpen) {
            pendingPaths << message.args;
            pendingRaise = true;
            flushPending();
        } else if (message.command == SingleInstance::kAdopted && window
                   && !message.args.isEmpty()) {
            window->closeTabForPath(message.args.first());
        }
    });

    // Si esta instancia nació de «Abrir en una nueva ventana»: deja el cursor donde
    // estaba y, solo con el documento ya cargado, avisa a la instancia de origen para
    // que cierre su pestaña. Si la carga falla, la pestaña original se conserva.
    auto finishHandoff = [&](MainWindow *w) {
        if (launch.paths.isEmpty() || !w->hasOpenFile(launch.paths.first()))
            return;
        if (launch.cursor >= 0)
            w->setCursorPosition(launch.cursor);
        if (!launch.handoffFrom.isEmpty())
            SingleInstance::sendTo(launch.handoffFrom,
                                   {SingleInstance::kAdopted, {launch.paths.first()}});
    };

    spawn = [&](const QString &openPath, bool relaunch) {
        sessionReady = false;
        window = std::make_unique<MainWindow>();
        MainWindow *w = window.get();
        w->setInstanceName(singleInstance.name());

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
        QTimer::singleShot(0, w, [&, w, openPath, relaunch] {
            if (relaunch)
                w->relaunchSession(openPath);
            else
                w->startSession(openPath);
            sessionReady = true;
            if (!relaunch)
                finishHandoff(w);
            flushPending();
        });
    };

    // El primer archivo lo abre startSession (con su prioridad sobre la recuperación
    // de borrador); los demás, como cualquier petición posterior, en su pestaña.
    const QString cmdLineFile = launch.paths.value(0);
    pendingPaths = launch.paths.mid(1);
    spawn(cmdLineFile, false);

    return app.exec();
}
