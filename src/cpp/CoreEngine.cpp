#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QScopeGuard>
#include <QSettings>
#include <QStandardPaths>

#include "internal-helpers.hpp"

// Q_STATIC_LOGGING_CATEGORY(munadi, "CoreEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(coreEngine, "CoreEngine")

namespace Munadi
{

CoreEngine::CoreEngine(QObject *parent) : QObject{parent}
{
    if (CoreEngine::s_singletonInstance != nullptr)
        logFatal(coreEngine)
            << "A second instance was constructed; only one instance may exist per process";

    if (const QString appName = QCoreApplication::applicationName(); appName.isEmpty())
        logCritical(coreEngine) << "Not setting up persistnent config due to missing application "
                                   "name. Settings won't be saved.";
    else
    {
        this->settings = std::make_unique<QSettings>(QSettings::IniFormat, QSettings::UserScope,
                                                     QCoreApplication::organizationName(), appName);

        logDebug(coreEngine) << "Settings file: " << this->settings->fileName();

        const QDir &configDir{QFileInfo(settings->fileName()).absolutePath()};

        this->configLock =
            std::make_unique<QLockFile>(configDir.absoluteFilePath(".settings.lock"_L1));
    }

    if (const QFileInfo oldConfigFile{
            QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + ".conf"_L1};
        oldConfigFile.exists())
    {
        const QString oldConfigPath{oldConfigFile.absoluteFilePath()};
        logDebug(coreEngine) << "Old config detected at " << oldConfigPath;

        this->oldSettings = std::make_unique<QSettings>(oldConfigPath);
    }

    this->applyLocale();
    qApp->installEventFilter(this);

    this->aboutToQuitConnection =
        QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &CoreEngine::onAboutToQuit,
                         Qt::DirectConnection);

    if (this->aboutToQuitConnection == nullptr)
        logWarning(coreEngine) << "aboutToQuit signal not connected";
    else
        logDebug(coreEngine) << "aboutToQuit signal connected";

    CoreEngine::s_singletonInstance = this;

    emit this->coreOnline();
}

CoreEngine::~CoreEngine()
{
    QObject::disconnect(this->aboutToQuitConnection);

    if (CoreEngine::s_singletonInstance == this)
        CoreEngine::s_singletonInstance = nullptr;
}

CoreEngine &CoreEngine::instance()
{
    if (CoreEngine::s_singletonInstance == nullptr)
        logFatal(coreEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *CoreEngine::s_singletonInstance;
}

bool CoreEngine::exists() noexcept
{ return CoreEngine::s_singletonInstance != nullptr; }

QVariant CoreEngine::readSettings(QAnyStringView key, const QVariant &defaultValue)
{
    if (key.isEmpty())
    {
        logCritical(coreEngine) << "Provided key is empty";

        return {};
    }

    QWriteLocker lock(&varLock);

    return this->readValueOfKey(key, defaultValue);
}

QVariantList CoreEngine::readSettingsArray(QAnyStringView group)
{
    if (group.isEmpty())
    {
        logCritical(coreEngine) << "Provided key group is empty";

        return {};
    }

    QWriteLocker lock(&varLock);
    QVariantList list;
    qsizetype    size{this->settings->beginReadArray(group)};

    logDebug(coreEngine) << "Array size for \"" << group << "\" is " << size;

    for (qsizetype i = 0; i < size; ++i)
    {
        this->settings->setArrayIndex(static_cast<int>(i));

        QVariantMap map;

        logDebug(coreEngine) << "Child keys in array " << i << ": " << this->settings->childKeys();

        for (const QString &key : this->settings->childKeys())
        {
            const QVariant value{readValueOfKey(key)};

            logDebug(coreEngine) << "Value of key \"" << key << "\": " << value;

            map.insert(key, value);
        }

        list.append(map);
    }

    this->settings->endArray();

    return list;
}

void CoreEngine::writeSettings(QAnyStringView key, const QVariant &value)
{
    if (key.isEmpty())
    {
        logCritical(coreEngine) << "Provided key is empty";

        return;
    }

    QWriteLocker lock(&varLock);

    this->setValueOfKey(key, value, true);
}

void CoreEngine::writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps)
{
    if (group.isEmpty())
    {
        logCritical(coreEngine) << "Provided key group is empty";

        return;
    }

    QWriteLocker lock(&varLock);

    this->settings->beginWriteArray(group);

    for (qsizetype i = 0; i < entryMaps.size(); ++i)
    {
        const QVariantMap &map{entryMaps.at(i).toMap()};

        this->settings->setArrayIndex(static_cast<int>(i));

        for (const auto &[key, value] : map.asKeyValueRange())
            this->setValueOfKey(key, value);
    }

    this->settings->endArray();

    this->settings->sync();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreEngine::eventFilter(QObject *object, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::LocaleChange:
        // OS-level change
        logDebug(coreEngine) << "Locale change detected";
        this->applyLocale();
        return true;
    case QEvent::LanguageChange:
        // App-level change, e.g. language selected from options
        logDebug(coreEngine) << "New translations loaded";
        emit this->localeChanged();
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
//             logCritical(coreEngine) << "Config busy, could not acquire lock";
//             e = EBUSY;
//             break;
//         case QLockFile::PermissionError:
//             logCritical(coreEngine) << "Permission error in config, could not acquire lock";
//             e = EACCES;
//             break;
//         default:
//             logCritical(coreEngine) << "IO error in config, could not acquire lock";
//             break;
//         }

//         this->threadLock.unlock();

//         return e;
//     }

//     return 0;
// }

void CoreEngine::applyLocale()
{
    qApp->removeTranslator(&translator); // no-op if none installed

    QLocale l(QLocale::system());

    if (l.language() == QLocale::Arabic || l.language() == QLocale::Bangla)
    {
        l = QLocale(l.language(), l.territory());
        QLocale::setDefault(l);
        if (translator.load(l, QLatin1String("munadi"), QLatin1String("_"),
                            QLatin1String(":/qt/qml/Munadi/i18n")))
            qApp->installTranslator(&translator);
    }
    else
    {
        QLocale::setDefault(QLocale(QLocale::English, QLocale::UnitedStates));
    }
}

void CoreEngine::onAboutToQuit()
{ emit this->coreOffline(); }

QVariant CoreEngine::readValueOfKey(QAnyStringView key, const QVariant &defaultValue)
{
    bool           invalidKey = false;
    const QVariant value{this->settings->value(key)};

    switch (this->settings->status())
    {
    case QSettings::AccessError:
        logWarning(coreEngine) << "Failed to read config";
        break;
    case QSettings::FormatError:
        logWarning(coreEngine) << "Config for key \"" << key
                               << "\" is malformed, returning default value";
        invalidKey = true;
        break;
    default:
        break;
    }

    if (value.isNull())
    {
        logWarning(coreEngine) << "Failed to fetch a valid value for key \"" << key
                               << "\", returning default value";

        invalidKey = true;
    }

    return (invalidKey) ? defaultValue : value;
}

// void CoreEngine::releaseConfigLock()
// {
//     this->configLock->unlock();
// }

void CoreEngine::setValueOfKey(QAnyStringView key, const QVariant &value, bool flushOnWrite)
{
    if (value.isNull())
        logWarning(coreEngine) << "Value is empty. Make sure this is intended.";

    this->settings->setValue(key, value);

    if (flushOnWrite)
        this->settings->sync();

    switch (this->settings->status())
    {
    case QSettings::AccessError:
        logWarning(coreEngine) << "Failed to read config";
        break;
    case QSettings::FormatError:
        logWarning(coreEngine) << "Config for key \"" << key
                               << "\" is malformed, returning default value";
        break;
    default:
        break;
    }
}
} // namespace Munadi