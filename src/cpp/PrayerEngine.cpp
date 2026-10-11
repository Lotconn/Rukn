#include "internal-helpers.hpp"
#include <PrayerEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "PrayerEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

PrayerEngine::PrayerEngine(QObject *parent) : QObject{parent}
{
    if (PrayerEngine::s_singletonInstance != nullptr)
        logFatal(munadi)
            << "A second instance was constructed; only one instance may exist per process";

    this->defaultCalcParams();
    this->defaultPrayerConfig();
    CoreEngine::onceOnline(this, [this] { this->onCoreOnline(); }, munadi());

    if (const auto conn =
            QObject::connect(this, &PrayerEngine::invalidate, this, &PrayerEngine::onInvalidate);
        conn == nullptr)
        logWarning(munadi) << "Invalidate signal not connected";
    else
        logDebug(munadi) << "Invalidate signal connected";

    PrayerEngine::s_singletonInstance = this;
    PrayerEngine::s_online            = true;

    emit this->engineOnline();
}

PrayerEngine::~PrayerEngine() = default;

bool PrayerEngine::instanceExists() noexcept
{ return PrayerEngine::s_singletonInstance != nullptr; }

PrayerEngine *PrayerEngine::instance() noexcept
{
    if (!PrayerEngine::instanceExists())
        logFatal(munadi)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return PrayerEngine::s_singletonInstance;
}

bool PrayerEngine::isOnline() noexcept
{ return PrayerEngine::s_online; }

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
        logCritical(munadi) << field << "is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.value(PrayerEngine::toString(prayerEnum))
        .toHash()
        .value(field.toString());
}

QString PrayerEngine::getCalcMethod()
{ return this->m_calcMethodKey; }

QString PrayerEngine::getMadhab()
{ return this->m_madhabKey; }

const QMetaEnum &PrayerEngine::metaPrayerEnum()
{ return PrayerEngine::s_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(const QString &key, const QVariantHash &entry)
{
    const int enumVal{PrayerEngine::s_metaPrayerEnum.keyToValue(qPrintable(key))};

    if (enumVal == -1)
    {
        logCritical(munadi) << key << " resolves to unknown enum value";

        return;
    }

    const QString enumName{QString(PrayerEngine::s_metaPrayerEnum.scope()) + "::" + key};

    if (static_cast<Prayer>(enumVal) == Prayer::Sunrise ||
        static_cast<Prayer>(enumVal) == Prayer::None)
    {
        logWarning(munadi) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    if (!isValidEntry(entry, this->prayerConfigEntrySize, this->prayerConfigFields))
    {
        logCritical(munadi) << "[" << entry << "] is not a valid entry in prayerConfig";

        return;
    }

    const QWriteLocker lock{&this->varLock};
    QVariantHash       tempMap{this->m_prayerConfig.value(key).toHash()};

    if (tempMap == entry)
    {
        logDebug(munadi) << "Skipping setting identical entry";

        return;
    }

    tempMap.insert(entry);
    this->m_prayerConfig.insert(key, tempMap);
    emit this->prayerConfigChanged(this->m_prayerConfig);
    emit this->invalidate();
}

QString PrayerEngine::toString(Prayer prayerEnum)
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

void PrayerEngine::defaultCalcParams()
{
    this->m_calcMethodKey = "MuslimWorldLeague"_L1;
    this->m_calcParams    = std::make_unique<Adhan::CalculationParameters>(
        PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)());
    this->m_madhabKey = "Shafi"_L1;

    // Adhan::CalculationMethod() defaults to madhab == Shafi and all prayer times adjustments to 0
    // except Dhuhr == 1
}

void PrayerEngine::defaultPrayerConfig()
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

void PrayerEngine::loadConfigs()
{
    logDebug(munadi) << "Loading prayer settings from config file";

    bool          configMigrated{true};
    const QString migrateKey{"PrayerEngineConfigMigrated"_L1};

    this->loadPrayerConfig(configMigrated);
    loadConfigKey({}, "PrayerConfig/CalcMethod"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setCalcMethod(name.toString()); });
    loadConfigKey({}, "PrayerConfig/Madhab"_L1, migrateKey,
                  [this](const QVariant &name) -> void { this->setMadhab(name.toString()); });

    if (configMigrated)
        core->writeSettings("PrayerEngineConfigMigrated"_L1, true, false);
}

void PrayerEngine::loadPrayerConfig(bool &configMigrated)
{
    QVariantHash cfgMap{this->m_prayerConfig};
    QVariantList oldPrayerConfig;
    const bool   migrated{core->readSettings("PrayerEngineConfigMigrated"_L1).toBool()};

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

    this->setPrayerConfig(cfgMap);

    if (migrated)
        configMigrated = false;
}

void PrayerEngine::onceOnline(QObject *context, std::function<void()> slot,
                              const QLoggingCategory &category, int retriesLeft)
{
    if (!PrayerEngine::instanceExists())
    {
        if (retriesLeft <= 0)
        {
            logCritical(category) << "PrayerEngine still not constructed after several retries";

            return;
        }

        QTimer::singleShot(0, context, [&context, slot, &category, &retriesLeft]() mutable {
            PrayerEngine::onceOnline(context, std::move(slot), category, retriesLeft - 1);
        });

        return;
    }

    if (const auto conn = QObject::connect(PrayerEngine::instance(), &PrayerEngine::engineOnline,
                                           context, slot, Qt::DirectConnection);
        conn == nullptr)
        logWarning(category) << "prayerOnline signal not connected";
    else
        logDebug(category) << "prayerOnline signal connected";

    if (PrayerEngine::isOnline())
        slot();
}

void PrayerEngine::saveConfigs()
{
    logDebug(munadi) << "Saving prayer configurations";

    for (const QString &prayerName : this->prayerConfigKeys)
    {
        // prayerName == "Sunrise" and prayerEnum == "None" checks are not necessary
        if (prayerName.isEmpty())
            continue;

        for (const auto &[key, value] : m_prayerConfig.value(prayerName).toHash().asKeyValueRange())
            core->writeSettings("PrayerConfig/Prayer/"_L1 + prayerName + '/'_L1 + key, value);
    }

    core->writeSettings("PrayerConfig/CalcMethod"_L1, this->m_calcMethodKey);
    core->writeSettings("PrayerConfig/Madhab"_L1, this->m_madhabKey);
}

void PrayerEngine::setCalcMethod(const QString &method)
{

    const QWriteLocker lock{&varLock};

    logDebug(munadi) << "method: " << method;
    if (setConfig(PrayerEngine::s_calcMethods, method, this->m_calcMethodKey))
        emit this->invalidate();
}

void PrayerEngine::setMadhab(const QString &madhab)
{
    const QWriteLocker lock{&varLock};

    logDebug(munadi) << "madhab: " << madhab;
    if (setConfig(PrayerEngine::s_madhab, madhab, this->m_madhabKey))
        emit this->invalidate();
}

void PrayerEngine::setPrayerConfig(const QVariantHash &prayerConfig)
{
    if (prayerConfig.isEmpty())
    {
        logDebug(munadi) << "Skipping empty config list";

        return;
    }

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
    connectToCoreOffline(this, [this] -> void { PrayerEngine::onCoreOffline(); }, munadi());
}

void PrayerEngine::onInvalidate()
{
    const QWriteLocker lock{&this->varLock};

    *this->m_calcParams        = PrayerEngine::s_calcMethods.value(this->m_calcMethodKey)();
    this->m_calcParams->madhab = PrayerEngine::s_madhab.value(this->m_madhabKey);
    this->m_calcParams->adjustments.fajr =
        this->getPrayerConfig(Prayer::Fajr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.dhuhr =
        this->getPrayerConfig(Prayer::Dhuhr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.asr =
        this->getPrayerConfig(Prayer::Asr, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.maghrib =
        this->getPrayerConfig(Prayer::Maghrib, "adjustment"_L1).toInt();
    this->m_calcParams->adjustments.isha =
        this->getPrayerConfig(Prayer::Isha, "adjustment"_L1).toInt();
}

} // namespace Munadi