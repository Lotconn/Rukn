#include <QAudioOutput>
#include <QCoreApplication>
#include <QStandardPaths>

#include "internal-helpers.hpp"
#include <MultimediaEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(cat, "MultimediaEngine")

namespace App {

using Prayer = PrayerEngine::Prayer;

MultimediaEngine *MultimediaEngine::create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */) {
  logDebug(cat) << "Creating instance from QML call";

  qmlEngine->singletonInstance<PrayerEngine *>("App"_L1, "PrayerEngine"_L1);

  return new MultimediaEngine{};
}

void MultimediaEngine::addAdhan(const QUrl &fileUrl) noexcept {
  const add_file_t result = this->fileman->addFile(fileUrl, FileMan::Audio | FileMan::Video);

  if (result.status() != 0 && result.status() != EEXIST) {
    logCritical(cat) << "FileMan::addFile() failed with error: " << result.status();
    return;
  }

  this->m_adhanList.append(QVariantMap{
      {u"name"_s, FileMan::getFileName(fileUrl)      },
      {u"path"_s, result.location().toDisplayString()}
  });

  emit this->adhanListChanged();
}

// QMediaPlayer::play()/stop() toggles playbackState property and in-turn emits playbackStateChanged
// signal synchronously.
void MultimediaEngine::adhanPlay() noexcept {
  const QString prayerName{PrayerEngine::toString(pEngine->currentPrayer())};
  const int  adhanId{this->m_adhanConfig.value(prayerName).toHash().value("adhan_id"_L1).toInt()};
  const QUrl src{this->m_adhanList.at(adhanId).toMap().value("path"_L1).toUrl()};

  this->adhanPlayer->setAudioOutput(this->adhanOutput);
  this->adhanOutput->setMuted(this->m_adhanMuted);
  this->adhanOutput->setVolume(
      this->m_adhanConfig.value(prayerName).toHash().value("volume"_L1).toFloat());

  if (const float volume{this->adhanOutput->volume()}; volume <= 0 || this->adhanOutput->isMuted())
    return;

  logDebug(cat) << "Adhan file: " << src.toDisplayString();
  this->adhanPlayer->setSource(src);

  if (!this->isAdhanPlaying())
    this->adhanPlayer->play();
  else
    logDebug(cat) << "Play ignored; media already playing";
}

void MultimediaEngine::adhanStop() noexcept {
  if (this->isAdhanPlaying()) {
    this->adhanPlayer->stop();
    this->adhanPlayer->setAudioOutput(nullptr);
  } else
    logDebug(cat) << "Stop ignored; media is not playing";
}

QVariant MultimediaEngine::getAdhanConfig(Prayer prayerEnum, const QString &field) noexcept {
  if (!this->adhanConfigFields.contains(field)) {
    logCritical(cat) << field << "is not a valid field in AdhanConfig";
    return {};
  }

  return this->m_adhanConfig.value(PrayerEngine::toString(prayerEnum)).toHash().value(field);
}

const QLoggingCategory &MultimediaEngine::logCategory() noexcept { return cat(); }

void MultimediaEngine::modifyAdhanConfig(const QString &key, const QVariantHash &entry) noexcept {
  const int enumVal{PrayerEngine::metaPrayerEnum().keyToValue(qPrintable(key))};

  if (enumVal == -1) {
    logCritical(cat) << key << " resolves to unknown enum value";
    return;
  }

  const QString enumName{QString{PrayerEngine::metaPrayerEnum().scope()} + u"::"_s + key};

  if (const auto p = static_cast<Prayer>(enumVal); p == Prayer::Sunrise || p == Prayer::None) {
    logWarning(cat) << "Modifying \"" << enumName << "\" is not allowed";
    return;
  }

  QVariantHash tempMap{this->m_adhanConfig.value(key).toHash()};

  if (!isValidEntry(entry, this->adhanConfigEntrySize, this->adhanConfigFields)) {
    logCritical(cat) << "[" << entry << "] is not a valid entry in adhanConfig";
    return;
  }

  if (tempMap == entry) {
    logDebug(cat) << "Skipping setting identical entry";
    return;
  }

  tempMap.insert(entry);
  this->m_adhanConfig.insert(key, tempMap);
  emit this->adhanConfigChanged();
}

void MultimediaEngine::previewPlay(int adhanListIdx) noexcept {
  if (adhanListIdx < 0 || adhanListIdx >= this->m_adhanList.size()) {
    logCritical(cat) << "Index " << adhanListIdx << " out of valid range";
    return;
  }

  this->previewPlayer->setAudioOutput(this->previewOutput);
  this->previewPlayer->setSource(
      this->m_adhanList.at(adhanListIdx).toMap().value("path"_L1).toUrl());
  // this->previewOutput->setVolume(1.0) // not necessary, defaults to 1.0

  if (!this->isPreviewPlaying())
    this->previewPlayer->play();
  else
    logDebug(cat) << "Play ignored; media already playing";
}

void MultimediaEngine::previewStop() noexcept {
  if (this->previewPlayer->isPlaying()) {
    this->previewPlayer->stop();
    this->previewPlayer->setAudioOutput(nullptr);
  } else
    logDebug(cat) << "Stop ignored; media is not playing";
}

void MultimediaEngine::removeAdhan(qsizetype rmIdx) noexcept {
  if (rmIdx < this->defaultAdhans || rmIdx >= this->m_adhanList.size()) {
    logCritical(cat) << "Invalid index value \"" << rmIdx << "\"";
    return;
  }

  const QUrl      fileUrl{this->m_adhanList.at(rmIdx).toMap().value("path"_L1).toUrl()};
  const rm_file_t result(this->fileman->rmFile(QUrl(fileUrl)));

  logDebug(cat) << "Result: " << result.status_l() << " " << result.status_h();

  if (result.status_l() != 0 && result.status_l() != ENOENT && result.status_l() != EINVAL) {
    logCritical(cat) << "Fileman::rmFile() failed with error: " << result.status_l();
    return;
  }

  for (const auto &[field, entry] : this->m_adhanConfig.asKeyValueRange()) {
    QVariantHash map{entry.toHash()};
    const int    currentIdx{map.value("adhan_id"_L1).toInt()};

    if (currentIdx == rmIdx) {
      if (field == u"Fajr"_s)
        map.insert("adhan_id"_L1, MultimediaEngine::defaultFajrAdhan);
      else if (field == u"Maghrib"_s)
        map.insert("adhan_id"_L1, MultimediaEngine::defaultMaghribAdhan);
      else
        map.insert("adhan_id"_L1, MultimediaEngine::defaultAdhan);
    } else if (currentIdx > rmIdx)
      map.insert("adhan_id"_L1, currentIdx - 1);

    this->m_adhanConfig.insert(field, map);
  }

  this->m_adhanList.removeAt(rmIdx);
  this->m_adhanList.squeeze();
  emit this->adhanListChanged();
  emit this->adhanConfigChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

MultimediaEngine::MultimediaEngine(QObject *parent)
    : AppBase{cat(), parent}, adhanOutput{new QAudioOutput{this}},
      adhanPlayer{new QMediaPlayer{this}}, fileman{new FileMan{this}},
      previewOutput{new QAudioOutput{this}}, previewPlayer{new QMediaPlayer{this}} {
  this->bootEngine();
  logDebug(cat) << "FileMan::refreshStore() return: " << this->fileman->refreshStore();
  this->adhanPlayer->setAudioOutput(this->adhanOutput);

  if (const QMetaObject::Connection conn =
          QObject::connect(this->adhanPlayer, &QMediaPlayer::playbackStateChanged, this,
                           &MultimediaEngine::onAdhanPlaybackChanged);
      conn == nullptr)
    logWarning(cat) << "Failed to establish adhan playback state connection";
  else
    logDebug(cat) << "Adhan playback state connection established";

  if (const QMetaObject::Connection conn =
          QObject::connect(this->previewPlayer, &QMediaPlayer::playbackStateChanged, this,
                           &MultimediaEngine::onPreviewPlaybackChanged);
      conn == nullptr)
    logWarning(cat) << "Failed to establish preview playback state connection";
  else
    logDebug(cat) << "Preview playback state connection established";

  this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::connectToAdhanAlarm() const noexcept {
  if (const auto conn =
          QObject::connect(core, &CoreEngine::adhanAlarm, this, &MultimediaEngine::onAdhanAlarm);
      conn == nullptr)
    logWarning(cat) << "adhanAlarm signal not connected";
  else
    logDebug(cat) << "adhanAlarm signal connected";
}

void MultimediaEngine::defaultAdhanConfig() noexcept {
  const std::double_t defaultVolume{1.0}; // 0.0 - 1.0
  const std::double_t defaultFajrVolume{0.2};
  const std::double_t defaultIshaVolume{0.5};

  this->configFields = {
      {u"AllMuted"_s, QMetaType::Bool},
  };

  this->m_adhanConfig = {
      {u"Fajr"_s,    QVariantHash{{u"adhan_id"_s, MultimediaEngine::defaultFajrAdhan},
                               {u"volume"_s, defaultFajrVolume}}                 },
      {u"Dhuhr"_s,
       QVariantHash{{u"adhan_id"_s, MultimediaEngine::defaultAdhan}, {u"volume"_s, defaultVolume}}},
      {u"Asr"_s,
       QVariantHash{{u"adhan_id"_s, MultimediaEngine::defaultAdhan}, {u"volume"_s, defaultVolume}}},
      {u"Maghrib"_s, QVariantHash{{u"adhan_id"_s, MultimediaEngine::defaultMaghribAdhan},
                                  {u"volume"_s, defaultVolume}}               },
      {u"Isha"_s,    QVariantHash{{u"adhan_id"_s, MultimediaEngine::defaultAdhan},
                               {u"volume"_s, defaultIshaVolume}}                 }
  };

  const QVariantHash maps{this->m_adhanConfig.value("Fajr"_L1).toHash()};

  // do not use braces {} in the following QSet init
  this->adhanConfigKeys      = QSet(this->m_adhanConfig.keyBegin(), this->m_adhanConfig.keyEnd());
  this->adhanConfigMapSize   = this->m_adhanConfig.size();
  this->adhanConfigEntrySize = maps.size();

  for (const auto &[key, value] : maps.asKeyValueRange())
    this->adhanConfigFields.insert(key, value.typeId());
}

void MultimediaEngine::defaultAdhanList() noexcept {
  this->m_adhanList = {
      QVariantMap{{u"name"_s, u"Makkah (built-in)"_s},
                  {u"path"_s, u"qrc:/qt/qml/App/data/adhan-makkah.ogg"_s}  },
      QVariantMap{{u"name"_s, u"Ar-Rashid (built-in)"_s},
                  {u"path"_s, u"qrc:/qt/qml/App/data/adhan-arRashid.ogg"_s}},
      QVariantMap{{u"name"_s, u"Fajr (built-in)"_s},
                  {u"path"_s, u"qrc:/qt/qml/App/data/fajr-adhan.ogg"_s}    }
  };

  const QVariantMap maps{this->m_adhanList.at(0).toMap()};

  this->adhanListMapSize = maps.size();
  this->defaultAdhans    = this->m_adhanList.size();

  for (const auto &[key, value] : maps.asKeyValueRange())
    this->adhanListKeys.insert(key, value.typeId());
}

void MultimediaEngine::defaultConfigs() noexcept {
  this->m_adhanMuted = false;

  this->defaultAdhanConfig();
  this->defaultAdhanList();
}

QVariantList &MultimediaEngine::getAdhanList() noexcept { return this->m_adhanList; }
bool          MultimediaEngine::isAdhanPlaying() noexcept { return this->adhanPlayer->isPlaying(); }
bool MultimediaEngine::isPreviewPlaying() noexcept { return this->previewPlayer->isPlaying(); }

void MultimediaEngine::loadAdhanConfig() noexcept {
  QVariantHash cfgMap{this->m_adhanConfig};

  for (const QString &prayerName : std::as_const(this->adhanConfigKeys)) {
    const int i = PrayerEngine::metaPrayerEnum().keyToValue(qPrintable(prayerName));

    if (i == -1) {
      logCritical(cat) << prayerName << " resolves to unknown enum value";
      continue;
    }

    if (const auto p = static_cast<Prayer>(i); p == Prayer::Sunrise || p == Prayer::None) {
      logWarning(cat) << "Modifying \"" << prayerName << "\" is not allowed";
      return;
    }

    QVariantHash entry{this->m_adhanConfig.value(prayerName).toHash()};

    for (const auto &[key, value] : this->adhanConfigFields.asKeyValueRange()) {
      QVariant cfgVal{core->readSettings("AdhanConfig/Prayer/"_L1 + prayerName + '/'_L1 + key,
                                         entry.value(key))};

      logDebug(cat) << "Value for key \"" << key << "\" in list item " << prayerName << ": "
                    << cfgVal;

      if (!cfgVal.convert(QMetaType(this->adhanConfigFields.value(key)))) {
        logWarning(cat) << "Failed converting stored value for \"" << key
                        << "\" to expected type; keeping default";
        continue;
      }

      entry.insert(key, cfgVal);
    }

    cfgMap.insert(prayerName, entry);
  }

  this->setAdhanConfig(cfgMap);
}

void MultimediaEngine::loadConfigs() noexcept {
  logDebug(cat) << "Loading adhan settings from config file";

  this->loadAdhanConfig();
  this->setAdhanList(core->readSettingsArray("Adhans"_L1));
  loadConfigKey("AdhanConfig/AllMuted"_L1, this, "adhanMuted"_L1, cat());
}

void MultimediaEngine::saveConfigs() noexcept {
  // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
  // (later) settings are not saved

  logDebug(cat) << "Saving adhan configurations";

  for (const QString &prayerName : std::as_const(this->adhanConfigKeys)) {
    // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
    if (prayerName.isEmpty())
      continue;

    for (const auto &[key, value] :
         this->m_adhanConfig.value(prayerName).toHash().asKeyValueRange())
      core->writeSettings("AdhanConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
  }

  logDebug(cat) << "Saving adhan list";

  core->writeSettingsArray("Adhans"_L1, this->m_adhanList);
  core->writeSettings("AdhanConfig/AllMuted"_L1, this->m_adhanMuted, true);
}

void MultimediaEngine::setAdhanConfig(const QVariantHash &adhanConfig) noexcept {
  if (adhanConfig.isEmpty()) {
    logDebug(cat) << "Skipping empty config list";
    return;
  }

  if (adhanConfig == this->m_adhanConfig) {
    logDebug(cat) << "Skipping setting identical list";
    return;
  }

  if (!isValidEntry(adhanConfig, this->adhanConfigEntrySize, this->adhanConfigFields,
                    this->adhanConfigMapSize)) {
    logCritical(cat) << "Config [" << adhanConfig << "] is not a valid configuration";
    return;
  }

  this->m_adhanConfig = adhanConfig;

  emit this->adhanConfigChanged();
}

void MultimediaEngine::setAdhanList(const QVariantList &adhanList) noexcept {
  if (adhanList.isEmpty()) {
    logDebug(cat) << "Skipping empty adhan list";
    return;
  }

  if (adhanList == this->m_adhanList) {
    logDebug(cat) << "Skipping setting identical list";
    return;
  }

  if (adhanList.first(this->defaultAdhans) != this->m_adhanList.first(this->defaultAdhans)) {
    logCritical(cat) << "Dropping malformed list [" << adhanList << "]";
    logDebug(cat) << "List attempts to modify protected default list";
    logDebug(cat) << "\n"
                  << adhanList.first(this->defaultAdhans) << "\nvs\n"
                  << this->m_adhanList.first(this->defaultAdhans);
    return;
  }

  if (!isValidEntry(adhanList, this->adhanListMapSize, this->adhanListKeys)) {
    logCritical(cat) << "Dropping malformed list [" << adhanList << "]";
    return;
  }

  this->m_adhanList = adhanList;

  emit this->adhanListChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void MultimediaEngine::onAdhanAlarm() noexcept {
  logDebug(cat) << "Received adhan alarm at: " << QDateTime::currentDateTime().toString();
  this->adhanPlay();
}

void MultimediaEngine::onAdhanPlaybackChanged(QMediaPlayer::PlaybackState state) {
  switch (state) {
  case QMediaPlayer::PlayingState:
    logInfo(cat) << "Adhan playing";
    break;
  case QMediaPlayer::StoppedState:
    logInfo(cat) << "Adhan stopped";
    break;
  case QMediaPlayer::PausedState:
    logInfo(cat) << "Adhan paused";
    break;
  }

  emit this->adhanPlayingChanged(state == QMediaPlayer::PlayingState);
}

void MultimediaEngine::onCoreOnline() noexcept {
  AppBase::onCoreOnline();
  this->connectToAdhanAlarm();
}

void MultimediaEngine::onPreviewPlaybackChanged(QMediaPlayer::PlaybackState state) {
  switch (state) {
  case QMediaPlayer::PlayingState:
    logInfo(cat) << "Preview playing";
    break;
  case QMediaPlayer::StoppedState:
    logInfo(cat) << "Preview stopped";
    break;
  case QMediaPlayer::PausedState:
    logInfo(cat) << "Preview paused";
    break;
  }

  emit this->previewPlayingChanged(state == QMediaPlayer::PlayingState);
}
} // namespace App