#include "internal-helpers.hpp"
#include <UiEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

UiEngine::UiEngine(QObject *parent) : MunadiEngine<UiEngine>{parent}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    PrayerEngine::onceOnline(this, [this] -> void { this->onPrayerEngineOnline(); });
    this->makeOnline();
}

const QLoggingCategory &UiEngine::logCategory() noexcept
{ return munadi(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

QVariantList &UiEngine::calcMethodsDisplayNames() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_calcMethodsDisplayNames;
}

void UiEngine::connectToInvalidate() const noexcept
{
    if (const auto conn = QObject::connect(pEngine, &PrayerEngine::invalidate, this,
                                           &UiEngine::onInvalidate, Qt::DirectConnection);
        conn == nullptr)
        logWarning(munadi) << "Invalidate signal not connected";
    else
        logDebug(munadi) << "Invalidate signal connected";
}

void UiEngine::connectToLocaleChanged() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::localeChanged, this, &UiEngine::onLocaleChanged);
        conn == nullptr)
        logWarning(munadi) << "localeChanged signal not connected";
    else
        logDebug(munadi) << "localeChanged signal connected";
}

void UiEngine::defaultConfigs() noexcept
{
    constexpr static double defaultBlur{0.45};    // NOLINT(*-magic-numbers)
    constexpr static double defaultOpacity{0.77}; // NOLINT(*-magic-numbers)
    constexpr static quint8 defaultLangIdx{0};
    constexpr static quint8 defaultThemeIdx{0};
    const QWriteLocker      lock{&this->varLock};

    this->m_bgImage.store(true);
    this->m_blur.store(defaultBlur);
    this->m_opacity.store(defaultOpacity);
    this->m_langDisplayIndex.storeRelaxed(defaultLangIdx);
    this->m_themeDisplayIndex.storeRelaxed(defaultThemeIdx);
}

QString UiEngine::getCurrentLang() const noexcept
{
    return this->m_langDisplayNames.at(this->m_langDisplayIndex.loadRelaxed())
        .toMap()
        .value("name"_L1)
        .toString();
}

QString UiEngine::getCurrentTheme() const noexcept
{
    return this->m_themeDisplayNames.at(this->m_themeDisplayIndex.loadRelaxed())
        .toMap()
        .value("name"_L1)
        .toString();
}

void UiEngine::initConfigs() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->m_calcMethodsDisplayIndex.storeRelaxed(
        UiEngine::indexOfValue(this->m_calcMethodsDisplayNames, pEngine->getCalcMethod()));
    this->m_madhabDisplayIndex.storeRelaxed(
        UiEngine::indexOfValue(this->m_madhabDisplayNames, pEngine->getMadhab()));

    logDebug(munadi) << "calcMethodsIdx inited as: ["
                     << this->m_calcMethodsDisplayIndex.loadRelaxed() << ", "
                     << pEngine->getCalcMethod() << "]";
    logDebug(munadi) << "madhabIdx inited as: [" << this->m_madhabDisplayIndex.loadRelaxed() << ", "
                     << pEngine->getMadhab() << "]";
}

int UiEngine::indexOfValue(const QVariantList &list, const QString &value) noexcept
{
    logDebug(munadi) << "Value: " << value;
    const auto it{std::ranges::find_if(list, [&value](const QVariant &item) -> bool {
        return item.toHash().value("name"_L1).toString() == value;
    })};

    return (it == list.end()) ? -1 : static_cast<int>(std::distance(list.begin(), it));
}

void UiEngine::initTexts() noexcept
{
    const QWriteLocker lock{&this->varLock};

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

    this->m_themeDisplayNames = {
        QVariantHash{{"name"_L1, "System"_L1}, {"trText"_L1, tr("System (default)")}},
        QVariantHash{ {"name"_L1, "Light"_L1},            {"trText"_L1, tr("Light")}},
        QVariantHash{  {"name"_L1, "Dark"_L1},             {"trText"_L1, tr("Dark")}}
    };
}

QVariantList &UiEngine::langDisplayNames() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_langDisplayNames;
}

void UiEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{"UiEngineConfigMigrated"_L1};

    loadConfigKey("App/bgImage"_L1, "App/BackgroundImage"_L1, migrateKey, this, "bgImage"_L1,
                  configMigrated);

    loadConfigKey(
        "App/blur"_L1, "App/Blur"_L1, migrateKey,
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

    loadConfigKey("App/opacity"_L1, "App/Opacity"_L1, migrateKey, this, "opacity"_L1,
                  configMigrated);

    loadConfigKey(
        "App/themeListIndex"_L1, "App/Theme"_L1, migrateKey,
        [this](const QVariant &val) -> void {
            bool      ok{false};
            const int value{val.toInt(&ok)};

            // A readlock isn't necessary

            if (ok)
                fetchAndSetListIdx(value, UiEngine::oldThemeDisplayNames, this->m_themeDisplayIndex,
                                   [](const QString & /*unused*/) -> void {});
            else
                this->m_themeDisplayIndex.storeRelaxed(
                    UiEngine::indexOfValue(this->m_themeDisplayNames, val.toString()));

            emit this->uiConfigChanged();
        },
        configMigrated);

    if (configMigrated)
        core->writeSettings("UiEngineConfigMigrated"_L1, true);
}

QVariantList &UiEngine::madhabDisplayNames() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_madhabDisplayNames;
}

QVariantList &UiEngine::prayerDisplayNames() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_prayerDisplayNames;
}

void UiEngine::saveConfigs() noexcept
{
    const QReadLocker lock{&this->varLock};

    logDebug(munadi) << "Saving UI configurations";

    core->writeSettings("App/BackgroundImage"_L1, this->m_bgImage.load());
    core->writeSettings("App/Blur"_L1, this->m_blur.load());
    core->writeSettings("App/Language"_L1, this->getCurrentLang());
    core->writeSettings("App/Opacity"_L1, this->m_opacity.load());
    core->writeSettings("App/Theme"_L1, this->getCurrentTheme());
}

void UiEngine::setCalcMethodDisplayIdx(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_calcMethodsDisplayNames,
                       this->m_calcMethodsDisplayIndex,
                       [](const QString &name) -> void { pEngine->setCalcMethod(name); });
}

void UiEngine::setLangDisplayIdx(const QVariant &index) noexcept
{
    // Taking a readlock isn't necessary

    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_langDisplayNames, this->m_langDisplayIndex,
                       [](const QString &name) -> void { core->setLang(name); });
    emit this->uiConfigChanged();
}

void UiEngine::setMadhabDisplayIdx(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_madhabDisplayNames, this->m_madhabDisplayIndex,
                       [](const QString &name) -> void { pEngine->setMadhab(name); });
}

// int UiEngine::textsRefresher()
// { return textsRefresherReal(); }

QVariantList &UiEngine::themeDisplayNames() noexcept
{
    const QReadLocker lock{&this->varLock};

    return this->m_themeDisplayNames;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::onCoreOnline() noexcept
{
    this->connectToLocaleChanged();
    this->initTexts();
    this->connectToCoreOffline();
}

void UiEngine::onInvalidate() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->m_calcMethodsDisplayIndex.storeRelaxed(
        UiEngine::indexOfValue(this->m_calcMethodsDisplayNames, pEngine->getCalcMethod()));
    this->m_madhabDisplayIndex.storeRelaxed(
        UiEngine::indexOfValue(this->m_madhabDisplayNames, pEngine->getMadhab()));

    emit this->configChanged();
}

void UiEngine::onLocaleChanged() noexcept
{
    logDebug(munadi) << "Detected language change, refreshing texts...";
    this->initTexts();
    emit this->translationChanged();
}

void UiEngine::onPrayerEngineOnline() noexcept
{
    logDebug(munadi) << "Executing onPrayerEngineOnline()";
    this->connectToInvalidate();
    this->loadConfigs(); // loadConfigs() needs to be before initConfigs()
    this->initConfigs();
}
} // namespace Munadi