#include "internal-helpers.hpp"
#include <LocationEngine.hpp>
#include <PrayerEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "PrayerEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

PrayerEngine::PrayerEngine(QObject *parent) : MunadiEngine<PrayerEngine>{parent}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    LocationEngine::onceOnline(this, [this] -> void { this->onLocationEngineOnline(); });
    this->makeOnline();
}

Prayer PrayerEngine::currentPrayer() noexcept
{ return Prayer::Maghrib; }

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

PrayerEngine::Prayer PrayerEngine::fromString(QAnyStringView prayerName) noexcept
{
    const int val{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName.toString()))};

    return (val == -1) ? Prayer::None : static_cast<Prayer>(val);
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

QVariant PrayerEngine::getPrayerConfig(Prayer prayerEnum, QAnyStringView field) noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->getPrayerConfig_p(prayerEnum, field);
}

QVariant PrayerEngine::getPrayerConfig_p(Prayer prayerEnum, QAnyStringView field) const noexcept
{
    if (!this->prayerConfigFields.contains(field.toString()))
    {
        logCritical(munadi) << field << "is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.value(PrayerEngine::toString(prayerEnum))
        .toHash()
        .value(field.toString());
}

const QLoggingCategory &PrayerEngine::logCategory() noexcept
{ return munadi(); }

const QMetaEnum &PrayerEngine::metaPrayerEnum() noexcept
{ return PrayerEngine::s_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept
{
    const int     enumVal{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(key))};
    const QString enumName{QString(PrayerEngine::s_metaPrayerEnum.scope()) + "::" + key};

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

QString PrayerEngine::toString(Prayer prayerEnum) noexcept
{
    const char *key{PrayerEngine::s_metaPrayerEnum.valueToKey(prayerEnum)};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(munadi) << "\"" << prayerEnum << "\" does not resolve to a valid Prayer enum";
        return {};
    }

    return key;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::connectToInvalidate() const noexcept
{
    if (const auto conn =
            QObject::connect(this, &PrayerEngine::invalidate, this, &PrayerEngine::onInvalidate);
        conn == nullptr)
        logWarning(munadi) << "Invalidate signal not connected";
    else
        logDebug(munadi) << "Invalidate signal connected";
}

void PrayerEngine::defaultCalcParams() noexcept
{
    this->m_calcMethodKey = "MuslimWorldLeague"_L1;
    this->m_calcParams    = std::make_unique<Adhan::CalculationParameters>(
        PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)());
    this->m_madhabKey = "Shafi"_L1;

    // Adhan::CalculationMethod() defaults to madhab == Shafi and all prayer times adjustments
    // to 0 except Dhuhr == 1
}

void PrayerEngine::defaultConfigs() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->defaultCalcParams();
    this->defaultPrayerConfig();
}

void PrayerEngine::defaultPrayerConfig() noexcept
{
    this->m_prayerConfig = {
        {   "Fajr"_L1, QVariantHash{{"adjustment"_L1, 0.0}}},
        {  "Dhuhr"_L1, QVariantHash{{"adjustment"_L1, 0.0}}},
        {    "Asr"_L1, QVariantHash{{"adjustment"_L1, 0.0}}},
        {"Maghrib"_L1, QVariantHash{{"adjustment"_L1, 0.0}}},
        {   "Isha"_L1, QVariantHash{{"adjustment"_L1, 0.0}}},
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
    const QReadLocker lock{&this->varLock};

    logDebug(munadi) << "Saving prayer configurations";

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

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::onLocationEngineOnline() noexcept
{ this->connectToInvalidate(); }

void PrayerEngine::onInvalidate() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->m_calcParams = std::make_unique<Adhan::CalculationParameters>(
        PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)());
    this->m_calcParams->madhab = PrayerEngine::s_madhab.value(this->m_madhabKey);

    // The following needs to call the private getPrayerConfig for avoiding locking contention
    this->m_calcParams->adjustments.fajr =
        this->getPrayerConfig_p(Prayer::Fajr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.dhuhr =
        this->getPrayerConfig_p(Prayer::Dhuhr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.asr =
        this->getPrayerConfig_p(Prayer::Asr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.maghrib =
        this->getPrayerConfig_p(Prayer::Maghrib, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.isha =
        this->getPrayerConfig_p(Prayer::Isha, "adjustment"_L1).toInt();
}

} // namespace Munadi