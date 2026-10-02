/// \file
/// \brief Implementación de SingleInstance.

#include "singleinstance.h"

#include <QDataStream>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>
#include <QTimer>

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
    if (registry.isEmpty())
        return;
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

/// Carpeta privada del usuario para el registro, o vacío si no hay garantías.
QString privateRegistryDir()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (!dir.isEmpty() && QFileInfo(dir).isDir() && QFileInfo(dir).isWritable())
        return dir;
    // Sin directorio de ejecución por usuario (otros sistemas, o un entorno mínimo): una
    // carpeta propia bajo la temporal, que es compartida. Solo vale si la creamos
    // nosotros, o ya era nuestra, y sin acceso para nadie más.
    dir = QDir::tempPath() + QStringLiteral("/md-editor-%1").arg(userName());
    if (!QDir().mkpath(dir))
        return {};
    const QFileInfo info(dir);
    if (info.isSymLink() || !info.isDir() || info.owner() != QFileInfo(QDir::homePath()).owner())
        return {};
    QFile::setPermissions(dir, QFileDevice::ReadOwner | QFileDevice::WriteOwner
                                   | QFileDevice::ExeOwner);
    return dir;
}

// Tope por instancia: una que no contesta (colgada, parada en un depurador) no debe
// gastar el presupuesto entero. Una viva contesta en milisegundos.
constexpr int kPerInstanceMs = 500;

SendResult trySend(const QString &name, const QByteArray &data, int timeoutMs)
{
    // `timeoutMs` es el total: cada espera usa lo que queda, no el valor entero (antes
    // conectar, escribir y cerrar podían sumar el triple).
    QElapsedTimer clock;
    clock.start();
    const auto left = [&] { return qMax(1, timeoutMs - int(clock.elapsed())); };
    QLocalSocket socket;
    socket.connectToServer(name);
    if (!socket.waitForConnected(left()))
        return socket.error() == QLocalSocket::ServerNotFoundError
                       || socket.error() == QLocalSocket::ConnectionRefusedError
                   ? SendResult::NoServer
                   : SendResult::Failed;
    const bool ok = socket.write(data) == data.size()
                    && (socket.bytesToWrite() == 0 || socket.waitForBytesWritten(left()));
    socket.disconnectFromServer();
    if (socket.state() != QLocalSocket::UnconnectedState)
        socket.waitForDisconnected(left());
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
    const QString dir = privateRegistryDir();
    return dir.isEmpty() ? QString() : dir + QStringLiteral("/md-editor.instances");
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
    if (registry.isEmpty())
        return names;
    if (QFile f(registry); f.open(QIODevice::ReadOnly | QIODevice::Text))
        names = QString::fromUtf8(f.readAll()).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    return names;
}

bool SingleInstance::otherInstancesAlive(const QString &self, const QString &registry,
                                         int timeoutMs)
{
    QStringList dead;
    bool alive = false;
    for (const QString &name : registeredNames(registry)) {
        if (name == self)
            continue;
        QLocalSocket socket;
        socket.connectToServer(name);
        if (socket.waitForConnected(timeoutMs))
            alive = true;
        else if (socket.error() == QLocalSocket::ServerNotFoundError
                 || socket.error() == QLocalSocket::ConnectionRefusedError)
            dead << name;
        socket.abort();
    }
    if (!dead.isEmpty())
        editRegistry(registry, [&dead](QStringList &list) {
            for (const QString &n : dead)
                list.removeAll(n);
        });
    return alive;
}

void SingleInstance::sendAsync(const QString &name, const Message &message, int timeoutMs)
{
    auto *socket = new QLocalSocket(QCoreApplication::instance());
    const QByteArray data = encode(message);
    connect(socket, &QLocalSocket::connected, socket, [socket, data] {
        socket->write(data);
        socket->disconnectFromServer();  // espera a escribir lo pendiente antes de cerrar
    });
    connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    connect(socket, &QLocalSocket::errorOccurred, socket, &QObject::deleteLater);
    QTimer::singleShot(timeoutMs, socket, &QObject::deleteLater);  // por si nunca contesta
    socket->connectToServer(name);
}

bool SingleInstance::queryOpen(const QString &name, const QStringList &paths, QStringList &open,
                               int timeoutMs)
{
    QElapsedTimer clock;
    clock.start();
    const auto left = [&] { return qMax(1, timeoutMs - int(clock.elapsed())); };
    QLocalSocket socket;
    socket.connectToServer(name);
    if (!socket.waitForConnected(left()))
        return false;
    const QByteArray data = encode({kQueryOpen, paths});
    if (socket.write(data) != data.size()
        || (socket.bytesToWrite() != 0 && !socket.waitForBytesWritten(left())))
        return false;
    // La respuesta puede llegar troceada: se acumula hasta que decodifica.
    QByteArray buffer;
    Message reply;
    while (!decode(buffer, reply)) {
        if (clock.elapsed() >= timeoutMs || !socket.waitForReadyRead(left()))
            return false;
        buffer.append(socket.readAll());
        if (buffer.size() > kMaxMessage)
            return false;
    }
    if (reply.command != kReply)
        return false;
    open = reply.args;
    return true;
}

bool SingleInstance::deliverPaths(const QStringList &paths, QStringList *leftover,
                                  const QString &registry, int timeoutMs)
{
    QElapsedTimer clock;
    clock.start();
    // Lo que queda del presupuesto, acotado por instancia.
    const auto slice = [&] { return qBound(1, timeoutMs - int(clock.elapsed()), kPerInstanceMs); };
    QStringList remaining = paths;
    const QStringList names = registeredNames(registry);
    for (auto it = names.crbegin();
         it != names.crend() && !remaining.isEmpty() && clock.elapsed() < timeoutMs; ++it) {
        QStringList owned;
        if (!queryOpen(*it, remaining, owned, slice()) || owned.isEmpty())
            continue;
        owned.removeIf([&remaining](const QString &p) { return !remaining.contains(p); });
        if (owned.isEmpty() || !sendTo(*it, {kOpen, owned}, slice()))
            continue;
        for (const QString &p : std::as_const(owned))
            remaining.removeAll(p);
    }
    // Todo lo pedido ya lo tenía alguien: esa instancia lo trajo al frente.
    if (remaining.isEmpty() && !paths.isEmpty())
        return true;
    const bool ok = sendToLatest({kOpen, remaining}, registry, qMax(100, timeoutMs - int(clock.elapsed())));
    if (!ok && leftover)
        *leftover = remaining;
    return ok;
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
    QElapsedTimer clock;
    clock.start();
    const QStringList names = registeredNames(registry);
    for (auto it = names.crbegin();
         it != names.crend() && !delivered && clock.elapsed() < timeoutMs; ++it) {
        const int slice = qBound(1, timeoutMs - int(clock.elapsed()), kPerInstanceMs);
        switch (trySend(*it, data, slice)) {
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
    // El socket, solo para el usuario: por defecto otros usuarios podrían conectarse.
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
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
                if (message.command == kQueryOpen) {
                    // Pregunta: se contesta por este mismo socket y no sube a nadie más.
                    const QStringList open = m_openFiles ? m_openFiles(message.args) : QStringList();
                    socket->write(encode({kReply, open}));
                    socket->flush();
                    return;
                }
                emit messageReceived(message);
            } else if (buffer->size() > kMaxMessage) {
                socket->abort();
            }
        };
        connect(socket, &QLocalSocket::readyRead, socket, tryDecode);
        tryDecode();  // por si ya llegó antes de conectar la señal
    }
}
