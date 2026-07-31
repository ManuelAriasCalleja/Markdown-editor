#ifndef DICTIONARYINSTALLER_H
#define DICTIONARYINSTALLER_H

/// \file
/// \brief Descarga de un diccionario de Hunspell a la carpeta del usuario.

#include <QObject>
#include <QString>

class QNetworkAccessManager;

/// Descarga el par `.aff`/`.dic` de un idioma y lo deja en
/// `SpellChecker::userDictionaryDir()`, que es la primera ruta que consulta el
/// corrector. Es la vía de «instalar el diccionario» sin pedir contraseña ni
/// depender del gestor de paquetes: funciona igual en las tres plataformas y en
/// distribuciones donde el paquete `hunspell-xx` no existe.
///
/// Es la ÚNICA parte del programa que usa la red, y solo cuando el usuario pulsa
/// el botón: nada se descarga por su cuenta. El origen es el repositorio de
/// diccionarios de LibreOffice, el mismo del que salen los que se empaquetan
/// (`scripts/fetch-dictionaries.sh`); las URLs las da `mdspell::dictionaryUrls`.
///
/// Se descargan los dos ficheros y solo al final se escriben: si el segundo falla,
/// no queda un diccionario a medias que el motor intentaría cargar.
class DictionaryInstaller : public QObject
{
    Q_OBJECT

public:
    explicit DictionaryInstaller(QObject *parent = nullptr);

    /// \brief ¿Hay descarga conocida para este idioma? (los nueve de la interfaz).
    static bool canInstall(const QString &lang);

    /// \brief Descarga el diccionario de `lang`. Emite `finished` al terminar.
    /// No hace nada si ya hay una descarga en curso.
    void install(const QString &lang);

signals:
    /// \brief Descarga terminada. `ok` false → `error` explica qué falló (traducido).
    void finished(bool ok, const QString &error);

private:
    QNetworkAccessManager *m_net = nullptr;
    bool m_busy = false;
};

#endif  // DICTIONARYINSTALLER_H
