#include <QtTest>

#include "singleinstance.h"

#include <future>

// Pruebas de la instancia única: el protocolo (puro) y la entrega real por socket
// entre un servidor y un cliente del mismo proceso.
class TestSingleInstance : public QObject
{
    Q_OBJECT

private slots:
    void encodeDecodeRoundTrips();
    void decodeRejectsGarbage();
    void noServerMeansNoDelivery();
    void deliversPathsToTheListener();
};

void TestSingleInstance::encodeDecodeRoundTrips()
{
    const QStringList paths{QStringLiteral("/tmp/a b.md"), QStringLiteral("/tmp/ñ\nx.md")};
    QStringList out;
    QVERIFY(SingleInstance::decode(SingleInstance::encode(paths), out));
    QCOMPARE(out, paths);
    QVERIFY(SingleInstance::decode(SingleInstance::encode({}), out));
    QVERIFY(out.isEmpty());
}

void TestSingleInstance::decodeRejectsGarbage()
{
    QStringList out;
    QVERIFY(!SingleInstance::decode({}, out));
    QVERIFY(!SingleInstance::decode("basura", out));
    QByteArray truncated = SingleInstance::encode({QStringLiteral("/x.md")});
    truncated.chop(2);
    QVERIFY(!SingleInstance::decode(truncated, out));
}

void TestSingleInstance::noServerMeansNoDelivery()
{
    QVERIFY(!SingleInstance::sendToRunning(
        QStringLiteral("md-editor-test-nadie-%1").arg(QCoreApplication::applicationPid()),
        {QStringLiteral("/x.md")}, 200));
}

void TestSingleInstance::deliversPathsToTheListener()
{
    const QString name = QStringLiteral("md-editor-test-%1").arg(QCoreApplication::applicationPid());
    SingleInstance server;
    QVERIFY(server.listen(name));
    QSignalSpy spy(&server, &SingleInstance::pathsReceived);

    // El cliente bloquea (waitFor*): va en otro hilo, como en producción (otro
    // proceso), para que el bucle de eventos del servidor siga atendiendo.
    const QStringList paths{QStringLiteral("/tmp/uno.md"), QStringLiteral("/tmp/dos.md")};
    std::future<bool> sent = std::async(std::launch::async, [&] {
        return SingleInstance::sendToRunning(name, paths, 3000);
    });
    QVERIFY(spy.wait(5000));
    QCOMPARE(spy.first().first().toStringList(), paths);
    QVERIFY(sent.get());
}

QTEST_MAIN(TestSingleInstance)
#include "tst_singleinstance.moc"
