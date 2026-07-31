#include <QtTest>

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QDialog>
#include <QImage>
#include <QKeySequence>
#include <QLabel>
#include <QListWidget>
#include <QMenu>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QTabBar>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

#include "chromezoom.h"
#include "editorstack.h"
#include "focuseditor.h"
#include "helpdialog.h"
#include "symbolpicker.h"
#include "mainwindow.h"

// Pruebas de las funciones puras de zoom de la interfaz (chromezoom) y de la
// apariencia de la barra de formato, que escala y recolorea con ellas.
class TestChromeZoom : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void scaledPointSizeAppliesDelta();
    void scaledPointSizeClampsToOne();
    void scaledPointSizePassesThroughInvalidBase();

    void scaledWindowSizeGrowsWithFont();
    void scaledWindowSizeStaysInScreen();

    void emptyMenuHasNoMinimum();
    void menuWidthGrowsWithLongerText();
    void shortcutColumnWidensMenu();
    void submenuArrowWidensMenu();

    void toolbarIconInkContrastsWithTheme();
    void tabBarFontFollowsZoom();
    void dialogFontFollowsZoom();
    void helpDialogFollowsZoom();
    void symbolPickerFollowsZoom();
    void aboutBoxWidensWithZoom();
};

void TestChromeZoom::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

void TestChromeZoom::cleanup()
{
    QSettings().clear();
}

void TestChromeZoom::scaledPointSizeAppliesDelta()
{
    QCOMPARE(chromezoom::scaledPointSize(12.0, 3), 15.0);
    QCOMPARE(chromezoom::scaledPointSize(12.0, -4), 8.0);
    QCOMPARE(chromezoom::scaledPointSize(12.0, 0), 12.0);
}

void TestChromeZoom::scaledPointSizeClampsToOne()
{
    // Un desfase muy negativo no baja de 1 punto.
    QCOMPARE(chromezoom::scaledPointSize(10.0, -50), 1.0);
}

void TestChromeZoom::scaledPointSizePassesThroughInvalidBase()
{
    // Base no válida (la fuente no usa puntos): se devuelve tal cual para que el
    // llamante no escale ese widget.
    QCOMPARE(chromezoom::scaledPointSize(-1.0, 5), -1.0);
}

void TestChromeZoom::scaledWindowSizeGrowsWithFont()
{
    // La ventana de lectura crece en la misma proporción que la fuente (si no, el
    // texto ampliado parte las líneas donde no toca). Sin tope de pantalla.
    QCOMPARE(chromezoom::scaledWindowSize(QSize(820, 620), 1.0), QSize(820, 620));
    QCOMPARE(chromezoom::scaledWindowSize(QSize(820, 620), 1.5), QSize(1230, 930));
    // Escala no válida: el tamaño base, tal cual.
    QCOMPARE(chromezoom::scaledWindowSize(QSize(820, 620), 0.0), QSize(820, 620));
}

void TestChromeZoom::scaledWindowSizeStaysInScreen()
{
    // Con un zoom grande el tamaño ideal no cabe: se recorta a la pantalla, por
    // dimensiones (aquí el alto sí cabe y el ancho no).
    QCOMPARE(chromezoom::scaledWindowSize(QSize(820, 620), 3.0, QSize(1000, 2000)),
             QSize(1000, 1860));
    // Un tope no válido (sin pantalla conocida) no recorta nada.
    QCOMPARE(chromezoom::scaledWindowSize(QSize(820, 620), 2.0, QSize()),
             QSize(1640, 1240));
}

void TestChromeZoom::emptyMenuHasNoMinimum()
{
    QMenu menu;
    QCOMPARE(chromezoom::menuMinimumWidth(menu), 0);

    menu.addSeparator();  // un separador no cuenta como acción con texto
    QCOMPARE(chromezoom::menuMinimumWidth(menu), 0);
}

void TestChromeZoom::menuWidthGrowsWithLongerText()
{
    QMenu shortMenu;
    shortMenu.addAction(QStringLiteral("Ok"));
    QMenu longMenu;
    longMenu.addAction(QStringLiteral("Insertar columna a la izquierda"));

    QVERIFY(chromezoom::menuMinimumWidth(longMenu)
            > chromezoom::menuMinimumWidth(shortMenu));
}

void TestChromeZoom::shortcutColumnWidensMenu()
{
    // Mismo texto; con atajo el menú reserva una columna más → es más ancho.
    QMenu plain;
    plain.addAction(QStringLiteral("Guardar"));

    QMenu withShortcut;
    QAction *a = withShortcut.addAction(QStringLiteral("Guardar"));
    a->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_S));

    QVERIFY(chromezoom::menuMinimumWidth(withShortcut)
            > chromezoom::menuMinimumWidth(plain));
}

void TestChromeZoom::submenuArrowWidensMenu()
{
    QMenu plain;
    plain.addAction(QStringLiteral("Tema"));

    QMenu withSub;
    QMenu *sub = withSub.addMenu(QStringLiteral("Tema"));
    sub->addAction(QStringLiteral("Claro"));

    QVERIFY(chromezoom::menuMinimumWidth(withSub)
            > chromezoom::menuMinimumWidth(plain));
}

// Luminancia media de los píxeles opacos de un pixmap (la tinta del glifo del
// icono), en [0,1]. Sirve para distinguir tinta clara de oscura.
static double meanInkLuma(const QPixmap &pm)
{
    const QImage img = pm.toImage();
    double sum = 0.0;
    int n = 0;
    for (int y = 0; y < img.height(); ++y) {
        for (int x = 0; x < img.width(); ++x) {
            const QColor c = img.pixelColor(x, y);
            if (c.alpha() > 200) {  // píxeles de la tinta, no los bordes translúcidos
                sum += 0.2126 * c.redF() + 0.7152 * c.greenF() + 0.0722 * c.blueF();
                ++n;
            }
        }
    }
    return n > 0 ? sum / n : -1.0;
}

void TestChromeZoom::toolbarIconInkContrastsWithTheme()
{
    // Regresión: los iconos generados de la barra (negrita, listas…) van
    // «horneados» con un color, así que deben REGENERARSE al cambiar la paleta —
    // o quedan con la tinta del tema anterior y dejan de contrastar. Lo dispara
    // MainWindow::changeEvent ante ApplicationPaletteChange (no la señal de un
    // solo stack), así que `qApp->setPalette` debe bastar para recolorearlos.
    const QPalette saved = qApp->palette();
    MainWindow w;
    w.show();
    QVERIFY(QTest::qWaitForWindowExposed(&w));

    QToolBar *bar = w.findChild<QToolBar *>(QStringLiteral("formatToolBar"));
    QVERIFY(bar);
    QAction *iconAction = nullptr;  // la primera acción con icono (negrita)
    for (QAction *a : bar->actions())
        if (!a->icon().isNull()) { iconAction = a; break; }
    QVERIFY(iconAction);
    const int px = bar->iconSize().width();
    QVERIFY(px > 0);

    // Fondo de ventana oscuro → tinta clara (luma alta). El cambio de paleta de
    // qApp llega a la ventana como evento; hay que vaciar la cola para que
    // changeEvent regenere los iconos.
    QPalette dark = saved;
    dark.setColor(QPalette::Window, QColor(0x35, 0x35, 0x35));
    qApp->setPalette(dark);
    qApp->processEvents();
    QVERIFY2(meanInkLuma(iconAction->icon().pixmap(px)) > 0.5,
             "con tema oscuro el icono debe tener tinta clara");

    // Fondo claro → tinta oscura (luma baja).
    QPalette light = saved;
    light.setColor(QPalette::Window, QColor(0xf0, 0xf0, 0xf0));
    qApp->setPalette(light);
    qApp->processEvents();
    QVERIFY2(meanInkLuma(iconAction->icon().pixmap(px)) < 0.5,
             "con tema claro el icono debe tener tinta oscura");

    qApp->setPalette(saved);
}

void TestChromeZoom::tabBarFontFollowsZoom()
{
    // Regresión: los rótulos de las pestañas (los nombres de los documentos
    // abiertos) se quedaban al tamaño base mientras el editor, los menús y el resto
    // del chrome crecían con el zoom. Deben seguir el mismo desfase en puntos.
    MainWindow w;
    QTabBar *bar = w.findChild<QTabBar *>();
    EditorStack *stack = w.findChild<EditorStack *>();
    QVERIFY(bar);
    QVERIFY(stack);
    FocusEditor *editor = stack->editor();
    const qreal baseTab = bar->font().pointSizeF();
    const qreal baseEditor = editor->font().pointSizeF();
    const int baseWidth = bar->sizeHint().width();
    QVERIFY(baseTab > 0);

    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QCOMPARE(bar->font().pointSizeF(), baseTab + 2);
    // La barra debe además rehacer su maqueta con la fuente nueva (si no, el
    // rótulo crecido se vería recortado dentro de la pestaña vieja). Se mira el
    // ancho, no el alto: el mínimo del estilo puede absorber el alto extra (así
    // pasa con la plataforma offscreen de los tests), el ancho del rótulo no.
    QVERIFY(bar->sizeHint().width() > baseWidth);
    // Al mismo desfase que el editor: pestañas y texto no se desincronizan.
    QCOMPARE(bar->font().pointSizeF() - baseTab,
             editor->font().pointSizeF() - baseEditor);

    QVERIFY(QMetaObject::invokeMethod(&w, "resetZoom"));
    QCOMPARE(bar->font().pointSizeF(), baseTab);
}

void TestChromeZoom::dialogFontFollowsZoom()
{
    // Regresión: los diálogos (Preferencias, el manual, «Acerca de», el mapa de
    // caracteres, los avisos…) son ventanas propias, y Qt no propaga a una ventana la
    // fuente de su padre: se quedaban al tamaño base de la aplicación mientras el
    // editor, los menús y las barras crecían con el zoom. Se escalan por CLASE
    // (QDialog), así que basta un QDialog cualquiera para comprobarlo: los concretos
    // son modales (exec bloquearía el test) o los abren otros controladores.
    MainWindow w;
    const qreal baseApp = QApplication::font().pointSizeF();
    QVERIFY(baseApp > 0);

    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));

    // Un diálogo abierto DESPUÉS del zoom nace escalado, y sus hijos heredan. Qt
    // resuelve la fuente de clase al pulir el widget (al construirlo aún no sabe de
    // qué clase es), lo que en la aplicación real pasa siempre antes de mostrarlo.
    QDialog opened(&w);
    auto *child = new QLabel(QStringLiteral("texto"), &opened);
    opened.ensurePolished();
    QCOMPARE(opened.font().pointSizeF(), baseApp + 2);
    QCOMPARE(child->font().pointSizeF(), baseApp + 2);

    // Y uno ya abierto sigue los cambios de zoom (el manual y el mapa de caracteres
    // son no modales: se quedan a un lado mientras se amplía).
    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QCOMPARE(opened.font().pointSizeF(), baseApp + 3);
    QCOMPARE(child->font().pointSizeF(), baseApp + 3);

    // La base es siempre la fuente de la aplicación, nunca la que el diálogo tenga
    // ya puesta: volver a cero deja el tamaño original, sin acumular.
    QVERIFY(QMetaObject::invokeMethod(&w, "resetZoom"));
    QCOMPARE(opened.font().pointSizeF(), baseApp);
    QDialog later(&w);
    later.ensurePolished();
    QCOMPARE(later.font().pointSizeF(), baseApp);
}

void TestChromeZoom::helpDialogFollowsZoom()
{
    // El manual (F1) es único y no modal: puede quedarse abierto mientras se cambia
    // el zoom, así que además de nacer escalado tiene que seguir los cambios.
    MainWindow w;
    const qreal baseApp = QApplication::font().pointSizeF();
    const QKeySequence helpShortcut(QKeySequence::HelpContents);
    QVERIFY(!helpShortcut.isEmpty());
    QAction *manual = nullptr;
    for (QAction *action : w.findChildren<QAction *>())
        if (action->shortcut() == helpShortcut)
            manual = action;
    QVERIFY(manual);

    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    manual->trigger();  // no modal: solo muestra la ventana

    HelpDialog *dialog = w.findChild<HelpDialog *>();
    QVERIFY(dialog);
    QCOMPARE(dialog->font().pointSizeF(), baseApp + 2);
    QListWidget *index = dialog->findChild<QListWidget *>();
    QVERIFY(index);
    const int baseIndexWidth = index->maximumWidth();
    const QSize baseSize = dialog->size();

    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QCOMPARE(dialog->font().pointSizeF(), baseApp + 3);
    // La columna del índice se remide con la fuente nueva; con un ancho fijo en
    // píxeles, sus rótulos («Uso de la aplicación») se veían recortados al ampliar.
    QVERIFY(index->maximumWidth() > baseIndexWidth);

    // Y la ventana sigue al tamaño de la fuente: con una fuente grande en una ventana
    // del mismo tamaño, el texto parte las líneas de forma poco natural. Se comprueba
    // reduciendo la fuente (el tamaño ideal de una fuente grande puede estar topado
    // por la pantalla —la de los tests es pequeña— y no se notaría el crecimiento).
    for (int i = 0; i < 5; ++i)
        QVERIFY(QMetaObject::invokeMethod(&w, "zoomOutText"));
    QCOMPARE(dialog->font().pointSizeF(), baseApp - 2);
    QVERIFY(dialog->width() < baseSize.width());
    QVERIFY(dialog->height() < baseSize.height());
    QVERIFY(index->maximumWidth() < baseIndexWidth);
}

void TestChromeZoom::symbolPickerFollowsZoom()
{
    // Los diálogos que abre un colaborador (este lo abre InsertController, del
    // documento activo) no pasan por MainWindow: los alcanza el filtro instalado en
    // QApplication. El mapa de caracteres es además no modal y sus símbolos llevan
    // tamaño propio —mayor que el del texto—, así que también tienen que escalar.
    MainWindow w;
    const qreal baseApp = QApplication::font().pointSizeF();
    QAction *symbols = nullptr;
    for (QAction *action : w.findChildren<QAction *>())
        if (action->text() == QStringLiteral("S\u00edmbolos especiales..."))
            symbols = action;
    QVERIFY(symbols);

    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    symbols->trigger();  // no modal: solo muestra la ventana

    SymbolPicker *picker = w.findChild<SymbolPicker *>();
    QVERIFY(picker);
    QCOMPARE(picker->font().pointSizeF(), baseApp + 2);
    QToolButton *button = picker->findChild<QToolButton *>();
    QVERIFY(button);
    QVERIFY(button->font().pointSizeF() > picker->font().pointSizeF());
    const int baseSide = button->maximumWidth();  // la celda es de tama\u00f1o fijo

    // Y la casilla crece con el s\u00edmbolo (si no, el glifo ampliado se recorta). Se
    // ampl\u00eda de varios pasos, no de uno: la celda sale de QFontMetrics::height(), que
    // es un entero de p\u00edxeles y con algunas fuentes da el mismo valor para dos
    // tama\u00f1os en puntos seguidos (as\u00ed fallaba en el CI de Linux, no en local).
    for (int i = 0; i < 5; ++i)
        QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QCOMPARE(picker->font().pointSizeF(), baseApp + 7);
    QVERIFY(button->maximumWidth() > baseSide);
}

void TestChromeZoom::aboutBoxWidensWithZoom()
{
    // «Acerca de» es un QMessageBox: el ancho del texto lo decide él, a la medida de
    // la fuente base, así que con el zoom subido partía las frases por donde no toca.
    // Se le fuerza con un espaciador en su rejilla —lo único que respeta—, y eso
    // depende de cómo Qt maqueta el QMessageBox: de ahí este test.
    MainWindow w;
    QAction *about = nullptr;
    for (QAction *action : w.findChildren<QAction *>())
        if (action->text() == QStringLiteral("&Acerca de"))
            about = action;
    QVERIFY(about);

    // El diálogo es modal (exec bloquea): se mide y se cierra desde el bucle de
    // eventos que abre el propio exec.
    const auto aboutWidth = [about] {
        int width = 0;
        QTimer::singleShot(0, [&width] {
            if (QWidget *dialog = QApplication::activeModalWidget()) {
                width = dialog->width();
                dialog->close();
            }
        });
        about->trigger();
        return width;
    };

    const int baseWidth = aboutWidth();
    QVERIFY(baseWidth > 0);
    for (int i = 0; i < 5; ++i)
        QVERIFY(QMetaObject::invokeMethod(&w, "zoomInText"));
    QVERIFY(aboutWidth() > baseWidth);
}

QTEST_MAIN(TestChromeZoom)
#include "tst_chromezoom.moc"
