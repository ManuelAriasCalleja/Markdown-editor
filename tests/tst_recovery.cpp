#include <QtTest>

#include <QStandardPaths>

#include "recoverymanager.h"

// Pruebas del gestor de recuperación. QStandardPaths en modo de prueba redirige
// AppDataLocation a un directorio temporal, así que no se tocan datos reales.
// Cada documento (pestaña) tiene su propio borrador con un slot único; los
// borradores huérfanos se enumeran con leftoverDrafts() al arrancar.
class TestRecovery : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void noDraftInitially();
    void saveCreatesDraft();
    void bodyAndOriginalRoundTrip();
    void emptyOriginalMeansUntitled();
    void clearRemovesDraft();
    void saveOverwritesPreviousDraft();
    void instancesAreIsolated();
    void leftoverDraftsEnumeratesAll();
    void removeDraftDeletesFiles();
};

void TestRecovery::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestRecovery::cleanup()
{
    // Cada prueba parte sin borradores: borra los de TODAS las instancias.
    for (const RecoveryManager::Draft &d : RecoveryManager::leftoverDrafts())
        RecoveryManager::removeDraft(d);
}

void TestRecovery::noDraftInitially()
{
    QVERIFY(!RecoveryManager().hasDraft());
    QVERIFY(RecoveryManager().draftBody().isEmpty());
    QVERIFY(RecoveryManager::leftoverDrafts().isEmpty());
}

void TestRecovery::saveCreatesDraft()
{
    RecoveryManager r;
    QVERIFY(!r.hasDraft());
    r.saveDraft(QStringLiteral("/tmp/doc.md"), QStringLiteral("# Hola"));
    QVERIFY(r.hasDraft());
}

void TestRecovery::bodyAndOriginalRoundTrip()
{
    RecoveryManager r;
    r.saveDraft(QStringLiteral("/home/u/doc.md"),
                QStringLiteral("# Título\n\nlínea con acentos: ñ á\n"));
    QCOMPARE(r.draftOriginalPath(), QStringLiteral("/home/u/doc.md"));
    QCOMPARE(r.draftBody(), QStringLiteral("# Título\n\nlínea con acentos: ñ á\n"));

    // Persistido en disco: leftoverDrafts (estático) lo encuentra de otra sesión.
    const QList<RecoveryManager::Draft> drafts = RecoveryManager::leftoverDrafts();
    QCOMPARE(drafts.size(), 1);
    QCOMPARE(drafts.first().originalPath, QStringLiteral("/home/u/doc.md"));
    QCOMPARE(drafts.first().body, QStringLiteral("# Título\n\nlínea con acentos: ñ á\n"));
}

void TestRecovery::emptyOriginalMeansUntitled()
{
    RecoveryManager r;
    r.saveDraft(QString(), QStringLiteral("texto sin título"));
    QVERIFY(r.hasDraft());
    QVERIFY(r.draftOriginalPath().isEmpty());
    QCOMPARE(r.draftBody(), QStringLiteral("texto sin título"));
}

void TestRecovery::clearRemovesDraft()
{
    RecoveryManager r;
    r.saveDraft(QStringLiteral("/tmp/x.md"), QStringLiteral("contenido"));
    QVERIFY(r.hasDraft());
    r.clearDraft();
    QVERIFY(!r.hasDraft());
    QVERIFY(r.draftBody().isEmpty());
    QVERIFY(r.draftOriginalPath().isEmpty());
}

void TestRecovery::saveOverwritesPreviousDraft()
{
    RecoveryManager r;
    r.saveDraft(QStringLiteral("/a.md"), QStringLiteral("primero"));
    r.saveDraft(QStringLiteral("/b.md"), QStringLiteral("segundo"));
    QCOMPARE(r.draftOriginalPath(), QStringLiteral("/b.md"));
    QCOMPARE(r.draftBody(), QStringLiteral("segundo"));
    // Una sola instancia = un solo borrador (lo sobreescribe, no acumula).
    QCOMPARE(RecoveryManager::leftoverDrafts().size(), 1);
}

void TestRecovery::instancesAreIsolated()
{
    RecoveryManager a;
    RecoveryManager b;
    a.saveDraft(QStringLiteral("/a.md"), QStringLiteral("doc A"));
    b.saveDraft(QStringLiteral("/b.md"), QStringLiteral("doc B"));
    // Cada instancia ve SOLO su propio borrador (no se pisan).
    QCOMPARE(a.draftBody(), QStringLiteral("doc A"));
    QCOMPARE(b.draftBody(), QStringLiteral("doc B"));
    // Borrar el de una no afecta al de la otra.
    a.clearDraft();
    QVERIFY(!a.hasDraft());
    QVERIFY(b.hasDraft());
    QCOMPARE(b.draftBody(), QStringLiteral("doc B"));
}

void TestRecovery::leftoverDraftsEnumeratesAll()
{
    RecoveryManager a, b, c;
    a.saveDraft(QStringLiteral("/uno.md"), QStringLiteral("uno"));
    b.saveDraft(QString(), QStringLiteral("dos sin título"));
    c.saveDraft(QStringLiteral("/tres.md"), QStringLiteral("tres"));

    const QList<RecoveryManager::Draft> drafts = RecoveryManager::leftoverDrafts();
    QCOMPARE(drafts.size(), 3);
    // Recolectamos los cuerpos para no depender del orden exacto.
    QStringList bodies;
    for (const RecoveryManager::Draft &d : drafts)
        bodies << d.body;
    QVERIFY(bodies.contains(QStringLiteral("uno")));
    QVERIFY(bodies.contains(QStringLiteral("dos sin título")));
    QVERIFY(bodies.contains(QStringLiteral("tres")));
}

void TestRecovery::removeDraftDeletesFiles()
{
    RecoveryManager a, b;
    a.saveDraft(QStringLiteral("/a.md"), QStringLiteral("A"));
    b.saveDraft(QStringLiteral("/b.md"), QStringLiteral("B"));
    QList<RecoveryManager::Draft> drafts = RecoveryManager::leftoverDrafts();
    QCOMPARE(drafts.size(), 2);
    for (const RecoveryManager::Draft &d : drafts)
        RecoveryManager::removeDraft(d);
    QVERIFY(RecoveryManager::leftoverDrafts().isEmpty());
    QVERIFY(!a.hasDraft());
    QVERIFY(!b.hasDraft());
}

QTEST_GUILESS_MAIN(TestRecovery)
#include "tst_recovery.moc"
