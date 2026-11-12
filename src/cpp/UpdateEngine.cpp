#include <QUrl>
#include <QUrlQuery>

#include "internal-helpers.hpp"
#include <UpdateEngine.hpp>
#include <config.hpp>

Q_STATIC_LOGGING_CATEGORY(cat, "UpdateEngine")

namespace App
{

UpdateEngine *UpdateEngine::create(QQmlEngine * /* qmlEngine */, QJSEngine * /* jsEngine */)
{
    logDebug(cat) << "Creating instance from QML call";

    return new UpdateEngine{};
}

const QLoggingCategory &UpdateEngine::logCategory() noexcept
{ return cat(); }

void UpdateEngine::updateCheck() const noexcept
{
    QUrl      jadid{UpdateEngine::apiUrl.resolved(QUrl{"jadid"_L1})};
    QUrlQuery jadidParams;

#ifdef Q_OS_WIN
    jadidParams.addQueryItem("os"_L1, "win"_L1);
#elifdef Q_OS_MACOS
    jadidParams.addQueryItem("os"_L1, "osx"_L1);
#elifdef Q_OS_ANDROID
    jadidParams.addQueryItem("os"_L1, "android"_L1);
#elifdef Q_OS_LINUX
    jadidParams.addQueryItem("os"_L1, "linux"_L1);
#endif

#ifdef ARABIC
    jadidParams.addQueryItem("lang"_L1, "ar"_L1);
#else
    jadidParams.addQueryItem("lang"_L1, "en"_L1);
#endif

    jadidParams.addQueryItem("v"_L1, QStringLiteral(APP_VERSION));
    jadid.setQuery(jadidParams);

    logDebug(cat) << "Full query URL: " << jadid.toDisplayString();

    this->netMan->get(QNetworkRequest{jadid});
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

UpdateEngine::UpdateEngine(QObject *parent)
    : AppBase{cat(), parent}, netMan{new QNetworkAccessManager{this}}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    this->connectToNetMan();

    if (this->m_checkUpdates)
        this->updateCheck();

    this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateEngine::connectToNetMan() const noexcept
{
    if (const auto conn = QObject::connect(this->netMan, &QNetworkAccessManager::finished, this,
                                           &UpdateEngine::onFinished);
        conn == nullptr)
        logWarning(cat) << "Network Access Manager finished signal not connected";
    else
        logDebug(cat) << "Network Access Manager finished signal connected";
}

void UpdateEngine::defaultConfigs() noexcept
{ this->m_checkUpdates = false; }

// void UpdateEngine::loadConfigs() noexcept
// {
//     logDebug(cat) << "Loading configs from persistent config file";

//     bool          configMigrated{true};
//     const QString migrateKey{u"UpdateEngineConfigMigrated"_s};

//     // loadConfigKey({}, "App/CheckUpdates"_L1, {}, this, "checkUpdates"_L1);

//     // if (configMigrated)
//     //     core->writeSettings("UpdateEngineConfigMigrated"_L1, true);
// }

// void UpdateEngine::saveConfigs() noexcept
// {
//     logDebug(cat) << "Saving update configurations";

//     // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
//     // (later) settings are not saved

//     // core->writeSettings("App/CheckUpdates"_L1, this->m_checkUpdates, true);
// }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateEngine::onFinished(QNetworkReply *reply)
{
    const QString replyString{reply->readAll()};

    if (reply->error() == QNetworkReply::NoError && replyString.length() > 0)
    {
        logDebug(cat) << "No error found in current reply";

        emit this->updateAvailable(replyString);
    }
    else
    {
        logCritical(cat) << "Could not resolve provided query";
        logDebug(cat) << reply->errorString();
    }

    reply->deleteLater();
}
} // namespace App