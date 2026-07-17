#include <QtTest>

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QTabWidget>
#include <QTemporaryDir>
#include <QTextEdit>

#include "documentio.h"
#include "editorstack.h"
#include "mainwindow.h"

// Pruebas de caracterización del ciclo de vida de las pestañas de MainWindow
// (addTab / setActiveStack / openPathInTab / closeTab / reopenClosedTab /
// cycleTab). Fijan el comportamiento OBSERVABLE —número de pestañas, cuál es el
// documento activo (m_stack), qué archivo tiene cada pestaña y la política de
// reutilización/cierre— para que futuros cambios en esa zona (históricamente
// propensa a errores de ciclo de vida) no la alteren sin querer.
//
// Es friend de MainWindow para tocar los métodos privados de pestañas. Todas las
// asertaciones son sobre estado observable, no sobre miembros concretos. Se evita
// con cuidado abrir diálogos modales (colgarían el test headless): los documentos
// se mantienen SIN modificar (así maybeSave no pregunta) y no se cargan rutas
// inexistentes (evita el QMessageBox de error de carga).
class TestTabLifecycle : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void freshWindowHasOneUntitledTab();
    void addTabAppendsAndActivates();
    void openFileReusesEmptyUntitledTab();
    void openSecondFileOpensNewTabAndActivatesIt();
    void openAlreadyOpenFileSwitchesInsteadOfDuplicating();
    void switchingTabRepointsActiveStack();
    void closeTabRemovesItAndActivatesAnother();
    void closingLastTabResetsToUntitled();
    void closedFileTabCanBeReopened();
    void cycleTabWrapsAround();

private:
    QTemporaryDir m_dir;
    // Crea un .md con `body` en el directorio temporal y devuelve su ruta absoluta.
    QString writeDoc(const QString &name, const QString &body);
    static QString fileOf(EditorStack *s);
};

void TestTabLifecycle::initTestCase()
{
    // QSettings aislado (no toca la config real del usuario); se limpia en cleanup.
    QCoreApplication::setOrganizationName(QStringLiteral("md-editor-test"));
    QCoreApplication::setApplicationName(QStringLiteral("md-editor-test"));
    QVERIFY(m_dir.isValid());
}

void TestTabLifecycle::cleanup()
{
    QSettings().clear();
}

QString TestTabLifecycle::writeDoc(const QString &name, const QString &body)
{
    const QString path = m_dir.filePath(name);
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return {};
    f.write(body.toUtf8());
    f.close();
    return path;
}

QString TestTabLifecycle::fileOf(EditorStack *s)
{
    return s ? s->documentIo()->currentFile() : QString();
}

void TestTabLifecycle::freshWindowHasOneUntitledTab()
{
    MainWindow w;
    w.show();
    QCOMPARE(w.m_tabs->count(), 1);
    QVERIFY(w.m_stack != nullptr);
    QVERIFY(fileOf(w.m_stack).isEmpty());
    QVERIFY(!w.m_stack->documentIo()->isModified());
}

void TestTabLifecycle::addTabAppendsAndActivates()
{
    MainWindow w;
    w.show();
    EditorStack *first = w.m_stack;

    EditorStack *added = w.addTab();
    QCOMPARE(w.m_tabs->count(), 2);
    QCOMPARE(w.m_stack, added);          // la nueva pestaña queda activa
    QVERIFY(added != first);
    QVERIFY(fileOf(added).isEmpty());    // documento nuevo vacío
}

void TestTabLifecycle::openFileReusesEmptyUntitledTab()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n\nUno\n"));

    w.openPathInTab(a);
    // La pestaña inicial estaba vacía y sin cambios: se reutiliza, no se abre otra.
    QCOMPARE(w.m_tabs->count(), 1);
    QCOMPARE(fileOf(w.m_stack), a);
    QVERIFY(!w.m_stack->documentIo()->isModified());
}

void TestTabLifecycle::openSecondFileOpensNewTabAndActivatesIt()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));
    const QString b = writeDoc(QStringLiteral("b.md"), QStringLiteral("# B\n"));

    w.openPathInTab(a);   // reutiliza la pestaña vacía
    w.openPathInTab(b);   // la actual ya tiene archivo: pestaña nueva

    QCOMPARE(w.m_tabs->count(), 2);
    QCOMPARE(fileOf(w.stackAt(0)), a);
    QCOMPARE(fileOf(w.stackAt(1)), b);
    QCOMPARE(w.m_stack, w.stackAt(1));  // la recién abierta queda activa
}

void TestTabLifecycle::openAlreadyOpenFileSwitchesInsteadOfDuplicating()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));
    const QString b = writeDoc(QStringLiteral("b.md"), QStringLiteral("# B\n"));

    w.openPathInTab(a);
    w.openPathInTab(b);   // activa = b
    w.openPathInTab(a);   // ya abierto: debe SALTAR a su pestaña, no duplicar

    QCOMPARE(w.m_tabs->count(), 2);
    QCOMPARE(fileOf(w.m_stack), a);
}

void TestTabLifecycle::switchingTabRepointsActiveStack()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));
    const QString b = writeDoc(QStringLiteral("b.md"), QStringLiteral("# B\n"));

    w.openPathInTab(a);
    w.openPathInTab(b);
    QCOMPARE(fileOf(w.m_stack), b);

    w.m_tabs->setCurrentIndex(0);        // cambiar de pestaña dispara setActiveStack
    QCOMPARE(w.m_stack, w.stackAt(0));
    QCOMPARE(fileOf(w.m_stack), a);
    // El título de la ventana sigue al documento activo.
    QVERIFY(w.windowTitle().contains(QStringLiteral("a.md")));
}

void TestTabLifecycle::closeTabRemovesItAndActivatesAnother()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));
    const QString b = writeDoc(QStringLiteral("b.md"), QStringLiteral("# B\n"));

    w.openPathInTab(a);
    w.openPathInTab(b);
    QCOMPARE(w.m_tabs->count(), 2);

    w.closeTab(1);                       // cierra b (sin cambios: sin diálogo)
    QCOMPARE(w.m_tabs->count(), 1);
    QCOMPARE(fileOf(w.stackAt(0)), a);
    QCOMPARE(w.m_stack, w.stackAt(0));   // queda activa la que sobrevive
}

void TestTabLifecycle::closingLastTabResetsToUntitled()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));

    w.openPathInTab(a);
    QCOMPARE(w.m_tabs->count(), 1);

    w.closeTab(0);                       // última pestaña: no se elimina, se reinicia
    QCOMPARE(w.m_tabs->count(), 1);
    QVERIFY(fileOf(w.m_stack).isEmpty());
    QVERIFY(!w.m_stack->documentIo()->isModified());
}

void TestTabLifecycle::closedFileTabCanBeReopened()
{
    MainWindow w;
    w.show();
    const QString a = writeDoc(QStringLiteral("a.md"), QStringLiteral("# A\n"));
    const QString b = writeDoc(QStringLiteral("b.md"), QStringLiteral("# B\n"));

    w.openPathInTab(a);
    w.openPathInTab(b);
    w.closeTab(1);                       // cierra b; su ruta se recuerda
    QCOMPARE(w.m_tabs->count(), 1);

    w.reopenClosedTab();                 // debe reabrir b
    QCOMPARE(w.m_tabs->count(), 2);
    QCOMPARE(fileOf(w.m_stack), b);
}

void TestTabLifecycle::cycleTabWrapsAround()
{
    MainWindow w;
    w.show();
    w.addTab();
    w.addTab();
    QCOMPARE(w.m_tabs->count(), 3);      // pestañas 0, 1, 2

    w.m_tabs->setCurrentIndex(0);
    w.cycleTab(-1);                      // hacia atrás desde la 0 -> envuelve a la 2
    QCOMPARE(w.m_tabs->currentIndex(), 2);
    w.cycleTab(1);                       // hacia delante desde la 2 -> envuelve a la 0
    QCOMPARE(w.m_tabs->currentIndex(), 0);
}

QTEST_MAIN(TestTabLifecycle)
#include "tst_tablifecycle.moc"
