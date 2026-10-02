#include <QtTest>

#include "singleinstance.h"

#include <future>

// Pruebas de la coordinación entre instancias: el protocolo y la línea de comandos
// (puros), el registro por orden de creación y la entrega real por socket entre
// servidores y clientes del mismo proceso.
class TestSingleInstance : public QObject
{
    Q_OBJECT

private slots:
    void encodeDecodeRoundTrips();
    void decodeRejectsGarbage();
    void parsesCommandLine();
    void noInstanceMeansNoDelivery();
    void deliversToTheLatestInstance();
    void fallsBackWhenTheLatestCloses();
    void prunesDeadEntries();
    void adoptedReachesTheOrigin();
    void detectsOtherLiveInstances();
    void registryIsPrivateToTheUser();
    void openFilesGoToTheirOwner();
    void asyncSendDoesNotBlockAndDelivers();
    void hungInstancesDoNotMultiplyTheWait();
    void nothingOpenGoesToTheLatest();
    void noInstancesLeavesThePathsToTheCaller();

private:
    static QString uniq(const char *tag)
    {
        return QStringLiteral("md-editor-test-%1-%2").arg(QLatin1String(tag)).arg(
            QCoreApplication::applicationPid());
    }
    QString registry() const { return m_dir.filePath(QStringLiteral("reg.instances")); }
    QTemporaryDir m_dir;
};

void TestSingleInstance::encodeDecodeRoundTrips()
{
    const SingleInstance::Message in{SingleInstance::kOpen,
                                     {QStringLiteral("/tmp/a b.md"), QStringLiteral("/tmp/ñ\nx.md")}};
    SingleInstance::Message out;
    QVERIFY(SingleInstance::decode(SingleInstance::encode(in), out));
    QCOMPARE(out.command, in.command);
    QCOMPARE(out.args, in.args);
    QVERIFY(SingleInstance::decode(SingleInstance::encode({SingleInstance::kOpen, {}}), out));
    QVERIFY(out.args.isEmpty());
}

void TestSingleInstance::decodeRejectsGarbage()
{
    SingleInstance::Message out;
    QVERIFY(!SingleInstance::decode({}, out));
    QVERIFY(!SingleInstance::decode("basura", out));
    QByteArray truncated = SingleInstance::encode({SingleInstance::kOpen, {QStringLiteral("/x.md")}});
    truncated.chop(2);
    QVERIFY(!SingleInstance::decode(truncated, out));
}

void TestSingleInstance::parsesCommandLine()
{
    const QString root = QDir::rootPath();
    auto a = SingleInstance::parseArguments(
        {QStringLiteral("--new-window"), QStringLiteral("--cursor=42"),
         QStringLiteral("--handoff-from=md-editor-x-1"), root + QStringLiteral("a.md"),
         QStringLiteral("--desconocida"), root + QStringLiteral("b.md")});
    QVERIFY(a.newWindow);
    QCOMPARE(a.cursor, 42);
    QCOMPARE(a.handoffFrom, QStringLiteral("md-editor-x-1"));
    QCOMPARE(a.paths, (QStringList{root + QStringLiteral("a.md"), root + QStringLiteral("b.md")}));

    // Relativa → absoluta; tras «--» todo son rutas; cursor inválido → -1.
    a = SingleInstance::parseArguments(
        {QStringLiteral("--cursor=zz"), QStringLiteral("--"), QStringLiteral("--raro.md")});
    QCOMPARE(a.cursor, -1);
    QVERIFY(!a.newWindow);
    QCOMPARE(a.paths.size(), 1);
    QVERIFY(QFileInfo(a.paths.first()).isAbsolute());
    QVERIFY(a.paths.first().endsWith(QStringLiteral("--raro.md")));
}

void TestSingleInstance::noInstanceMeansNoDelivery()
{
    QVERIFY(!SingleInstance::sendToLatest({SingleInstance::kOpen, {}}, registry(), 200));
    QVERIFY(!SingleInstance::sendTo(uniq("nadie"), {SingleInstance::kOpen, {}}, 200));
}

void TestSingleInstance::deliversToTheLatestInstance()
{
    SingleInstance first, second;
    QVERIFY(first.listen(uniq("a"), registry()));
    QVERIFY(second.listen(uniq("b"), registry()));
    QCOMPARE(SingleInstance::registeredNames(registry()), (QStringList{uniq("a"), uniq("b")}));
    QSignalSpy spyFirst(&first, &SingleInstance::messageReceived);
    QSignalSpy spySecond(&second, &SingleInstance::messageReceived);

    // El cliente bloquea (waitFor*): va en otro hilo, como en producción (otro
    // proceso), para que el bucle de eventos de los servidores siga atendiendo.
    const QStringList paths{QStringLiteral("/tmp/uno.md"), QStringLiteral("/tmp/dos.md")};
    auto sent = std::async(std::launch::async, [&] {
        return SingleInstance::sendToLatest({SingleInstance::kOpen, paths}, registry(), 3000);
    });
    QVERIFY(spySecond.wait(5000));
    QVERIFY(sent.get());
    QCOMPARE(spySecond.first().first().value<SingleInstance::Message>().args, paths);
    QCOMPARE(spyFirst.count(), 0);
}

void TestSingleInstance::fallsBackWhenTheLatestCloses()
{
    SingleInstance first;
    QVERIFY(first.listen(uniq("c"), registry()));
    {
        SingleInstance second;
        QVERIFY(second.listen(uniq("d"), registry()));
    }  // al cerrar se da de baja
    QCOMPARE(SingleInstance::registeredNames(registry()), (QStringList{uniq("c")}));
    QSignalSpy spy(&first, &SingleInstance::messageReceived);
    auto sent = std::async(std::launch::async, [&] {
        return SingleInstance::sendToLatest({SingleInstance::kOpen, {}}, registry(), 3000);
    });
    QVERIFY(spy.wait(5000));
    QVERIFY(sent.get());
}

void TestSingleInstance::prunesDeadEntries()
{
    // Una entrada huérfana (cierre anómalo) al final: se salta, se retira del
    // registro y la entrega llega a la anterior.
    SingleInstance live;
    QVERIFY(live.listen(uniq("e"), registry()));
    QFile f(registry());
    QVERIFY(f.open(QIODevice::Append | QIODevice::Text));
    f.write(("\n" + uniq("muerta")).toUtf8());
    f.close();
    QSignalSpy spy(&live, &SingleInstance::messageReceived);
    auto sent = std::async(std::launch::async, [&] {
        return SingleInstance::sendToLatest({SingleInstance::kOpen, {}}, registry(), 3000);
    });
    QVERIFY(spy.wait(5000));
    QVERIFY(sent.get());
    QCOMPARE(SingleInstance::registeredNames(registry()), (QStringList{uniq("e")}));
}

void TestSingleInstance::adoptedReachesTheOrigin()
{
    SingleInstance origin;
    QVERIFY(origin.listen(uniq("f"), registry()));
    QSignalSpy spy(&origin, &SingleInstance::messageReceived);
    auto sent = std::async(std::launch::async, [&] {
        return SingleInstance::sendTo(origin.name(),
                                      {SingleInstance::kAdopted, {QStringLiteral("/x.md")}}, 3000);
    });
    QVERIFY(spy.wait(5000));
    QVERIFY(sent.get());
    const auto msg = spy.first().first().value<SingleInstance::Message>();
    QCOMPARE(msg.command, SingleInstance::kAdopted);
    QCOMPARE(msg.args, (QStringList{QStringLiteral("/x.md")}));
}

void TestSingleInstance::detectsOtherLiveInstances()
{
    SingleInstance me, other;
    QVERIFY(me.listen(uniq("g"), registry()));
    // Solo yo (más una entrada huérfana de un cierre anómalo): no hay otra viva.
    QFile f(registry());
    QVERIFY(f.open(QIODevice::Append | QIODevice::Text));
    f.write(("\n" + uniq("huerfana")).toUtf8());
    f.close();
    QVERIFY(!SingleInstance::otherInstancesAlive(me.name(), registry(), 300));
    QCOMPARE(SingleInstance::registeredNames(registry()), (QStringList{me.name()}));  // y se retira
    // Con otra viva, sí. (La conexión se acepta a nivel de sistema: no hace falta bucle.)
    QVERIFY(other.listen(uniq("h"), registry()));
    QVERIFY(SingleInstance::otherInstancesAlive(me.name(), registry(), 300));
    // Registro inutilizable (vacío): sin otras, sin error.
    QVERIFY(!SingleInstance::otherInstancesAlive(me.name(), QString(), 100));
}

void TestSingleInstance::registryIsPrivateToTheUser()
{
    const QString file = SingleInstance::registryFile();
    if (file.isEmpty())
        QSKIP("sin directorio privado en este entorno (válido: no hay registro)");
    // Los bits de grupo/otros son semántica POSIX: en Windows los permisos van por ACL y
    // Qt informa todos los bits aunque el directorio sea del usuario.
    if (QSysInfo::kernelType() == QLatin1String("winnt"))
        QSKIP("permisos POSIX: no aplican en Windows");
    const QFileInfo dir(QFileInfo(file).absolutePath());
    QVERIFY(dir.isDir());
    QVERIFY(!(dir.permissions() & (QFileDevice::ReadGroup | QFileDevice::WriteGroup
                                   | QFileDevice::ReadOther | QFileDevice::WriteOther)));
}

void TestSingleInstance::openFilesGoToTheirOwner()
{
    // A (antigua) ya tiene /a.md abierto; B es la última. Pedir /a.md y /b.md: el
    // primero va a A (no se duplica en B) y el segundo a la última.
    SingleInstance a, b;
    QVERIFY(a.listen(uniq("i"), registry()));
    QVERIFY(b.listen(uniq("j"), registry()));
    a.setOpenFilesProvider([](const QStringList &p) {
        return p.contains(QStringLiteral("/a.md")) ? QStringList{QStringLiteral("/a.md")} : QStringList();
    });
    QSignalSpy spyA(&a, &SingleInstance::messageReceived);
    QSignalSpy spyB(&b, &SingleInstance::messageReceived);
    auto done = std::async(std::launch::async, [&] {
        return SingleInstance::deliverPaths({QStringLiteral("/a.md"), QStringLiteral("/b.md")},
                                            nullptr, registry(), 3000);
    });
    QVERIFY(spyA.wait(5000));
    QVERIFY(spyB.count() > 0 || spyB.wait(5000));
    QVERIFY(done.get());
    QCOMPARE(spyA.first().first().value<SingleInstance::Message>().args,
             QStringList{QStringLiteral("/a.md")});
    QCOMPARE(spyB.first().first().value<SingleInstance::Message>().args,
             QStringList{QStringLiteral("/b.md")});
}

void TestSingleInstance::nothingOpenGoesToTheLatest()
{
    SingleInstance a, b;
    QVERIFY(a.listen(uniq("k"), registry()));
    QVERIFY(b.listen(uniq("l"), registry()));
    QSignalSpy spyA(&a, &SingleInstance::messageReceived);
    QSignalSpy spyB(&b, &SingleInstance::messageReceived);
    auto done = std::async(std::launch::async, [&] {
        return SingleInstance::deliverPaths({QStringLiteral("/n.md")}, nullptr, registry(), 3000);
    });
    QVERIFY(spyB.wait(5000));
    QVERIFY(done.get());
    QCOMPARE(spyA.count(), 0);
}

void TestSingleInstance::noInstancesLeavesThePathsToTheCaller()
{
    QStringList leftover;
    QVERIFY(!SingleInstance::deliverPaths({QStringLiteral("/x.md"), QStringLiteral("/y.md")},
                                          &leftover, registry(), 200));
    QCOMPARE(leftover, (QStringList{QStringLiteral("/x.md"), QStringLiteral("/y.md")}));
}

void TestSingleInstance::asyncSendDoesNotBlockAndDelivers()
{
    SingleInstance server;
    QVERIFY(server.listen(uniq("m"), registry()));
    QSignalSpy spy(&server, &SingleInstance::messageReceived);
    QElapsedTimer t;
    t.start();
    SingleInstance::sendAsync(server.name(), {SingleInstance::kAdopted, {QStringLiteral("/q.md")}});
    QVERIFY(t.elapsed() < 100);  // vuelve ya, sin esperar al servidor (que está en ESTE hilo)
    QVERIFY(spy.wait(5000));
    QCOMPARE(spy.first().first().value<SingleInstance::Message>().args,
             QStringList{QStringLiteral("/q.md")});
}

void TestSingleInstance::hungInstancesDoNotMultiplyTheWait()
{
    // Dos instancias que aceptan la conexión pero no contestan (su bucle no corre
    // mientras este hilo duerme): el reparto del presupuesto acota la espera total.
    SingleInstance a, b;
    QVERIFY(a.listen(uniq("n"), registry()));
    QVERIFY(b.listen(uniq("o"), registry()));
    QElapsedTimer t;
    t.start();
    auto done = std::async(std::launch::async, [&] {
        QStringList leftover;
        const bool ok = SingleInstance::deliverPaths({QStringLiteral("/z.md")}, &leftover,
                                                     registry(), 800);
        return std::make_pair(ok, t.elapsed());
    });
    QTest::qSleep(2500);  // el hilo principal no atiende a nadie
    const auto [ok, elapsed] = done.get();
    Q_UNUSED(ok)
    // Con 2 instancias a 1,5 s por paso eran >4 s; ahora ronda el presupuesto.
    QVERIFY2(elapsed < 1800, qPrintable(QString::number(elapsed)));
}

QTEST_MAIN(TestSingleInstance)
#include "tst_singleinstance.moc"
