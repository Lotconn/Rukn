#include "internal-helpers.hpp"
#include <UiEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

UiEngine::UiEngine(QObject *parent) : QObject{parent}
{
    if (UiEngine::s_singletonInstance != nullptr)
        logFatal(munadi)
            << "A second instance was constructed; only one instance may exist per process";

    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); }, munadi());
    PrayerEngine::onceOnline(this, [this] -> void { this->onPrayerEngineOnline(); }, munadi());

    UiEngine::s_singletonInstance = this;
}

UiEngine::~UiEngine() = default;

UiEngine *UiEngine::instance() noexcept
{
    if (UiEngine::s_singletonInstance == nullptr)
        logFatal(munadi)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return UiEngine::s_singletonInstance;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::connectToCoreOffline() const
{
    if (const auto conn = QObject::connect(core, &CoreEngine::coreOffline, this,
                                           &UiEngine::onCoreOffline, Qt::DirectConnection);
        conn == nullptr)
        logWarning(munadi) << "coreOffline signal not connected";
    else
        logDebug(munadi) << "coreOffline signal connected";
}

void UiEngine::connectToInvalidate() const
{
    if (const auto conn = QObject::connect(pEngine, &PrayerEngine::invalidate, this,
                                           &UiEngine::onInvalidate, Qt::DirectConnection);
        conn == nullptr)
        logWarning(munadi) << "Invalidate signal not connected";
    else
        logDebug(munadi) << "Invalidate signal connected";
}

void UiEngine::connectToLocaleChanged() const
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::localeChanged, this, &UiEngine::onLocaleChanged);
        conn == nullptr)
        logWarning(munadi) << "localeChanged signal not connected";
    else
        logDebug(munadi) << "localeChanged signal connected";
}

void UiEngine::initConfigs()
{
    this->m_calcMethodsDisplayIndex =
        UiEngine::indexOfValue(this->m_calcMethodsDisplayNames, pEngine->getCalcMethod());
    this->m_madhabDisplayIndex =
        UiEngine::indexOfValue(this->m_madhabDisplayNames, pEngine->getMadhab());

    logDebug(munadi) << "calcMethodsIdx inited as: [" << this->m_calcMethodsDisplayIndex << ", "
                     << pEngine->getCalcMethod() << "]";
    logDebug(munadi) << "madhabIdx inited as: [" << this->m_madhabDisplayIndex << ", "
                     << pEngine->getMadhab() << "]";
}

void UiEngine::defaultTheme()
{ this->setThemeListIdx(0); }

void UiEngine::dummy(const QStringList & /*unused*/)
{
}

int UiEngine::indexOfValue(const QVariantList &list, const QString &value)
{
    logDebug(munadi) << "Value: " << value;
    const auto it{std::ranges::find_if(list, [&value](const QVariant &item) -> bool {
        return item.toHash().value("name"_L1).toString() == value;
    })};

    return (it == list.end()) ? -1 : static_cast<int>(std::distance(list.begin(), it));
}

void UiEngine::initTexts()
{
    this->m_calcMethodsDisplayNames = {
        QVariantHash{    {"name"_L1, "MuslimWorldLeague"_L1},{"trText"_L1, tr("Muslim World League")}                                                             },
        QVariantHash{{"name"_L1, "MoonsightingCommittee"_L1},
                     {"trText"_L1, tr("Moonsighting Committee")}                                       },
        QVariantHash{            {"name"_L1, "UmmAlQura"_L1},          {"trText"_L1, tr("Umm al-Qura")}},
        QVariantHash{                {"name"_L1, "Dubai"_L1},                {"trText"_L1, tr("Dubai")}},
        QVariantHash{             {"name"_L1, "Egyptian"_L1},             {"trText"_L1, tr("Egyptian")}},
        QVariantHash{              {"name"_L1, "Karachi"_L1},              {"trText"_L1, tr("Karachi")}},
        QVariantHash{               {"name"_L1, "Kuwait"_L1},               {"trText"_L1, tr("Kuwait")}},
        QVariantHash{         {"name"_L1, "NorthAmerica"_L1},
                     {"trText"_L1, tr("Islamic Society of North America")}                             },
        QVariantHash{                {"name"_L1, "Qatar"_L1},                {"trText"_L1, tr("Qatar")}},
        QVariantHash{            {"name"_L1, "Singapore"_L1},            {"trText"_L1, tr("Singapore")}},
        QVariantHash{               {"name"_L1, "Turkey"_L1},      {"trText"_L1, tr("Diyanet, Turkey")}},
        QVariantHash{               {"name"_L1, "Tehran"_L1}, {"trText"_L1, tr("University of Tehran")}},
        QVariantHash{                {"name"_L1, "Other"_L1},                {"trText"_L1, tr("Other")}}
    };

    this->m_madhabDisplayNames = {
        QVariantHash{ {"name"_L1, "Shafi"_L1}, {"trText"_L1, tr("Majority")}},
        QVariantHash{{"name"_L1, "Hanafi"_L1},   {"trText"_L1, tr("Hanafi")}},
    };

    this->m_prayerDisplayNames = {
        QVariantHash{   {"name"_L1, "Fajr"_L1},    {"trText"_L1, tr("Fajr")}},
        QVariantHash{{"name"_L1, "Sunrise"_L1}, {"trText"_L1, tr("Sunrise")}},
        QVariantHash{  {"name"_L1, "Dhuhr"_L1},   {"trText"_L1, tr("Dhuhr")}},
        QVariantHash{    {"name"_L1, "Asr"_L1},     {"trText"_L1, tr("Asr")}},
        QVariantHash{{"name"_L1, "Maghrib"_L1}, {"trText"_L1, tr("Maghrib")}},
        QVariantHash{   {"name"_L1, "Isha"_L1},    {"trText"_L1, tr("Isha")}}
    };

    this->m_themeList = {tr("System"), tr("Light"), tr("Dark")};
}

void UiEngine::loadConfigs()
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{"UiEngineConfigMigrated"_L1};

    loadConfigKey(
        "Adhan/calcMethodIndex"_L1, {}, migrateKey,
        [](const QVariant &val) -> void {
            QAtomicInt null;

            fetchAndSetListIdx(val.toInt(), UiEngine::oldCalcMethodsDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setCalcMethod(name); });
        },
        configMigrated);

    loadConfigKey(
        "Adhan/madhab"_L1, {}, migrateKey,
        [](const QVariant &val) -> void {
            QAtomicInt null;

            fetchAndSetListIdx(val.toInt(), UiEngine::oldMadhabDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setMadhab(name); });
        },
        configMigrated);

    if (configMigrated)
        core->writeSettings("UiEngineConfigMigrated"_L1, true, false);
}

// int UiEngine::textsRefresher()
// { return textsRefresherReal(); }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

void UiEngine::saveConfigs()
{ return; }

void UiEngine::setCalcMethodDisplayIdx(const QVariant &index)
{
    qDebug() << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_calcMethodsDisplayNames,
                       this->m_calcMethodsDisplayIndex,
                       [](const QString &name) -> void { pEngine->setCalcMethod(name); });
}

void UiEngine::setMadhabDisplayIdx(const QVariant &index)
{
    fetchAndSetListIdx(index.toInt(), this->m_madhabDisplayNames, this->m_madhabDisplayIndex,
                       [](const QString &name) -> void { pEngine->setMadhab(name); });
}

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::onInvalidate()
{
    this->m_calcMethodsDisplayIndex =
        UiEngine::indexOfValue(this->m_calcMethodsDisplayNames, pEngine->getCalcMethod());
    this->m_madhabDisplayIndex =
        UiEngine::indexOfValue(this->m_madhabDisplayNames, pEngine->getMadhab());

    emit this->configChanged();
}

void UiEngine::onCoreOffline()
{ this->saveConfigs(); }

void UiEngine::onCoreOnline()
{
    this->connectToLocaleChanged();
    this->initTexts();
    this->loadConfigs();
    this->connectToCoreOffline();
}

void UiEngine::onPrayerEngineOnline()
{
    logDebug(munadi) << "Executing onPrayerEngineOnline()";
    this->connectToInvalidate();
    this->initConfigs();
}

void UiEngine::onLocaleChanged()
{
    this->initTexts();

    emit this->translationChanged();
}
} // namespace Munadi