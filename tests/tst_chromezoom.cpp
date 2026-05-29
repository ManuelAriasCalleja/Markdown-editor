#include <QtTest>

#include <QApplication>
#include <QKeySequence>
#include <QMenu>

#include "chromezoom.h"

// Pruebas de las funciones puras de zoom de la interfaz (chromezoom).
class TestChromeZoom : public QObject
{
    Q_OBJECT

private slots:
    void scaledPointSizeAppliesDelta();
    void scaledPointSizeClampsToOne();
    void scaledPointSizePassesThroughInvalidBase();

    void emptyMenuHasNoMinimum();
    void menuWidthGrowsWithLongerText();
    void shortcutColumnWidensMenu();
    void submenuArrowWidensMenu();
};

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

QTEST_MAIN(TestChromeZoom)
#include "tst_chromezoom.moc"
