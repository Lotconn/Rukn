#include "internal-helpers.hpp"
#include <MultimediaEngine.hpp>
#include <QAudioOutput>

Q_STATIC_LOGGING_CATEGORY(munadi, "MultimediaEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

MultimediaEngine::MultimediaEngine(QObject *parent)
    : MunadiEngine<MultimediaEngine>{parent}, athanOutput{new QAudioOutput(this)},
      athanPlayer{new QMediaPlayer(this)}, fileman{new FileMan(this)},
      previewOutput{new QAudioOutput(this)}, previewPlayer{new QMediaPlayer(this)}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });

    logDebug(munadi) << "FileMan::refreshStore() return: " << fileman->refreshStore();

    this->athanPlayer->setAudioOutput(this->athanOutput);

    if (const QMetaObject::Connection conn = QObject::connect(
            this->athanPlayer, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) -> void {
                emit this->athanPlayingChanged(state == QMediaPlayer::PlayingState);
            });
        conn == nullptr)
        logWarning(munadi) << "Failed to establish athan playback state connection";
    else
        logDebug(munadi) << "Athan playback state connection established";

    if (const QMetaObject::Connection conn = QObject::connect(
            this->previewPlayer, &QMediaPlayer::playbackStateChanged, this,
            [this](QMediaPlayer::PlaybackState state) -> void {
                emit this->previewPlayingChanged(state == QMediaPlayer::PlayingState);
            });
        conn == nullptr)
        logWarning(munadi) << "Failed to establish preview playback state connection";
    else
        logDebug(munadi) << "Preview playback state connection established";

    this->makeOnline();
}

QVariant MultimediaEngine::getAthanConfig(Prayer prayerEnum, QAnyStringView field)
{
    if (!this->athanConfigFields.contains(field.toString()))
    {
        logCritical(munadi) << field << "is not a valid field in AthanConfig";
        return {};
    }

    return this->m_athanConfig.value(PrayerEngine::toString(prayerEnum))
        .toHash()
        .value(field.toString());
}

void MultimediaEngine::addAthan(const QUrl &fileUrl)
{
    const add_file_t result = fileman->addFile(fileUrl, FileMan::Audio | FileMan::Video);

    if (result.status() != 0 && result.status() != EEXIST)
    {
        logCritical(munadi) << "FileMan::addFile() failed with error: " << result.status();

        return;
    }

    this->m_athanList.append(QVariantMap{
        {"name",       FileMan::getFileName(fileUrl)},
        {"path", result.location().toDisplayString()}
    });

    emit this->athanListChanged(this->m_athanList);
}

void MultimediaEngine::athanStop()
{
    if (this->athanPlayer->isPlaying())
    {
        this->athanPlayer->stop();
        this->athanPlayer->setAudioOutput(nullptr);
    }
    else
        logDebug(munadi) << "Stop ignored; media is not playing";
}

const QLoggingCategory &MultimediaEngine::logCategory()
{ return munadi(); }

void MultimediaEngine::modifyAthanConfig(const QString &key, const QVariantHash &entry)
{
    const int enumVal{PrayerEngine::metaPrayerEnum().keyToValue(qPrintable(key))};

    if (enumVal == -1)
    {
        logCritical(munadi) << key << " resolves to unknown enum value";

        return;
    }

    const QString enumName{QString(PrayerEngine::metaPrayerEnum().scope()) + "::" + key};

    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(munadi) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    if (!isValidEntry(entry, this->athanConfigEntrySize, this->athanConfigFields))
    {
        logCritical(munadi) << "[" << entry << "] is not a valid entry in athanConfig";

        return;
    }

    const QWriteLocker lock{&this->varLock};
    QVariantHash       tempMap{this->m_athanConfig.value(key).toHash()};

    if (tempMap == entry)
    {
        logDebug(munadi) << "Skipping setting identical entry";

        return;
    }

    tempMap.insert(entry);
    this->m_athanConfig.insert(key, tempMap);
    emit athanConfigChanged(this->m_athanConfig);
}

void MultimediaEngine::athanPlay()
{
    const QString prayerName{PrayerEngine::toString(PrayerEngine::instance()->currentPrayer())};
    const int     athanIdx{
        this->m_athanConfig.value(prayerName).toHash().value("athan_idx"_L1).toInt()};

    this->athanPlayer->setAudioOutput(this->athanOutput);
    this->athanOutput->setMuted(this->m_athanMuted);
    this->athanOutput->setVolume(
        this->m_athanConfig.value(prayerName).toHash().value("volume"_L1).toFloat());

    if (const float volume{this->athanOutput->volume()};
        volume <= 0 || this->athanOutput->isMuted())
        return;

    this->athanPlayer->setSource(this->m_athanList.at(athanIdx).toMap().value("path"_L1).toUrl());
    this->athanPlayer->play();
}

void MultimediaEngine::previewPlay(int athanListIdx)
{
    if (athanListIdx < 0 || athanListIdx >= this->m_athanList.size())
    {
        logCritical(munadi) << "Index " << athanListIdx << " out of valid range";

        return;
    }

    this->previewPlayer->setAudioOutput(this->previewOutput);
    this->previewPlayer->setSource(
        this->m_athanList.at(athanListIdx).toMap().value("path"_L1).toUrl());
    // this->previewOutput->setVolume(1.0) // not necessary, defaults to 1.0
    this->previewPlayer->play();
}

void MultimediaEngine::previewStop()
{
    if (this->previewPlayer->isPlaying())
    {
        this->previewPlayer->stop();
        this->previewPlayer->setAudioOutput(nullptr);
    }
    else
        logDebug(munadi) << "Stop ignored; media is not playing";
}

void MultimediaEngine::removeAthan(const qsizetype &rmIdx)
{
    const QWriteLocker lock{&this->varLock};

    if (rmIdx < this->defaultAthans || rmIdx >= m_athanList.size())
    {
        logCritical(munadi) << "Invalid index value \"" << rmIdx << "\"";

        return;
    }

    const QUrl      fileUrl{this->m_athanList.at(rmIdx).toMap().value("path").toUrl()};
    const rm_file_t result(fileman->rmFile(QUrl(fileUrl)));

    logDebug(munadi) << "Result: " << result.status_l() << " " << result.status_h();

    if (result.status_l() != 0 && result.status_l() != ENOENT && result.status_l() != EINVAL)
    {
        logCritical(munadi) << "Fileman::rmFile() failed with error: " << result.status_l();

        return;
    }

    for (const auto &[field, entry] : this->m_athanConfig.asKeyValueRange())
    {
        QVariantHash map{entry.toHash()};
        const int    currentIdx{map.value("athan_idx").toInt()};

        if (currentIdx == rmIdx)
        {
            if (field == "Fajr"_L1)
                map.insert("athan_idx", MultimediaEngine::defaultFajrAthan);
            else if (field == "Maghrib"_L1)
                map.insert("athan_idx", MultimediaEngine::defaultMaghribAthan);
            else
                map.insert("athan_idx", MultimediaEngine::defaultAthan);
        }
        else if (currentIdx > rmIdx)
            map.insert("athan_idx", currentIdx - 1);

        this->m_athanConfig.insert(field, map);
    }

    this->m_athanList.removeAt(rmIdx);
    this->m_athanList.squeeze();

    emit this->athanListChanged(this->m_athanList);
    emit this->athanConfigChanged(this->m_athanConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::defaultAthanConfig()
{
    const std::double_t defaultVolume{1.0}; // 0.0 - 1.0
    const std::double_t defaultFajrVolume{0.2};
    const std::double_t defaultIshaVolume{0.5};

    this->configFields = {
        {"AllMuted"_L1, QMetaType::Bool},
    };

    this->m_athanConfig = {
        {   "Fajr"_L1, QVariantHash{{"athan_idx", defaultFajrAthan}, {"volume", defaultFajrVolume}}},
        {  "Dhuhr"_L1,         QVariantHash{{"athan_idx", defaultAthan}, {"volume", defaultVolume}}},
        {    "Asr"_L1,         QVariantHash{{"athan_idx", defaultAthan}, {"volume", defaultVolume}}},
        {"Maghrib"_L1,  QVariantHash{{"athan_idx", defaultMaghribAthan}, {"volume", defaultVolume}}},
        {   "Isha"_L1,     QVariantHash{{"athan_idx", defaultAthan}, {"volume", defaultIshaVolume}}}
    };

    const QVariantHash maps{this->m_athanConfig.value("Fajr"_L1).toHash()};

    this->athanConfigKeys      = QSet(this->m_athanConfig.keyBegin(), this->m_athanConfig.keyEnd());
    this->athanConfigMapSize   = this->m_athanConfig.size();
    this->athanConfigEntrySize = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->athanConfigFields.insert(key, value.typeId());
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

void MultimediaEngine::defaultConfigs()
{
    this->defaultAthanConfig();
    this->defaultAthanList();
}

bool MultimediaEngine::isAthanPlaying()
{ return this->athanPlayer->isPlaying(); }

bool MultimediaEngine::isPreviewPlaying()
{ return this->previewPlayer->isPlaying(); }

void MultimediaEngine::loadAthanConfig(bool &configMigrated)
{
    QVariantHash cfgMap{this->m_athanConfig};
    QVariantList oldAthanConfig;
    const bool   migrated{core->readSettings("MultimediaEngineConfigMigrated"_L1).toBool()};

    if (!migrated)
        oldAthanConfig = core->readOldSettingsJsArray("Adhan/prayerConfig"_L1);

    if (!oldAthanConfig.isEmpty())
        logInfo(munadi) << "Migrating from old config";

    for (const QString &prayerName : this->athanConfigKeys)
    {
        const int i = PrayerEngine::metaPrayerEnum().keyToValue(qPrintable(prayerName));

        if (i == -1)
        {
            logCritical(munadi) << prayerName << " resolves to unknown enum value";

            continue;
        }

        if (static_cast<Prayer>(i) == Prayer::Sunrise || static_cast<Prayer>(i) == Prayer::None)
        {
            logWarning(munadi) << "Modifying \"" << prayerName << "\" is not allowed";

            return;
        }

        QVariantHash entry{this->m_athanConfig.value(prayerName).toHash()};

        for (const QString &key : this->athanConfigFields.keys())
        {
            QVariant cfgVal;

            cfgVal = (oldAthanConfig.isEmpty())
                         ? core->readSettings("AthanConfig/Prayer/"_L1 + prayerName + '/'_L1 + key,
                                              entry.value(key))
                         : oldAthanConfig.at(i).toHash().value(key);

            logDebug(munadi) << "Value for key \"" << key << "\" in list item " << prayerName
                             << ": " << cfgVal;

            if (!cfgVal.convert(QMetaType(this->athanConfigFields.value(key))))
            {
                logWarning(munadi) << "Failed converting stored value for \"" << key
                                   << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgMap.insert(prayerName, entry);
    }

    this->setAthanConfig(cfgMap);

    if (migrated)
        configMigrated = false;
}

void MultimediaEngine::loadAthanList(bool &configMigrated)
{
    if (core->readSettings("MultimediaEngineConfigMigrated"_L1).toBool())
    {
        this->setAthanList(core->readSettingsArray("Athans"_L1));

        configMigrated = false;

        return;
    }

    const QVariantList oldAthanList(core->readOldSettingsJsArray("Adhan/audioList"_L1));

    this->setAthanList(oldAthanList);
    logInfo(munadi) << "Migrated from old config";
}

void MultimediaEngine::loadConfigs()
{
    logDebug(munadi) << "Loading athan settings from config file";

    bool          configMigrated{true};
    const QString migrateKey{"PrayerEngineConfigMigrated"_L1};

    this->loadAthanConfig(configMigrated);
    this->loadAthanList(configMigrated);
    loadConfigKey("Adhan/athanMuted"_L1, "AthanConfig/AllMuted"_L1, migrateKey, this,
                  "athanMuted"_L1, configMigrated);

    if (configMigrated)
        core->writeSettings("MultimediaEngineConfigMigrated"_L1, true, false);
}

void MultimediaEngine::saveConfigs()
{
    logDebug(munadi) << "Saving athan configurations";

    for (const QString &prayerName : this->athanConfigKeys)
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        for (const auto &[key, value] : m_athanConfig.value(prayerName).toHash().asKeyValueRange())
            core->writeSettings("AthanConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
    }

    logDebug(munadi) << "Saving athan list";

    core->writeSettingsArray("Athans"_L1, this->m_athanList);
    core->writeSettings("AthanConfig/AllMuted"_L1, this->m_athanMuted.load());
}

void MultimediaEngine::setAthanConfig(const QVariantHash &athanConfig)
{
    if (athanConfig.isEmpty())
    {
        logDebug(munadi) << "Skipping empty config list";

        return;
    }

    const QWriteLocker lock{&this->varLock};

    if (athanConfig == this->m_athanConfig)
    {
        logDebug(munadi) << "Skipping setting identical list";

        return;
    }

    if (!isValidEntry(athanConfig, this->athanConfigEntrySize, this->athanConfigFields,
                      this->athanConfigMapSize))
    {
        logCritical(munadi) << "Config [" << athanConfig << "] is not a valid configuration";

        return;
    }

    this->m_athanConfig = athanConfig;

    emit this->athanConfigChanged(this->m_athanConfig);
}

void MultimediaEngine::setAthanList(const QVariantList &athanList)
{
    if (athanList.isEmpty())
    {
        logDebug(munadi) << "Skipping empty athan list";

        return;
    }

    const QWriteLocker lock{&this->varLock};

    if (athanList == this->m_athanList)
    {
        logDebug(munadi) << "Skipping setting identical list";

        return;
    }

    if (athanList.first(this->defaultAthans) != this->m_athanList.first(this->defaultAthans))
    {
        logCritical(munadi) << "Dropping malformed list [" << athanList << "]";
        logDebug(munadi) << "List attempts to modify protected default list";
        logDebug(munadi) << "\n"
                         << athanList.first(this->defaultAthans) << "\nvs\n"
                         << this->m_athanList.first(this->defaultAthans);

        return;
    }

    if (!isValidEntry(athanList, this->athanListMapSize, this->athanListKeys))
    {
        logCritical(munadi) << "Dropping malformed list [" << athanList << "]";

        return;
    }

    this->m_athanList = athanList;
    emit this->athanListChanged(this->m_athanList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Munadi