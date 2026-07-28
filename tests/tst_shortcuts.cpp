#include <QtTest>

#include <QAction>
#include <QApplication>
#include <QKeySequence>
#include <QMenuBar>
#include <QSet>

#include "mainwindow.h"

// Atajos de teclado de la ventana principal. Nacen de tres fallos reales, los tres
// invisibles desde el código: un atajo puede estar «puesto» y no funcionar.
//
//  - «Cerrar pestaña» usaba setShortcut(QKeySequence::Close), y en X11 esa
//    constante tiene DOS enlaces (Ctrl+F4 y Ctrl+W) de los que el singular se
//    queda con el primero: Ctrl+W no cerraba nada pese a estar documentado.
//  - «Buscar siguiente/anterior» (F3 / Mayús+F3) estaban en la ayuda pero no
//    existían en el código.
//  - «Preferencias» usaba QKeySequence::Preferences, que viene VACÍO fuera de
//    macOS: la entrada se quedaba sin atajo.
//
// Y, de propina, la comprobación que habría cazado el intento de arreglo: dos
// acciones con el mismo atajo lo dejan AMBIGUO y Qt no dispara ninguna (pasa si
// se usa QKeySequence::FindNext, que incluye Ctrl+G, ya tomado por «Ir a
// encabezado»).
class TestShortcuts : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();

    void closeTabAcceptsCtrlW();
    void findNextAndPreviousAreBound();
    void preferencesHasAShortcut();
    void noTwoActionsShareAShortcut();

private:
    // Busca una acción por su texto visible en todos los menús de la ventana.
    static QAction *actionNamed(const MainWindow &w, const QString &text);
    // Todas las acciones de la barra de menús, submenús incluidos.
    static QList<QAction *> allActions(const MainWindow &w);
};

void TestShortcuts::initTestCase()
{
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
}

QList<QAction *> TestShortcuts::allActions(const MainWindow &w)
{
    QList<QAction *> out;
    QList<QMenu *> pending;
    for (QAction *a : w.menuBar()->actions())
        if (a->menu())
            pending << a->menu();
    while (!pending.isEmpty()) {
        QMenu *menu = pending.takeFirst();
        for (QAction *a : menu->actions()) {
            if (a->menu())
                pending << a->menu();
            else if (!a->isSeparator())
                out << a;
        }
    }
    return out;
}

QAction *TestShortcuts::actionNamed(const MainWindow &w, const QString &text)
{
    for (QAction *a : allActions(w)) {
        QString plain = a->text();
        plain.remove(QLatin1Char('&'));  // «&Preferencias...» -> «Preferencias...»
        if (plain == text)
            return a;
    }
    return nullptr;
}

void TestShortcuts::closeTabAcceptsCtrlW()
{
    MainWindow w;
    QAction *close = actionNamed(w, QStringLiteral("Cerrar pestaña"));
    QVERIFY(close);
    QVERIFY2(close->shortcuts().contains(QKeySequence(Qt::CTRL | Qt::Key_W)),
             "Ctrl+W debe cerrar la pestaña (la ayuda lo documenta)");
}

void TestShortcuts::findNextAndPreviousAreBound()
{
    MainWindow w;
    QAction *next = actionNamed(w, QStringLiteral("Buscar siguiente"));
    QAction *prev = actionNamed(w, QStringLiteral("Buscar anterior"));
    QVERIFY(next);
    QVERIFY(prev);
    QCOMPARE(next->shortcut(), QKeySequence(Qt::Key_F3));
    QCOMPARE(prev->shortcut(), QKeySequence(Qt::SHIFT | Qt::Key_F3));
}

void TestShortcuts::preferencesHasAShortcut()
{
    MainWindow w;
    QAction *prefs = actionNamed(w, QStringLiteral("Preferencias..."));
    QVERIFY(prefs);
    QVERIFY2(!prefs->shortcut().isEmpty(),
             "QKeySequence::Preferences viene vacío fuera de macOS: hay que suplirlo");
}

void TestShortcuts::noTwoActionsShareAShortcut()
{
    MainWindow w;
    QHash<QString, QString> owner;  // atajo -> primera acción que lo declaró
    for (QAction *a : allActions(w)) {
        for (const QKeySequence &seq : a->shortcuts()) {
            if (seq.isEmpty())
                continue;
            const QString key = seq.toString();
            const QString previous = owner.value(key);
            QVERIFY2(previous.isEmpty(),
                     qPrintable(QStringLiteral("Atajo ambiguo %1: «%2» y «%3»")
                                    .arg(key, previous, a->text())));
            owner.insert(key, a->text());
        }
    }
}

QTEST_MAIN(TestShortcuts)
#include "tst_shortcuts.moc"
