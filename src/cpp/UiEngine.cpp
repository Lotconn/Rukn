#include "internal-helpers.hpp"
#include <UiEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(uiEngine, "UiEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

UiEngine::UiEngine(QObject *parent) : QObject{parent}
{
    if (UiEngine::s_singletonInstance != nullptr)
        logFatal(uiEngine)
            << "A second instance was constructed; only one instance may exist per process";

    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    PrayerEngine::onceOnline(this, [this] -> void { this->onPrayerEngineOnline(); });

    UiEngine::s_singletonInstance = this;
}

UiEngine::~UiEngine() = default;

UiEngine *UiEngine::instance() noexcept
{
    if (UiEngine::s_singletonInstance == nullptr)
        logFatal(uiEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return UiEngine::s_singletonInstance;
}

QString UiEngine::prayerDisplayName(Prayer prayerEnum)
{ return this->m_prayerDisplayNames.value(prayerEnum); }

QStringList UiEngine::prayerDisplayNamesList()
{
    QStringList list;

    for (const auto &[key, value] : this->m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::connectToCoreOffline() const
{
    if (const auto conn = QObject::connect(CoreEngine::instance(), &CoreEngine::coreOffline, this,
                                           &UiEngine::onCoreOffline, Qt::DirectConnection);
        conn == nullptr)
        logWarning(uiEngine) << "coreOffline signal not connected";
    else
        logDebug(uiEngine) << "coreOffline signal connected";
}

void UiEngine::connectToInvalidate() const
{
    if (const auto conn = QObject::connect(PrayerEngine::instance(), &PrayerEngine::invalidate,
                                           this, &UiEngine::onInvalidate, Qt::DirectConnection);
        conn == nullptr)
        logWarning(uiEngine) << "Invalidate signal not connected";
    else
        logDebug(uiEngine) << "Invalidate signal connected";
}

void UiEngine::connectToLocaleChanged() const
{
    if (const auto conn = QObject::connect(CoreEngine::instance(), &CoreEngine::localeChanged, this,
                                           &UiEngine::onLocaleChanged);
        conn == nullptr)
        logWarning(uiEngine) << "localeChanged signal not connected";
    else
        logDebug(uiEngine) << "localeChanged signal connected";
}

void UiEngine::initConfigs()
{
    this->m_calcMethodsDisplayIndex = UiEngine::indexOfValue(
        this->m_calcMethodsDisplayNames, PrayerEngine::instance()->getCalcMethod());
    this->m_madhabDisplayIndex =
        UiEngine::indexOfValue(this->m_madhabDisplayNames, PrayerEngine::instance()->getMadhab());

    logDebug(uiEngine) << "calcMethodsIdx inited as: [" << this->m_calcMethodsDisplayIndex << ", "
                       << PrayerEngine::instance()->property(qPrintable("calcMethod"_L1)).toString()
                       << "]";
    logDebug(uiEngine) << "madhabIdx inited as: [" << this->m_madhabDisplayIndex << ", "
                       << PrayerEngine::instance()->property(qPrintable("madhab"_L1)).toString()
                       << "]";
}

void UiEngine::defaultTheme()
{ this->setThemeListIdx(0); }

void UiEngine::dummy(const QStringList & /*unused*/)
{
}

int UiEngine::indexOfValue(const QVariantList &list, const QString &value)
{
    logDebug(uiEngine) << "Value: " << value;
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
        {   Prayer::Fajr,    tr("Fajr")},
        {Prayer::Sunrise, tr("Sunrise")},
        {  Prayer::Dhuhr,   tr("Dhuhr")},
        {    Prayer::Asr,     tr("Asr")},
        {Prayer::Maghrib, tr("Maghrib")},
        {   Prayer::Isha,    tr("Isha")}
    };

    this->m_themeList = {tr("System"), tr("Light"), tr("Dark")};
}

void UiEngine::loadConfigs()
{
    logDebug(uiEngine) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{"UiEngineConfigMigrated"_L1};

    loadConfigKey(
        "Adhan/calcMethodIndex"_L1, {}, migrateKey,
        [&idxVar = this->oldCalcMethodsDisplayIndex](const QVariant &val) -> void {
            fetchAndSetListIdx(
                val.toInt(), UiEngine::oldCalcMethodsDisplayNames, idxVar,
                [](const QString &name) -> void { PrayerEngine::instance()->setCalcMethod(name); });
        },
        configMigrated);

    loadConfigKey(
        "Adhan/madhab"_L1, {}, migrateKey,
        [&idxVar = this->oldMadhabDisplayIndex](const QVariant &val) -> void {
            fetchAndSetListIdx(
                val.toInt(), UiEngine::oldMadhabDisplayNames, idxVar,
                [](const QString &name) -> void { PrayerEngine::instance()->setMadhab(name); });
        },
        configMigrated);

    if (configMigrated)
        CoreEngine::instance()->writeSettings("UiEngineConfigMigrated"_L1, true, false);
}

int UiEngine::textsRefresher()
{ return textsRefresherReal(); }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

void UiEngine::saveConfigs()
{ return; }

void UiEngine::setCalcMethodDisplayIdx(const QVariant &index)
{
    qDebug() << "Received index: " << index;
    fetchAndSetListIdx(
        index.toInt(), this->m_calcMethodsDisplayNames, this->m_calcMethodsDisplayIndex,
        [](const QString &name) -> void { PrayerEngine::instance()->setCalcMethod(name); });
}

void UiEngine::setMadhabDisplayIdx(const QVariant &index)
{
    fetchAndSetListIdx(
        index.toInt(), this->m_madhabDisplayNames, this->m_madhabDisplayIndex,
        [](const QString &name) -> void { PrayerEngine::instance()->setMadhab(name); });
}

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::onInvalidate()
{
    this->m_calcMethodsDisplayIndex = UiEngine::indexOfValue(
        this->m_calcMethodsDisplayNames,
        PrayerEngine::instance()->property(qPrintable("calcMethod"_L1)).toString());
    this->m_madhabDisplayIndex = UiEngine::indexOfValue(
        this->m_madhabDisplayNames,
        PrayerEngine::instance()->property(qPrintable("madhab"_L1)).toString());

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
    logDebug(uiEngine) << "Executing onPrayerEngineOnline()";
    this->connectToInvalidate();
    this->initConfigs();
}

void UiEngine::onLocaleChanged()
{
    this->initTexts();

    emit this->translationChanged();
}
} // namespace Munadi