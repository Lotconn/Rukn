#include <QUrl>
#include <QUrlQuery>

#include "internal-helpers.hpp"
#include <UpdateEngine.hpp>
#include <version.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "UpdateEngine")

namespace Munadi
{

UpdateEngine *UpdateEngine::create(QQmlEngine * /* qmlEngine */, QJSEngine * /* jsEngine */)
{
    logDebug(munadi) << "Creating instance from QML call";

    return new UpdateEngine{};
}

const QLoggingCategory &UpdateEngine::logCategory() noexcept
{ return munadi(); }

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

    logDebug(munadi) << "Full query URL: " << jadid.toDisplayString();

    this->netMan->get(QNetworkRequest{jadid});
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

UpdateEngine::UpdateEngine(QObject *parent)
    : MunadiBase{munadi(), parent}, netMan{new QNetworkAccessManager{this}}
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
        logWarning(munadi) << "Network Access Manager finished signal not connected";
    else
        logDebug(munadi) << "Network Access Manager finished signal connected";
}

void UpdateEngine::defaultConfigs() noexcept
{ this->m_checkUpdates = true; }

void UpdateEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{u"UpdateEngineConfigMigrated"_s};

    loadConfigKey("App/checkForUpdates"_L1, "App/CheckUpdates"_L1, migrateKey, this,
                  "checkUpdates"_L1, configMigrated);

    if (configMigrated)
        core->writeSettings("UpdateEngineConfigMigrated"_L1, true);
}

void UpdateEngine::saveConfigs() noexcept
{
    logDebug(munadi) << "Saving update configurations";

    // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
    // (later) settings are not saved

    core->writeSettings("App/CheckUpdates"_L1, this->m_checkUpdates, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateEngine::onFinished(QNetworkReply *reply)
{
    const QString replyString{reply->readAll()};

    if (reply->error() == QNetworkReply::NoError && replyString.length() > 0)
    {
        logDebug(munadi) << "No error found in current reply";

        emit this->updateAvailable(replyString);
    }
    else
    {
        logCritical(munadi) << "Could not resolve provided query";
        logDebug(munadi) << reply->errorString();
    }

    reply->deleteLater();
}
} // namespace Munadi