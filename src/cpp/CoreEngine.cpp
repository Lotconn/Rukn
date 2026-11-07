#include <QApplication>
#include <QDir>
#include <QDirListing>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSystemTrayIcon>

#include "internal-helpers.hpp"

#ifdef MUNADI_FLATPAK_DIST
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QRandomGenerator>
#endif

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/socket.h>
#endif

Q_STATIC_LOGGING_CATEGORY(munadi, "CoreEngine")

namespace Munadi
{

const QHash<QString, std::function<void(CoreEngine *)>> CoreEngine::triggerRelays{
    {u"24h_tick"_s,     &CoreEngine::dateChanged  },
    {u"60s_tick"_s,     &CoreEngine::minuteChanged},
    {u"alarms_reset"_s, &CoreEngine::alarmsReset  },
    {u"athan"_s,        &CoreEngine::athanAlarm   }
};

CoreEngine::CoreEngine(QObject *parent) : MunadiBase{munadi(), parent}, m_alarm{createAlarm(this)}
{
    this->bootEngine();

#ifdef Q_OS_UNIX
    logDebug(munadi) << "Unix signal connection status: "
                     << CoreEngine::installUnixSignalHandlers();

    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd.data()) != 0)
        logFatal(munadi) << "Couldn't create TERM socketpair";

    snTerm = new QSocketNotifier{sigtermFd[1], QSocketNotifier::Read, this};

    QObject::connect(snTerm, &QSocketNotifier::activated, this, &CoreEngine::onUnixSigTerm);
#endif

    const QString appName{QCoreApplication::applicationName()};
    const QString orgName{QCoreApplication::organizationName()};

    if (appName.isEmpty())
        logCritical(munadi) << "Not setting up persistnent config due to missing application "
                               "name. Settings won't be saved.";
    else
    {
        const QString stateDir{QStandardPaths::writableLocation(QStandardPaths::StateLocation)};

        this->settings =
            new QSettings{QSettings::IniFormat, QSettings::UserScope, orgName, appName, this};
        this->stateFile = new QSettings{stateDir + u"rc"_s, QSettings::IniFormat, this};

        logDebug(munadi) << "Settings file: " << this->settings->fileName();
        logDebug(munadi) << "State file: " << this->stateFile->fileName();
    }

    if (this->oldSettings = new QSettings{orgName, appName, this};
        QFileInfo::exists(this->oldSettings->fileName()))
        logInfo(munadi) << "Old config detected, relevant configurations will be migrated";

    logDebug(munadi) << "Old config path: " << this->oldSettings->fileName();

    // These are in order of dependency, not in lexical order
    qApp->installEventFilter(this);
    this->connectToAboutToQuit();
    this->connectToTrigger();
    this->migrateStore();
    this->loadConfigs();

    this->makeOnline();
    this->applyLocale();
    this->installTickers();
}

const QLoggingCategory &CoreEngine::logCategory() noexcept
{ return munadi(); }

Alarm &CoreEngine::alarmManager() const noexcept
{ return *this->m_alarm; }

QString CoreEngine::getAppVersion() noexcept
{ return CoreEngine::s_appVersion; }

bool CoreEngine::isAutoStartSet() const noexcept
{ return this->m_autoStartSet; }

bool CoreEngine::isVersionNew() const noexcept
{ return this->m_isVersionNew; }

QVariant CoreEngine::readOldSettings(QAnyStringView oldKey, const QVariant &defaultValue) noexcept
{
    if (oldKey.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    if (this->oldSettings == nullptr)
    {
        logDebug(munadi) << "Old settings doesn't exist, skipping migration";

        return {};
    }

    return CoreEngine::readValueOfKey(*this->oldSettings, oldKey, defaultValue);
}

QVariantList CoreEngine::readOldSettingsJsArray(QAnyStringView    oldKey,
                                                const QJsonArray &defaultArray) noexcept
{
    QJsonParseError     err;
    const QVariant      oldValue{this->readOldSettings(oldKey)};
    const QJsonDocument jsonDoc{QJsonDocument::fromJson(oldValue.toByteArray(), &err)};

    if (err.error != QJsonParseError::NoError)
    {
        logWarning(munadi) << "Failed to parse JSON";
        logDebug(munadi) << "Error at offset " << err.offset << ": " << err.errorString();

        return defaultArray.toVariantList();
    }

    if (jsonDoc.isEmpty() || jsonDoc.isNull())
    {
        logWarning(munadi) << "Parsed JSON is empty, using default value";

        return defaultArray.toVariantList();
    }

    if (!jsonDoc.isArray())
    {
        logWarning(munadi) << "Parsed JSON is not an array, using default value";

        return defaultArray.toVariantList();
    }

    return jsonDoc.array().toVariantList();
}

QVariant CoreEngine::readSettings(QAnyStringView key, const QVariant &defaultValue) noexcept
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    if (this->settings == nullptr)
        return defaultValue;

    return CoreEngine::readValueOfKey(*this->settings, key, defaultValue);
}

QVariantList CoreEngine::readSettingsArray(QAnyStringView group) noexcept
{
    if (group.isEmpty())
    {
        logCritical(munadi) << "Provided key group is empty";

        return {};
    }

    QVariantList    list;
    const qsizetype size{this->settings->beginReadArray(group)};

    if (this->settings == nullptr)
        return {};

    logDebug(munadi) << "Array size for \"" << group << "\" is " << size;

    for (qsizetype i = 0; i < size; ++i)
    {
        this->settings->setArrayIndex(static_cast<int>(i));

        QVariantMap map;

        logDebug(munadi) << "Child keys in array " << i << ": " << this->settings->childKeys();

        // NOLINTNEXTLINE(clazy-range-loop*)
        for (const QString &key : this->settings->childKeys())
        {
            const QVariant value{CoreEngine::readValueOfKey(*this->settings, key)};

            logDebug(munadi) << "Value of key \"" << key << "\": " << value;

            map.insert(key, value);
        }

        list.append(map);
    }

    this->settings->endArray();

    return list;
}

QVariant CoreEngine::readState(const QString &key, const QVariant &defaultValue) noexcept
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    if (this->stateFile == nullptr)
        return defaultValue;

    return CoreEngine::readValueOfKey(*this->stateFile, key, defaultValue);
}

void CoreEngine::set60sTicker() noexcept
{
    constexpr static quint8 minSecs{60};

    logDebug(munadi) << "Registering 60s_ticker";

    this->m_alarm->clear("60s_tick"_L1);
    // This is not regular arithmetic, do not simplify the math here
    this->m_alarm->add("60s_tick"_L1,
                       QDateTime::fromSecsSinceEpoch(
                           ((QDateTime::currentSecsSinceEpoch() / minSecs) + 1) * minSecs),
                       minSecs);
}

void CoreEngine::setAutoStart(bool enabled) noexcept // NOLINT(*member*to-static)
{
    if (this->m_autoStartSet == enabled)
        return;

    const bool oldValue{this->m_autoStartSet};

#ifdef Q_OS_WIN
    this->setAutoStart_win(enabled);
#elifdef MUNADI_FLATPAK_DIST
    this->setAutoStart_flatpak(enabled);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    CoreEngine::setAutoStart_linux(enabled);
#endif

    if (this->m_autoStartSet != oldValue)
        emit this->autoStartChanged(this->m_autoStartSet);
}

void CoreEngine::setLang(const QString &lang) noexcept
{
    QString currLang{QLocale::languageToString(QLocale{}.language())};

    logDebug(munadi) << "lang: " << lang;
    logDebug(munadi) << "locale: " << CoreEngine::s_lang.value(lang);
    logDebug(munadi) << "current lang: " << currLang;

    if (setConfig(CoreEngine::s_lang, lang, currLang))
        this->applyLocale(CoreEngine::s_lang.value(lang));
}

void CoreEngine::writeSettings(QAnyStringView key, const QVariant &value,
                               bool flushOnWrite) noexcept
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return;
    }

    if (this->settings == nullptr)
        return;

    CoreEngine::setValueOfKey(*this->settings, key, value, flushOnWrite);
}

void CoreEngine::writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                                    bool flushOnWrite) noexcept
{
    if (group.isEmpty())
    {
        logCritical(munadi) << "Provided key group is empty";

        return;
    }

    if (this->settings == nullptr)
        return;

    this->settings->remove(group);
    this->settings->beginWriteArray(group);

    for (qsizetype i = 0; i < entryMaps.size(); ++i)
    {
        const QVariantMap &map{entryMaps.at(i).toMap()};

        this->settings->setArrayIndex(static_cast<int>(i));

        for (const auto &[key, value] : map.asKeyValueRange())
            CoreEngine::setValueOfKey(*this->settings, key, value);
    }

    this->settings->endArray();

    if (flushOnWrite)
        this->settings->sync();
}

void CoreEngine::writeState(const QString &key, const QVariant &value, bool flushOnWrite) noexcept
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return;
    }

    if (this->stateFile == nullptr)
        return;

    CoreEngine::setValueOfKey(*this->stateFile, key, value, flushOnWrite);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Public Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::onReceivedMessage(quint32 instanceId, const QByteArray &message)
{
    logDebug(munadi) << "Received message from instance: " << instanceId;
    logInfo(munadi) << "Received Message: " << message;
    emit this->exposeInstance();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreEngine::eventFilter(QObject *object, QEvent *event) noexcept
{
    if (object != qApp)
        return QObject::eventFilter(object, event);

    switch (event->type())
    {
    case QEvent::LocaleChange:
        // OS-level change
        logDebug(munadi) << "Locale change detected";
        this->applyLocale();
        return true;
    case QEvent::LanguageChange:
        // App-level change, e.g. language selected from options
        logDebug(munadi) << "Translations changed";

        return true;
    default:
        return QObject::eventFilter(object, event);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::applyLocale(const QLocale &locale) noexcept
{
    logDebug(munadi) << "Removing current translator: " << this->translator.language();
    qApp->removeTranslator(&this->translator); // no-op if none installed

    QLocale l{locale};

    if (CoreEngine::s_revLang.contains(l.language()))
    {
        l = QLocale{l.language(), l.territory()};

        QLocale::setDefault(l);

        if (this->translator.load(l, "munadi"_L1, "_"_L1, ":/qt/qml/Munadi/i18n"_L1))
        {
            logDebug(munadi) << "Installing updated translator: " << this->translator.language();
            qApp->installTranslator(&this->translator);
        }
    }
    else
    {
        logInfo(munadi) << "Language not supported, using en_US as fallback";
        QLocale::setDefault(QLocale{"en_US"_L1});
    }

    // Querying the current default has its advantages
    QApplication::setLayoutDirection(QLocale{}.language() == QLocale::Arabic ? Qt::RightToLeft
                                                                             : Qt::LeftToRight);

    if (QQmlEngine *engine = qmlEngine(this))
    {
        logDebug(munadi) << "Reloading Qml translations";
        engine->retranslate();
    }

    emit this->localeChanged();
}

void CoreEngine::connectToAboutToQuit() const noexcept
{
    if (const auto conn = QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                                           &CoreEngine::onAboutToQuit);
        conn == nullptr)
        logWarning(munadi) << "aboutToQuit signal not connected";
    else
        logDebug(munadi) << "aboutToQuit signal connected";
}

void CoreEngine::connectToTrigger() const noexcept
{
    if (const auto conn =
            QObject::connect(this->m_alarm, &Alarm::trigger, this, &CoreEngine::onTrigger);
        conn == nullptr)
        logWarning(munadi) << "Trigger signal not connected";
    else
        logDebug(munadi) << "Trigger signal connected";
}

QLocale CoreEngine::getCurrentLocale() noexcept
{ return QLocale{}; }

void CoreEngine::installTickers() noexcept
{
    constexpr static quint32 daySecs{86400};

    this->m_alarm->clear("24h_tick"_L1);
    this->m_alarm->add("24h_tick"_L1, QDate::currentDate().startOfDay().addDays(1), daySecs);

    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        logWarning(munadi) << "System tray is not available";
        logDebug(munadi) << "Not registering 60s_ticker";
        return;
    }

    this->set60sTicker();
}

int CoreEngine::installUnixSignalHandlers()
{
#ifdef Q_OS_UNIX
    struct sigaction term{};

    term.sa_handler = CoreEngine::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags  = 0;
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &term, nullptr) != 0)
        return ENOTCONN;

    if (sigaction(SIGTERM, &term, nullptr) != 0)
        return ENOTCONN;

    return 0;
#endif
}

void CoreEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{u"CoreEngineConfigMigrated"_s};

    // Assign to the member directly to avoid resetting autostart to system on startup
    // WIP: Do not save state, use state from system directly
    loadConfigKey(
        "App/autoStartUp"_L1, "App/AutoStart"_L1, migrateKey,
        [this](const QVariant &enabled) -> void { this->m_autoStartSet = enabled.toBool(); },
        configMigrated);

    loadConfigKey(
        "App/newsVersion"_L1, "App/LastVersion"_L1, migrateKey,
        [this](const QVariant &version) -> void {
            // the following string comparison does what it implies
            if (version.toString() >= CoreEngine::s_appVersion)
                this->m_isVersionNew = false;
        },
        configMigrated);

    if (configMigrated)
        this->writeSettings("CoreEngineConfigMigrated"_L1, true);
}

void CoreEngine::migrateStore() noexcept
{
    if (this->readSettings("StoreMigrated"_L1, false).toBool())
        return;

    const QString appName{QCoreApplication::applicationName()};
    const QString oldStorePath{QDir::toNativeSeparators(
        QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + u"/"_s + appName)};

    if (QFileInfo::exists(oldStorePath) && !QFileInfo{oldStorePath}.isDir())
    {
        logDebug(munadi)
            << oldStorePath
            << "already exists which is not a directory, refusing to initiate migration";
        return;
    }

    const QDir newStoreDir{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)};

    logInfo(munadi) << "Old store detected, attempting migration...";
    logDebug(munadi) << "Old store path: " << oldStorePath;
    logDebug(munadi) << "New store path: " << newStoreDir.absolutePath();

    if (newStoreDir.absolutePath() == oldStorePath)
    {
        logInfo(munadi) << "Old and new store are the same, skipping migration";
        return;
    }

    using F = QDirListing::IteratorFlag;
    QDir oldStoreDir{oldStorePath};
    bool ok{};

    newStoreDir.mkpath(u"."_s);

    for (const QDirListing::DirEntry &dirEntry :
         QDirListing{oldStorePath, F::Default | F::IncludeHidden})
    {
        const QString src{dirEntry.fileName()};
        const QString destPath{newStoreDir.filePath(src)};

        if (src == appName)
            continue;

        if (bool ret{oldStoreDir.rename(src, destPath)}; ret)
        {
            logDebug(munadi) << "Moving " << src << " to " << destPath;
            ok = ret;
        }
        else
            logWarning(munadi) << "Error while moving " << src << " to " << destPath;
    }

    if (ok)
        this->writeSettings("StoreMigrated"_L1, true, true);
}

QVariant CoreEngine::readValueOfKey(QSettings &settings, QAnyStringView key,
                                    const QVariant &defaultValue) noexcept
{
    logDebug(munadi) << "Fetching value of key \"" << key << "\"";

    QVariant value{settings.value(key)};

    switch (settings.status())
    {
    case QSettings::AccessError:
        logWarning(munadi) << "Failed to read config";
        return defaultValue;
    case QSettings::FormatError:
        logWarning(munadi) << "Config for key \"" << key
                           << "\" is malformed, returning default value";
        return defaultValue;
    default:
        break;
    }

    if (value.isNull())
    {
        logWarning(munadi) << "Failed to fetch a valid value for key \"" << key
                           << "\", returning default value";

        return defaultValue;
    }

    logDebug(munadi) << "Key: \"" << key << "\"; Value: \"" << value << "\"";

    return value;
}

// void CoreEngine::releaseConfigLock()
// {
//     this->configLock->unlock();
// }

void CoreEngine::saveConfigs() noexcept
{
    logDebug(munadi) << "Saving configurations";

    this->writeSettings("App/AutoStart"_L1, this->m_autoStartSet);
    this->writeSettings("App/LastVersion"_L1, CoreEngine::s_appVersion, true);
}

void CoreEngine::setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                               bool flushOnWrite) noexcept
{
    if (value.isNull())
        logWarning(munadi) << "Value is empty. Make sure this is intended.";

    settings.setValue(key, value);

    if (flushOnWrite)
        settings.sync();

    switch (settings.status())
    {
    case QSettings::AccessError:
        logWarning(munadi) << "Failed to read config";
        break;
    case QSettings::FormatError:
        logWarning(munadi) << "Config for key \"" << key
                           << "\" is malformed, returning default value";
        break;
    default:
        // logDebug(munadi) << key << "=" << value << " successfully saved";
        break;
    }
}

// NOLINTNEXTLINE(*member-function*static)
void CoreEngine::setAutoStart_flatpak(bool enabled) noexcept
{
    Q_UNUSED(enabled)
#ifdef MUNADI_FLATPAK_DIST
    QDBusConnection bus{QDBusConnection::sessionBus()};

    QDBusMessage msg{QDBusMessage::createMethodCall(
        "org.freedesktop.portal.Desktop"_L1, "/org/freedesktop/portal/desktop"_L1,
        "org.freedesktop.portal.Background"_L1, "RequestBackground"_L1)};

    // NOLINTNEXTLINE(*magic-numbers)
    const int     token{QRandomGenerator::global()->bounded(1000, 9999)};
    const QString sender{bus.baseService().mid(1).replace(u'.', u'_')};
    const QString handleToken{u"munadi_%1"_s.arg(token)};
    const QString requestPath{
        u"/org/freedesktop/portal/desktop/request/%1/%2"_s.arg(sender, handleToken)};

    interimStore = enabled;
    bus.connect("org.freedesktop.portal.Desktop"_L1, requestPath,
                "org.freedesktop.portal.Request"_L1, "Response"_L1, this,
                // NOLINTNEXTLINE(clazy-connect-not-normalized)
                SLOT(onAutoStartResponse(uint, QVariantMap)));

    const QVariantMap options{
        {u"autostart"_s,    enabled                                       },
        {u"background"_s,   enabled                                       },
        {u"commandline"_s,  QStringList{u"munadi"_s, u"--hidden"_s}       },
        {u"reason"_s,       u"Automatically launch application at login"_s},
        {u"handle_token"_s, u"munadi_%1"_s.arg(token)                     },
    };

    msg << ""_L1 << options; // Empty parent window ID
    QDBusMessage response{bus.call(msg)};

    // Handle response and connect to signal for final status
    if (response.type() != QDBusMessage::ReplyMessage)
        logWarning(munadi) << "Portal request failed: " << response.errorMessage();
#endif
}

// NOLINTNEXTLINE(*member-function*static)
void CoreEngine::setAutoStart_linux(bool enabled) noexcept
{
    Q_UNUSED(enabled)
#ifndef MUNADI_FLATPAK_DIST
    const QString desktopName{qApp->desktopFileName() + u"-sfx.desktop"_s};
    const QString autostartPath{
        QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + u"/autostart/"_s +
        desktopName};

    if (!enabled)
    {
        if (!QFileInfo::exists(autostartPath) || !QFile::remove(autostartPath))
            logWarning(munadi) << "Couldn't remove autostart file " << autostartPath;
        else
            logInfo(munadi) << "Autostart disabled successfully";

        this->m_autoStartSet = enabled;
        return;
    }

    const QString desktopPath{
        QStandardPaths::locate(QStandardPaths::ApplicationsLocation, desktopName)};

    if (desktopPath.isEmpty())
    {
        logWarning(munadi) << "No desktop file found installed in system";
        return;
    }

    logDebug(munadi) << "Using desktop file at " << desktopPath;

    QFile desktopFile{desktopPath};

    if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        logCritical(munadi) << "Couldn't open " << desktopPath << " for reading";
        return;
    }

    const QString contents{
        QString{desktopFile.readAll()}.replace("bin/munadi"_L1, "bin/munadi --hidden"_L1)};

    desktopFile.close();

    QSaveFile file{autostartPath};

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        logCritical(munadi) << "Couldn't open " << autostartPath << " for writing";
        return;
    }

    file.write(contents.toUtf8());

    if (!file.commit())
        logCritical(munadi) << "Failed to write at " << autostartPath;
    else
    {
        logDebug(munadi) << "File " << autostartPath << " successfully written";

        this->m_autoStartSet = enabled;
    }
#endif
}

// NOLINTNEXTLINE(*member-function*static)
void CoreEngine::setAutoStart_win(bool enabled) noexcept
{
    Q_UNUSED(enabled)
#ifdef Q_OS_WIN
    const QString appName{QCoreApplication::applicationName()};
    const QString appPath{QDir::toNativeSeparators(QCoreApplication::applicationFilePath())};
    QSettings     settings{R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)"_L1,
                           QSettings::NativeFormat};

    (enabled) ? settings.setValue(appName, appPath + " --hidden"_L1) : settings.remove(appName);

    switch (settings.status())
    {
    case QSettings::AccessError:
        logWarning(munadi) << "Failed to read config";
        return;
    case QSettings::FormatError:
        logWarning(munadi) << "Value \"" << appPath << "\" for key \"" << appName
                           << "\" is malformed";
        return;
    default:
        this->m_autoStartSet = enabled;
        break;
    }
#endif
}

void CoreEngine::termSignalHandler(int /* unused */)
{
#ifdef Q_OS_UNIX
    const char    a{1};
    const ssize_t ret{::write(sigtermFd[0], &a, sizeof(a))};
    Q_UNUSED(ret)
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::onAboutToQuit() noexcept
{
    this->saveConfigs();
    emit this->engineOffline();

#ifdef Q_OS_UNIX
    ::close(sigtermFd[0]);
    ::close(sigtermFd[1]);
#endif
}

void CoreEngine::onAutoStartResponse(uint ret, const QVariantMap & /* unused */) noexcept
{
    if (ret > 0)
        logWarning(munadi) << "Request to autostart was cancelled";
    else
    {
        logDebug(munadi) << "Autostart set successfully";

        m_autoStartSet = interimStore;
    }
}

void CoreEngine::onTrigger(const QString &id) noexcept
{
    logDebug(munadi) << "trigger id: " << id;

    if (!CoreEngine::triggerRelays.contains(id))
    {
        logWarning(munadi) << "\"" << id << "\" is not a valid alarm ID";
        return;
    }

    // This is actually `emit signal()`
    std::invoke(CoreEngine::triggerRelays.value(id), this);

    if (id == u"alarms_reset"_s)
        this->installTickers();
}

void CoreEngine::onUnixSigTerm()
{
#ifdef Q_OS_UNIX
    snTerm->setEnabled(false);

    char          tmp{0};
    const ssize_t ret{::read(sigtermFd[1], &tmp, sizeof(tmp))};
    Q_UNUSED(ret)

    qApp->quit();
#endif
}
} // namespace Munadi