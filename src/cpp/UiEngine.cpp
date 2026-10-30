#include "internal-helpers.hpp"
#include <MultimediaEngine.hpp>
#include <UiEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;
using Sunnah = PrayerEngine::Sunnah;

UiEngine *UiEngine::create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */)
{
    logDebug(munadi) << "Creating instance from QML call";

    qmlEngine->singletonInstance<MultimediaEngine *>("Munadi"_L1, "MultimediaEngine"_L1);

    return new UiEngine{};
}

const QLoggingCategory &UiEngine::logCategory() noexcept
{ return munadi(); }

QString UiEngine::getFormFactor() noexcept
{
#if defined Q_OS_WIN || defined Q_OS_MACOS || (defined Q_OS_LINUX && !defined Q_OS_ANDROID)
    return u"desktop"_s;
#else
    return u"mobile"_s;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Public Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::resetDateOffset() noexcept
{ this->setDateOffset(0); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

UiEngine::UiEngine(QObject *parent) : MunadiBase{munadi(), parent}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    PrayerEngine::onceOnline(this, [this] -> void { this->onPrayerEngineOnline(); });
    MultimediaEngine::onceOnline(this, [this] -> void { this->onMultimediaEngineOnline(); });
    this->makeOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

QVariantList &UiEngine::calcMethodsDisplayNames() noexcept
{ return this->m_calcMethodsDisplayNames; }

QString UiEngine::calendarDateDisplay() noexcept
{ return this->m_calendarDateDisplay.value(this->calendarKey); }

QVariantList &UiEngine::calendarDisplayNames() noexcept
{ return this->m_calendarDisplayNames; }

void UiEngine::connectToAlarmsReset() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::alarmsReset, this, &UiEngine::onAlarmsReset);
        conn == nullptr)
        logWarning(munadi) << "alarmsReset signal not connected";
    else
        logDebug(munadi) << "alarmsReset signal connected";
}

void UiEngine::connectToAthanPlayingChanged() const noexcept
{
    if (const auto conn = QObject::connect(mEngine, &MultimediaEngine::athanPlayingChanged, this,
                                           &UiEngine::onAthanPlayingChanged);
        conn == nullptr)
        logWarning(munadi) << "athanPlayingChanged signal not connected";
    else
        logDebug(munadi) << "athanPlayingChanged signal connected";
}

void UiEngine::connectToCalcChanged() const noexcept
{
    if (const auto conn =
            QObject::connect(pEngine, &PrayerEngine::calcChanged, this, &UiEngine::onCalcChanged);
        conn == nullptr)
        logWarning(munadi) << "calcChanged signal not connected";
    else
        logDebug(munadi) << "calcChanged signal connected";
}

void UiEngine::connectToDateChanged() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::dateChanged, this, &UiEngine::onDateChanged);
        conn == nullptr)
        logWarning(munadi) << "dateChanged signal not connected";
    else
        logDebug(munadi) << "dateChanged signal connected";
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

void UiEngine::connectToMinuteChanged() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::minuteChanged, this, &UiEngine::onMinuteChanged);
        conn == nullptr)
        logWarning(munadi) << "minuteChanged signal not connected";
    else
        logDebug(munadi) << "minuteChanged signal connected";
}

void UiEngine::defaultConfigs() noexcept
{
    constexpr static double defaultBlur{0.45};    // NOLINT(*-magic-numbers)
    constexpr static quint8 defaultCalendarIdx{0};
    constexpr static qint8  defaultHijriOffset{0};
    constexpr static double defaultOpacity{0.77}; // NOLINT(*-magic-numbers)
    constexpr static quint8 defaultLangIdx{0};
    constexpr static quint8 defaultThemeIdx{0};

    this->m_bgImage              = true;
    this->m_blur                 = defaultBlur;
    this->m_calendarDisplayIndex = defaultCalendarIdx;
    this->m_dateOffset           = 0;
    this->m_hijriOffset          = defaultHijriOffset;
    this->m_isoClock             = false;
    this->m_opacity              = defaultOpacity;
    this->m_langDisplayIndex     = defaultLangIdx;
    this->m_showOnAthan          = false;
    this->m_themeDisplayIndex    = defaultThemeIdx;
    this->calendarKey            = u"Both"_s;
}

QString UiEngine::getCurrentLang() const noexcept
{
    return this->m_langDisplayNames.at(this->m_langDisplayIndex)
        .toHash()
        .value("name"_L1)
        .toString();
}

QString UiEngine::getCurrentTheme() const noexcept
{
    return this->m_themeDisplayNames.at(this->m_themeDisplayIndex)
        .toHash()
        .value("name"_L1)
        .toString();
}

void UiEngine::initConfigs() noexcept
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

int UiEngine::indexOfValue(const QVariantList &list, const QString &value) noexcept
{
    logDebug(munadi) << "Value: " << value;
    const auto it{std::ranges::find_if(list, [&value](const QVariant &item) -> bool {
        return item.toHash().value("name"_L1).toString() == value;
    })};

    return (it == list.end()) ? -1 : static_cast<int>(std::distance(list.begin(), it));
}

void UiEngine::initCalendarDates() noexcept
{
    const QDate   date{QDate::currentDate().addDays(this->m_dateOffset)};
    const QLocale l;
    const QString gregorian{
        QCalendar{}.dateTimeToString(UiEngine::calendarFormat, {}, date, {}, l)};
    const QString hijri{QCalendar{QCalendar::System::IslamicCivil}.dateTimeToString(
        UiEngine::calendarFormat, {}, date.addDays(this->m_hijriOffset), {}, l)};

    this->m_calendarDateDisplay = {
        {u"Both"_s,      gregorian + u" | "_s + hijri},
        {u"Gregorian"_s, gregorian                   },
        {u"Hijri"_s,     hijri                       },
    };
}

void UiEngine::initTexts() noexcept
{
    this->initCalendarDates();

    // IMPORTANT:
    // Do not sort any of these lists, unless you know what you're doing

    this->m_calendarDisplayNames = {
        QVariantHash{{u"name"_s, u"Both"_s},      {u"trText"_s, tr("Both")}     },
        QVariantHash{{u"name"_s, u"Hijri"_s},     {u"trText"_s, tr("Hijri")}    },
        QVariantHash{{u"name"_s, u"Gregorian"_s}, {u"trText"_s, tr("Gregorian")}}
    };

    this->m_calcMethodsDisplayNames = {
        QVariantHash{{u"name"_s, u"MuslimWorldLeague"_s},     {u"trText"_s, tr("Muslim World League")} },
        QVariantHash{{u"name"_s, u"MoonsightingCommittee"_s},
                     {u"trText"_s, tr("Moonsighting Committee")}                                       },
        QVariantHash{{u"name"_s, u"UmmAlQura"_s},             {u"trText"_s, tr("Umm al-Qura")}         },
        QVariantHash{{u"name"_s, u"Dubai"_s},                 {u"trText"_s, tr("Dubai")}               },
        QVariantHash{{u"name"_s, u"Egyptian"_s},              {u"trText"_s, tr("Egyptian")}            },
        QVariantHash{{u"name"_s, u"Karachi"_s},               {u"trText"_s, tr("Karachi")}             },
        QVariantHash{{u"name"_s, u"Kuwait"_s},                {u"trText"_s, tr("Kuwait")}              },
        QVariantHash{{u"name"_s, u"NorthAmerica"_s},
                     {u"trText"_s, tr("Islamic Society of North America")}                             },
        QVariantHash{{u"name"_s, u"Qatar"_s},                 {u"trText"_s, tr("Qatar")}               },
        QVariantHash{{u"name"_s, u"Singapore"_s},             {u"trText"_s, tr("Singapore")}           },
        QVariantHash{{u"name"_s, u"Turkey"_s},                {u"trText"_s, tr("Diyanet, Turkey")}     },
        QVariantHash{{u"name"_s, u"Tehran"_s},                {u"trText"_s, tr("University of Tehran")}},
        QVariantHash{{u"name"_s, u"Other"_s},                 {u"trText"_s, tr("Other")}               }
    };

    this->m_langDisplayNames = {
        QVariantHash{{u"name"_s, u"System"_s},  {u"trText"_s, tr("System (default)")}},
        QVariantHash{{u"name"_s, u"English"_s}, {u"trText"_s, u"English"_s}          },
        QVariantHash{{u"name"_s, u"Arabic"_s},  {u"trText"_s, u"العربية"_s}          },
        QVariantHash{{u"name"_s, u"Bangla"_s},  {u"trText"_s, u"বাংলা"_s}            }
    };

    this->m_madhabDisplayNames = {
        QVariantHash{{u"name"_s, u"Shafi"_s},  {u"trText"_s, tr("Majority")}},
        QVariantHash{{u"name"_s, u"Hanafi"_s}, {u"trText"_s, tr("Hanafi")}  },
    };

    this->m_prayerDisplayNames = {
        QVariantHash{{u"name"_s, u"Fajr"_s},           {u"trText"_s, tr("Fajr")}    },
        QVariantHash{{u"name"_s, u"Sunrise"_s},        {u"trText"_s, tr("Sunrise")} },
        QVariantHash{{u"name"_s, u"Dhuhr"_s},          {u"trText"_s, tr("Dhuhr")}   },
        QVariantHash{{u"name"_s, u"Asr"_s},            {u"trText"_s, tr("Asr")}     },
        QVariantHash{{u"name"_s, u"Maghrib"_s},        {u"trText"_s, tr("Maghrib")} },
        QVariantHash{{u"name"_s, u"Isha"_s},           {u"trText"_s, tr("Isha")}    },
        QVariantHash{{u"name"_s, u"Midnight"_s},       {u"trText"_s, tr("Midnight")}},
        QVariantHash{{u"name"_s, u"LastThirdNight"_s},
                     {u"trText"_s, tr("Last Third of the Night")}                   }
    };

    this->m_themeDisplayNames = {
        QVariantHash{{u"name"_s, u"System"_s}, {u"trText"_s, tr("System (default)")}},
        QVariantHash{{u"name"_s, u"Light"_s},  {u"trText"_s, tr("Light")}           },
        QVariantHash{{u"name"_s, u"Dark"_s},   {u"trText"_s, tr("Dark")}            }
    };
}

QVariantList &UiEngine::langDisplayNames() noexcept
{ return this->m_langDisplayNames; }

void UiEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{u"UiEngineConfigMigrated"_s};

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
            int null{};

            fetchAndSetListIdx(val.toInt(), UiEngine::oldCalcMethodsDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setCalcMethod(name); });
        },
        configMigrated);

    loadConfigKey(
        "Adhan/calendarPreferenceIndex"_L1, "App/Calendar"_L1, migrateKey,
        [this, &configMigrated](const QVariant &val) -> void {
            bool      ok{false};
            const int value{val.toInt(&ok)};

            // A readlock here shouldn't be necessary, m_calendarDisplayNames is ideally read only

            if (ok && configMigrated)
            {
                fetchAndSetListIdx(
                    value, UiEngine::oldCalendarDisplayNames, this->m_calendarDisplayIndex,
                    [this](const QString &name) -> void { this->setCalendarKey(name); });
                emit this->calendarIdxChanged();
            }
            else
            {
                const int index{
                    UiEngine::indexOfValue(this->m_calendarDisplayNames, val.toString())};

                this->setCalendarDisplayIdx(index);
            }
        },
        configMigrated);

    loadConfigKey("Adhan/hijriDateOffset"_L1, "App/HijriOffset"_L1, migrateKey, this,
                  "hijriOffset"_L1, configMigrated);

    loadConfigKey("App/isoClock"_L1, "App/ISOClock"_L1, migrateKey, this, "isoClock"_L1,
                  configMigrated);

    loadConfigKey({}, "App/Language"_L1, {}, [this](const QVariant &value) -> void {
        const int index = UiEngine::indexOfValue(this->m_langDisplayNames, value.toString());

        logDebug(munadi) << "Loaded from config: langIdx: " << index;
        this->setLangDisplayIdx(index);
    });

    loadConfigKey(
        "Adhan/madhab"_L1, {}, migrateKey,
        [](const QVariant &val) -> void {
            int null{};

            fetchAndSetListIdx(val.toInt(), UiEngine::oldMadhabDisplayNames, null,
                               [](const QString &name) -> void { pEngine->setMadhab(name); });
        },
        configMigrated);

    loadConfigKey("App/opacity"_L1, "App/Opacity"_L1, migrateKey, this, "opacity"_L1,
                  configMigrated);

    loadConfigKey("App/showOnAthan"_L1, "App/ShowOnAthan"_L1, migrateKey, this, "showOnAthan"_L1,
                  configMigrated);

    loadConfigKey(
        "App/themeListIndex"_L1, "App/Theme"_L1, migrateKey,
        [this, &configMigrated](const QVariant &val) -> void {
            bool      ok{false};
            const int value{val.toInt(&ok)};

            // A readlock here shouldn't be necessary, m_themeDisplayNames is ideally read only

            if (ok && configMigrated)
                fetchAndSetListIdx(value, UiEngine::oldThemeDisplayNames, this->m_themeDisplayIndex,
                                   [](const QString & /*unused*/) -> void {});
            else
                this->m_themeDisplayIndex =
                    UiEngine::indexOfValue(this->m_themeDisplayNames, val.toString());

            emit this->themeChanged();
        },
        configMigrated);

    if (configMigrated)
        core->writeSettings("UiEngineConfigMigrated"_L1, true);
}

QVariantList &UiEngine::madhabDisplayNames() noexcept
{ return this->m_madhabDisplayNames; }

QVariantList &UiEngine::prayerDisplayNames() noexcept
{ return this->m_prayerDisplayNames; }

QVariantHash UiEngine::prayerDisplayTimes() const noexcept
{
    const QLocale l;
    const QDate   date{QDate::currentDate().addDays(this->m_dateOffset)};
    const QString format{(this->m_isoClock) ? u"hh:mm"_s : u"hh:mm ap"_s};

    return {
        {u"Fajr"_s,           l.toString(pEngine->timeForPrayer(Prayer::Fajr,         date), format)},
        {u"Sunrise"_s,        l.toString(pEngine->timeForPrayer(Prayer::Sunrise,      date), format)},
        {u"Dhuhr"_s,          l.toString(pEngine->timeForPrayer(Prayer::Dhuhr,        date), format)},
        {u"Asr"_s,            l.toString(pEngine->timeForPrayer(Prayer::Asr,          date), format)},
        {u"Maghrib"_s,        l.toString(pEngine->timeForPrayer(Prayer::Maghrib,      date), format)},
        {u"Isha"_s,           l.toString(pEngine->timeForPrayer(Prayer::Isha,         date), format)},
        {u"Midnight"_s,       l.toString(pEngine->timeForSunnahTime(Sunnah::Midnight, date), format)},
        {u"LastThirdNight"_s,
         l.toString(pEngine->timeForSunnahTime(Sunnah::LastThirdNight,                date), format)}
    };
}

void UiEngine::saveConfigs() noexcept
{
    // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
    // (later) settings are not saved

    logDebug(munadi) << "Saving UI configurations";

    core->writeSettings("App/BackgroundImage"_L1, this->m_bgImage);
    core->writeSettings("App/Blur"_L1, this->m_blur);
    core->writeSettings("App/Calendar"_L1, this->calendarKey);
    core->writeSettings("App/HijriOffset", this->m_hijriOffset);
    core->writeSettings("App/ISOClock"_L1, this->m_isoClock);
    core->writeSettings("App/Language"_L1, this->getCurrentLang());
    core->writeSettings("App/Opacity"_L1, this->m_opacity);
    core->writeSettings("App/ShowOnAthan"_L1, this->m_showOnAthan);
    core->writeSettings("App/Theme"_L1, this->getCurrentTheme(), true);
}

void UiEngine::setCalcMethodDisplayIdx(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_calcMethodsDisplayNames,
                       this->m_calcMethodsDisplayIndex,
                       [](const QString &name) -> void { pEngine->setCalcMethod(name); });
}

void UiEngine::setCalendarDisplayIdx(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_calendarDisplayNames, this->m_calendarDisplayIndex,
                       [this](const QString &name) -> void { this->setCalendarKey(name); });
    emit this->calendarIdxChanged();
}

void UiEngine::setCalendarKey(const QString &name) noexcept
{
    if (setConfig(this->m_calendarDateDisplay, name, this->calendarKey))
        emit this->calendarChanged();
}

void UiEngine::setDateOffset(int offset) noexcept
{
    logDebug(munadi) << "Received offset: " << offset;

    this->m_dateOffset = offset;

    this->initCalendarDates();
    emit this->displayTimesChanged();
    emit this->calendarChanged();
}

void UiEngine::setHijriOffset(double offset) noexcept
{
    logDebug(munadi) << "Received offset: " << offset;

    const int old{this->m_hijriOffset};

    this->m_hijriOffset = static_cast<int>(offset);

    logDebug(munadi) << "Replaced " << old << " with " << this->m_hijriOffset;
    this->initCalendarDates();
    emit this->hijriOffsetChanged();
    emit this->calendarChanged();
}

void UiEngine::setLangDisplayIdx(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;
    fetchAndSetListIdx(index.toInt(), this->m_langDisplayNames, this->m_langDisplayIndex,
                       [](const QString &name) -> void { core->setLang(name); });
    emit this->langIdxChanged();
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
{ return this->m_themeDisplayNames; }

QString UiEngine::timeLeftDisplay() noexcept
{
    const QString currentPrayer{
        this->m_prayerDisplayNames.at(qToUnderlying(pEngine->currentPrayer()))
            .toHash()
            .value("trText"_L1)
            .toString()};

    if (mEngine->isAthanPlaying())
        return currentPrayer + u" "_s + tr("athan") + u"..."_s;

    const QTime timeLeft{QTime::fromMSecsSinceStartOfDay(pEngine->timeToNextPrayer())};

    if (!timeLeft.isValid())
        return {};

    const int index{qToUnderlying(pEngine->nextPrayer())};

    if (index > qToUnderlying(Prayer::Isha))
        return {};

    const QString format{(timeLeft.hour()) > 0 ? tr("h'h' m'm'") : tr("m'm'")};
    const QString nextPrayer{
        this->m_prayerDisplayNames.at(index).toHash().value("trText"_L1).toString()};

    if (timeLeft.msecsTo(QTime{0, 1, 0}) > 0)
        return tr("Seconds to") + u" "_s + nextPrayer;

    return QLocale{}.toString(timeLeft, format) + u" "_s + tr("to") + u" "_s + nextPrayer;
}

double UiEngine::timeLeftProgress() noexcept
{
    const double ratio{static_cast<double>(pEngine->timeToNextPrayer()) /
                       pEngine->timeForCurrentPrayer().msecsTo(pEngine->timeForNextPrayer())};

    if (ratio < 0.0)
        return 0.0;

    if (ratio > 1.0)
        return 1.0;

    return ratio;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::onAlarmsReset() noexcept
{
    this->initCalendarDates();
    emit this->calendarChanged();
}

void UiEngine::onAthanPlayingChanged(bool /* playing */) noexcept
{
    emit this->calendarChanged();
    emit this->timeChanged();
}

void UiEngine::onCalcChanged() noexcept
{
    this->m_calcMethodsDisplayIndex =
        UiEngine::indexOfValue(this->m_calcMethodsDisplayNames, pEngine->getCalcMethod());
    this->m_madhabDisplayIndex =
        UiEngine::indexOfValue(this->m_madhabDisplayNames, pEngine->getMadhab());

    emit this->prayerCalcChanged();
    emit this->displayTimesChanged();
    emit this->timeChanged();
}

void UiEngine::onCoreOnline() noexcept
{
    this->connectToLocaleChanged();
    this->initTexts();
    this->connectToAlarmsReset();
    this->connectToDateChanged();
    this->connectToMinuteChanged();
    this->connectToCoreOffline();
}

void UiEngine::onDateChanged() noexcept
{
    this->initCalendarDates();
    emit this->calendarChanged();
    emit this->timeChanged();
}

void UiEngine::onLocaleChanged() noexcept
{
    logDebug(munadi) << "Detected language change, refreshing texts...";
    this->initTexts();
    emit this->calendarChanged();
    emit this->displayTimesChanged();
    emit this->translationChanged();
    emit this->timeChanged();
}

void UiEngine::onMinuteChanged() noexcept
{
    logDebug(munadi) << "60s_tick event received";
    emit this->timeChanged();
}

void UiEngine::onMultimediaEngineOnline() noexcept
{ this->connectToAthanPlayingChanged(); }

void UiEngine::onPrayerEngineOnline() noexcept
{
    logDebug(munadi) << "Executing onPrayerEngineOnline()";
    this->connectToCalcChanged();
    this->loadConfigs(); // loadConfigs() needs to be before initConfigs()
    this->initConfigs();
}
} // namespace Munadi