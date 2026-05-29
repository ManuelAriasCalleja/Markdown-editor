cmake_minimum_required(VERSION 3.16)

project(md-editor
    VERSION 1.0.2
    DESCRIPTION "Editor/visor de archivos Markdown en Qt6"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Genera compile_commands.json para que clangd / el IDE resuelvan las
# cabeceras de Qt y ofrezcan autocompletado y diagnósticos correctos.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt necesita estos pasos automáticos para generar el código de meta-objetos,
# los recursos y los formularios.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport LinguistTools)

# Toda la lógica vive en una biblioteca estática para poder enlazarla tanto
# desde el ejecutable como desde las pruebas. El ejecutable es solo main().
add_library(md-editor-core STATIC
    src/mainwindow.cpp
    src/mainwindow.h
    src/splitviewcontroller.cpp
    src/splitviewcontroller.h
    src/diskwatcher.cpp
    src/diskwatcher.h
    src/exportcontroller.cpp
    src/exportcontroller.h
    src/filecontroller.cpp
    src/filecontroller.h
    src/tablecontroller.cpp
    src/tablecontroller.h
    src/formatcontroller.cpp
    src/formatcontroller.h
    src/formulacontroller.cpp
    src/formulacontroller.h
    src/insertcontroller.cpp
    src/insertcontroller.h
    src/distractionfreecontroller.cpp
    src/distractionfreecontroller.h
    src/chromezoom.cpp
    src/chromezoom.h
    src/codehighlighter.cpp
    src/codehighlighter.h
    src/findreplacebar.cpp
    src/findreplacebar.h
    src/focuseditor.cpp
    src/focuseditor.h
    src/helpdialog.cpp
    src/helpdialog.h
    src/outlinepanel.cpp
    src/outlinepanel.h
    src/recentfilesmanager.cpp
    src/recentfilesmanager.h
    src/recoverymanager.cpp
    src/recoverymanager.h
    src/languageregistry.cpp
    src/languageregistry.h
    src/blockconstructs.cpp
    src/blockconstructs.h
    src/listcontinuation.cpp
    src/listcontinuation.h
    src/mathblocks.cpp
    src/mathblocks.h
    src/tableedit.cpp
    src/tableedit.h
    src/exporters.cpp
    src/exporters.h
    src/appsettings.cpp
    src/appsettings.h
    src/documentio.cpp
    src/documentio.h
    src/themecontroller.cpp
    src/themecontroller.h
    src/themespec.cpp
    src/themespec.h
)
# Qt6::GuiPrivate da acceso al QZip privado (QZipReader/QZipWriter), con el que
# exporters.cpp reempaqueta el ODF para incrustarle el idioma sin dependencias
# externas. Es API privada de Qt (puede cambiar entre versiones), pero no añade
# ninguna biblioteca de terceros.
target_link_libraries(md-editor-core PUBLIC Qt6::Widgets Qt6::PrintSupport Qt6::GuiPrivate)
target_include_directories(md-editor-core PUBLIC src)

# Icono de la aplicación a nivel de SO (el de ventana ya se fija en runtime con
# app.setWindowIcon). En Windows se incrusta vía recurso .rc; en macOS se copia
# el .icns al bundle. En Linux lo provee el .desktop (Icon=md-editor).
set(app_icon_sources "")
if(WIN32)
    enable_language(RC)
    list(APPEND app_icon_sources src/icons/md-editor.rc)
elseif(APPLE)
    list(APPEND app_icon_sources src/icons/md-editor.icns)
endif()

# WIN32: en Windows usa el subsistema GUI (sin consola junto a la ventana).
# MACOSX_BUNDLE: en macOS genera un .app. Cada flag lo ignora la plataforma que
# no corresponde, así que la build de Linux es idéntica a antes.
add_executable(md-editor WIN32 MACOSX_BUNDLE
    src/main.cpp
    src/resources.qrc
    ${app_icon_sources})
target_link_libraries(md-editor PRIVATE md-editor-core)

# macOS: incluye el .icns dentro del bundle (carpeta Resources).
if(APPLE)
    set_source_files_properties(src/icons/md-editor.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION Resources)
endif()

# --- Internacionalización (Qt Linguist) ---
# Compila los .ts a .qm (con lrelease) durante la build y los empotra en el
# ejecutable bajo el prefijo de recurso /i18n. Crea además los objetivos
# 'update_translations' (lupdate: refresca los .ts desde el código) y
# 'release_translations' (lrelease). El código a escanear vive en la biblioteca.
#
# El español es el idioma de origen: su .ts solo aporta las formas de plural
# (que el texto fuente no puede expresar). Se pasa como PLURALS_TS_FILE en vez
# de en TS_FILES, de modo que 'update_translations' lo actualiza pero lconvert
# lo filtra a -pluralonly: se mantiene parcial por sí solo, sin inflarse con
# todas las cadenas. Aun así se compila y empotra su .qm como los demás.
qt_add_translations(md-editor
    SOURCE_TARGETS md-editor-core
    PLURALS_TS_FILE translations/md-editor_es.ts
    TS_FILES
        translations/md-editor_en.ts
        translations/md-editor_de.ts
        translations/md-editor_fr.ts
        translations/md-editor_it.ts
        translations/md-editor_pt.ts
        translations/md-editor_pl.ts
        translations/md-editor_nl.ts
        translations/md-editor_ro.ts
    RESOURCE_PREFIX "/i18n"
)

# --- Empaquetado e instalación multiplataforma ---
# El código es Qt6 puro y portable; aquí solo se añaden las reglas de instalación
# y, en Windows/macOS, el despliegue de las bibliotecas de Qt. En Linux no cambia
# nada del flujo de build habitual.
include(GNUInstallDirs)

# macOS: metadatos del paquete .app (nombre, identificador y versión).
if(APPLE)
    set_target_properties(md-editor PROPERTIES
        MACOSX_BUNDLE_BUNDLE_NAME "md-editor"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.mdeditor.app"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_ICON_FILE "md-editor.icns")
endif()

# Instalación: el .app en macOS, el ejecutable en bin/ en Linux y Windows, y el
# lanzador .desktop solo en Linux.
install(TARGETS md-editor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(UNIX AND NOT APPLE)
    install(FILES md-editor.desktop
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
    # Iconos hicolor PNG (varios tamaños) + SVG escalable. Necesarios para que
    # el lanzador del escritorio y los empaquetadores tipo AppImage encuentren
    # un icono asociado a la aplicación.
    foreach(size 16 24 32 48 64 128 256)
        install(FILES src/icons/md-editor-${size}.png
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${size}x${size}/apps
            RENAME md-editor.png)
    endforeach()
    install(FILES src/icons/md-editor.svg
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
endif()

# En Windows/macOS, empaqueta las DLL/frameworks de Qt junto a la app con
# windeployqt/macdeployqt al instalar. En Linux se usa el Qt del sistema. Se
# omite con elegancia si la versión de Qt no trae el generador (necesita 6.3+).
if((WIN32 OR APPLE) AND COMMAND qt_generate_deploy_app_script)
    qt_generate_deploy_app_script(
        TARGET md-editor
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script})
endif()

# --- Pruebas (Qt Test) ---
# Se compilan solo si está disponible el componente Test.
find_package(Qt6 QUIET COMPONENTS Test)
if(Qt6Test_FOUND)
    enable_testing()
    foreach(test
        tst_languageregistry
        tst_blockconstructs
        tst_listcontinuation
        tst_mathblocks
        tst_tableedit
        tst_exporters
        tst_markdownroundtrip
        tst_outline
        tst_recovery
        tst_appsettings
        tst_documentio
        tst_themecontroller
        tst_themespec
        tst_links
        tst_splitview
        tst_chromezoom
        tst_filecontroller
        tst_formulacontroller)
        add_executable(${test} tests/${test}.cpp)
        target_link_libraries(${test} PRIVATE md-editor-core Qt6::Test)
        add_test(NAME ${test} COMMAND ${test})
        # Las pruebas usan QApplication: ejecútalas sin pantalla (headless).
        set_tests_properties(${test} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endforeach()

    # Guard de i18n: las traducciones deben estar completas y al día con el
    # código. Usa el lupdate de Qt6 (no el del PATH, que suele ser de Qt5).
    # Es un script de shell, así que solo se registra donde hay bash (no Windows).
    if(NOT WIN32)
        add_test(NAME tst_translations
                 COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/tests/check-translations.sh
                         $<TARGET_FILE:Qt6::lupdate>
                         ${CMAKE_CURRENT_SOURCE_DIR}/src
                         ${CMAKE_CURRENT_SOURCE_DIR}/translations)
    endif()
endif()
