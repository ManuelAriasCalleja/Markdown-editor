#include <QtTest>

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QImage>
#include <QKeySequence>
#include <QMenu>
#include <QPalette>
#include <QPixmap>
#include <QSettings>
#include <QToolBar>

#include "chromezoom.h"
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

    void emptyMenuHasNoMinimum();
    void menuWidthGrowsWithLongerText();
    void shortcutColumnWidensMenu();
    void submenuArrowWidensMenu();

    void toolbarIconInkContrastsWithTheme();
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

QTEST_MAIN(TestChromeZoom)
#include "tst_chromezoom.moc"
