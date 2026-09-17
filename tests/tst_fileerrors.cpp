#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include "fileerrors.h"

// Pruebas de las pistas de error de archivo: cada causa real tiene que dar un
// remedio distinto, no el mismo «no se pudo» para todo.
class TestFileErrors : public QObject
{
    Q_OBJECT

private slots:
    void readMissingFileSaysItMoved();
    void readDirectorySaysItIsAFolder();
    void writeIntoMissingFolderNamesTheFolder();
    void writeReadOnlyFileSuggestsAnotherName();
    void writeReadOnlyFolderSuggestsAnotherFolder();
    void hintsAreDifferentPerCause();
};

void TestFileErrors::readMissingFileSaysItMoved()
{
    QTemporaryDir dir;
    const QString hint = mdfileerr::hint(dir.filePath(QStringLiteral("no.md")),
                                         mdfileerr::Op::Read);
    QVERIFY(hint.contains(QStringLiteral("movido")));
}

void TestFileErrors::readDirectorySaysItIsAFolder()
{
    QTemporaryDir dir;
    QVERIFY(mdfileerr::hint(dir.path(), mdfileerr::Op::Read).contains(QStringLiteral("carpeta")));
}

void TestFileErrors::writeIntoMissingFolderNamesTheFolder()
{
    QTemporaryDir dir;
    const QString folder = dir.filePath(QStringLiteral("desaparecida"));
    const QString hint = mdfileerr::hint(folder + QStringLiteral("/a.md"), mdfileerr::Op::Write);
    QVERIFY(hint.contains(QDir::toNativeSeparators(folder)));
    QVERIFY(hint.contains(QStringLiteral("ya no existe")));
}

void TestFileErrors::writeReadOnlyFileSuggestsAnotherName()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("a.md"));
    QFile f(path);
    QVERIFY(f.open(QIODevice::WriteOnly));
    f.close();
    QVERIFY(QFile::setPermissions(path, QFileDevice::ReadOwner));
    if (QFileInfo(path).isWritable())
        QSKIP("Se ejecuta como root: los permisos no impiden escribir");
    QVERIFY(mdfileerr::hint(path, mdfileerr::Op::Write).contains(QStringLiteral("solo lectura")));
}

void TestFileErrors::writeReadOnlyFolderSuggestsAnotherFolder()
{
    QTemporaryDir dir;
    const QString folder = dir.filePath(QStringLiteral("ro"));
    QVERIFY(QDir().mkpath(folder));
    QVERIFY(QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::ExeOwner));
    const bool writable = QFileInfo(folder).isWritable();
    const QString hint = mdfileerr::hint(folder + QStringLiteral("/a.md"), mdfileerr::Op::Write);
    QFile::setPermissions(folder, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                      | QFileDevice::ExeOwner);  // que se pueda borrar
    if (writable)
        QSKIP("Se ejecuta como root: los permisos no impiden escribir");
    QVERIFY(hint.contains(QStringLiteral("permiso")));
}

void TestFileErrors::hintsAreDifferentPerCause()
{
    QTemporaryDir dir;
    const QString missing = mdfileerr::hint(dir.filePath(QStringLiteral("no.md")),
                                            mdfileerr::Op::Read);
    const QString folder = mdfileerr::hint(dir.path(), mdfileerr::Op::Read);
    const QString generic = mdfileerr::hint(dir.filePath(QStringLiteral("nuevo.md")),
                                            mdfileerr::Op::Write);
    QVERIFY(missing != folder);
    QVERIFY(folder != generic);
    QVERIFY(!generic.isEmpty());
}

QTEST_MAIN(TestFileErrors)
#include "tst_fileerrors.moc"
