#include "internal-helpers.hpp"
#include <MultimediaEngine.hpp>
#include <QAudioOutput>

Q_STATIC_LOGGING_CATEGORY(munadi, "MultimediaEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

MultimediaEngine *MultimediaEngine::create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */)
{
    logDebug(munadi) << "Creating instance from QML call";

    qmlEngine->singletonInstance<PrayerEngine *>("Munadi"_L1, "PrayerEngine"_L1);

    return new MultimediaEngine{};
}

void MultimediaEngine::addAthan(const QUrl &fileUrl) noexcept
{
    const add_file_t result = this->fileman->addFile(fileUrl, FileMan::Audio | FileMan::Video);

    if (result.status() != 0 && result.status() != EEXIST)
    {
        logCritical(munadi) << "FileMan::addFile() failed with error: " << result.status();

        return;
    }

    {
        const QWriteLocker lock{&this->varLock};

        this->m_athanList.append(QVariantMap{
            {u"name"_s, FileMan::getFileName(fileUrl)      },
            {u"path"_s, result.location().toDisplayString()}
        });
    }

    emit this->athanListChanged();
}

// QMediaPlayer::play()/stop() toggles playbackState property and in-turn emits playbackStateChanged
// signal synchronously. Mind the slots bound to these signals when adding read/write locks
void MultimediaEngine::athanPlay() noexcept
{
    {
        const QWriteLocker lock{&this->varLock};
        const QString      prayerName{PrayerEngine::toString(pEngine->currentPrayer())};
        const int          athanIdx{
            this->m_athanConfig.value(prayerName).toHash().value("athan_idx"_L1).toInt()};
        const QUrl src{this->m_athanList.at(athanIdx).toMap().value("path"_L1).toUrl()};

        this->athanPlayer->setAudioOutput(this->athanOutput);
        this->athanOutput->setMuted(this->m_athanMuted.load());
        this->athanOutput->setVolume(
            this->m_athanConfig.value(prayerName).toHash().value("volume"_L1).toFloat());

        if (const float volume{this->athanOutput->volume()};
            volume <= 0 || this->athanOutput->isMuted())
            return;

        logDebug(munadi) << "Athan file: " << src.toDisplayString();
        this->athanPlayer->setSource(src);
    }

    if (!this->isAthanPlaying())
        this->athanPlayer->play();
    else
        logDebug(munadi) << "Play ignored; media already playing";
}

void MultimediaEngine::athanStop() noexcept
{
    if (this->isAthanPlaying())
    {
        this->athanPlayer->stop();

        const QWriteLocker lock{&this->varLock};

        this->athanPlayer->setAudioOutput(nullptr);
    }
    else
        logDebug(munadi) << "Stop ignored; media is not playing";
}

QVariant MultimediaEngine::getAthanConfig(Prayer prayerEnum, const QString &field) noexcept
{
    const QReadLocker lock{&this->varLock};

    if (!this->athanConfigFields.contains(field))
    {
        logCritical(munadi) << field << "is not a valid field in AthanConfig";
        return {};
    }

    return this->m_athanConfig.value(PrayerEngine::toString(prayerEnum)).toHash().value(field);
}

const QLoggingCategory &MultimediaEngine::logCategory() noexcept
{ return munadi(); }

void MultimediaEngine::modifyAthanConfig(const QString &key, const QVariantHash &entry) noexcept
{
    const int enumVal{PrayerEngine::metaPrayerEnum().keyToValue(qPrintable(key))};

    if (enumVal == -1)
    {
        logCritical(munadi) << key << " resolves to unknown enum value";

        return;
    }

    const QString enumName{QString{PrayerEngine::metaPrayerEnum().scope()} + u"::"_s + key};

    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(munadi) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    {
        const QWriteLocker lock{&this->varLock};
        QVariantHash       tempMap{this->m_athanConfig.value(key).toHash()};

        if (!isValidEntry(entry, this->athanConfigEntrySize, this->athanConfigFields))
        {
            logCritical(munadi) << "[" << entry << "] is not a valid entry in athanConfig";

            return;
        }

        if (tempMap == entry)
        {
            logDebug(munadi) << "Skipping setting identical entry";

            return;
        }

        tempMap.insert(entry);
        this->m_athanConfig.insert(key, tempMap);
    }

    emit this->athanConfigChanged();
}

void MultimediaEngine::previewPlay(int athanListIdx) noexcept
{
    {
        const QWriteLocker lock{&this->varLock};

        if (athanListIdx < 0 || athanListIdx >= this->m_athanList.size())
        {
            logCritical(munadi) << "Index " << athanListIdx << " out of valid range";

            return;
        }

        this->previewPlayer->setAudioOutput(this->previewOutput);
        this->previewPlayer->setSource(
            this->m_athanList.at(athanListIdx).toMap().value("path"_L1).toUrl());
        // this->previewOutput->setVolume(1.0) // not necessary, defaults to 1.0
    }

    if (!this->isPreviewPlaying())
        this->previewPlayer->play();
    else
        logDebug(munadi) << "Play ignored; media already playing";
}

void MultimediaEngine::previewStop() noexcept
{
    if (this->previewPlayer->isPlaying())
    {
        this->previewPlayer->stop();

        const QWriteLocker lock{&this->varLock};

        this->previewPlayer->setAudioOutput(nullptr);
    }
    else
        logDebug(munadi) << "Stop ignored; media is not playing";
}

void MultimediaEngine::removeAthan(qsizetype rmIdx) noexcept
{
    {
        const QWriteLocker lock{&this->varLock};

        if (rmIdx < this->defaultAthans || rmIdx >= this->m_athanList.size())
        {
            logCritical(munadi) << "Invalid index value \"" << rmIdx << "\"";

            return;
        }

        const QUrl      fileUrl{this->m_athanList.at(rmIdx).toMap().value("path"_L1).toUrl()};
        const rm_file_t result(this->fileman->rmFile(QUrl(fileUrl)));

        logDebug(munadi) << "Result: " << result.status_l() << " " << result.status_h();

        if (result.status_l() != 0 && result.status_l() != ENOENT && result.status_l() != EINVAL)
        {
            logCritical(munadi) << "Fileman::rmFile() failed with error: " << result.status_l();

            return;
        }

        for (const auto &[field, entry] : this->m_athanConfig.asKeyValueRange())
        {
            QVariantHash map{entry.toHash()};
            const int    currentIdx{map.value("athan_idx"_L1).toInt()};

            if (currentIdx == rmIdx)
            {
                if (field == u"Fajr"_s)
                    map.insert("athan_idx"_L1, MultimediaEngine::defaultFajrAthan);
                else if (field == u"Maghrib"_s)
                    map.insert("athan_idx"_L1, MultimediaEngine::defaultMaghribAthan);
                else
                    map.insert("athan_idx"_L1, MultimediaEngine::defaultAthan);
            }
            else if (currentIdx > rmIdx)
                map.insert("athan_idx"_L1, currentIdx - 1);

            this->m_athanConfig.insert(field, map);
        }

        this->m_athanList.removeAt(rmIdx);
        this->m_athanList.squeeze();
    }

    emit this->athanListChanged();
    emit this->athanConfigChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

MultimediaEngine::MultimediaEngine(QObject *parent)
    : MunadiBase{munadi(), parent}, athanOutput{new QAudioOutput{this}},
      athanPlayer{new QMediaPlayer{this}}, fileman{new FileMan{this}},
      previewOutput{new QAudioOutput{this}}, previewPlayer{new QMediaPlayer{this}}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });

    logDebug(munadi) << "FileMan::refreshStore() return: " << this->fileman->refreshStore();

    this->athanPlayer->setAudioOutput(this->athanOutput);

    if (const QMetaObject::Connection conn =
            QObject::connect(this->athanPlayer, &QMediaPlayer::playbackStateChanged, this,
                             &MultimediaEngine::onAthanPlaybackChanged);
        conn == nullptr)
        logWarning(munadi) << "Failed to establish athan playback state connection";
    else
        logDebug(munadi) << "Athan playback state connection established";

    if (const QMetaObject::Connection conn =
            QObject::connect(this->previewPlayer, &QMediaPlayer::playbackStateChanged, this,
                             &MultimediaEngine::onPreviewPlaybackChanged);
        conn == nullptr)
        logWarning(munadi) << "Failed to establish preview playback state connection";
    else
        logDebug(munadi) << "Preview playback state connection established";

    this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::connectToAthanAlarm() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::athanAlarm, this, &MultimediaEngine::onAthanAlarm);
        conn == nullptr)
        logWarning(munadi) << "athanAlarm signal not connected";
    else
        logDebug(munadi) << "athanAlarm signal connected";
}

void MultimediaEngine::defaultAthanConfig() noexcept
{
    const std::double_t defaultVolume{1.0}; // 0.0 - 1.0
    const std::double_t defaultFajrVolume{0.2};
    const std::double_t defaultIshaVolume{0.5};

    this->configFields = {
        {u"AllMuted"_s, QMetaType::Bool},
    };

    this->m_athanConfig = {
        {u"Fajr"_s,    QVariantHash{{u"athan_idx"_s, MultimediaEngine::defaultFajrAthan},
                                 {u"volume"_s, defaultFajrVolume}}  },
        {u"Dhuhr"_s,   QVariantHash{{u"athan_idx"_s, MultimediaEngine::defaultAthan},
                                  {u"volume"_s, defaultVolume}}    },
        {u"Asr"_s,     QVariantHash{{u"athan_idx"_s, MultimediaEngine::defaultAthan},
                                {u"volume"_s, defaultVolume}}        },
        {u"Maghrib"_s, QVariantHash{{u"athan_idx"_s, MultimediaEngine::defaultMaghribAthan},
                                    {u"volume"_s, defaultVolume}}},
        {u"Isha"_s,    QVariantHash{{u"athan_idx"_s, MultimediaEngine::defaultAthan},
                                 {u"volume"_s, defaultIshaVolume}}  }
    };

    const QVariantHash maps{this->m_athanConfig.value("Fajr"_L1).toHash()};

    this->athanConfigKeys =
        QSet(this->m_athanConfig.keyBegin(), this->m_athanConfig.keyEnd()); // do not use braces {}
    this->athanConfigMapSize   = this->m_athanConfig.size();
    this->athanConfigEntrySize = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->athanConfigFields.insert(key, value.typeId());
}

void MultimediaEngine::defaultAthanList() noexcept
{
    this->m_athanList = {
        QVariantMap{{u"name"_s, u"Makkah (built-in)"_s},
                    {u"path"_s, u"qrc:/qt/qml/Munadi/data/athan-makkah.ogg"_s}  },
        QVariantMap{{u"name"_s, u"Ar-Rashid (built-in)"_s},
                    {u"path"_s, u"qrc:/qt/qml/Munadi/data/athan-arRashid.ogg"_s}},
        QVariantMap{{u"name"_s, u"Fajr (built-in)"_s},
                    {u"path"_s, u"qrc:/qt/qml/Munadi/data/fajr-athan.ogg"_s}    }
    };

    const QVariantMap maps{this->m_athanList.at(0).toMap()};

    this->athanListMapSize = maps.size();
    this->defaultAthans    = this->m_athanList.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->athanListKeys.insert(key, value.typeId());
}

void MultimediaEngine::defaultConfigs() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->m_athanMuted.store(false);
    this->defaultAthanConfig();
    this->defaultAthanList();
}

QVariantList &MultimediaEngine::getAthanList() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_athanList;
}

// Do not add any readlocker here
bool MultimediaEngine::isAthanPlaying() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->athanPlayer->isPlaying();
}

// Do not add any readlocker here
bool MultimediaEngine::isPreviewPlaying() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->previewPlayer->isPlaying();
}

void MultimediaEngine::loadAthanConfig(bool &configMigrated) noexcept
{
    QVariantHash       cfgMap{this->m_athanConfig};
    QVariantList       oldAthanConfig;
    const QWriteLocker lock{&this->varLock};
    const bool         migrated{core->readSettings("MultimediaEngineConfigMigrated"_L1).toBool()};

    if (!migrated)
        oldAthanConfig = core->readOldSettingsJsArray("Adhan/prayerConfig"_L1);

    if (!oldAthanConfig.isEmpty())
        logInfo(munadi) << "Migrating from old config";

    for (const QString &prayerName : std::as_const(this->athanConfigKeys))
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

        for (const auto &[key, value] : this->athanConfigFields.asKeyValueRange())
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

void MultimediaEngine::loadAthanList(bool &configMigrated) noexcept
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

void MultimediaEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading athan settings from config file";

    bool          configMigrated{true};
    const QString migrateKey{u"PrayerEngineConfigMigrated"_s};

    this->loadAthanConfig(configMigrated);
    this->loadAthanList(configMigrated);
    loadConfigKey("Adhan/athanMuted"_L1, "AthanConfig/AllMuted"_L1, migrateKey, this,
                  "athanMuted"_L1, configMigrated);

    if (configMigrated)
        core->writeSettings("MultimediaEngineConfigMigrated"_L1, true);
}

void MultimediaEngine::saveConfigs() noexcept
{
    const QReadLocker lock{&this->varLock};

    // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
    // (later) settings are not saved

    logDebug(munadi) << "Saving athan configurations";

    for (const QString &prayerName : std::as_const(this->athanConfigKeys))
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        for (const auto &[key, value] :
             this->m_athanConfig.value(prayerName).toHash().asKeyValueRange())
            core->writeSettings("AthanConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
    }

    logDebug(munadi) << "Saving athan list";

    core->writeSettingsArray("Athans"_L1, this->m_athanList);
    core->writeSettings("AthanConfig/AllMuted"_L1, this->m_athanMuted.load(), true);
}

void MultimediaEngine::setAthanConfig(const QVariantHash &athanConfig) noexcept
{
    if (athanConfig.isEmpty())
    {
        logDebug(munadi) << "Skipping empty config list";

        return;
    }

    {
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
    }

    emit this->athanConfigChanged();
}

void MultimediaEngine::setAthanList(const QVariantList &athanList) noexcept
{
    if (athanList.isEmpty())
    {
        logDebug(munadi) << "Skipping empty athan list";

        return;
    }

    {
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
    }

    emit this->athanListChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::onAthanAlarm() noexcept
{
    logDebug(munadi) << "Received athan alarm at: " << QDateTime::currentDateTime().toString();
    this->athanPlay();
}

void MultimediaEngine::onAthanPlaybackChanged(QMediaPlayer::PlaybackState state)
{
    switch (state)
    {
    case QMediaPlayer::PlayingState:
        logInfo(munadi) << "Athan playing";
        break;
    case QMediaPlayer::StoppedState:
        logInfo(munadi) << "Athan stopped";
        break;
    case QMediaPlayer::PausedState:
        logInfo(munadi) << "Athan paused";
        break;
    }

    emit this->athanPlayingChanged(state == QMediaPlayer::PlayingState);
}

void MultimediaEngine::onCoreOnline() noexcept
{
    MunadiBase::onCoreOnline();
    this->connectToAthanAlarm();
}

void MultimediaEngine::onPreviewPlaybackChanged(QMediaPlayer::PlaybackState state)
{
    switch (state)
    {
    case QMediaPlayer::PlayingState:
        logInfo(munadi) << "Preview playing";
        break;
    case QMediaPlayer::StoppedState:
        logInfo(munadi) << "Preview stopped";
        break;
    case QMediaPlayer::PausedState:
        logInfo(munadi) << "Preview paused";
        break;
    }

    emit this->previewPlayingChanged(state == QMediaPlayer::PlayingState);
}
} // namespace Munadi