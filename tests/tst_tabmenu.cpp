#include <QtTest>

#include <QDir>
#include <QFileInfo>

#include "tabmenu.h"

// Pruebas de la lógica pura del menú contextual de pestañas (tabmenu): derivar de
// la ruta del documento qué mostrar (nombre y ruta a copiar, carpeta a abrir) y
// cuándo deshabilitar las acciones (documento «Sin título», sin archivo en disco).
class TestTabMenu : public QObject
{
    Q_OBJECT
private slots:
    void emptyPathDisablesActions();
    void filePathFillsNameAndFolder();
    void nestedPathSplitsCorrectly();
    void relativePathIsMadeAbsolute();
};

void TestTabMenu::emptyPathDisablesActions()
{
    const tabmenu::FileInfo info = tabmenu::infoForPath(QString());
    QVERIFY(!info.hasFile);
    QVERIFY(info.fileName.isEmpty());
    QVERIFY(info.fullPath.isEmpty());
    QVERIFY(info.containingFolder.isEmpty());
}

void TestTabMenu::filePathFillsNameAndFolder()
{
    // Sobre QDir::rootPath() («/» en Unix, «C:/» en Windows): infoForPath
    // normaliza a absoluta, y en Windows «/home/…» no lo es —le falta la
    // unidad—, así que QFileInfo le antepone la unidad actual («D:/home/…» en
    // el runner de CI) y la comparación literal fallaba.
    const QString root = QDir::rootPath();
    const tabmenu::FileInfo info =
        tabmenu::infoForPath(root + QStringLiteral("home/usuario/docs/apuntes.md"));
    QVERIFY(info.hasFile);
    QCOMPARE(info.fileName, QStringLiteral("apuntes.md"));
    QCOMPARE(info.fullPath, root + QStringLiteral("home/usuario/docs/apuntes.md"));
    QCOMPARE(info.containingFolder, root + QStringLiteral("home/usuario/docs"));
}

void TestTabMenu::nestedPathSplitsCorrectly()
{
    // Un nombre con puntos y espacios: el nombre es solo el último componente.
    const QString root = QDir::rootPath();
    const tabmenu::FileInfo info =
        tabmenu::infoForPath(root + QStringLiteral("a/b c/mi archivo.final.md"));
    QVERIFY(info.hasFile);
    QCOMPARE(info.fileName, QStringLiteral("mi archivo.final.md"));
    QCOMPARE(info.containingFolder, root + QStringLiteral("a/b c"));
}

void TestTabMenu::relativePathIsMadeAbsolute()
{
    // currentFile() suele ser absoluta, pero si llegara relativa se normaliza a
    // absoluta (respecto al directorio de trabajo) para que sea copiable/abrible.
    const tabmenu::FileInfo info = tabmenu::infoForPath(QStringLiteral("nota.md"));
    QVERIFY(info.hasFile);
    QCOMPARE(info.fileName, QStringLiteral("nota.md"));
    QVERIFY(QDir::isAbsolutePath(info.fullPath));
    QVERIFY(QDir::isAbsolutePath(info.containingFolder));
    QCOMPARE(info.fullPath, QDir::current().absoluteFilePath(QStringLiteral("nota.md")));
}

QTEST_MAIN(TestTabMenu)
#include "tst_tabmenu.moc"
