/// \file
/// \brief Implementación de la descarga de diccionarios a la carpeta del usuario.

#include "dictionaryinstaller.h"

#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>
#include <QUrl>

#include "spellchecker.h"
#include "spellscan.h"

namespace {

// Nombre con el que se guarda el diccionario: el que espera pickDictionary
// (es_ES, de_DE…), no el del fichero upstream (de_DE_frami, fr…).
QString targetName(const QString &lang)
{
    QString key = lang;
    key.replace(QLatin1Char('-'), QLatin1Char('_'));
    static const QHash<QString, QString> kBase = {
        {QStringLiteral("es"), QStringLiteral("es_ES")}, {QStringLiteral("en"), QStringLiteral("en_US")},
        {QStringLiteral("de"), QStringLiteral("de_DE")}, {QStringLiteral("fr"), QStringLiteral("fr_FR")},
        {QStringLiteral("it"), QStringLiteral("it_IT")}, {QStringLiteral("pt"), QStringLiteral("pt_BR")},
        {QStringLiteral("pl"), QStringLiteral("pl_PL")}, {QStringLiteral("nl"), QStringLiteral("nl_NL")},
        {QStringLiteral("ro"), QStringLiteral("ro_RO")},
    };
    const QString base = kBase.value(key.section(QLatin1Char('_'), 0, 0).toLower());
    return key.contains(QLatin1Char('_')) ? key : base;
}

}  // namespace

DictionaryInstaller::DictionaryInstaller(QObject *parent)
    : QObject(parent)
    , m_net(new QNetworkAccessManager(this))
{
}

bool DictionaryInstaller::canInstall(const QString &lang)
{
    return !mdspell::dictionaryUrls(lang).first.isEmpty();
}

void DictionaryInstaller::install(const QString &lang)
{
    if (m_busy)
        return;
    const auto urls = mdspell::dictionaryUrls(lang);
    const QString name = targetName(lang);
    if (urls.first.isEmpty() || name.isEmpty()) {
        emit finished(false, tr("No hay descarga disponible para ese idioma."));
        return;
    }
    m_busy = true;

    // Se piden los dos y se guardan al final, juntos: un .dic sin su .aff no es
    // un diccionario y el motor lo ignoraría sin decir por qué.
    auto *affReply = m_net->get(QNetworkRequest(QUrl(urls.first)));
    connect(affReply, &QNetworkReply::finished, this, [this, affReply, urls, name] {
        affReply->deleteLater();
        if (affReply->error() != QNetworkReply::NoError) {
            m_busy = false;
            emit finished(false, affReply->errorString());
            return;
        }
        const QByteArray aff = affReply->readAll();

        auto *dicReply = m_net->get(QNetworkRequest(QUrl(urls.second)));
        connect(dicReply, &QNetworkReply::finished, this, [this, dicReply, aff, name] {
            dicReply->deleteLater();
            m_busy = false;
            if (dicReply->error() != QNetworkReply::NoError) {
                emit finished(false, dicReply->errorString());
                return;
            }
            const QString dir = SpellChecker::userDictionaryDir();
            if (!QDir().mkpath(dir)) {
                emit finished(false, tr("No se pudo crear la carpeta %1.").arg(dir));
                return;
            }
            QFile affFile(dir + QLatin1Char('/') + name + QStringLiteral(".aff"));
            QFile dicFile(dir + QLatin1Char('/') + name + QStringLiteral(".dic"));
            if (!affFile.open(QIODevice::WriteOnly) || affFile.write(aff) < 0
                || !dicFile.open(QIODevice::WriteOnly)
                || dicFile.write(dicReply->readAll()) < 0) {
                emit finished(false, tr("No se pudo escribir en %1.").arg(dir));
                return;
            }
            affFile.close();
            dicFile.close();
            emit finished(true, QString());
        });
    });
}
