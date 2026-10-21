#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>
#include <QSet>
#include <adhan/CalculationMethod.hpp>
// #include <adhan/Coordinates.hpp>
// #include <adhan/DateUtils.hpp>
#include <adhan/PrayerTimes.hpp>

#include <MunadiEngine.hpp>

#define pEngine Munadi::PrayerEngine::instance()

namespace Munadi
{

class PrayerEngine : public MunadiEngine<PrayerEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantHash prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)
    // Q_PROPERTY(PrayerEngine::Prayer currentPrayer READ currentPrayer NOTIFY timeChanged)

  public:
    explicit PrayerEngine(QObject *parent = nullptr);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Types & Enums
    ////////////////////////////////////////////////////////////////////////////////////////////////

    enum Prayer : quint8
    {
        Fajr    = static_cast<quint8>(Adhan::Prayer::Fajr),
        Sunrise = static_cast<quint8>(Adhan::Prayer::Sunrise),
        Dhuhr   = static_cast<quint8>(Adhan::Prayer::Dhuhr),
        Asr     = static_cast<quint8>(Adhan::Prayer::Asr),
        Maghrib = static_cast<quint8>(Adhan::Prayer::Maghrib),
        Isha    = static_cast<quint8>(Adhan::Prayer::Isha),
        None    = static_cast<quint8>(Adhan::Prayer::None),
    };
    Q_ENUM(Prayer)

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto fromString(QAnyStringView prayerName) noexcept -> Prayer;
    static auto logCategory() noexcept -> const QLoggingCategory &;
    static auto metaPrayerEnum() noexcept -> const QMetaEnum &;

    auto currentPrayer() noexcept -> Prayer;
    auto getCalcMethod() noexcept -> const QString &;
    auto getMadhab() noexcept -> const QString &;
    auto getPrayerConfig(Prayer prayerEnum, QAnyStringView field) noexcept -> QVariant;
    auto setCalcMethod(const QString &method) noexcept -> void;
    auto setMadhab(const QString &madhab) noexcept -> void;

    Q_INVOKABLE void modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept;
    Q_INVOKABLE static QString toString(Prayer prayerEnum) noexcept;

    // void timeChanged();

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static const QHash<QString, std::function<Adhan::CalculationParameters()>> s_calcMethods{
        {    QLatin1StringView{"MuslimWorldLeague"},
         []() -> Adhan::CalculationParameters {
         return Adhan::CalculationMethod::MuslimWorldLeague();
         }                                                                                        },
        {             QLatin1StringView{"Egyptian"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Egyptian(); }    },
        {              QLatin1StringView{"Karachi"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Karachi(); }     },
        {            QLatin1StringView{"UmmAlQura"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::UmmAlQura(); }   },
        {                QLatin1StringView{"Dubai"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Dubai(); }       },
        {QLatin1StringView{"MoonsightingCommittee"},
         []() -> Adhan::CalculationParameters {
         return Adhan::CalculationMethod::MoonsightingCommittee();
         }                                                                                        },
        {         QLatin1StringView{"NorthAmerica"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::NorthAmerica(); }},
        {               QLatin1StringView{"Kuwait"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Kuwait(); }      },
        {                QLatin1StringView{"Qatar"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Qatar(); }       },
        {            QLatin1StringView{"Singapore"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Singapore(); }   },
        {               QLatin1StringView{"Tehran"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Tehran(); }      },
        {               QLatin1StringView{"Turkey"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Turkey(); }      },
        {                QLatin1StringView{"Other"},
         []() -> Adhan::CalculationParameters { return Adhan::CalculationMethod::Other(); }       }
    };

    inline static const QHash<QString, Adhan::Madhab> s_madhab{
        { QLatin1StringView("Shafi"),  Adhan::Madhab::Shafi},
        {QLatin1StringView("Hanafi"), Adhan::Madhab::Hanafi},
    };

    inline static const QHash<Prayer, Adhan::Prayer> s_prayerEnumMap{
        {   PrayerEngine::Fajr,    Adhan::Prayer::Fajr},
        {PrayerEngine::Sunrise, Adhan::Prayer::Sunrise},
        {  PrayerEngine::Dhuhr,   Adhan::Prayer::Dhuhr},
        {    PrayerEngine::Asr,     Adhan::Prayer::Asr},
        {PrayerEngine::Maghrib, Adhan::Prayer::Maghrib},
        {   PrayerEngine::Isha,    Adhan::Prayer::Isha},
        {   PrayerEngine::None,    Adhan::Prayer::None}
    };

    inline static const QHash<Adhan::Prayer, Prayer> s_revPrayerEnumMap{
        {   Adhan::Prayer::Fajr,    PrayerEngine::Fajr},
        {Adhan::Prayer::Sunrise, PrayerEngine::Sunrise},
        {  Adhan::Prayer::Dhuhr,   PrayerEngine::Dhuhr},
        {    Adhan::Prayer::Asr,     PrayerEngine::Asr},
        {Adhan::Prayer::Maghrib, PrayerEngine::Maghrib},
        {   Adhan::Prayer::Isha,    PrayerEngine::Isha},
        {   Adhan::Prayer::None,    PrayerEngine::None}
    };

    // std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;
    inline static const QMetaEnum s_metaPrayerEnum{QMetaEnum::fromType<Prayer>()};

    QString                                       m_calcMethodKey;
    std::unique_ptr<Adhan::CalculationParameters> m_calcParams;
    QString                                       m_madhabKey;
    QVariantHash                                  m_prayerConfig;

    QHash<QString, int> prayerConfigFields;
    QSet<QString>       prayerConfigKeys;
    qsizetype           prayerConfigEntrySize{};
    qsizetype           prayerConfigMapSize{};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // auto currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    auto               connectToInvalidate() const noexcept -> void;
    auto               defaultCalcParams() noexcept -> void;
    auto               defaultConfigs() noexcept -> void override;
    auto               defaultPrayerConfig() noexcept -> void;
    [[nodiscard]] auto getPrayerConfig_p(Prayer prayerEnum, QAnyStringView field) const noexcept
        -> QVariant;
    auto loadConfigs() noexcept -> void override;
    auto loadPrayerConfig(bool &configMigrated) noexcept -> void;
    auto saveConfigs() noexcept -> void override;
    auto setPrayerConfig(const QVariantHash &prayerConfig) noexcept -> void;

  signals:
    auto invalidate() -> void;
    auto prayerConfigChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onInvalidate() noexcept -> void;
    auto onLocationEngineOnline() noexcept -> void;
};

} // namespace Munadi