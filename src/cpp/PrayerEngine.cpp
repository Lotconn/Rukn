#include <adhan/Qibla.hpp>

#include "internal-helpers.hpp"
#include <LocationEngine.hpp>
#include <PrayerEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "PrayerEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

PrayerEngine *PrayerEngine::create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */)
{
    logDebug(munadi) << "Creating instance from QML call";

    qmlEngine->singletonInstance<LocationEngine *>("Munadi"_L1, "LocationEngine"_L1);

    return new PrayerEngine{};
}

PrayerEngine::Prayer PrayerEngine::currentPrayer() noexcept
{
    const QReadLocker lock{&this->varLock};
    const Prayer      prayer{
        PrayerEngine::s_revPrayerEnumMap.value(this->m_prayerTimes->currentPrayer())};

    return (this->m_prayerTimes->date == QDate::currentDate() && prayer == Prayer::None)
               ? Prayer::Isha
               : prayer;
}

PrayerEngine::Prayer PrayerEngine::nextPrayer() noexcept
{
    const QReadLocker lock{&this->varLock};
    const Prayer prayer{PrayerEngine::s_revPrayerEnumMap.value(this->m_prayerTimes->nextPrayer())};

    return (this->m_prayerTimes->date == QDate::currentDate() && prayer == Prayer::None)
               ? Prayer::Fajr
               : prayer;
}

PrayerEngine::Prayer PrayerEngine::fromString(const QString &prayerName) noexcept
{
    const int val{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName))};

    return (val == -1) ? Prayer::None : static_cast<Prayer>(val);
}

PrayerEngine::Sunnah PrayerEngine::fromString_sunnahTime(const QString &sunnahName) noexcept
{
    const int val{PrayerEngine::s_metaSunnahEnum.keyToValue(qPrintable(sunnahName))};

    return (val == -1) ? Sunnah::None : static_cast<Sunnah>(val);
}

const QString &PrayerEngine::getCalcMethod() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_calcMethodKey;
}

const QString &PrayerEngine::getMadhab() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_madhabKey;
}

QVariant PrayerEngine::getPrayerConfig(Prayer prayerEnum, const QString &field) noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->getPrayerConfig_p(prayerEnum, field);
}

QVariant PrayerEngine::getPrayerConfig_p(Prayer prayerEnum, const QString &field) const noexcept
{
    if (!this->prayerConfigFields.contains(field))
    {
        logCritical(munadi) << field << " is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.value(PrayerEngine::toString(prayerEnum)).toHash().value(field);
}

double PrayerEngine::getQiblaDegrees(const std::optional<QGeoCoordinate> &coordinates)
{
    return (coordinates.has_value())
               ? Adhan::qibla(Adhan::Coordinates{coordinates->latitude(), coordinates->longitude()})
               : this->m_qiblaDegrees.load();
}

const QLoggingCategory &PrayerEngine::logCategory() noexcept
{ return munadi(); }

const QMetaEnum &PrayerEngine::metaPrayerEnum() noexcept
{ return PrayerEngine::s_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept
{
    const int     enumVal{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(key))};
    const QString enumName{QString(PrayerEngine::s_metaPrayerEnum.scope()) + u"::"_s + key};

    if (enumVal == -1)
    {
        logCritical(munadi) << key << " resolves to unknown enum value";

        return;
    }

    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(munadi) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    {
        const QWriteLocker lock{&this->varLock};
        QVariantHash       tempMap{this->m_prayerConfig.value(key).toHash()};

        if (!isValidEntry(entry, this->prayerConfigEntrySize, this->prayerConfigFields))
        {
            logCritical(munadi) << "[" << entry << "] is not a valid entry in prayerConfig";

            return;
        }

        if (tempMap == entry)
        {
            logDebug(munadi) << "Skipping setting identical entry";

            return;
        }

        logDebug(munadi) << "Assigning value " << entry << " to key " << key;
        tempMap.insert(entry);
        this->m_prayerConfig.insert(key, tempMap);
    }

    emit this->prayerConfigChanged();
    emit this->invalidate();
}

void PrayerEngine::setCalcMethod(const QString &method) noexcept
{
    logDebug(munadi) << "method: " << method;

    if ([&method, this] -> bool {
            const QWriteLocker lock{&this->varLock};
            return setConfig(PrayerEngine::s_calcMethods, method, this->m_calcMethodKey);
        }())
        emit this->invalidate();
}

void PrayerEngine::setMadhab(const QString &madhab) noexcept
{
    logDebug(munadi) << "madhab: " << madhab;

    if ([&madhab, this] -> bool {
            const QWriteLocker lock{&this->varLock};
            return setConfig(PrayerEngine::s_madhab, madhab, this->m_madhabKey);
        }())
        emit this->invalidate();
}

// Recommended to pass local date
// NOLINTBEGIN(bugprone-easily-swappable-parameters)
qint64 PrayerEngine::timeBetweenPrayers(Prayer current, Prayer next,
                                        const std::optional<QDate> &current_date,
                                        const std::optional<QDate> &next_date) noexcept
{
    const QDateTime currentAt{this->timeForPrayer(current, current_date)};
    const QDateTime nextAt{this->timeForPrayer(next, [&] -> std::optional<QDate> {
        if (next_date.has_value())
            return next_date;

        if (next == Prayer::None)
            return (current_date.has_value()) ? current_date->addDays(1)
                                              : QDateTime::currentDateTimeUtc().addDays(1).date();

        return current_date;
    }())};

    return currentAt.msecsTo(nextAt); // both are converted to UTC by Qt
}

qint64 PrayerEngine::timeBetweenPrayers(const QString &currentPrayer, const QString &nextPrayer,
                                        const std::optional<QDate> &current_date,
                                        const std::optional<QDate> &next_date) noexcept
{ // NOLINTEND(bugprone-easily-swappable-parameters)
    return this->timeBetweenPrayers(PrayerEngine::fromString(currentPrayer),
                                    PrayerEngine::fromString(nextPrayer), current_date, next_date);
}

QDateTime PrayerEngine::timeForCurrentPrayer(const std::optional<QDateTime> &when) noexcept
{
    const QDateTime          dateTime{(when.has_value()) ? when.value().toLocalTime()
                                                         : QDateTime::currentDateTime()};
    const QDate              date{dateTime.date()};
    const Adhan::PrayerTimes pTimes{[this, &when, &date] {
        const QReadLocker lock{&this->varLock};

        return (when.has_value())
                   ? Adhan::PrayerTimes{*this->coordinates, date.toStdSysDays(), *this->calcParams}
                   : *this->m_prayerTimes;
    }()};
    const Prayer             prayer{PrayerEngine::s_revPrayerEnumMap.value(
        pTimes.currentPrayer(dateTime.toStdSysMilliseconds()))};

    return (pTimes.date == date && prayer == Prayer::None)
               ? this->timeForPrayer(Prayer::Isha, date.addDays(-1))
               : this->timeForPrayer(prayer, date);
}

// Recommended to pass local date
QDateTime PrayerEngine::timeForPrayer(Prayer prayerEnum, const std::optional<QDate> &date) noexcept
{
    // Return midnight time 00:00:00 of same day for Prayer::None
    if (prayerEnum == Prayer::None)
        return (date.has_value()) ? date->startOfDay().toLocalTime()
                                  : QDate::currentDate().startOfDay().toLocalTime();

    Adhan::OptInstant date_time;
    {
        const QReadLocker lock{&this->varLock};

        if (date.has_value())
        {
            const Adhan::PrayerTimes pTimes{*this->coordinates, date->toStdSysDays(),
                                            *this->calcParams};

            date_time = pTimes.timeForPrayer(PrayerEngine::s_prayerEnumMap.value(prayerEnum));
        }
        else
            date_time =
                this->m_prayerTimes->timeForPrayer(PrayerEngine::s_prayerEnumMap.value(prayerEnum));
    }

    return (date_time.has_value()) ? QDateTime::fromStdTimePoint(date_time.value()).toLocalTime()
                                   : QDateTime{};
}

// Recommended to pass local date
QDateTime PrayerEngine::timeForPrayer(const QString              &prayerName,
                                      const std::optional<QDate> &date) noexcept
{ return this->timeForPrayer(PrayerEngine::fromString(prayerName), date); }

QDateTime PrayerEngine::timeForNextPrayer(const std::optional<QDateTime> &when) noexcept
{
    const QDateTime          dateTime{(when.has_value()) ? when.value().toLocalTime()
                                                         : QDateTime::currentDateTime()};
    const QDate              date{dateTime.date()};
    const Adhan::PrayerTimes pTimes{[this, &date, &when] {
        const QReadLocker lock{&this->varLock};

        return (when.has_value())
                   ? Adhan::PrayerTimes{*this->coordinates, date.toStdSysDays(), *this->calcParams}
                   : *this->m_prayerTimes;
    }()};
    const Prayer             prayer{
        PrayerEngine::s_revPrayerEnumMap.value(pTimes.nextPrayer(dateTime.toStdSysMilliseconds()))};

    return (pTimes.date == date && prayer == Prayer::None)
               ? this->timeForPrayer(Prayer::Fajr, date.addDays(1))
               : this->timeForPrayer(prayer, date);
}

// Recommended to pass local date
QDateTime PrayerEngine::timeForSunnahTime(Sunnah                      sunnahEnum,
                                          const std::optional<QDate> &date) noexcept
{
    Adhan::OptInstant date_time;
    {
        const QReadLocker lock{&this->varLock};

        if (date.has_value())
        {
            const Adhan::PrayerTimes pTimes{*this->coordinates, date->toStdSysDays(),
                                            *this->calcParams};
            const Adhan::SunnahTimes sTimes{pTimes};

            date_time = sTimes.timeForSunnahTime(PrayerEngine::s_sunnahEnumMap.value(sunnahEnum));
        }
        else
            date_time = this->m_sunnahTimes->timeForSunnahTime(
                PrayerEngine::s_sunnahEnumMap.value(sunnahEnum));
    }

    return (date_time.has_value()) ? QDateTime::fromStdTimePoint(date_time.value()).toLocalTime()
                                   : QDateTime{};
}

// Recommended to pass local date
QDateTime PrayerEngine::timeForSunnahTime(const QString              &sunnahName,
                                          const std::optional<QDate> &date) noexcept
{ return this->timeForSunnahTime(PrayerEngine::fromString_sunnahTime(sunnahName), date); }

// Recommended to pass local date-time
qint64 PrayerEngine::timeToNextPrayer(const std::optional<QDateTime> &since_ms) noexcept
{
    const QDateTime since{(since_ms.has_value()) ? since_ms->toLocalTime()
                                                 : QDateTime::currentDateTime()};
    const QDate     nextDay{since.addDays(1).date()};

    // logDebug(munadi) << "Since: " << since.toString();
    // logDebug(munadi) << "Next day date: " << nextDay.toString();

    std::unique_ptr<Adhan::PrayerTimes> currentPT;
    QDateTime                           nextTime;
    std::unique_ptr<Adhan::PrayerTimes> nextPT;
    {
        const QReadLocker lock{&this->varLock};

        currentPT =
            (since_ms.has_value())
                ? std::make_unique<Adhan::PrayerTimes>(
                      *this->coordinates, std::chrono::year_month_day{since.date().toStdSysDays()},
                      *this->calcParams)
                : std::make_unique<Adhan::PrayerTimes>(*this->m_prayerTimes);
        nextPT = std::make_unique<Adhan::PrayerTimes>(
            *this->coordinates, std::chrono::year_month_day{nextDay.toStdSysDays()},
            *this->calcParams);
    }

    Adhan::Prayer next{currentPT->nextPrayer(since.toStdSysMilliseconds())};

    // logDebug(munadi) << "Current prayer: "
    //                  << Adhan::PrayerUtils::to_string(
    //                         currentPT->currentPrayer(since.toStdSysMilliseconds()));
    // logDebug(munadi) << "Next prayer: " << Adhan::PrayerUtils::to_string(next);

    if (currentPT->date == since.toUTC().date() && next == Adhan::Prayer::None)
    {
        next = nextPT->nextPrayer(since.toStdSysMilliseconds());
        nextTime =
            this->timeForPrayer(PrayerEngine::s_revPrayerEnumMap.value(next), nextDay).toUTC();
    }
    else
        nextTime =
            this->timeForPrayer(PrayerEngine::s_revPrayerEnumMap.value(next), since.date()).toUTC();

    // logDebug(munadi) << "Resolved next prayer: " << Adhan::PrayerUtils::to_string(next);
    // logDebug(munadi) << "Next prayer at: " << nextTime.toString();
    // logDebug(munadi)
    //     << "Time to next prayer: "
    //     << QTime::fromMSecsSinceStartOfDay(static_cast<int>(since.msecsTo(nextTime))).toString();

    return since.msecsTo(nextTime); // nextTime is converted to UTC by Qt
}

QString PrayerEngine::toString(Prayer prayerEnum) noexcept
{
    const char *key{PrayerEngine::s_metaPrayerEnum.valueToKey(qToUnderlying(prayerEnum))};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(munadi) << "\"" << prayerEnum << "\" does not resolve to a valid Prayer enum";
        return {};
    }

    return key;
}

QString PrayerEngine::toString(Sunnah sunnahEnum) noexcept
{
    const char *key{PrayerEngine::s_metaSunnahEnum.valueToKey(qToUnderlying(sunnahEnum))};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(munadi) << "\"" << sunnahEnum << "\" does not resolve to a valid Prayer enum";
        return {};
    }

    return key;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

PrayerEngine::PrayerEngine(QObject *parent) : MunadiBase{munadi(), parent}
{
    this->bootEngine();

    this->connectToInvalidate();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    LocationEngine::onceOnline(this, [this] -> void { this->onLocationEngineOnline(); });
    this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::connectToAlarmsReset() noexcept
{
    if (const auto conn = QObject::connect(core, &CoreEngine::alarmsReset, this,
                                           [this] -> void { emit this->invalidate(); });
        conn == nullptr)
        logWarning(munadi) << "alarmsReset signal not connected";
    else
        logDebug(munadi) << "alarmsReset signal connected";
}

void PrayerEngine::connectToAthanAlarm() noexcept
{
    if (const auto conn = QObject::connect(core, &CoreEngine::athanAlarm, this,
                                           [this] -> void { emit this->calcChanged(); });
        conn == nullptr)
        logWarning(munadi) << "athanAlarm signal not connected";
    else
        logDebug(munadi) << "athanAlarm signal connected";
}

void PrayerEngine::connectToDateChanged() noexcept
{
    if (const auto conn = QObject::connect(core, &CoreEngine::dateChanged, this,
                                           [this] -> void { emit this->invalidate(); });
        conn == nullptr)
        logWarning(munadi) << "dateChanged signal not connected";
    else
        logDebug(munadi) << "dateChanged signal connected";
}

void PrayerEngine::connectToInvalidate() const noexcept
{
    if (const auto conn =
            QObject::connect(this, &PrayerEngine::invalidate, this, &PrayerEngine::onInvalidate);
        conn == nullptr)
        logWarning(munadi) << "invalidate signal not connected";
    else
        logDebug(munadi) << "invalidate signal connected";
}

void PrayerEngine::connectToLocationChanged() noexcept
{
    if (const auto conn = QObject::connect(lEngine, &LocationEngine::locationChanged, this,
                                           [this] -> void { emit this->invalidate(); });
        conn == nullptr)
        logWarning(munadi) << "locationChanged signal not connected";
    else
        logDebug(munadi) << "locationChanged signal connected";
}

void PrayerEngine::connectToMinuteChanged() const noexcept
{
    if (const auto conn = QObject::connect(core, &CoreEngine::minuteChanged, this,
                                           &PrayerEngine::onMinuteChanged);
        conn == nullptr)
        logWarning(munadi) << "minuteChanged signal not connected";
    else
        logDebug(munadi) << "minuteChanged signal connected";
}

void PrayerEngine::defaultConfigs()
{
    {
        const QWriteLocker lock{&this->varLock};

        this->m_calcMethodKey = u"MuslimWorldLeague"_s;
        this->m_madhabKey     = u"Shafi"_s;

        this->defaultPrayerConfig();
    }

    this->updateCalc();
}

void PrayerEngine::defaultPrayerConfig() noexcept
{
    this->m_prayerConfig = {
        {u"Fajr"_s,    QVariantHash{{u"adjustment"_s, 0.0}}},
        {u"Dhuhr"_s,   QVariantHash{{u"adjustment"_s, 0.0}}},
        {u"Asr"_s,     QVariantHash{{u"adjustment"_s, 0.0}}},
        {u"Maghrib"_s, QVariantHash{{u"adjustment"_s, 0.0}}},
        {u"Isha"_s,    QVariantHash{{u"adjustment"_s, 0.0}}},
    };

    const QVariantHash maps{this->m_prayerConfig.value("Fajr"_L1).toHash()};

    this->prayerConfigKeys = QSet(this->m_prayerConfig.keyBegin(), this->m_prayerConfig.keyEnd());
    this->prayerConfigMapSize   = this->m_prayerConfig.size();
    this->prayerConfigEntrySize = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->prayerConfigFields.insert(key, value.typeId());
}

void PrayerEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading prayer settings from config file";

    bool          configMigrated{true};
    const QString migrateKey{"PrayerEngineConfigMigrated"_L1};

    // Adding a QReadLocker/QWriteLocker here is strongly discouraged. Acquiring a locker here may
    // deadlock. Put it inside setter functions if necessary.

    this->loadPrayerConfig(configMigrated);
    loadConfigKey({}, "PrayerConfig/CalcMethod"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setCalcMethod(name.toString()); });
    loadConfigKey({}, "PrayerConfig/Madhab"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setMadhab(name.toString()); });

    if (configMigrated)
        core->writeSettings("PrayerEngineConfigMigrated"_L1, true);
}

void PrayerEngine::loadPrayerConfig(bool &configMigrated) noexcept
{
    QVariantHash       cfgMap{this->m_prayerConfig};
    QVariantList       oldPrayerConfig;
    const QWriteLocker lock{
        &this->varLock}; // safe as long as no QReadLocker holder is in this block
    const bool migrated{core->readSettings("PrayerEngineConfigMigrated"_L1).toBool()};

    if (!migrated)
        oldPrayerConfig = core->readOldSettingsJsArray("Adhan/prayerConfig"_L1);

    if (!oldPrayerConfig.isEmpty())
        logInfo(munadi) << "Migrating from old config";

    for (const QString &prayerName : this->prayerConfigKeys)
    {
        const int i = PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName));

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

        QVariantHash entry{this->m_prayerConfig.value(prayerName).toHash()};

        for (const QString &key : this->prayerConfigFields.keys())
        {
            QVariant cfgVal;

            cfgVal = (oldPrayerConfig.isEmpty())
                         ? core->readSettings("PrayerConfig/Prayer/"_L1 + prayerName + '/'_L1 + key,
                                              entry.value(key))
                         : oldPrayerConfig.at(i).toHash().value(key);

            logDebug(munadi) << "Value for key \"" << key << "\" in list item " << prayerName
                             << ": " << cfgVal;

            if (!cfgVal.convert(QMetaType(this->prayerConfigFields.value(key))))
            {
                logWarning(munadi) << "Failed converting stored value for \"" << key
                                   << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgMap.insert(prayerName, entry);
    }

    // setPrayerConfig() also has a QWriteLocker
    this->setPrayerConfig(cfgMap);

    if (migrated)
        configMigrated = false;
}

void PrayerEngine::saveConfigs() noexcept
{
    logDebug(munadi) << "Saving prayer configurations";

    const QReadLocker lock{&this->varLock};

    for (const QString &prayerName : this->prayerConfigKeys)
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        for (const auto &[key, value] :
             this->m_prayerConfig.value(prayerName).toHash().asKeyValueRange())
            core->writeSettings("PrayerConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
    }

    core->writeSettings("PrayerConfig/CalcMethod"_L1, this->m_calcMethodKey);
    core->writeSettings("PrayerConfig/Madhab"_L1, this->m_madhabKey);
}

void PrayerEngine::setPrayerConfig(const QVariantHash &prayerConfig) noexcept
{
    if (prayerConfig.isEmpty())
    {
        logDebug(munadi) << "Skipping empty config list";

        return;
    }

    {
        const QWriteLocker lock{&this->varLock};

        if (prayerConfig == this->m_prayerConfig)
        {
            logDebug(munadi) << "Skipping setting identical list";

            return;
        }

        if (!isValidEntry(prayerConfig, this->prayerConfigEntrySize, this->prayerConfigFields,
                          this->prayerConfigMapSize))
        {
            logCritical(munadi) << "Config [" << prayerConfig << "] is not a valid configuration";

            return;
        }

        this->m_prayerConfig = prayerConfig;
    }

    emit this->prayerConfigChanged();
}

void PrayerEngine::updateCalc()
{
    {
        const QWriteLocker lock{&this->varLock};

        this->calcParams = std::make_unique<Adhan::CalculationParameters>(
            PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)());

        // The following needs to call the private getPrayerConfig for avoiding locking contention
        this->calcParams->adjustments.fajr =
            this->getPrayerConfig_p(Prayer::Fajr, "adjustment"_L1).toInt();
        this->calcParams->adjustments.dhuhr =
            this->getPrayerConfig_p(Prayer::Dhuhr, "adjustment"_L1).toInt();
        this->calcParams->adjustments.asr =
            this->getPrayerConfig_p(Prayer::Asr, "adjustment"_L1).toInt();
        this->calcParams->adjustments.maghrib =
            this->getPrayerConfig_p(Prayer::Maghrib, "adjustment"_L1).toInt();
        this->calcParams->adjustments.isha =
            this->getPrayerConfig_p(Prayer::Isha, "adjustment"_L1).toInt();

        this->calcParams->madhab = PrayerEngine::s_madhab.value(this->m_madhabKey);
        this->coordinates =
            std::make_unique<Adhan::Coordinates>(lEngine->getLocation().coordinate().latitude(),
                                                 lEngine->getLocation().coordinate().longitude());

        this->m_qiblaDegrees = Adhan::qibla(*this->coordinates);
        this->m_prayerTimes  = std::make_unique<Adhan::PrayerTimes>(
            *this->coordinates, QDate::currentDate().toStdSysDays(), *this->calcParams);
        this->m_sunnahTimes = std::make_unique<Adhan::SunnahTimes>(*this->m_prayerTimes);
    }

    core->alarmManager().clear("athan"_L1);
    core->alarmManager().add("athan"_L1, this->timeForPrayer(Prayer::Fajr));
    core->alarmManager().add("athan"_L1, this->timeForPrayer(Prayer::Dhuhr));
    core->alarmManager().add("athan"_L1, this->timeForPrayer(Prayer::Asr));
    core->alarmManager().add("athan"_L1, this->timeForPrayer(Prayer::Maghrib));
    core->alarmManager().add("athan"_L1, this->timeForPrayer(Prayer::Isha));
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::onCoreOnline() noexcept
{
    MunadiBase::onCoreOnline();
    this->connectToAlarmsReset();
    this->connectToAthanAlarm();
    this->connectToDateChanged();
    this->connectToMinuteChanged();
}

void PrayerEngine::onInvalidate()
{
    logDebug(munadi) << "Updating calculations...";

    this->updateCalc();
    emit this->calcChanged();
}

void PrayerEngine::onMinuteChanged() noexcept
{
    logDebug(munadi) << "60s_tick event received";
    emit this->calcChanged();
}

void PrayerEngine::onLocationEngineOnline() noexcept
{ this->connectToLocationChanged(); }

} // namespace Munadi