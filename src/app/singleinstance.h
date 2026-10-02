/// \file
/// \brief Varias instancias coordinadas: entregar archivos a la última y traspasar pestañas.
#pragma once

#include <QByteArray>
#include <QLocalServer>
#include <QObject>
#include <QString>
#include <QStringList>

/// \brief Servidor local de cada instancia, cliente de las demás y registro compartido.
///
/// Cada instancia escucha en un socket propio (`instanceName`) y se anota, por orden
/// de creación, en un fichero de registro. Al abrir un `.md` desde el explorador, el
/// proceso nuevo no construye ventana: se lo entrega a **la última instancia creada**
/// (`sendToLatest`) y sale. «Abrir en una nueva ventana» lanza una instancia más
/// (`--new-window`), que pasa a ser la última y avisa a la de origen (`kAdopted`) para
/// que cierre la pestaña traspasada.
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    /// \brief Mensaje entre instancias: una orden y sus argumentos.
    struct Message {
        QString command;
        QStringList args;
    };
    /// Abrir `args` en pestañas y traer la ventana al frente (sin argumentos, solo al frente).
    static inline const QString kOpen = QStringLiteral("open");
    /// El documento `args[0]` ya vive en otra ventana: cerrar su pestaña aquí.
    static inline const QString kAdopted = QStringLiteral("adopted");

    /// \brief Opciones de la línea de comandos.
    struct LaunchArgs {
        QStringList paths;        ///< archivos, ya absolutos
        bool newWindow = false;   ///< `--new-window`: ser una instancia propia, sin entregar a otra
        int cursor = -1;          ///< `--cursor=N`: posición del cursor en el primer archivo
        QString handoffFrom;      ///< `--handoff-from=NOMBRE`: instancia que debe cerrar la pestaña
    };

    explicit SingleInstance(QObject *parent = nullptr);
    ~SingleInstance() override;

    /// \brief Interpreta los argumentos (sin el nombre del programa). Las rutas se hacen
    /// absolutas aquí: el directorio de trabajo de ESTE proceso no es el de quien las
    /// recibe. Una opción desconocida se ignora; tras `--` todo son rutas.
    static LaunchArgs parseArguments(const QStringList &args);

    /// \brief Nombre del socket de la instancia de este proceso (por usuario y PID).
    static QString instanceName(qint64 pid);
    /// \brief Fichero de registro por defecto, en un directorio **privado del usuario**
    /// (`XDG_RUNTIME_DIR` o, a falta de él, una carpeta 0700 propia bajo la temporal,
    /// comprobando que el dueño es el usuario). Vacío si no hay dónde ponerlo con
    /// garantías: sin registro no hay entrega entre instancias, nunca una ruta ajena.
    static QString registryFile();

    /// \brief Serializa un mensaje para el socket (puro, para los tests).
    static QByteArray encode(const Message &message);
    /// \brief Inversa de encode(). Un mensaje corrupto o incompleto devuelve falso.
    static bool decode(const QByteArray &data, Message &message);

    /// \brief Instancias registradas, de la más antigua a la más reciente.
    static QStringList registeredNames(const QString &registry = registryFile());

    /// \brief ¿Hay otra instancia viva además de `self`? Comprueba que aceptan la conexión
    /// (las entradas huérfanas de un cierre anómalo no cuentan, y se retiran).
    static bool otherInstancesAlive(const QString &self, const QString &registry = registryFile(),
                                    int timeoutMs = 300);

    /// \brief Entrega `message` a la instancia `name`. Falso si no está o no lo recibió.
    static bool sendTo(const QString &name, const Message &message, int timeoutMs = 1500);

    /// \brief Entrega `message` a la última instancia viva del registro, y retira del
    /// registro las que ya no responden (cierres anómalos).
    static bool sendToLatest(const Message &message, const QString &registry = registryFile(),
                             int timeoutMs = 1500);

    /// \brief Empieza a escuchar en `name` y se anota la última en `registry`.
    bool listen(const QString &name, const QString &registry = registryFile());
    /// \brief Nombre en el que escucha (vacío si no se pudo).
    QString name() const { return m_name; }

signals:
    /// \brief Otra instancia o ejecución envió un mensaje.
    void messageReceived(const SingleInstance::Message &message);

private:
    void onNewConnection();

    QLocalServer m_server;
    QString m_name;
    QString m_registry;
};

Q_DECLARE_METATYPE(SingleInstance::Message)
