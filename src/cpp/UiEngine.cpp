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

    logDebug(munadi) << "Booting engine...";

    this->defaultConfigs();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); }, munadi());
    PrayerEngine::onceOnline(this, [this] -> void { this->onPrayerEngineOnline(); }, munadi());

    UiEngine::s_singletonInstance = this;

    logDebug(munadi) << "Engine online";
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

void UiEngine::defaultConfigs()
{
    constexpr static double defaultBlur{0.45};    // NOLINT(*-magic-numbers)
    constexpr static double defaultOpacity{0.77}; // NOLINT(*-magic-numbers)
    constexpr static quint8 defaultLangIdx{0};
    constexpr static quint8 defaultThemeIdx{0};

    this->m_bgImage          = true;
    this->m_blur             = defaultBlur;
    this->m_opacity          = defaultOpacity;
    this->m_langDisplayIndex = defaultLangIdx;

    this->setThemeListIdx(defaultThemeIdx);
}

void UiEngine::dummy(const QStringList & /*unused*/)
{
}

QString UiEngine::getLangKey()
{ return this->m_langDisplayNames[this->m_langDisplayIndex].toMap().value("name"_L1).toString(); }

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

    this->m_langDisplayNames = {
        QVariantHash{ {"name"_L1, "System"_L1}, {"trText"_L1, tr("System (default)")}},
        QVariantHash{{"name"_L1, "English"_L1},           {"trText"_L1, "English"_L1}},
        QVariantHash{ {"name"_L1, "Arabic"_L1},              {"trText"_L1, "العربية"}},
        QVariantHash{ {"name"_L1, "Bangla"_L1},                {"trText"_L1, "বাংলা"}}
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

    loadConfigKey("App/bgImage"_L1, "App/bgImage"_L1, migrateKey, this, "bgImage"_L1,
                  configMigrated);

    loadConfigKey(
        "App/blur"_L1, "App/blur"_L1, migrateKey,
        [this, &migrateKey](const QVariant &value) -> void {
            const double maxBlur{64};
            double       realValue{value.toDouble()};

            if (!core->readSettings(migrateKey).toBool())
                realValue /= maxBlur;

            logDebug(munadi) << "blur: " << realValue;
            this->setProperty("blur", realValue);
        },
        configMigrated);

    loadConfigKey(
        "Adhan/calcMethodIndex"_L1, {}, migrateKey,
        [](const QVariant &val) -> void {
            QAtomicInt null;

            fetchAndSetListIdx(val.toInt(), UiEngine::oldCalcMethodsDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setCalcMethod(name); });
        },
        configMigrated);

    loadConfigKey({}, "App/Language"_L1, {}, [this](const QVariant &value) -> void {
        const int index = UiEngine::indexOfValue(this->m_langDisplayNames, value.toString());

        logDebug(munadi) << "Loaded from config: langIdx: " << index;
        this->setLangDisplayIdx(index);
    });

    loadConfigKey(
        "Adhan/madhab"_L1, {}, migrateKey,
        [](const QVariant &val) -> void {
            QAtomicInt null;

            fetchAndSetListIdx(val.toInt(), UiEngine::oldMadhabDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setMadhab(name); });
        },
        configMigrated);

    loadConfigKey("App/opacity"_L1, "App/opacity"_L1, migrateKey, this, "opacity"_L1,
                  configMigrated);

    if (configMigrated)
        core->writeSettings("UiEngineConfigMigrated"_L1, true, false);
}

void UiEngine::saveConfigs()
{
    logDebug(munadi) << "Saving UI configurations";

    core->writeSettings("App/bgImage"_L1, this->m_bgImage.load());
    core->writeSettings("App/blur"_L1, this->m_blur.load());
    core->writeSettings("App/Language"_L1, this->getLangKey());
    core->writeSettings("App/opacity"_L1, this->m_opacity.load());
}

void UiEngine::setCalcMethodDisplayIdx(const QVariant &index)
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_calcMethodsDisplayNames,
                       this->m_calcMethodsDisplayIndex,
                       [](const QString &name) -> void { pEngine->setCalcMethod(name); });
}

void UiEngine::setLangDisplayIdx(const QVariant &index)
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_langDisplayNames, this->m_langDisplayIndex,
                       [](const QString &name) -> void { core->setLang(name); });
}

void UiEngine::setMadhabDisplayIdx(const QVariant &index)
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_madhabDisplayNames, this->m_madhabDisplayIndex,
                       [](const QString &name) -> void { pEngine->setMadhab(name); });
}

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

// int UiEngine::textsRefresher()
// { return textsRefresherReal(); }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

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
{
    logDebug(munadi) << "Going offline, executing offline hooks";
    this->saveConfigs();
}

void UiEngine::onCoreOnline()
{
    this->connectToLocaleChanged();
    this->initTexts();
    this->connectToCoreOffline();
}

void UiEngine::onPrayerEngineOnline()
{
    logDebug(munadi) << "Executing onPrayerEngineOnline()";
    this->connectToInvalidate();
    this->loadConfigs(); // loadConfigs() needs to be before initConfigs()
    this->initConfigs();
}

void UiEngine::onLocaleChanged()
{
    logDebug(munadi) << "Detected language change, refreshing texts...";
    this->initTexts();
    emit this->translationChanged();
}
} // namespace Munadi