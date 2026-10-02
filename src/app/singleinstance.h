/// \file
/// \brief Instancia única: una segunda ejecución entrega sus archivos a la primera.
#pragma once

#include <QByteArray>
#include <QLocalServer>
#include <QObject>
#include <QString>
#include <QStringList>

/// \brief Servidor local de la instancia primaria y cliente de las siguientes.
///
/// Al abrir un `.md` desde el explorador con el editor ya en marcha, el proceso
/// nuevo no construye ninguna ventana: le pasa las rutas a la instancia existente
/// (`sendToRunning`) y sale. La existente las recibe por `pathsReceived`.
class SingleInstance : public QObject
{
    Q_OBJECT

public:
    explicit SingleInstance(QObject *parent = nullptr);

    /// \brief Nombre del servidor, distinto por usuario (varias sesiones en la misma máquina).
    static QString serverName();

    /// \brief Serializa una lista de rutas para el socket (puro, para los tests).
    static QByteArray encode(const QStringList &paths);
    /// \brief Inversa de encode(). Un mensaje corrupto o incompleto devuelve falso.
    static bool decode(const QByteArray &data, QStringList &paths);

    /// \brief Entrega `paths` a la instancia que escucha en `name`.
    /// \return verdadero si había una y recibió el mensaje; falso si no hay ninguna.
    static bool sendToRunning(const QString &name, const QStringList &paths, int timeoutMs = 1500);

    /// \brief Empieza a escuchar en `name`; limpia el socket huérfano de un cierre anómalo.
    bool listen(const QString &name);

signals:
    /// \brief Otra ejecución pidió abrir `paths` (vacío = solo traer la ventana al frente).
    void pathsReceived(const QStringList &paths);

private:
    void onNewConnection();

    QLocalServer m_server;
};
