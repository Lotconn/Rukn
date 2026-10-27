#include <QApplication>
#include <QDir>
#include <QEvent>
#include <QScopeGuard>
#include <QSettings>
#include <QStandardPaths>

#include "internal-helpers.hpp"

Q_STATIC_LOGGING_CATEGORY(munadi, "CoreEngine")

namespace Munadi
{

const QHash<QString, std::function<void(CoreEngine *)>> CoreEngine::doTrigger{
    {u"24h_tick"_s,     &CoreEngine::dateChanged  },
    {u"60s_tick"_s,     &CoreEngine::minuteChanged},
    {u"alarms_reset"_s, &CoreEngine::alarmsReset  },
    {u"athan"_s,        &CoreEngine::athanAlarm   }
};

CoreEngine::CoreEngine(QObject *parent) : MunadiEngine<CoreEngine>{parent}, alarm{createAlarm(this)}
{
    this->bootEngine();

    if (const QString appName = QCoreApplication::applicationName(); appName.isEmpty())
        logCritical(munadi) << "Not setting up persistnent config due to missing application "
                               "name. Settings won't be saved.";
    else
    {
        this->settings = new QSettings{QSettings::IniFormat, QSettings::UserScope,
                                       QCoreApplication::organizationName(), appName, this};

        logDebug(munadi) << "Settings file: " << this->settings->fileName();

        const QDir &configDir{QFileInfo(settings->fileName()).absolutePath()};

        this->configLock = new QLockFile{configDir.absoluteFilePath(u".settings.lock"_s)};
    }

    if (const QFileInfo oldConfigFile{
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + u".conf"_s};
        oldConfigFile.exists())
    {
        const QString oldConfigPath{oldConfigFile.absoluteFilePath()};
        logInfo(munadi) << "Old config detected, relevant configurations will be migrated";
        logDebug(munadi) << "Old config path: " << oldConfigPath;

        this->oldSettings = new QSettings(oldConfigPath, QSettings::NativeFormat, this);
    }

    // These are in order of dependency, not in lexical order
    qApp->installEventFilter(this);
    this->connectToAboutToQuit();
    this->connectToTrigger();

    this->makeOnline();
    this->applyLocale();
    this->installTickers();
}

const QLoggingCategory &CoreEngine::logCategory() noexcept
{ return munadi(); }

Alarm &CoreEngine::alarmManager() const noexcept
{ return *this->alarm; }

QVariant CoreEngine::readOldSettings(QAnyStringView oldKey, const QVariant &defaultValue) noexcept
{
    if (oldKey.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    const QWriteLocker lock{&this->varLock};

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

    const QWriteLocker lock{&this->varLock};

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

    QVariantList       list;
    const QWriteLocker lock{&this->varLock};
    const qsizetype    size{this->settings->beginReadArray(group)};

    if (this->settings == nullptr)
        return {};

    logDebug(munadi) << "Array size for \"" << group << "\" is " << size;

    for (qsizetype i = 0; i < size; ++i)
    {
        this->settings->setArrayIndex(static_cast<int>(i));

        QVariantMap map;

        logDebug(munadi) << "Child keys in array " << i << ": " << this->settings->childKeys();

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

// bool CoreEngine::removeOldSettingsKey(QAnyStringView key)
// {
//     if (key.isEmpty())
//     {
//         logCritical(munadi) << "Provided key is empty";

//         return false;
//     }

//     const QWriteLocker lock{&this->varLock};

//     this->oldSettings->remove(key);

//     return true;
// }

void CoreEngine::set60sTicker() noexcept
{
    constexpr static quint8 minSecs{60};

    logDebug(munadi) << "Registering 60s_ticker";

    this->alarm->clear("60s_tick"_L1);
    // This is not regular arithmetic, do not simplify the math here
    this->alarm->add("60s_tick"_L1,
                     QDateTime::fromSecsSinceEpoch(
                         ((QDateTime::currentSecsSinceEpoch() / minSecs) + 1) * minSecs),
                     minSecs);
}

void CoreEngine::setLang(const QString &lang) noexcept
{

    logDebug(munadi) << "lang: " << lang;
    logDebug(munadi) << "locale: " << CoreEngine::s_lang.value(lang);

    bool configSet{false};
    {
        const QWriteLocker lock{&this->varLock};
        QString            currLang{QLocale::languageToString(QLocale{}.language())};

        logDebug(munadi) << "current lang: " << currLang;
        configSet = setConfig(CoreEngine::s_lang, lang, currLang);
    }

    if (configSet)
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

    const QWriteLocker lock{&this->varLock};

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

    const QWriteLocker lock{&this->varLock};

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

// int CoreEngine::acquireConfigLock()
// {

//     constexpr qint32 lockLifetime = 40000; // 40 sec
//     constexpr qint32 waitDuration = 5000;  // 5 sec

//     QMutexLocker lock(&threadLock);

//     this->configLock->setStaleLockTime(lockLifetime);

//     if (!this->configLock->tryLock(waitDuration))
//     {
//         int e = EIO;

//         switch (this->configLock->error())
//         {
//         case QLockFile::LockFailedError:
//             logCritical(munadi) << "Config busy, could not acquire lock";
//             e = EBUSY;
//             break;
//         case QLockFile::PermissionError:
//             logCritical(munadi) << "Permission error in config, could not acquire lock";
//             e = EACCES;
//             break;
//         default:
//             logCritical(munadi) << "IO error in config, could not acquire lock";
//             break;
//         }

//         this->threadLock.unlock();

//         return e;
//     }

//     return 0;
// }

void CoreEngine::applyLocale(const QLocale &locale) noexcept
{
    logDebug(munadi) << "Removing current translator: " << this->translator.language();
    qApp->removeTranslator(&this->translator); // no-op if none installed

    QLocale l{locale};

    if (l.language() == QLocale::English || l.language() == QLocale::Arabic ||
        l.language() == QLocale::Bangla)
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
        QLocale::setDefault(QLocale{QLocale::English, QLocale::UnitedStates});
    }

    QApplication::setLayoutDirection(QLocale{}.language() == QLocale::Arabic ? Qt::RightToLeft
                                                                             : Qt::LeftToRight);

    if (QQmlEngine *engine = qmlEngine(this))
    {
        logDebug(munadi) << "Reloading Qml";
        engine->retranslate();
    }

    emit this->localeChanged();
}

void CoreEngine::connectToAboutToQuit() const noexcept
{
    if (const auto conn = QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                                           &CoreEngine::onAboutToQuit, Qt::DirectConnection);
        conn == nullptr)
        logWarning(munadi) << "aboutToQuit signal not connected";
    else
        logDebug(munadi) << "aboutToQuit signal connected";
}

void CoreEngine::connectToTrigger() const noexcept
{
    if (const auto conn =
            QObject::connect(this->alarm, &Alarm::trigger, this, &CoreEngine::onTrigger);
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

    this->alarm->clear("24h_tick"_L1);
    this->alarm->add("24h_tick"_L1, QDate::currentDate().startOfDay().addDays(1), daySecs);
    this->set60sTicker();
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
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::onAboutToQuit() noexcept
{ emit this->engineOffline(); }

void CoreEngine::onTrigger(const QString &id) noexcept
{
    logDebug(munadi) << "trigger id: " << id;

    if (!CoreEngine::doTrigger.contains(id))
    {
        logWarning(munadi) << "\"" << id << "\" is not a valid alarm ID";
        return;
    }

    // This is actually `emit signal()`
    std::invoke(CoreEngine::doTrigger.value(id), this);

    if (id == u"alarms_reset"_s)
        this->installTickers();
}

} // namespace Munadi