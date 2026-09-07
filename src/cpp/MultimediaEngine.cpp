#include "internal-helpers.hpp"
#include <MultimediaEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "MultimediaEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(multimediaEngine, "MultimediaEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

MultimediaEngine::MultimediaEngine(QObject *parent) : QObject{parent}, fileman{new FileMan()}
{
    if (MultimediaEngine::s_singletonInstance != nullptr)
        logFatal(multimediaEngine)
            << "A second instance was constructed; only one instance may exist per process";

    logDebug(multimediaEngine) << "FileMan::refreshStore() return: " << fileman->refreshStore();

    this->defaultAthanConfig();
    this->defaultAthanList();
    QTimer::singleShot(0, this, &MultimediaEngine::connectToCoreOnline);

    MultimediaEngine::s_singletonInstance = this;
}

MultimediaEngine::~MultimediaEngine()
{
    QObject::disconnect(this->coreOfflineConnection);
    QObject::disconnect(this->coreOnlineConnection);

    if (MultimediaEngine::s_singletonInstance == this)
        MultimediaEngine::s_singletonInstance = nullptr;
}

MultimediaEngine &MultimediaEngine::instance()
{
    if (MultimediaEngine::s_singletonInstance == nullptr)
        logFatal(multimediaEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *MultimediaEngine::s_singletonInstance;
}

QVariant MultimediaEngine::getAthanConfig(Prayer prayerEnum, QAnyStringView field)
{
    if (!this->athanConfigKeys.contains(field.toString()))
    {
        logCritical(multimediaEngine) << field << "is not a valid field in AthanConfig";
        return {};
    }

    return this->m_athanConfig.at(qToUnderlying(prayerEnum)).toMap().value(field.toString());
}

void MultimediaEngine::addAthan(const QUrl &fileUrl)
{
    const add_file_t result = fileman->addFile(fileUrl, FileMan::Audio | FileMan::Video);

    if (result.status() != 0 && result.status() != EEXIST)
    {
        logCritical(multimediaEngine)
            << "FileMan::addFile() failed with error: " << result.status();

        return;
    }

    this->m_athanList.append(QVariantMap{
        {"name",       FileMan::getFileName(fileUrl)},
        {"path", result.location().toDisplayString()}
    });

    emit this->athanListChanged(this->m_athanList);
}

void MultimediaEngine::modifyAthanConfig(Prayer prayerEnum, const QVariantMap &entry)
{
    const char *key{PrayerEngine::metaPrayerEnum().valueToKey(prayerEnum)};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(multimediaEngine)
            << "\"" << prayerEnum << "\" does not resolve to a valid Prayer enum";

        return;
    }

    const QString enumName{QString(PrayerEngine::metaPrayerEnum().scope()) + "::" + key};

    if (prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
    {
        logWarning(multimediaEngine) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    if (!isValidEntryList(QVariantList{entry}, this->athanConfigMapSize, this->athanConfigKeys, 1))
    {
        logCritical(multimediaEngine) << "[" << entry << "] is not a valid entry in athanConfig";

        return;
    }

    QWriteLocker lock{&this->varLock};
    QVariantMap  tempMap{this->m_athanConfig.at(qToUnderlying(prayerEnum)).toMap()};

    if (tempMap == entry)
    {
        logDebug(multimediaEngine) << "Skipping setting identical entry";

        return;
    }

    tempMap.insert(entry);
    this->m_athanConfig.replace(qToUnderlying(prayerEnum), tempMap);

    emit athanConfigChanged(this->m_athanConfig);
}

void MultimediaEngine::removeAthan(const qsizetype &rmIdx)
{
    if (rmIdx < this->defaultAthans || rmIdx >= m_athanList.size())
    {
        logCritical(multimediaEngine) << "Invalid index value \"" << rmIdx << "\"";

        return;
    }

    const QUrl      fileUrl{this->m_athanList.at(rmIdx).toMap().value("path").toUrl()};
    const rm_file_t result(fileman->rmFile(QUrl(fileUrl)));

    logDebug(multimediaEngine) << "Result: " << result.status_l() << " " << result.status_h();

    if (result.status_l() != 0 && result.status_l() != ENOENT)
    {
        logCritical(multimediaEngine)
            << "Fileman::rmFile() failed with error: " << result.status_l();

        return;
    }

    QWriteLocker lock{&this->varLock};

    for (int i = 0; i < this->athanConfigListSize; ++i)
    {
        QVariantMap tempMap{m_athanConfig.at(i).toMap()};
        const int   currentIdx{tempMap.value("athan_idx").toInt()};

        if (currentIdx == rmIdx)
        {
            switch (static_cast<Prayer>(i))
            {
            case Prayer::Fajr:
                tempMap.insert("athan_idx", MultimediaEngine::defaultFajrAthan);
                break;
            case Prayer::Maghrib:
                tempMap.insert("athan_idx", MultimediaEngine::defaultMaghribAthan);
                break;
            default:
                tempMap.insert("athan_idx", MultimediaEngine::defaultAthan);
                break;
            }
        }
        else if (currentIdx > rmIdx)
            tempMap.insert("athan_idx", currentIdx - 1);

        this->m_athanConfig.replace(i, tempMap);
    }

    this->m_athanList.removeAt(rmIdx);
    this->m_athanList.squeeze();

    CoreEngine::instance().removeSettingsKey("Athans/"_L1 + QString::number(rmIdx + 1));

    emit this->athanListChanged(this->m_athanList);
    emit this->athanConfigChanged(this->m_athanConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::connectToCoreOffline()
{
    this->coreOfflineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOffline, this,
                         &MultimediaEngine::onCoreOffline, Qt::DirectConnection);

    if (this->coreOfflineConnection == nullptr)
        logWarning(multimediaEngine) << "coreOffline signal not connected";
    else
        logDebug(multimediaEngine) << "coreOffline signal connected";
}

void MultimediaEngine::connectToCoreOnline()
{
    if (!CoreEngine::exists())
    {
        // CoreEngine not constructed yet on this tick; retry next tick.
        logDebug(multimediaEngine) << "CoreEngine doesn't exist yet";
        QTimer::singleShot(0, this, &MultimediaEngine::connectToCoreOnline);
        return;
    }

    this->coreOnlineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOnline, this,
                         &MultimediaEngine::onCoreOnline, Qt::DirectConnection);

    if (this->coreOnlineConnection == nullptr)
        logWarning(multimediaEngine) << "coreOnline signal not connected";
    else
        logDebug(multimediaEngine) << "coreOnline signal connected";

    // coreOnline() already fired before we connected; manually reconcile
    logDebug(multimediaEngine) << "CoreEngine already online; catching up";

    this->onCoreOnline();
}

void MultimediaEngine::defaultAthanConfig()
{
    const std::double_t defaultVolume{1.0}; // 0.0 - 1.0
    const std::double_t defaultFajrVolume{0.2};
    const std::double_t defaultIshaVolume{0.5};

    this->m_athanConfig = {
        QVariantMap{   {"athan_idx", defaultFajrAthan}, {"volume", defaultFajrVolume}}, // [0] Fajr
        QVariantMap{       {"athan_idx", defaultAthan},     {"volume", defaultVolume}}, // [1] Sunrise
        QVariantMap{       {"athan_idx", defaultAthan},     {"volume", defaultVolume}}, // [2] Dhuhr
        QVariantMap{       {"athan_idx", defaultAthan},     {"volume", defaultVolume}}, // [3] Asr
        QVariantMap{{"athan_idx", defaultMaghribAthan},     {"volume", defaultVolume}}, // [4] Maghrib
        QVariantMap{       {"athan_idx", defaultAthan}, {"volume", defaultIshaVolume}}  // [5] Isha
    };

    const QVariantMap maps{this->m_athanConfig.at(0).toMap()};

    this->athanConfigListSize = this->m_athanConfig.size();
    this->athanConfigMapSize  = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->athanConfigKeys.insert(key, value.typeId());
}

void MultimediaEngine::defaultAthanList()
{
    this->m_athanList = {
        QVariantMap{   {"name", "Makkah (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-makkah.ogg"}  },
        QVariantMap{{"name", "Ar-Rashid (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-arRashid.ogg"}},
        QVariantMap{     {"name", "Fajr (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/fajr-athan.ogg"}    }
    };

    const QVariantMap maps{this->m_athanList.at(0).toMap()};

    this->athanListMapSize = maps.size();
    this->defaultAthans    = this->m_athanList.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->athanListKeys.insert(key, value.typeId());
}

void MultimediaEngine::loadConfigs()
{
    logDebug(multimediaEngine) << "Loading athan settings from config file";

    QVariantList cfgList(this->m_athanConfig); // don't use cfgList{}

    for (qsizetype i = 0; i < this->athanConfigListSize; ++i)
    {
        const Prayer  prayerEnum{static_cast<Prayer>(i)};
        const QString prayerName{PrayerEngine::toString(prayerEnum)};

        if (prayerName.isEmpty() || prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
            continue;

        QVariantMap entry{m_athanConfig.at(i).toMap()};

        for (const QString &key : this->athanConfigKeys.keys())
        {
            QVariant cfgVal{CoreEngine::instance().readSettings(
                "AthanConfig/" + prayerName + "/" + key, entry.value(key))};

            logDebug(multimediaEngine)
                << "Value for key \"" << key << "\" in list item " << i << ": " << cfgVal;

            // should go before isValidEntry check
            if (!cfgVal.convert(QMetaType(this->athanConfigKeys.value(key))))
            {
                logWarning(multimediaEngine) << "Failed converting stored value for \"" << key
                                             << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgList.replace(i, entry);
    }

    this->setAthanConfig(cfgList);

    logDebug(multimediaEngine) << "Loading athan list from config file";

    this->setAthanList(CoreEngine::instance().readSettingsArray("Athans"_L1));
}

void MultimediaEngine::saveConfigs()
{
    logDebug(multimediaEngine) << "Saving athan configurations";

    for (qsizetype i = 0; i < this->athanConfigListSize; ++i)
    {
        const Prayer  prayerEnum{static_cast<Prayer>(i)};
        const QString prayerName{PrayerEngine::toString(prayerEnum)};

        if (prayerName == nullptr || prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
            continue;

        for (const auto &[key, value] : m_athanConfig.at(i).toMap().asKeyValueRange())
            CoreEngine::instance().writeSettings("AthanConfig/"_L1 + prayerName + '/'_L1 + key,
                                                 value);
    }

    // athanList save
    logDebug(multimediaEngine) << "Saving athan list";

    CoreEngine::instance().writeSettingsArray("Athans"_L1, this->m_athanList);
}

void MultimediaEngine::setAthanConfig(const QVariantList &athanConfig)
{
    if (athanConfig.isEmpty())
    {
        logDebug(multimediaEngine) << "Skipping empty config list";

        return;
    }

    if (athanConfig == this->m_athanConfig)
    {
        logDebug(multimediaEngine) << "Skipping setting identical list";

        return;
    }

    if (!isValidEntryList(athanConfig, this->athanConfigMapSize, this->athanConfigKeys,
                          this->athanConfigListSize))
    {
        logCritical(multimediaEngine)
            << "Config [" << athanConfig << "] is not a valid configuration";

        return;
    }

    QWriteLocker lock{&this->varLock};

    this->m_athanConfig = athanConfig;

    emit this->athanConfigChanged(this->m_athanConfig);
}

void MultimediaEngine::setAthanList(const QVariantList &athanList)
{
    if (athanList.isEmpty())
    {
        logDebug(multimediaEngine) << "Skipping empty athan list";

        return;
    }

    if (athanList == this->m_athanList)
    {
        logDebug(multimediaEngine) << "Skipping setting identical list";

        return;
    }

    if (athanList.first(this->defaultAthans) != this->m_athanList.first(this->defaultAthans))
    {
        logCritical(multimediaEngine) << "Dropping malformed list [" << athanList << "]";
        logDebug(multimediaEngine) << "List attempts to modify protected default list";

        return;
    }

    if (!isValidEntryList(athanList, this->athanListMapSize, this->athanListKeys))
    {
        logCritical(multimediaEngine) << "Dropping malformed list [" << athanList << "]";

        return;
    }

    QWriteLocker lock{&this->varLock};

    this->m_athanList = athanList;

    emit this->athanListChanged(this->m_athanList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::onCoreOffline()
{ this->saveConfigs(); }

void MultimediaEngine::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
}

} // namespace Munadi