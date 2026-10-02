/// \file
/// \brief Implementación de SingleInstance.

#include "singleinstance.h"

#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QLocalSocket>
#include <QLockFile>

namespace {
constexpr quint32 kMagic = 0x4D44454Du;  // "MDEM"
constexpr int kMaxMessage = 1 << 20;     // tope defensivo: 1 MiB

QString userName()
{
    QString user = qEnvironmentVariable("USER");
    if (user.isEmpty())
        user = qEnvironmentVariable("USERNAME");
    return user;
}

/// Aplica `edit` a la lista del registro bajo un cerrojo (varias instancias pueden
/// arrancar o cerrar a la vez).
template <typename F>
void editRegistry(const QString &registry, F edit)
{
    QLockFile lock(registry + QStringLiteral(".lock"));
    lock.setStaleLockTime(5000);
    if (!lock.tryLock(2000))
        return;
    QStringList names;
    if (QFile f(registry); f.open(QIODevice::ReadOnly | QIODevice::Text))
        names = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    edit(names);
    if (QFile f(registry); f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        f.write(names.join(QLatin1Char('\n')).toUtf8());
}

enum class SendResult { Ok, NoServer, Failed };

SendResult trySend(const QString &name, const QByteArray &data, int timeoutMs)
{
    QLocalSocket socket;
    socket.connectToServer(name);
    if (!socket.waitForConnected(timeoutMs))
        return socket.error() == QLocalSocket::ServerNotFoundError
                       || socket.error() == QLocalSocket::ConnectionRefusedError
                   ? SendResult::NoServer
                   : SendResult::Failed;
    const bool ok = socket.write(data) == data.size()
                    && (socket.bytesToWrite() == 0 || socket.waitForBytesWritten(timeoutMs));
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState)
        socket.waitForDisconnected(timeoutMs);
    return ok ? SendResult::Ok : SendResult::Failed;
}
}  // namespace

SingleInstance::SingleInstance(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<SingleInstance::Message>();
    connect(&m_server, &QLocalServer::newConnection, this, &SingleInstance::onNewConnection);
}

SingleInstance::~SingleInstance()
{
    if (!m_name.isEmpty())
        editRegistry(m_registry, [this](QStringList &names) { names.removeAll(m_name); });
}

SingleInstance::LaunchArgs SingleInstance::parseArguments(const QStringList &args)
{
    LaunchArgs out;
    bool onlyPaths = false;
    for (const QString &arg : args) {
        if (arg.isEmpty())
            continue;
        if (onlyPaths || !arg.startsWith(QLatin1String("--"))) {
            out.paths << QFileInfo(arg).absoluteFilePath();
        } else if (arg == QLatin1String("--")) {
            onlyPaths = true;
        } else if (arg == QLatin1String("--new-window")) {
            out.newWindow = true;
        } else if (arg.startsWith(QLatin1String("--cursor="))) {
            bool ok = false;
            const int n = arg.mid(9).toInt(&ok);
            out.cursor = ok && n >= 0 ? n : -1;
        } else if (arg.startsWith(QLatin1String("--handoff-from="))) {
            out.handoffFrom = arg.mid(15);
        }
    }
    return out;
}

QString SingleInstance::instanceName(qint64 pid)
{
    return QStringLiteral("md-editor-%1-%2").arg(userName()).arg(pid);
}

QString SingleInstance::registryFile()
{
    return QDir::tempPath() + QStringLiteral("/md-editor-%1.instances").arg(userName());
}

QByteArray SingleInstance::encode(const Message &message)
{
    QByteArray body;
    QDataStream out(&body, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_0);
    out << kMagic << message.command << message.args;
    QByteArray framed;
    QDataStream f(&framed, QIODevice::WriteOnly);
    f.setVersion(QDataStream::Qt_6_0);
    f << quint32(body.size());
    framed.append(body);
    return framed;
}

bool SingleInstance::decode(const QByteArray &data, Message &message)
{
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_6_0);
    quint32 size = 0;
    in >> size;
    if (in.status() != QDataStream::Ok || size > quint32(kMaxMessage)
        || data.size() != qsizetype(sizeof(quint32)) + qsizetype(size))
        return false;
    quint32 magic = 0;
    Message m;
    in >> magic >> m.command >> m.args;
    if (in.status() != QDataStream::Ok || magic != kMagic || !in.atEnd())
        return false;
    message = m;
    return true;
}

QStringList SingleInstance::registeredNames(const QString &registry)
{
    QStringList names;
    if (QFile f(registry); f.open(QIODevice::ReadOnly | QIODevice::Text))
        names = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    return names;
}

bool SingleInstance::sendTo(const QString &name, const Message &message, int timeoutMs)
{
    return trySend(name, encode(message), timeoutMs) == SendResult::Ok;
}

bool SingleInstance::sendToLatest(const Message &message, const QString &registry, int timeoutMs)
{
    const QByteArray data = encode(message);
    QStringList dead;
    bool delivered = false;
    const QStringList names = registeredNames(registry);
    for (auto it = names.crbegin(); it != names.crend() && !delivered; ++it) {
        switch (trySend(*it, data, timeoutMs)) {
        case SendResult::Ok:
            delivered = true;
            break;
        case SendResult::NoServer:
            dead << *it;  // solo se retira la que no existe; una lenta sigue viva
            break;
        case SendResult::Failed:
            break;
        }
    }
    if (!dead.isEmpty())
        editRegistry(registry, [&dead](QStringList &list) {
            for (const QString &n : dead)
                list.removeAll(n);
        });
    return delivered;
}

bool SingleInstance::listen(const QString &name, const QString &registry)
{
    if (!m_server.listen(name)) {
        // Un cierre anómalo deja el fichero del socket: sin servidor detrás, se retira.
        QLocalServer::removeServer(name);
        if (!m_server.listen(name))
            return false;
    }
    m_name = name;
    m_registry = registry;
    editRegistry(registry, [&name](QStringList &names) {
        names.removeAll(name);
        names << name;  // la última creada va al final
    });
    return true;
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
            Message message;
            if (decode(*buffer, message)) {
                buffer->clear();
                emit messageReceived(message);
            } else if (buffer->size() > kMaxMessage) {
                socket->abort();
            }
        };
        connect(socket, &QLocalSocket::readyRead, socket, tryDecode);
        tryDecode();  // por si ya llegó antes de conectar la señal
    }
}
