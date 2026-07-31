cmake_minimum_required(VERSION 3.16)

project(md-editor
    VERSION 2.8.3
    DESCRIPTION "Editor/visor de archivos Markdown en Qt6"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sanitizers (ASan + UBSan) para una build de diagnóstico: instrumenta TODO
# (biblioteca, ejecutable y tests) y aborta en el primer error de memoria o
# comportamiento indefinido. Se usa en CI corriendo la suite; en local:
#   cmake -S . -B build-san -DENABLE_SANITIZERS=ON && cmake --build build-san
#   ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-san --output-on-failure
# (detect_leaks=0 evita el ruido de fugas a la salida de Qt/fontconfig).
option(ENABLE_SANITIZERS "Compila con AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
if(ENABLE_SANITIZERS)
    # Se desactiva el sub-check `vptr` de UBSan: en una app Qt salta con falsos
    # positivos benignos al acceder a `this` desde slots que se emiten durante la
    # construcción/destrucción de un QObject (el vptr es aún/ya el de la base). Se
    # conservan ASan (memoria) y el resto de UBSan (enteros, OOB, alineación, null).
    add_compile_options(-fsanitize=address,undefined -fno-sanitize=vptr
                        -fno-omit-frame-pointer -fno-sanitize-recover=all)
    add_link_options(-fsanitize=address,undefined -fno-sanitize=vptr)
endif()

# Genera compile_commands.json para que clangd / el IDE resuelvan las
# cabeceras de Qt y ofrezcan autocompletado y diagnósticos correctos.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Qt necesita estos pasos automáticos para generar el código de meta-objetos,
# los recursos y los formularios.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Network: solo lo usa DictionaryInstaller, para descargar un diccionario cuando
# el usuario lo pide (ver src/spell/dictionaryinstaller.h). Es parte de Qt, no
# una dependencia de terceros nueva.
find_package(Qt6 REQUIRED COMPONENTS Widgets PrintSupport Network LinguistTools)

# Qt6::GuiPrivate (el QZip del export ODF, ver más abajo) hay que asegurarlo a
# mano porque las dos formas de obtenerlo son excluyentes según la versión:
#   • Hasta Qt 6.8 el target queda definido de rebote al encontrar Widgets/Gui,
#     y el componente `GuiPrivate` puede NO existir como paquete de
#     configuración (Debian no instala Qt6GuiPrivateConfig.cmake), así que
#     pedirlo aborta la configuración.
#   • Desde Qt 6.11 es al revés: el paquete existe pero el target ya no se
#     define solo, y la configuración muere con «Target "md-editor-core" links
#     to Qt6::GuiPrivate but the target was not found».
# Mirar si el target ya está cubre ambos sin condicionar por número de versión.
if(NOT TARGET Qt6::GuiPrivate)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
endif()

# Toda la lógica vive en una biblioteca estática para poder enlazarla tanto
# desde el ejecutable como desde las pruebas. El ejecutable es solo main().
add_library(md-editor-core STATIC
    # --- app ---
    src/app/appsettings.cpp
    src/app/appsettings.h
    src/app/closedtabs.cpp
    src/app/closedtabs.h
    src/app/chromezoom.cpp
    src/app/chromezoom.h
    src/app/commands.cpp
    src/app/commands.h
    src/app/mainwindow.cpp
    src/app/mainwindow.h
    src/app/mainwindowinput.cpp
    src/app/mainwindowmenus.cpp
    src/app/mainwindowsession.cpp
    src/app/mainwindowzoom.cpp
    src/app/tabmenu.cpp
    src/app/tabmenu.h
    # --- editor ---
    src/editor/autopair.cpp
    src/editor/autopair.h
    src/editor/blockconstructs.cpp
    src/editor/blockconstructs.h
    src/editor/editorstack.cpp
    src/editor/editorstack.h
    src/editor/focuseditor.cpp
    src/editor/focuseditor.h
    src/editor/imageobject.cpp
    src/editor/imageobject.h
    src/editor/formatcontroller.cpp
    src/editor/formatcontroller.h
    src/editor/insertcontroller.cpp
    src/editor/insertcontroller.h
    src/editor/linehighlight.cpp
    src/editor/linehighlight.h
    src/editor/listcontinuation.cpp
    src/editor/listcontinuation.h
    src/editor/tablecontroller.cpp
    src/editor/tablecontroller.h
    src/editor/typewriter.cpp
    src/editor/typewriter.h
    # --- view ---
    src/view/distractionfreecontroller.cpp
    src/view/distractionfreecontroller.h
    src/view/splitviewcontroller.cpp
    src/view/splitviewcontroller.h
    src/view/themecontroller.cpp
    src/view/themecontroller.h
    src/view/themespec.cpp
    src/view/themespec.h
    # --- io ---
    src/io/diskwatcher.cpp
    src/io/diskwatcher.h
    src/io/documentio.cpp
    src/io/documentio.h
    src/io/filecontroller.cpp
    src/io/filecontroller.h
    src/io/recentfilesmanager.cpp
    src/io/recentfilesmanager.h
    src/io/recoverymanager.cpp
    src/io/recoverymanager.h
    # --- markdown ---
    src/markdown/admonitions.cpp
    src/markdown/admonitions.h
    src/markdown/codespanfix.cpp
    src/markdown/codespanfix.h
    src/markdown/csvtable.cpp
    src/markdown/csvtable.h
    src/markdown/docstats.cpp
    src/markdown/docstats.h
    src/markdown/doctemplates.cpp
    src/markdown/doctemplates.h
    src/markdown/footnotes.cpp
    src/markdown/footnotes.h
    src/markdown/find.cpp
    src/markdown/find.h
    src/markdown/inputrules.cpp
    src/markdown/inputrules.h
    src/markdown/mark.cpp
    src/markdown/mark.h
    src/markdown/moveline.cpp
    src/markdown/moveline.h
    src/markdown/nav.cpp
    src/markdown/nav.h
    src/markdown/outline.cpp
    src/markdown/outline.h
    src/markdown/printdecor.cpp
    src/markdown/printdecor.h
    src/markdown/supsub.cpp
    src/markdown/supsub.h
    src/markdown/tablesort.cpp
    src/markdown/tablesort.h
    src/markdown/epubimport.cpp
    src/markdown/epubimport.h
    src/markdown/codeblock.cpp
    src/markdown/codeblock.h
    src/markdown/htmlimport.cpp
    src/markdown/htmlimport.h
    src/markdown/pandocimport.cpp
    src/markdown/pandocimport.h
    src/markdown/markdownrender.cpp
    src/markdown/markdownrender.h
    src/markdown/markdowntidy.cpp
    src/markdown/markdowntidy.h
    src/markdown/richpaste.cpp
    src/markdown/richpaste.h
    src/markdown/shortcodes.cpp
    src/markdown/shortcodes.h
    src/markdown/snippets.cpp
    src/markdown/snippets.h
    src/markdown/symbolcatalog.cpp
    src/markdown/symbolcatalog.h
    src/markdown/tableedit.cpp
    src/markdown/tableedit.h
    src/markdown/tasklist.cpp
    src/markdown/tasklist.h
    src/markdown/usertemplate.cpp
    src/markdown/usertemplate.h
    src/markdown/texttransform.cpp
    src/markdown/texttransform.h
    src/markdown/typography.cpp
    src/markdown/typography.h
    src/markdown/urldetect.cpp
    src/markdown/urldetect.h
    # --- highlight ---
    src/highlight/codehighlighter.cpp
    src/highlight/codehighlighter.h
    src/highlight/languageregistry.cpp
    src/highlight/languageregistry.h
    # --- math ---
    src/math/formulacontroller.cpp
    src/math/formulacontroller.h
    src/math/mathblocks.cpp
    src/math/mathblocks.h
    src/math/mathlayout.cpp
    src/math/mathlayout.h
    src/math/mathobject.cpp
    src/math/mathobject.h
    src/math/texparser.cpp
    # --- diagram ---
    src/diagram/diagramcontroller.cpp
    src/diagram/diagramcontroller.h
    src/diagram/diagram.cpp
    src/diagram/diagramdoc.cpp
    src/diagram/diagramdoc.h
    src/diagram/diagram.h
    src/diagram/diagramrenderer.cpp
    src/diagram/diagramrenderer.h
    # --- spell ---
    src/spell/spellchecker.cpp
    src/spell/dictionaryinstaller.cpp
    src/spell/spellchecker.h
    src/spell/spellcontroller.cpp
    src/spell/spellcontroller.h
    src/spell/spellscan.cpp
    src/spell/spellscan.h
    # --- export ---
    src/export/exportcontroller.cpp
    src/export/exportcontroller.h
    src/export/exporters.cpp
    src/export/exporters.h
    src/export/exportdocx.cpp
    src/export/exportepub.cpp
    src/export/exporthtml.cpp
    src/export/exportlatex.cpp
    src/export/exportodf.cpp
    src/export/exportutil.cpp
    src/export/exportutil.h
    # --- widgets ---
    src/widgets/commandpalettedialog.cpp
    src/widgets/commandpalettedialog.h
    src/widgets/filterlistdialog.cpp
    src/widgets/filterlistdialog.h
    src/widgets/findreplacebar.cpp
    src/widgets/findreplacebar.h
    src/widgets/formaticons.cpp
    src/widgets/formaticons.h
    src/widgets/gotoheadingdialog.cpp
    src/widgets/gotoheadingdialog.h
    src/widgets/helpdialog.cpp
    src/widgets/helpdialog.h
    src/widgets/outlinepanel.cpp
    src/widgets/outlinepanel.h
    src/widgets/preferencesdialog.cpp
    src/widgets/preferencesdialog.h
    src/widgets/codeblockoverlay.cpp
    src/widgets/codeblockoverlay.h
    src/widgets/snippetsdialog.cpp
    src/widgets/snippetsdialog.h
    src/widgets/tabletoolbar.cpp
    src/widgets/tabletoolbar.h
    src/widgets/usertemplatesdialog.cpp
    src/widgets/usertemplatesdialog.h
    src/widgets/symbolpicker.cpp
    src/widgets/symbolpicker.h
)
# Qt6::GuiPrivate da acceso al QZip privado (QZipReader/QZipWriter), con el que
# exporters.cpp reempaqueta el ODF para incrustarle el idioma sin dependencias
# externas. Es API privada de Qt (puede cambiar entre versiones), pero no añade
# ninguna biblioteca de terceros.
target_link_libraries(md-editor-core PUBLIC Qt6::Widgets Qt6::PrintSupport Qt6::Network Qt6::GuiPrivate)
target_include_directories(md-editor-core PUBLIC
    src
    src/app
    src/editor
    src/view
    src/io
    src/markdown
    src/highlight
    src/math
    src/diagram
    src/spell
    src/export
    src/widgets)
# Versión del proyecto disponible en el código (p. ej. el diálogo «Acerca de»),
# para no tenerla escrita a mano en dos sitios. PUBLIC: la ven el ejecutable y los tests.
target_compile_definitions(md-editor-core PUBLIC APP_VERSION="${PROJECT_VERSION}")

# --- Corrector ortográfico (opcional) ---
# Hunspell es la PRIMERA dependencia de terceros del proyecto, así que es
# OPCIONAL: si no está, el binario compila igual y el corrector queda inactivo
# (HAVE_HUNSPELL sin definir). La lógica pura (spellscan) se compila siempre; el
# motor y su integración se guardan tras HAVE_HUNSPELL.
#
# Por defecto se enlaza ESTÁTICO (SPELL_CHECK_STATIC): el motor viaja DENTRO del
# ejecutable, así el paquete descargable no depende de ninguna .so/.dll/.dylib de
# Hunspell. Esto es clave en Windows/macOS, donde windeployqt/macdeployqt solo
# empaquetan Qt (no las bibliotecas de terceros): al ir estático, no hay nada de
# Hunspell que desplegar. Solo quedan los diccionarios (ver dictionaries/).
#
# Cómo obtener Hunspell (con su biblioteca estática) por plataforma:
#   - Debian/Ubuntu: sudo apt-get install libhunspell-dev   (trae .a y .so)
#   - macOS (Homebrew): brew install hunspell                (trae .a)
#   - Windows (vcpkg):  vcpkg install hunspell:x64-windows-static
#     y configurar con -DCMAKE_TOOLCHAIN_FILE=<vcpkg>/scripts/buildsystems/vcpkg.cmake
option(SPELL_CHECK "Corrector ortográfico (requiere Hunspell)" ON)
option(SPELL_CHECK_STATIC "Enlazar Hunspell estáticamente (binario autocontenido)" ON)
# Que Hunspell sea opcional está bien para quien compila; para lo que se PUBLICA
# es una trampa: hasta la 2.8.3 los tres binarios descargables salieron sin
# corrector —nadie instalaba Hunspell en el workflow— y nada lo avisó, porque
# «no encontrado» solo imprime un STATUS. Los jobs de release configuran con
# SPELL_CHECK_REQUIRED=ON para que eso rompa la build en vez de silenciarse.
option(SPELL_CHECK_REQUIRED "Fallar si no se encuentra Hunspell (para empaquetar)" OFF)
if(SPELL_CHECK)
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(HUNSPELL hunspell)
    endif()
    # Respaldo sin pkg-config: en Windows no suele estar (y vcpkg no siempre deja
    # un .pc utilizable), así que sin esto el corrector se quedaba fuera de la
    # build de Windows aunque Hunspell estuviera instalado — y el binario que se
    # publicaba no tenía corrector sin que nada lo avisara. Busca cabecera y
    # biblioteca a mano; el bloque de abajo (estático/dinámico) sigue igual.
    if(NOT HUNSPELL_FOUND)
        find_path(HUNSPELL_INCLUDE_DIR NAMES hunspell/hunspell.hxx hunspell.hxx)
        find_library(HUNSPELL_LIBRARY NAMES hunspell hunspell-1.7 libhunspell)
        if(HUNSPELL_INCLUDE_DIR AND HUNSPELL_LIBRARY)
            set(HUNSPELL_FOUND TRUE)
            set(HUNSPELL_INCLUDE_DIRS ${HUNSPELL_INCLUDE_DIR})
            set(HUNSPELL_LIBRARIES ${HUNSPELL_LIBRARY})
            set(HUNSPELL_VERSION "sin pkg-config")
            message(STATUS "Corrector ortográfico: Hunspell encontrado sin pkg-config "
                           "(${HUNSPELL_LIBRARY})")
        endif()
    endif()
    if(HUNSPELL_FOUND)
        target_compile_definitions(md-editor-core PUBLIC HAVE_HUNSPELL)
        target_include_directories(md-editor-core PUBLIC ${HUNSPELL_INCLUDE_DIRS})

        set(_hunspell_link ${HUNSPELL_LIBRARIES})  # dinámico por defecto del pkg
        if(SPELL_CHECK_STATIC)
            # Busca el archivo estático (.a/.lib) forzando el sufijo, porque
            # find_library prefiere la compartida si están las dos.
            set(_saved_suffixes ${CMAKE_FIND_LIBRARY_SUFFIXES})
            if(WIN32)
                set(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a)
            else()
                set(CMAKE_FIND_LIBRARY_SUFFIXES .a)
            endif()
            find_library(HUNSPELL_STATIC_LIB
                NAMES ${HUNSPELL_LIBRARIES} hunspell-1.7 hunspell libhunspell
                HINTS ${HUNSPELL_STATIC_LIBRARY_DIRS} ${HUNSPELL_LIBRARY_DIRS})
            set(CMAKE_FIND_LIBRARY_SUFFIXES ${_saved_suffixes})
            if(HUNSPELL_STATIC_LIB)
                set(_hunspell_link ${HUNSPELL_STATIC_LIB})
                message(STATUS "Corrector ortográfico: Hunspell ${HUNSPELL_VERSION} "
                               "(estático: ${HUNSPELL_STATIC_LIB})")
            else()
                message(WARNING "Corrector ortográfico: no se encontró la biblioteca "
                                "ESTÁTICA de Hunspell; se enlaza dinámica (el paquete "
                                "necesitará libhunspell en destino). Usa "
                                "-DSPELL_CHECK_STATIC=OFF para silenciar este aviso.")
            endif()
        endif()
        if(NOT SPELL_CHECK_STATIC OR NOT HUNSPELL_STATIC_LIB)
            message(STATUS "Corrector ortográfico: Hunspell ${HUNSPELL_VERSION} (dinámico)")
        endif()
        target_link_libraries(md-editor-core PUBLIC ${_hunspell_link})
    elseif(SPELL_CHECK_REQUIRED)
        message(FATAL_ERROR
                "Corrector ortográfico: Hunspell no encontrado y SPELL_CHECK_REQUIRED=ON. "
                "El binario saldría SIN corrector; para empaquetar hay que instalarlo "
                "(libhunspell-dev / brew install hunspell / vcpkg) o compilarlo aparte y "
                "apuntar CMAKE_PREFIX_PATH a su prefijo.")
    else()
        message(STATUS "Corrector ortográfico: Hunspell no encontrado, se omite "
                       "(instala libhunspell-dev / brew / vcpkg para activarlo)")
    endif()
endif()

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
    src/app/main.cpp
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

# Diccionarios del corrector empaquetados (opcional). En Linux se usan los del
# sistema (/usr/share/hunspell), pero Windows/macOS no tienen ninguno: hay que
# llevarlos junto a la app. Si el usuario deja pares .aff/.dic en `dictionaries/`,
# se instalan donde el corrector los busca (ver searchPaths en spellchecker.cpp):
#   - Windows: junto al .exe (bin/dictionaries)
#   - macOS:   dentro del .app (Contents/Resources/dictionaries)
#   - Linux:   <prefix>/share/hunspell (por si se quiere una instalación cerrada)
# Vacío por defecto (no se commitean diccionarios: licencias y tamaño varían; ver
# dictionaries/README.md).
file(GLOB BUNDLED_DICTS "${CMAKE_CURRENT_SOURCE_DIR}/dictionaries/*.aff"
                        "${CMAKE_CURRENT_SOURCE_DIR}/dictionaries/*.dic")
if(BUNDLED_DICTS)
    if(APPLE)
        install(FILES ${BUNDLED_DICTS}
            DESTINATION "md-editor.app/Contents/Resources/dictionaries")
    elseif(WIN32)
        install(FILES ${BUNDLED_DICTS}
            DESTINATION "${CMAKE_INSTALL_BINDIR}/dictionaries")
    else()
        install(FILES ${BUNDLED_DICTS}
            DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/hunspell")
    endif()
    message(STATUS "Diccionarios empaquetados: ${BUNDLED_DICTS}")
endif()

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
        tst_tasklist
        tst_symbolcatalog
        tst_urldetect
        tst_richpaste
        tst_doctemplates
        tst_admonitions
        tst_markdownrender
        tst_shortcodes
        tst_footnotes
        tst_mathblocks
        tst_mathlayout
        tst_spellscan
        tst_spellchecker
        tst_diagram
        tst_diagramrenderer
        tst_tableedit
        tst_exporters
        tst_markdownroundtrip
        tst_outline
        tst_docstats
        tst_nav
        tst_csvtable
        tst_closedtabstack
        tst_tabmenu
        tst_commands
        tst_extraselections
        tst_linehighlight
        tst_find
        tst_headingshift
        tst_tablesort
        tst_tablesortui
        tst_outlinepanel
        tst_moveline
        tst_linecommands
        tst_markhighlight
        tst_supersub
        tst_typography
        tst_codeblock
        tst_htmlimport
        tst_epubimport
        tst_pandocimport
        tst_docx
        tst_printdecor
        tst_inputrules
        tst_texttransform
        tst_typewriter
        tst_snippets
        tst_usertemplate
        tst_autopair
        tst_markdowntidy
        tst_codespanfix
        tst_goldenexport
        tst_recovery
        tst_appsettings
        tst_documentio
        tst_themecontroller
        tst_themespec
        tst_links
        tst_splitview
        tst_tablifecycle
        tst_imageobject
        tst_chromezoom
        tst_shortcuts
        tst_help
        tst_filecontroller
        tst_formulacontroller)
        add_executable(${test} tests/${test}.cpp)
        target_link_libraries(${test} PRIVATE md-editor-core Qt6::Test)
        add_test(NAME ${test} COMMAND ${test})
        # Las pruebas usan QApplication: ejecútalas sin pantalla (headless).
        #
        # En Windows NO: su plugin «offscreen» no monta la base de datos de
        # fuentes, y sin ella QTextDocument::toMarkdown() deja de emitir los
        # marcadores de énfasis (entra «texto **negrita** aqui» y sale sin los
        # asteriscos), además de tragarse toda la salida de QTest. Eso hacía
        # fallar 7 pruebas —round-trip, pegado enriquecido, importadores,
        # notas al pie y los 12 goldens, todos más cortos de la cuenta— por un
        # defecto del entorno de pruebas, no del editor: con el plugin nativo
        # pasan todas. Los runners de Windows tienen sesión de escritorio, así
        # que el plugin nativo funciona sin más.
        if(WIN32)
            set(_qpa "windows")
        else()
            set(_qpa "offscreen")
        endif()
        set_tests_properties(${test} PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=${_qpa}")
    endforeach()

    # tst_help valida los .md de ayuda tal como se empaquetan, leyéndolos del
    # recurso Qt. El .qrc solo está en el ejecutable (la biblioteca no lo lleva),
    # así que se compila también dentro de este test.
    target_sources(tst_help PRIVATE src/resources.qrc)

    # Los golden tests necesitan la ruta a las referencias en el árbol fuente.
    target_compile_definitions(tst_goldenexport PRIVATE
        GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/golden")

    # La mitad de importación de tst_docx lee los .docx de prueba del árbol fuente
    # (se regeneran con scripts/make-docx-fixtures.py).
    target_compile_definitions(tst_docx PRIVATE
        DOCX_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures/docx")

    # Fuzzer del round-trip: se COMPILA pero NO se registra como prueba de CTest.
    # Se ejecuta a mano:
    #     QT_QPA_PLATFORM=offscreen ./build/tst_roundtripfuzz
    # y en CI desde el job `fuzz`, que lo invoca directamente (no por ctest, que
    # no lo ve) bajo ASan/UBSan y NO puede bloquear el workflow. Fuera del gate
    # porque sus entradas adversarias van contra el parser Markdown de Qt, donde
    # ya han aparecido dos fallos ajenos a esta app —un desbordamiento de pila
    # dependiente del build de Qt y el cuelgue de setMarkdown en Windows—: con un
    # test así en el gate, un fallo de Qt bloquearía la release. Su valor está en
    # avisar, y lo ha dado dos veces: cazó el desbordamiento de recursión del
    # parser TeX (de donde salió el tope de profundidad) y el cuelgue de Windows
    # (de donde salió el rodeo de mdrender::protect).
    add_executable(tst_roundtripfuzz tests/tst_roundtripfuzz.cpp)
    target_link_libraries(tst_roundtripfuzz PRIVATE md-editor-core Qt6::Test)

    # Reproductor del cuelgue del fuzzer en Windows. Enlaza SOLO Qt —nada de
    # md-editor-core, que es justo lo que demuestra— y ejecuta una variante por
    # proceso porque el fallo mata el proceso. Ver tools/qtmdrepro.cpp.
    add_executable(qtmdrepro tools/qtmdrepro.cpp)
    target_link_libraries(qtmdrepro PRIVATE Qt6::Widgets)

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
