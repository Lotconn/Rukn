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
    QTimer::singleShot(0, this, &PrayerEngine::connectToCoreOnline);

    PrayerEngine::s_singletonInstance = this;
}

PrayerEngine::~PrayerEngine()
{
    QObject::disconnect(this->coreOfflineConnection);
    QObject::disconnect(this->coreOnlineConnection);

    if (PrayerEngine::s_singletonInstance == this)
        PrayerEngine::s_singletonInstance = nullptr;
}

PrayerEngine &PrayerEngine::instance()
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        logFatal(prayerEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *PrayerEngine::s_singletonInstance;
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

PrayerEngine::Prayer PrayerEngine::fromString(QAnyStringView prayerName)
{
    const int val{PrayerEngine::m_metaPrayerEnum.keyToValue(qPrintable(prayerName.toString()))};

    return (val == -1) ? Prayer::None : static_cast<Prayer>(val);
}

QVariant PrayerEngine::getPrayerConfig(Prayer prayerEnum, QAnyStringView field)
{
    if (!this->prayerConfigKeys.contains(field.toString()))
    {
        logCritical(prayerEngine) << field << "is not a valid field in PrayerConfig";
        return {};
    }

    return this->m_prayerConfig.at(qToUnderlying(prayerEnum)).toMap().value(field.toString());
}

const QMetaEnum &PrayerEngine::metaPrayerEnum()
{ return PrayerEngine::m_metaPrayerEnum; }

void PrayerEngine::modifyPrayerConfig(Prayer prayerEnum, const QVariantMap &entry)
{
    const char *key{m_metaPrayerEnum.valueToKey(prayerEnum)};

    // QML-side invalid int index -> Prayer enum cast guard
    if (key == nullptr)
    {
        logCritical(prayerEngine) << "\"" << prayerEnum
                                  << "\" does not resolve to a valid Prayer enum";

        return;
    }

    const QString enumName{QString(PrayerEngine::m_metaPrayerEnum.scope()) + "::" + key};

    if (prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
    {
        logWarning(prayerEngine) << "Modifying \"" << enumName << "\" is not allowed";

        return;
    }

    if (!isValidEntry(entry, this->prayerConfigMapSize, this->prayerConfigKeys))
    {
        logCritical(prayerEngine) << "[" << entry << "] is not a valid entry in prayerConfig";

        return;
    }

    QWriteLocker lock{&this->varLock};
    QVariantMap  tempMap{this->m_prayerConfig.at(qToUnderlying(prayerEnum)).toMap()};

    if (tempMap == entry)
    {
        logDebug(prayerEngine) << "Skipping setting identical entry";

        return;
    }

    tempMap.insert(entry);
    this->m_prayerConfig.replace(qToUnderlying(prayerEnum), tempMap);
    emit prayerConfigChanged(this->m_prayerConfig);
}

QString PrayerEngine::toString(Prayer prayer)
{ return PrayerEngine::m_metaPrayerEnum.valueToKey(qToUnderlying(prayer)); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void PrayerEngine::connectToCoreOffline()
{
    this->coreOfflineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOffline, this,
                         &PrayerEngine::onCoreOffline, Qt::DirectConnection);

    if (this->coreOfflineConnection == nullptr)
        logWarning(prayerEngine) << "aboutToQuit signal not connected";
    else
        logDebug(prayerEngine) << "aboutToQuit signal connected";
}

void PrayerEngine::connectToCoreOnline()
{
    if (!CoreEngine::exists())
    {
        // CoreEngine not constructed yet on this tick; retry next tick.
        logDebug(prayerEngine) << "CoreEngine doesn't exist yet";
        QTimer::singleShot(0, this, &PrayerEngine::connectToCoreOnline);
        return;
    }

    this->coreOnlineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOnline, this,
                         &PrayerEngine::onCoreOnline, Qt::DirectConnection);

    if (this->coreOnlineConnection == nullptr)
        logWarning(prayerEngine) << "coreOnline signal not connected";
    else
        logDebug(prayerEngine) << "coreOnline signal connected";

    // coreOnline() already fired before we connected; manually reconcile
    logDebug(prayerEngine) << "CoreEngine already online; catching up";

    this->onCoreOnline();
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
        QVariantMap{{"adjustment", 0.0}}, // [0] Fajr
        QVariantMap{{"adjustment", 0.0}}, // [1] Sunrise
        QVariantMap{{"adjustment", 0.0}}, // [2] Dhuhr
        QVariantMap{{"adjustment", 0.0}}, // [3] Asr
        QVariantMap{{"adjustment", 0.0}}, // [4] Maghrib
        QVariantMap{{"adjustment", 0.0}}  // [5] Isha
    };

    const QVariantMap maps{this->m_prayerConfig.at(0).toMap()};

    this->prayerConfigListSize = this->m_prayerConfig.size();
    this->prayerConfigMapSize  = maps.size();

    for (const auto &[key, value] : maps.asKeyValueRange())
        this->prayerConfigKeys.insert(key, value.typeId());
}

void PrayerEngine::loadConfigs()
{
    logDebug(prayerEngine) << "Loading prayer settings from config file";

    QVariantList cfgList(this->m_prayerConfig); // don't use cfgList{}

    for (qsizetype i = 0; i < this->prayerConfigListSize; ++i)
    {
        const Prayer  prayerEnum{static_cast<Prayer>(i)};
        const QString prayerName{PrayerEngine::toString(prayerEnum)};

        if (prayerName.isEmpty() || prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
            continue;

        QVariantMap entry{m_prayerConfig.at(i).toMap()};

        for (const QString &key : this->prayerConfigKeys.keys())
        {
            QVariant cfgVal{CoreEngine::instance().readSettings(
                "PrayerConfig/" + prayerName + "/" + key, entry.value(key))};

            logDebug(prayerEngine)
                << "Value for key \"" << key << "\" in list item " << i << ": " << cfgVal;

            // should go before isValidEntry check
            if (!cfgVal.convert(QMetaType(this->prayerConfigKeys.value(key))))
            {
                logWarning(prayerEngine) << "Failed converting stored value for \"" << key
                                         << "\" to expected type; keeping default";
                continue;
            }

            entry.insert(key, cfgVal);
        }

        cfgList.replace(i, entry);
    }

    this->setPrayerConfig(cfgList);
}

void PrayerEngine::saveConfigs()
{
    logDebug(prayerEngine) << "Saving prayer configurations";

    for (qsizetype i = 0; i < this->prayerConfigListSize; ++i)
    {
        const Prayer  prayerEnum{static_cast<Prayer>(i)};
        const QString prayerName{PrayerEngine::toString(prayerEnum)};

        if (prayerName == nullptr || prayerEnum == Prayer::Sunrise || prayerEnum == Prayer::None)
            continue;

        for (const auto &[key, value] : m_prayerConfig.at(i).toMap().asKeyValueRange())
            CoreEngine::instance().writeSettings("PrayerConfig/" + prayerName + "/" + key, value);
    }
}

void PrayerEngine::setPrayerConfig(const QVariantList &prayerConfig)
{
    if (prayerConfig.isEmpty())
    {
        logDebug(prayerEngine) << "Skipping empty config list";

        return;
    }

    if (prayerConfig == this->m_prayerConfig)
    {
        logDebug(prayerEngine) << "Skipping setting identical list";

        return;
    }

    if (!isValidEntryList(prayerConfig, this->prayerConfigMapSize, this->prayerConfigKeys,
                          this->prayerConfigListSize))
    {
        logCritical(prayerEngine) << "Config [" << prayerConfig << "] is not a valid configuration";

        return;
    }

    QWriteLocker lock{&this->varLock};

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
    QWriteLocker lock{&this->varLock};

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