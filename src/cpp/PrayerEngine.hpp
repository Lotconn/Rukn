#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>

#include <adhan/CalculationMethod.hpp>
// #include <adhan/Coordinates.hpp>
// #include <adhan/DateUtils.hpp>
#include <adhan/PrayerTimes.hpp>

namespace Munadi
{

class PrayerEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantHash prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig RESET
                   defaultPrayerConfig NOTIFY prayerConfigChanged)
    // Q_PROPERTY(PrayerEngine::Prayer currentPrayer READ currentPrayer NOTIFY timeChanged)

  public:
    explicit PrayerEngine(QObject *parent = nullptr);
    ~PrayerEngine() override;

    static auto               instanceExists() noexcept -> bool;
    static auto               instance() noexcept -> PrayerEngine *;
    [[nodiscard]] static auto isOnline() noexcept -> bool;

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

    static auto fromString(QAnyStringView prayerName) -> Prayer;
    static auto metaPrayerEnum() -> const QMetaEnum &;
    static auto onceOnline(QObject *context, std::function<void()> slot,
                           int retriesLeft = 19) -> void; // NOLINT(*-magic-numbers)

    auto currentPrayer() -> Prayer;
    auto getCalcMethod() -> QString;
    auto getMadhab() -> QString;
    auto getPrayerConfig(Prayer prayerEnum, QAnyStringView field) -> QVariant;
    auto setCalcMethod(const QString &method) -> void;
    auto setMadhab(const QString &madhab) -> void;

    Q_INVOKABLE void           modifyPrayerConfig(const QString &key, const QVariantHash &entry);
    Q_INVOKABLE static QString toString(Prayer prayerEnum);

    // void timeChanged();

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static QPointer<PrayerEngine> s_singletonInstance;
    inline static bool                   s_online{false};

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
    qsizetype           prayerConfigEntrySize;
    qsizetype           prayerConfigMapSize;
    QReadWriteLock      varLock{QReadWriteLock::Recursive};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // auto currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    auto connectToCoreOffline() const -> void;
    auto defaultCalcParams() -> void;
    auto defaultPrayerConfig() -> void;
    auto loadConfigs() -> void;
    auto loadPrayerConfig(bool &configMigrated) -> void;
    auto saveConfigs() -> void;
    auto setPrayerConfig(const QVariantHash &prayerConfig) -> void;

  signals:
    auto engineOnline() -> void;
    auto invalidate() -> void;
    auto prayerConfigChanged(QVariantHash prayerConfig) -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
    auto onInvalidate() -> void;
};

} // namespace Munadi