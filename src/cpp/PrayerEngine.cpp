#include "internal-helpers.hpp"
#include <PrayerEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "PrayerEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(prayerEngine, "PrayerEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

PrayerEngine::PrayerEngine(QObject *parent) : QObject{parent}
{
    if (PrayerEngine::s_singletonInstance != nullptr)
        logFatal(prayerEngine)
            << "A second instance was constructed; only one instance may exist per process";

    this->defaultCalcParams();
    this->defaultPrayerConfig();
    CoreEngine::onceOnline(this, [this] { this->onCoreOnline(); });

    PrayerEngine::s_singletonInstance = this;
}

PrayerEngine::~PrayerEngine() = default;

PrayerEngine *PrayerEngine::instance() noexcept
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        logFatal(prayerEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return PrayerEngine::s_singletonInstance;
}

Prayer PrayerEngine::currentPrayer()
{ return Prayer::Maghrib; }

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

PrayerEngine::Prayer PrayerEngine::fromString(QAnyStringView prayerName)
{
    const int val{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName.toString()))};

    return (val == -1) ? Prayer::None : static_cast<Prayer>(val);
}

QVariant PrayerEngine::getPrayerConfig(Prayer prayerEnum, QAnyStringView field)
{
    if (!this->prayerConfigFields.contains(field.toString()))
    {
        logCritical(prayerEngine) << field << "is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.value(PrayerEngine::toString(prayerEnum))
        .toMap()
        .value(field.toString());
}

const QMetaEnum &PrayerEngine::metaPrayerEnum()
{ return PrayerEngine::s_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(const QString &key, const QVariantMap &entry)
{
    const int enumVal{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(key))};

    if (enumVal == -1)
    {
        logCritical(prayerEngine) << key << " resolves to unknown enum value";

        return;
    }

    const QString enumName{QString(PrayerEngine::s_metaPrayerEnum.scope()) + "::" + key};

    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(prayerEngine) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    if (!isValidEntry(entry, this->prayerConfigEntrySize, this->prayerConfigFields))
    {
        logCritical(prayerEngine) << "[" << entry << "] is not a valid entry in prayerConfig";

        return;
    }

    const QWriteLocker lock{&this->varLock};
    QVariantMap        tempMap{this->m_prayerConfig.value(key).toMap()};

    if (tempMap == entry)
    {
        logDebug(prayerEngine) << "Skipping setting identical entry";

        return;
    }

    tempMap.insert(entry);
    this->m_prayerConfig.insert(key, tempMap);
    emit prayerConfigChanged(this->m_prayerConfig);
}

QString PrayerEngine::toString(Prayer prayerEnum)
{
    const char *key{PrayerEngine::s_metaPrayerEnum.valueToKey(prayerEnum)};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(prayerEngine) << "\"" << prayerEnum
                                  << "\" does not resolve to a valid Prayer enum";
        return {};
    }

    return key;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::connectToCoreOffline() const
{
    if (const auto conn = QObject::connect(CoreEngine::instance(), &CoreEngine::coreOffline, this,
                                           &PrayerEngine::onCoreOffline, Qt::DirectConnection);
        conn == nullptr)
        logWarning(prayerEngine) << "coreOffline signal not connected";
    else
        logDebug(prayerEngine) << "coreOffline signal connected";
}

void PrayerEngine::defaultCalcParams()
{
    this->m_calcParams = std::make_unique<Adhan::CalculationParameters>(
        Adhan::CalculationMethod::MuslimWorldLeague());

    // Adhan::CalculationMethod() defaults to madhab == Shafi and all prayer times adjustments to 0
    // except Dhuhr == 1
}

void PrayerEngine::defaultPrayerConfig()
{
    this->m_prayerConfig = {
        {   "Fajr"_L1, QVariantMap{{"adjustment"_L1, 0.0}}},
        {  "Dhuhr"_L1, QVariantMap{{"adjustment"_L1, 0.0}}},
        {    "Asr"_L1, QVariantMap{{"adjustment"_L1, 0.0}}},
        {"Maghrib"_L1, QVariantMap{{"adjustment"_L1, 0.0}}},
        {   "Isha"_L1, QVariantMap{{"adjustment"_L1, 0.0}}},
    };

    const QVariantMap maps{this->m_prayerConfig.value("Fajr"_L1).toMap()};

    this->prayerConfigKeys = QSet(this->m_prayerConfig.keyBegin(), this->m_prayerConfig.keyEnd());
    this->prayerConfigMapSize   = this->m_prayerConfig.size();
    this->prayerConfigEntrySize = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->prayerConfigFields.insert(key, value.typeId());
}

void PrayerEngine::loadConfigs()
{
    logDebug(prayerEngine) << "Loading prayer settings from config file";

    bool configMigrated{true};

    this->loadPrayerConfig(configMigrated);

    if (configMigrated)
        CoreEngine::instance()->writeSettings("PrayerEngineConfigMigrated"_L1, true);
}

void PrayerEngine::loadPrayerConfig(bool &configMigrated)
{
    QVariantMap  cfgMap{this->m_prayerConfig};
    QVariantList oldPrayerConfig;
    const bool   migrated{
        CoreEngine::instance()->readSettings("PrayerEngineConfigMigrated"_L1).toBool()};

    if (!migrated)
        oldPrayerConfig = CoreEngine::instance()->readOldSettingsJsArray("Adhan/prayerConfig"_L1);

    if (!oldPrayerConfig.isEmpty())
        logInfo(prayerEngine) << "Migrating from old config";

    for (const QString &prayerName : this->prayerConfigKeys)
    {
        const int i = PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(prayerName));

        if (i == -1)
        {
            logCritical(prayerEngine) << prayerName << " resolves to unknown enum value";

            continue;
        }

        if (static_cast<Prayer>(i) == Prayer::Sunrise || static_cast<Prayer>(i) == Prayer::None)
        {
            logWarning(prayerEngine) << "Modifying \"" << prayerName << "\" is not allowed";

            return;
        }

        QVariantMap entry{this->m_prayerConfig.value(prayerName).toMap()};

        for (const QString &key : this->prayerConfigFields.keys())
        {
            QVariant cfgVal;

            cfgVal = (oldPrayerConfig.isEmpty())
                         ? CoreEngine::instance()->readSettings(
                               "PrayerConfig/"_L1 + prayerName + '/'_L1 + key, entry.value(key))
                         : oldPrayerConfig.at(i).toMap().value(key);

            logDebug(prayerEngine)
                << "Value for key \"" << key << "\" in list item " << prayerName << ": " << cfgVal;

            if (!cfgVal.convert(QMetaType(this->prayerConfigFields.value(key))))
            {
                logWarning(prayerEngine) << "Failed converting stored value for \"" << key
                                         << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgMap.insert(prayerName, entry);
    }

    this->setPrayerConfig(cfgMap);

    if (migrated)
        configMigrated = false;
}

void PrayerEngine::saveConfigs()
{
    logDebug(prayerEngine) << "Saving prayer configurations";

    for (const QString &prayerName : this->prayerConfigKeys)
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        for (const auto &[key, value] : m_prayerConfig.value(prayerName).toMap().asKeyValueRange())
            CoreEngine::instance()->writeSettings("PrayerConfig/"_L1 + prayerName + '/'_L1 + key,
                                                  value);
    }
}

void PrayerEngine::setPrayerConfig(const QVariantMap &prayerConfig)
{
    if (prayerConfig.isEmpty())
    {
        logDebug(prayerEngine) << "Skipping empty config list";

        return;
    }

    const QWriteLocker lock{&this->varLock};

    if (prayerConfig == this->m_prayerConfig)
    {
        logDebug(prayerEngine) << "Skipping setting identical list";

        return;
    }

    if (!isValidEntry(prayerConfig, this->prayerConfigEntrySize, this->prayerConfigFields,
                      this->prayerConfigMapSize))
    {
        logCritical(prayerEngine) << "Config [" << prayerConfig << "] is not a valid configuration";

        return;
    }

    this->m_prayerConfig = prayerConfig;

    emit this->prayerConfigChanged(this->m_prayerConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::onCoreOffline()
{ this->saveConfigs(); }

void PrayerEngine::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
}

void PrayerEngine::onInvalidate()
{
    const QWriteLocker lock{&this->varLock};

    this->m_calcParams->madhab = Adhan::CalculationMethod::MuslimWorldLeague().madhab;
    this->m_calcParams->adjustments.fajr =
        this->getPrayerConfig(Prayer::Fajr, "adjustment").toInt();
    this->m_calcParams->adjustments.dhuhr =
        this->getPrayerConfig(Prayer::Dhuhr, "adjustment").toInt();
    this->m_calcParams->adjustments.asr = this->getPrayerConfig(Prayer::Asr, "adjustment").toInt();
    this->m_calcParams->adjustments.maghrib =
        this->getPrayerConfig(Prayer::Maghrib, "adjustment").toInt();
    this->m_calcParams->adjustments.isha =
        this->getPrayerConfig(Prayer::Isha, "adjustment").toInt();
}

} // namespace Munadi