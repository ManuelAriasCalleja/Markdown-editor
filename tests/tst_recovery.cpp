#include <QtTest>

#include <QStandardPaths>

#include "recoverymanager.h"

// Pruebas del gestor de recuperación. QStandardPaths en modo de prueba redirige
// AppDataLocation a un directorio temporal, así que no se tocan datos reales.
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
};

void TestRecovery::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
}

void TestRecovery::cleanup()
{
    RecoveryManager().clearDraft();  // cada prueba parte sin borrador
}

void TestRecovery::noDraftInitially()
{
    QVERIFY(!RecoveryManager().hasDraft());
    QVERIFY(RecoveryManager().draftBody().isEmpty());
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

    // Una instancia nueva ve el mismo borrador (persistido en disco).
    RecoveryManager other;
    QVERIFY(other.hasDraft());
    QCOMPARE(other.draftBody(), QStringLiteral("# Título\n\nlínea con acentos: ñ á\n"));
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
}

QTEST_GUILESS_MAIN(TestRecovery)
#include "tst_recovery.moc"
