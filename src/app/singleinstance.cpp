/// \file
/// \brief Implementación de SingleInstance.

#include "singleinstance.h"

#include <QDataStream>
#include <QIODevice>
#include <QLocalSocket>

namespace {
constexpr quint32 kMagic = 0x4D44454Du;  // "MDEM"
constexpr int kMaxMessage = 1 << 20;     // tope defensivo: 1 MiB
}

SingleInstance::SingleInstance(QObject *parent) : QObject(parent)
{
    connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
}

QString SingleInstance::serverName()
{
    QString user = qEnvironmentVariable("USER");
    if (user.isEmpty())
        user = qEnvironmentVariable("USERNAME");
    return QStringLiteral("md-editor-") + user;
}

QByteArray SingleInstance::encode(const QStringList &paths)
{
    QByteArray body;
    QDataStream out(&body, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << kMagic << paths;
    QByteArray framed;
    QDataStream f(&framed, QIODevice::WriteOnly);
    f.setVersion(QDataStream::Qt_6_0);
    f << quint32(body.size());
    framed.append(body);
    return framed;
}

bool SingleInstance::decode(const QByteArray &data, QStringList &paths)
{
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_6_0);
    quint32 size = 0;
    in >> size;
    if (in.status() != QDataStream::Ok || size > quint32(kMaxMessage)
        || data.size() != qsizetype(sizeof(quint32)) + qsizetype(size))
        return false;
    quint32 magic = 0;
    QStringList list;
    in >> magic >> list;
    if (in.status() != QDataStream::Ok || magic != kMagic || !in.atEnd())
        return false;
    paths = list;
    return true;
}

bool SingleInstance::sendToRunning(const QString &name, const QStringList &paths, int timeoutMs)
{
    QLocalSocket socket;
    socket.connectToServer(name);
    if (!socket.waitForConnected(timeoutMs))
        return false;
    const QByteArray data = encode(paths);
    const bool ok = socket.write(data) == data.size()
                    && (socket.bytesToWrite() == 0 || socket.waitForBytesWritten(timeoutMs));
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState)
        socket.waitForDisconnected(timeoutMs);
    return ok;
}

bool SingleInstance::listen(const QString &name)
{
    if (m_server.listen(name))
        return true;
    // Un cierre anómalo deja el fichero del socket: sin servidor detrás, se retira.
    QLocalServer::removeServer(name);
    return m_server.listen(name);
}

void SingleInstance::onNewConnection()
{
    while (QLocalSocket *socket = m_server.nextPendingConnection()) {
        connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
        // El mensaje es pequeño y se escribe de una vez, pero puede llegar troceado:
        // se acumula hasta tenerlo completo (o hasta que el cliente cierre).
        auto *buffer = new QByteArray;
        connect(socket, &QObject::destroyed, socket, [buffer] { delete buffer; });
        auto tryDecode = [this, socket, buffer] {
            buffer->append(socket->readAll());
            QStringList paths;
            if (decode(*buffer, paths)) {
                buffer->clear();
                emit pathsReceived(paths);
            } else if (buffer->size() > kMaxMessage) {
                socket->abort();
            }
        };
        connect(socket, &QLocalSocket::readyRead, socket, tryDecode);
        tryDecode();  // por si ya llegó antes de conectar la señal
    }
}
