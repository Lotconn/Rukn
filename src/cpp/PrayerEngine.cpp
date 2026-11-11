#include <adhan/Qibla.hpp>

#include "internal-helpers.hpp"
#include <LocationEngine.hpp>
#include <PrayerEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(cat, "PrayerEngine")

namespace App
{

using Prayer = PrayerEngine::Prayer;

PrayerEngine *PrayerEngine::create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */)
{
    logDebug(cat) << "Creating instance from QML call";

    qmlEngine->singletonInstance<LocationEngine *>("App"_L1, "LocationEngine"_L1);

    return new PrayerEngine{};
}

PrayerEngine::Prayer PrayerEngine::currentPrayer() const noexcept
{
    const Prayer prayer{
        PrayerEngine::s_revPrayerEnumMap.value(this->m_prayerTimes->currentPrayer())};

    // `Adhan::PrayerUtils::currentPrayer()` can provide `Adhan::None` for some edge-cases or error
    // reasons. To retain the information, `None` is overridden to `Isha` only if its before Fajr at
    // current local date, passed verbatim otherwise.
    return (this->m_prayerTimes->date == QDate::currentDate() && prayer == Prayer::None)
               ? Prayer::Isha
               : prayer;
}

PrayerEngine::Prayer PrayerEngine::fromString(const QString &prayerName) noexcept
{
    const int val{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName))};

    // QMetaEnum object returns -1 for invalid `keyToValue()` parameter
    return (val == -1) ? Prayer::None : static_cast<Prayer>(val);
}

PrayerEngine::Sunnah PrayerEngine::fromString_sunnahTime(const QString &sunnahName) noexcept
{
    const int val{PrayerEngine::s_metaSunnahEnum.keyToValue(qPrintable(sunnahName))};

    // QMetaEnum object returns -1 for invalid `keyToValue()` parameter
    return (val == -1) ? Sunnah::None : static_cast<Sunnah>(val);
}

// Intentionally a read-only getter, not a reference accessor
QString PrayerEngine::getCalcMethod() const noexcept
{ return this->m_calcMethodKey; }

// Intentionally a read-only getter, not a reference accessor
QString PrayerEngine::getMadhhab() const noexcept
{ return this->m_madhhabKey; }

QVariant PrayerEngine::getPrayerConfig(Prayer prayerEnum, const QString &field) const noexcept
{
    if (!this->prayerConfigFields.contains(field))
    {
        logCritical(cat) << field << " is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.value(PrayerEngine::toString(prayerEnum)).toHash().value(field);
}

double PrayerEngine::getQiblaDegrees(const std::optional<QGeoCoordinate> &coordinates) const
{
    return (coordinates.has_value())
               ? Adhan::qibla(Adhan::Coordinates{coordinates->latitude(), coordinates->longitude()})
               : this->m_qiblaDegrees;
}

const QLoggingCategory &PrayerEngine::logCategory() noexcept
{ return cat(); }

// Intentionally a read-only getter, not a reference accessor, although a QMetaEnum object is
// inherently read-only
QMetaEnum PrayerEngine::metaPrayerEnum() noexcept
{ return PrayerEngine::s_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept
{
    const int     enumVal{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(key))};
    const QString enumName{QString(PrayerEngine::s_metaPrayerEnum.scope()) + u"::"_s + key};

    // QMetaEnum object returns -1 for invalid `keyToValue()` parameter
    if (enumVal == -1)
    {
        logCritical(cat) << key << " resolves to unknown enum value";
        return;
    }

    // These also don't exist in the table anyway, but still, to clean weeds early
    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(cat) << "Modifying \"" << enumName << "\" is not allowed";
        return;
    }

    // This is the configuration table nested inside the parent hash-table
    QVariantHash tempMap{this->m_prayerConfig.value(key).toHash()};

    // Using the QVariantHash overload of `isValidEntry()`
    if (!isValidEntry(entry, this->prayerConfigEntrySize, this->prayerConfigFields))
    {
        logCritical(cat) << "[" << entry << "] is not a valid entry in prayerConfig";
        return;
    }

    if (tempMap == entry)
    {
        logDebug(cat) << "Skipping setting identical entry";
        return;
    }

    logDebug(cat) << "Assigning value " << entry << " to key " << key;
    tempMap.insert(entry);                     // modifying the configuration hash here
    this->m_prayerConfig.insert(key, tempMap); // modifying the parent hash here

    emit this->prayerConfigChanged();          // `invalidate()` will be relayed automatically
}

PrayerEngine::Prayer PrayerEngine::nextPrayer() const noexcept
{
    const Prayer prayer{PrayerEngine::s_revPrayerEnumMap.value(this->m_prayerTimes->nextPrayer())};

    // `Adhan::PrayerUtils::nextPrayer()` can provide `Adhan::None` for some edge-cases or error
    // reasons. To retain the information, `None` is overridden to `Fajr` only if its after Isha at
    // current local date, passed verbatim otherwise.
    return (this->m_prayerTimes->date == QDate::currentDate() && prayer == Prayer::None)
               ? Prayer::Fajr
               : prayer;
}

void PrayerEngine::setCalcMethod(const QString &method) noexcept
{
    logDebug(cat) << "Using calculation method: " << method;

    if (setConfig(PrayerEngine::s_calcMethods, method, this->m_calcMethodKey, cat()))
        emit this->invalidate();
}

void PrayerEngine::setMadhhab(const QString &madhhab) noexcept
{
    logDebug(cat) << "madhhab: " << madhhab;

    if (setConfig(PrayerEngine::s_madhhab, madhhab, this->m_madhhabKey, cat()))
        emit this->invalidate();
}

qint64 PrayerEngine::timeBetweenPrayers(Prayer current, Prayer next,
                                        std::optional<QDate> current_date,
                                        std::optional<QDate> next_date) const noexcept
{
    const QDateTime currentAt{this->timeForPrayer(current, current_date)};

    // The following is an IIFE (Immediately Invoked Function Expression), keeping the var `const`
    const QDateTime nextAt{this->timeForPrayer(next, [&] -> std::optional<QDate> {
        if (next_date.has_value())
            return next_date;

        if (next == Prayer::None)
            return (current_date.has_value()) ? current_date->addDays(1)
                                              : QDateTime::currentDateTime().addDays(1).date();

        return current_date;
    }())};

    if (!currentAt.isValid() || !nextAt.isValid())
        return -1;

    return currentAt.msecsTo(nextAt); // both are converted to UTC by Qt
}

qint64 PrayerEngine::timeBetweenPrayers(const QString &currentPrayer, const QString &nextPrayer,
                                        std::optional<QDate> current_date,
                                        std::optional<QDate> next_date) const noexcept
{
    return this->timeBetweenPrayers(PrayerEngine::fromString(currentPrayer),
                                    PrayerEngine::fromString(nextPrayer), current_date, next_date);
}

QDateTime PrayerEngine::timeForCurrentPrayer(const std::optional<QDateTime> &when) const noexcept
{
    const QDateTime          dateTime{(when.has_value()) ? when.value().toLocalTime()
                                                         : QDateTime::currentDateTime()};
    const QDate              date{dateTime.date()};
    // Keeps the var const-specified this way
    const Adhan::PrayerTimes pTimes{
        (when.has_value())
            ? Adhan::PrayerTimes{*this->coordinates, date.toStdSysDays(), *this->calcParams}
            : *this->m_prayerTimes
    };
    const Prayer prayer{PrayerEngine::s_revPrayerEnumMap.value(
        pTimes.currentPrayer(dateTime.toStdSysMilliseconds()))};

    // same quirk as `currentPrayer()` only a bit more pronounced
    return (pTimes.date == date && prayer == Prayer::None)
               ? this->timeForPrayer(Prayer::Isha, date.addDays(-1))
               : this->timeForPrayer(prayer, date);
}

QDateTime PrayerEngine::timeForPrayer(Prayer prayerEnum, std::optional<QDate> date) const noexcept
{
    // Return midnight time 00:00:00 of same day for Prayer::None
    if (prayerEnum == Prayer::None)
        return (date.has_value()) ? date->startOfDay().toLocalTime()
                                  : QDate::currentDate().startOfDay().toLocalTime();

    Adhan::OptInstant date_time;

    if (date.has_value())
    {
        const Adhan::PrayerTimes pTimes{*this->coordinates, date->toStdSysDays(),
                                        *this->calcParams};

        date_time = pTimes.timeForPrayer(PrayerEngine::s_prayerEnumMap.value(prayerEnum));
    }
    else
        date_time =
            this->m_prayerTimes->timeForPrayer(PrayerEngine::s_prayerEnumMap.value(prayerEnum));

    return (date_time.has_value()) ? QDateTime::fromStdTimePoint(date_time.value()).toLocalTime()
                                   : QDateTime{};
}

QDateTime PrayerEngine::timeForPrayer(const QString       &prayerName,
                                      std::optional<QDate> date) const noexcept
{ return this->timeForPrayer(PrayerEngine::fromString(prayerName), date); }

QDateTime PrayerEngine::timeForNextPrayer(const std::optional<QDateTime> &when) const noexcept
{
    const QDateTime          dateTime{(when.has_value()) ? when.value().toLocalTime()
                                                         : QDateTime::currentDateTime()};
    const QDate              date{dateTime.date()};
    const Adhan::PrayerTimes pTimes{
        (when.has_value())
            ? Adhan::PrayerTimes{*this->coordinates, date.toStdSysDays(), *this->calcParams}
            : *this->m_prayerTimes
    };
    const Prayer prayer{
        PrayerEngine::s_revPrayerEnumMap.value(pTimes.nextPrayer(dateTime.toStdSysMilliseconds()))};

    // same quirks as `nextPrayer()` only a bit more pronounced
    return (pTimes.date == date && prayer == Prayer::None)
               ? this->timeForPrayer(Prayer::Fajr, date.addDays(1))
               : this->timeForPrayer(prayer, date);
}

QDateTime PrayerEngine::timeForSunnahTime(Sunnah               sunnahEnum,
                                          std::optional<QDate> date) const noexcept
{
    Adhan::OptInstant date_time;

    if (date.has_value())
    {
        const Adhan::PrayerTimes pTimes{*this->coordinates, date->toStdSysDays(),
                                        *this->calcParams};
        const Adhan::SunnahTimes sTimes{pTimes};

        date_time = sTimes.timeForSunnahTime(PrayerEngine::s_sunnahEnumMap.value(sunnahEnum));
    }
    else
        date_time =
            this->m_sunnahTimes->timeForSunnahTime(PrayerEngine::s_sunnahEnumMap.value(sunnahEnum));

    return (date_time.has_value()) ? QDateTime::fromStdTimePoint(date_time.value()).toLocalTime()
                                   : QDateTime{};
}

QDateTime PrayerEngine::timeForSunnahTime(const QString       &sunnahName,
                                          std::optional<QDate> date) const noexcept
{ return this->timeForSunnahTime(PrayerEngine::fromString_sunnahTime(sunnahName), date); }

qint64 PrayerEngine::timeToNextPrayer(const std::optional<QDateTime> &since_ms) const noexcept
{
    // Safe to use local clocks from first since we're mostly using the date parts
    const QDateTime since{(since_ms.has_value()) ? since_ms->toLocalTime()
                                                 : QDateTime::currentDateTime()};
    const QDate     nextDay{since.addDays(1).date()};

    // These disabled logs are very useful

    // logDebug(cat) << "Since: " << since.toString();
    // logDebug(cat) << "Next day date: " << nextDay.toString();

    const std::unique_ptr<Adhan::PrayerTimes> currentPT{
        (since_ms.has_value())
            ? std::make_unique<Adhan::PrayerTimes>(*this->coordinates, since.date().toStdSysDays(),
                                                   *this->calcParams)
            : std::make_unique<Adhan::PrayerTimes>(*this->m_prayerTimes)};

    const std::unique_ptr<Adhan::PrayerTimes> nextPT{std::make_unique<Adhan::PrayerTimes>(
        *this->coordinates, nextDay.toStdSysDays(), *this->calcParams)};

    QDateTime     nextTime;
    // Qt converts the time to UTC before converting to epoch time
    Adhan::Prayer next{currentPT->nextPrayer(since.toStdSysMilliseconds())};

    // logDebug(cat) << "Current prayer: "
    //                  << Adhan::PrayerUtils::to_string(
    //                         currentPT->currentPrayer(since.toStdSysMilliseconds()));
    // logDebug(cat) << "Next prayer: " << Adhan::PrayerUtils::to_string(next);

    // Quirks from `nextPrayer()` apply
    if (currentPT->date == since.date() && next == Adhan::Prayer::None)
    {
        next     = nextPT->nextPrayer(since.toStdSysMilliseconds());
        nextTime = this->timeForPrayer(PrayerEngine::s_revPrayerEnumMap.value(next), nextDay);
    }
    else
        nextTime = this->timeForPrayer(PrayerEngine::s_revPrayerEnumMap.value(next), since.date());

    // logDebug(cat) << "Resolved next prayer: " << Adhan::PrayerUtils::to_string(next);
    // logDebug(cat) << "Next prayer at: " << nextTime.toString();
    // logDebug(cat)
    //     << "Time to next prayer: "
    //     << QTime::fromMSecsSinceStartOfDay(static_cast<int>(since.msecsTo(nextTime))).toString();

    if (!nextTime.isValid())
        return -1;

    return since.msecsTo(nextTime); // both are converted to UTC by Qt
}

QString PrayerEngine::toString(Prayer prayerEnum) noexcept
{
    const char *key{PrayerEngine::s_metaPrayerEnum.valueToKey(qToUnderlying(prayerEnum))};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(cat) << "\"" << prayerEnum << "\" does not resolve to a valid Prayer enum";
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
        logCritical(cat) << "\"" << sunnahEnum << "\" does not resolve to a valid Prayer enum";
        return {};
    }

    return key;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

PrayerEngine::PrayerEngine(QObject *parent) : AppBase{cat(), parent}
{
    this->bootEngine();

    // This could've gone inside an overridden `bootEngine()`
    this->setupInternalConnections();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    LocationEngine::onceOnline(this, [this] -> void { this->onLocationEngineOnline(); });
    this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::defaultConfigs() noexcept
{
    this->m_calcMethodKey = u"MuslimWorldLeague"_s;
    this->m_madhhabKey    = u"Shafi"_s; // although Adhan-C++ already defaults to Shafi

    this->defaultPrayerConfig();
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

    // Extract the config atom from the parent hash, any entry works, as long as all are identical
    const QVariantHash maps{this->m_prayerConfig.value("Fajr"_L1).toHash()};

    // Do not use braces {} for QSet here; this approach is prefered for performance efficiency,
    // avoiding unnecessary memory allocations
    this->prayerConfigKeys = QSet(this->m_prayerConfig.keyBegin(), this->m_prayerConfig.keyEnd());
    this->prayerConfigMapSize   = this->m_prayerConfig.size();
    this->prayerConfigEntrySize = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->prayerConfigFields.insert(key, value.typeId());
}

void PrayerEngine::loadConfigs() noexcept
{
    logDebug(cat) << "Loading prayer settings from config file"; // from [PrayerConfig] group

    bool          configMigrated{true}; // do not pass this for keys having no related old keys
    const QString migrateKey{"PrayerEngineConfigMigrated"_L1};

    this->loadPrayerConfig(configMigrated);
    loadConfigKey({}, "PrayerConfig/CalcMethod"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setCalcMethod(name.toString()); });
    loadConfigKey({}, "PrayerConfig/Madhhab"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setMadhhab(name.toString()); });

    if (configMigrated)
        core->writeSettings("PrayerEngineConfigMigrated"_L1, true);
}

void PrayerEngine::loadPrayerConfig(bool &configMigrated) noexcept
{
    QVariantHash cfgMap{this->m_prayerConfig};
    QVariantList oldPrayerConfig;
    const bool   migrated{core->readSettings("PrayerEngineConfigMigrated"_L1).toBool()};

    if (!migrated)
        oldPrayerConfig = core->readOldSettingsJsArray("Adhan/prayerConfig"_L1);

    if (!oldPrayerConfig.isEmpty())
        logInfo(cat) << "Migrating from old config";

    for (const QString &prayerName : std::as_const(this->prayerConfigKeys))
    {
        const int i = PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName));

        if (i == -1)
        {
            logCritical(cat) << prayerName << " resolves to unknown enum value";

            continue;
        }

        if (static_cast<Prayer>(i) == Prayer::Sunrise || static_cast<Prayer>(i) == Prayer::None)
        {
            logWarning(cat) << "Modifying \"" << prayerName << "\" is not allowed";

            return;
        }

        QVariantHash entry{this->m_prayerConfig.value(prayerName).toHash()};

        for (const auto &[key, value] : this->prayerConfigFields.asKeyValueRange())
        {
            QVariant cfgVal;

            cfgVal = (oldPrayerConfig.isEmpty())
                         ? core->readSettings("PrayerConfig/Prayer/"_L1 + prayerName + '/'_L1 + key,
                                              entry.value(key))
                         : oldPrayerConfig.at(i).toHash().value(key);

            logDebug(cat) << "Value for key \"" << key << "\" in list item " << prayerName << ": "
                          << cfgVal;

            if (!cfgVal.convert(QMetaType(this->prayerConfigFields.value(key))))
            {
                logWarning(cat) << "Failed converting stored value for \"" << key
                                << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgMap.insert(prayerName, entry);
    }

    this->setPrayerConfig(cfgMap);

    // `m_prayerConfig`'s defaults are used when `oldPrayerConfig` resolves to invalid and unusable,
    // which is infalliable as long as `defaultPrayerConfig()` performs its due dilligence. That
    // said, the only responsibility now is to break the logical-AND chain to stop migration when
    // `migrated` already reads `true`
    if (migrated)
        configMigrated = false;
}

void PrayerEngine::saveConfigs() noexcept
{
    logDebug(cat) << "Saving prayer configurations"; // under [PrayerConfig] group

    // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
    // (later) settings are not saved

    // Take this for-loop here as `savePrayerConfig()`
    for (const QString &prayerName : std::as_const(this->prayerConfigKeys))
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        // Becomes "Prayer/Fajr/adjustment=0" under "[PrayerConfig]", for example
        for (const auto &[key, value] :
             this->m_prayerConfig.value(prayerName).toHash().asKeyValueRange())
            core->writeSettings("PrayerConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
    }

    core->writeSettings("PrayerConfig/CalcMethod"_L1, this->m_calcMethodKey);
    core->writeSettings("PrayerConfig/Madhhab"_L1, this->m_madhhabKey, true);
}

void PrayerEngine::setPrayerConfig(const QVariantHash &prayerConfig) noexcept
{
    if (prayerConfig.isEmpty())
    {
        logDebug(cat) << "Skipping empty config list";

        return;
    }

    if (prayerConfig == this->m_prayerConfig)
    {
        logDebug(cat) << "Skipping setting identical list";

        return;
    }

    if (!isValidEntry(prayerConfig, this->prayerConfigEntrySize, this->prayerConfigFields,
                      this->prayerConfigMapSize))
    {
        logCritical(cat) << "Config [" << prayerConfig << "] is not a valid configuration";

        return;
    }

    this->m_prayerConfig = prayerConfig;

    emit this->prayerConfigChanged();
}

void PrayerEngine::setupAlarmsResetRelay() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::alarmsReset, this, &PrayerEngine::invalidate);
        conn == nullptr)
        logWarning(cat) << "alarmsReset signal not relayed to invalidate";
    else
        logDebug(cat) << "alarmsReset signal relayed to invalidate";
}

void PrayerEngine::setupAthanAlarmRelay() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::athanAlarm, this, &PrayerEngine::calcChanged);
        conn == nullptr)
        logWarning(cat) << "athanAlarm signal not relayed to calcChanged";
    else
        logDebug(cat) << "athanAlarm signal relayed to calcChanged";
}

void PrayerEngine::setupDateChangedRelay() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::dateChanged, this, &PrayerEngine::invalidate);
        conn == nullptr)
        logWarning(cat) << "dateChanged signal not relayed to invalidate";
    else
        logDebug(cat) << "dateChanged signal relayed to invalidate";
}

void PrayerEngine::setupInternalConnections() const noexcept
{
    if (const auto conn = QObject::connect(this, &PrayerEngine::prayerConfigChanged, this,
                                           &PrayerEngine::invalidate);
        conn == nullptr)
        logWarning(cat) << "prayerConfigChanged signal not relayed to invalidate";
    else
        logDebug(cat) << "prayerConfigChanged signal relayed to invalidate";

    if (const auto conn =
            QObject::connect(this, &PrayerEngine::invalidate, this, &PrayerEngine::onInvalidate);
        conn == nullptr)
        logWarning(cat) << "invalidate signal not connected";
    else
        logDebug(cat) << "invalidate signal connected";
}

void PrayerEngine::setupLocationChangedRelay() const noexcept
{
    if (const auto conn = QObject::connect(lEngine, &LocationEngine::locationChanged, this,
                                           &PrayerEngine::invalidate);
        conn == nullptr)
        logWarning(cat) << "locationChanged signal not relayed to invalidate";
    else
        logDebug(cat) << "locationChanged signal relayed to invalidate";
}

void PrayerEngine::setupMinuteChangedRelay() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::minuteChanged, this, &PrayerEngine::calcChanged);
        conn == nullptr)
        logWarning(cat) << "minuteChanged signal not relayed to calcChanged";
    else
        logDebug(cat) << "minuteChanged signal relayed to calcChanged";
}

void PrayerEngine::updateCalc()
{
    // The old objects will be auto-discarded on new `make_unique()`

    this->calcParams = std::make_unique<Adhan::CalculationParameters>(
        PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)());

    // The following needs to call the private getPrayerConfig for avoiding locking contention
    this->calcParams->adjustments.fajr =
        this->getPrayerConfig(Prayer::Fajr, "adjustment"_L1).toInt();
    this->calcParams->adjustments.dhuhr =
        this->getPrayerConfig(Prayer::Dhuhr, "adjustment"_L1).toInt();
    this->calcParams->adjustments.asr = this->getPrayerConfig(Prayer::Asr, "adjustment"_L1).toInt();
    this->calcParams->adjustments.maghrib =
        this->getPrayerConfig(Prayer::Maghrib, "adjustment"_L1).toInt();
    this->calcParams->adjustments.isha =
        this->getPrayerConfig(Prayer::Isha, "adjustment"_L1).toInt();

    this->calcParams->madhab = PrayerEngine::s_madhhab.value(this->m_madhhabKey);
    // Using `location` property is both longer and more mouthful than this
    this->coordinates =
        std::make_unique<Adhan::Coordinates>(lEngine->getLocation().coordinate().latitude(),
                                             lEngine->getLocation().coordinate().longitude());

    this->m_qiblaDegrees = Adhan::qibla(*this->coordinates);
    this->m_prayerTimes  = std::make_unique<Adhan::PrayerTimes>(
        *this->coordinates, QDate::currentDate().toStdSysDays(), *this->calcParams);
    this->m_sunnahTimes = std::make_unique<Adhan::SunnahTimes>(*this->m_prayerTimes);

    core->alarmManager().clear("athan"_L1);

    // `Sunrise` is omitted, unless necessary
    if (const QDateTime dt{this->timeForPrayer(Prayer::Fajr)}; dt.isValid())
        core->alarmManager().add("athan"_L1, dt);
    if (const QDateTime dt{this->timeForPrayer(Prayer::Dhuhr)}; dt.isValid())
        core->alarmManager().add("athan"_L1, dt);
    if (const QDateTime dt{this->timeForPrayer(Prayer::Asr)}; dt.isValid())
        core->alarmManager().add("athan"_L1, dt);
    if (const QDateTime dt{this->timeForPrayer(Prayer::Maghrib)}; dt.isValid())
        core->alarmManager().add("athan"_L1, dt);
    if (const QDateTime dt{this->timeForPrayer(Prayer::Isha)}; dt.isValid())
        core->alarmManager().add("athan"_L1, dt);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::onCoreOnline() noexcept
{
    AppBase::onCoreOnline();
    // These four could be packed in one method, but better to keep granular for future benefits
    this->setupAlarmsResetRelay();
    this->setupAthanAlarmRelay();
    this->setupDateChangedRelay();
    this->setupMinuteChangedRelay();
}

void PrayerEngine::onInvalidate()
{
    logDebug(cat) << "Updating calculations...";

    this->updateCalc();
    emit this->calcChanged(); // deliberately NOT inside `updateCalc()`
}

void PrayerEngine::onLocationEngineOnline() noexcept
{ this->setupLocationChangedRelay(); }

} // namespace App