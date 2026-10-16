#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QScopeGuard>
#include <QSettings>
#include <QStandardPaths>

#include "internal-helpers.hpp"

Q_STATIC_LOGGING_CATEGORY(munadi, "CoreEngine")

namespace Munadi
{

CoreEngine::CoreEngine(QObject *parent) : MunadiEngine<CoreEngine>{parent}
{
    this->bootEngine();

    if (const QString appName = QCoreApplication::applicationName(); appName.isEmpty())
        logCritical(munadi) << "Not setting up persistnent config due to missing application "
                               "name. Settings won't be saved.";
    else
    {
        this->settings = new QSettings(QSettings::IniFormat, QSettings::UserScope,
                                       QCoreApplication::organizationName(), appName, this);

        logDebug(munadi) << "Settings file: " << this->settings->fileName();

        const QDir &configDir{QFileInfo(settings->fileName()).absolutePath()};

        this->configLock = new QLockFile(configDir.absoluteFilePath(".settings.lock"_L1));
    }

    if (const QFileInfo oldConfigFile{
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + ".conf"_L1};
        oldConfigFile.exists())
    {
        const QString oldConfigPath{oldConfigFile.absoluteFilePath()};
        logInfo(munadi) << "Old config detected, relevant configurations will be migrated";
        logDebug(munadi) << "Old config path: " << oldConfigPath;

        this->oldSettings = new QSettings(oldConfigPath, QSettings::NativeFormat, this);
    }

    this->applyLocale();
    qApp->installEventFilter(this);

    if (const auto conn = QObject::connect(qApp, &QCoreApplication::aboutToQuit, this,
                                           &CoreEngine::onAboutToQuit, Qt::DirectConnection);
        conn == nullptr)
        logWarning(munadi) << "aboutToQuit signal not connected";
    else
        logDebug(munadi) << "aboutToQuit signal connected";

    this->makeOnline();
}

const QLoggingCategory &CoreEngine::logCategory()
{ return munadi(); }

QVariant CoreEngine::readOldSettings(QAnyStringView oldKey, const QVariant &defaultValue)
{
    if (this->oldSettings == nullptr)
    {
        logDebug(munadi) << "Old settings doesn't exist, skipping migration";

        return {};
    }

    if (oldKey.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    const QWriteLocker lock{&varLock};

    return CoreEngine::readValueOfKey(*this->oldSettings, oldKey, defaultValue);
}

QVariantList CoreEngine::readOldSettingsJsArray(QAnyStringView    oldKey,
                                                const QJsonArray &defaultArray)
{
    if (this->oldSettings == nullptr)
    {
        logDebug(munadi) << "Old settings doesn't exist, skipping migration";

        return {};
    }

    if (oldKey.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    const QWriteLocker lock{&varLock};
    const QVariant     oldValue{CoreEngine::readValueOfKey(*this->oldSettings, oldKey)};

    QJsonParseError     err;
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

QVariant CoreEngine::readSettings(QAnyStringView key, const QVariant &defaultValue)
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return {};
    }

    const QWriteLocker lock{&varLock};

    return CoreEngine::readValueOfKey(*this->settings, key, defaultValue);
}

QVariantList CoreEngine::readSettingsArray(QAnyStringView group)
{
    if (group.isEmpty())
    {
        logCritical(munadi) << "Provided key group is empty";

        return {};
    }

    const QWriteLocker lock{&varLock};
    QVariantList       list;
    const qsizetype    size{this->settings->beginReadArray(group)};

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

//     const QWriteLocker lock{&varLock};

//     this->oldSettings->remove(key);

//     return true;
// }

void CoreEngine::setLang(const QString &lang)
{
    // const QWriteLocker lock{&varLock};

    logDebug(munadi) << "lang: " << lang;
    logDebug(munadi) << "locale: " << CoreEngine::s_lang.value(lang);

    QString currLang{QLocale::languageToString(QLocale{}.language())};

    logDebug(munadi) << "current lang: " << currLang;

    if (setConfig(CoreEngine::s_lang, lang, currLang))
        this->applyLocale(CoreEngine::s_lang.value(lang));
}

void CoreEngine::writeSettings(QAnyStringView key, const QVariant &value, bool flushOnWrite)
{
    if (key.isEmpty())
    {
        logCritical(munadi) << "Provided key is empty";

        return;
    }

    const QWriteLocker lock{&varLock};

    CoreEngine::setValueOfKey(*this->settings, key, value, flushOnWrite);
}

void CoreEngine::writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                                    bool flushOnWrite)
{
    if (group.isEmpty())
    {
        logCritical(munadi) << "Provided key group is empty";

        return;
    }

    const QWriteLocker lock{&varLock};

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

bool CoreEngine::eventFilter(QObject *object, QEvent *event)
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

void CoreEngine::applyLocale(const QLocale &locale)
{
    logDebug(munadi) << "Removing current translator: " << this->translator.language();
    qApp->removeTranslator(&this->translator); // no-op if none installed

    QLocale l{locale};

    if (l.language() == QLocale::English || l.language() == QLocale::Arabic ||
        l.language() == QLocale::Bangla)
    {
        l = QLocale{l.language(), l.territory()};

        QLocale::setDefault(l);

        if (this->translator.load(l, QLatin1String("munadi"), QLatin1String("_"),
                                  QLatin1String(":/qt/qml/Munadi/i18n")))
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

    if (QQmlEngine *engine = qmlEngine(this))
    {
        logDebug(munadi) << "Reloading Qml";
        engine->retranslate();
    }

    emit this->localeChanged();
}

QLocale CoreEngine::getCurrentLocale()
{ return QLocale{}; }

void CoreEngine::onAboutToQuit()
{ emit this->engineOffline(); }

QVariant CoreEngine::readValueOfKey(QSettings &settings, QAnyStringView key,
                                    const QVariant &defaultValue)
{
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

    return value;
}

// void CoreEngine::releaseConfigLock()
// {
//     this->configLock->unlock();
// }

void CoreEngine::setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                               bool flushOnWrite)
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
} // namespace Munadi