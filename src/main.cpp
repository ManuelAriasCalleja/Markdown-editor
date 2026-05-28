#include "mainwindow.h"

#include "appsettings.h"

#include <QApplication>
#include <QIcon>
#include <QLibraryInfo>
#include <QLocale>
#include <QTimer>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
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
    const QString langPref = AppSettings::language();
    QLocale locale = langPref.isEmpty() ? QLocale::system() : QLocale(langPref);

    QTranslator appTranslator;
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
    QTranslator qtTranslator;
    if (qtTranslator.load(locale, QStringLiteral("qtbase"), QStringLiteral("_"),
                          QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTranslator);
    // -------------------------------------------------------------------------

    // Icono de la aplicación (varias resoluciones). En X11 alimenta
    // _NET_WM_ICON, que es lo que muestra el conmutador alt+tab y la barra de
    // tareas; los gestores eligen la resolución que mejor les venga.
    QIcon icon;
    for (const int size : {16, 24, 32, 48, 64, 128, 256})
        icon.addFile(QStringLiteral(":/icons/md-editor-%1.png").arg(size));
    app.setWindowIcon(icon);

    MainWindow window;
    window.show();

    // Qué mostrar al arrancar (archivo de la línea de comandos > recuperación de
    // borrador > último documento) lo decide MainWindow::startSession. Se difiere
    // al bucle de eventos: al arrancar, QTextEdit hace un ajuste de trazado
    // interno y abrir en mitad de él provocaría un diálogo espurio.
    const QString cmdLineFile = (argc > 1) ? QString::fromLocal8Bit(argv[1]) : QString();
    QTimer::singleShot(0, &window, [&window, cmdLineFile] { window.startSession(cmdLineFile); });

    return app.exec();
}
