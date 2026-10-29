#pragma once

#include <QGeoCoordinate>
#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>
#include <QSet>
#include <adhan/CalculationMethod.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/SunnahTimes.hpp>

#include <MunadiEngine.hpp>

#define pEngine Munadi::PrayerEngine::instance()

namespace Munadi
{

class PrayerEngine : public MunadiBase, public MunadiEngine<PrayerEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(Prayer currentPrayer READ currentPrayer NOTIFY calcChanged)
    Q_PROPERTY(Prayer nextPrayer READ nextPrayer NOTIFY calcChanged)
    Q_PROPERTY(QVariantHash prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)
    Q_PROPERTY(double qibla READ getQiblaDegrees NOTIFY calcChanged)

  public:
    static auto create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> PrayerEngine *;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Types & Enums
    ////////////////////////////////////////////////////////////////////////////////////////////////

    enum class Prayer : quint8
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

    enum class Sunnah : quint8
    {
        None           = static_cast<quint8>(Adhan::Sunnah::None),
        Midnight       = static_cast<quint8>(Adhan::Sunnah::Midnight),
        LastThirdNight = static_cast<quint8>(Adhan::Sunnah::LastThirdNight),
    };
    Q_ENUM(Sunnah)

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto fromString(const QString &prayerName) noexcept -> Prayer;
    static auto fromString_sunnahTime(const QString &sunnahName) noexcept -> Sunnah;
    static auto logCategory() noexcept -> const QLoggingCategory &;
    static auto metaPrayerEnum() noexcept -> const QMetaEnum &;

    auto currentPrayer() noexcept -> Prayer;
    auto getCalcMethod() noexcept -> const QString &;
    auto getMadhab() noexcept -> const QString &;
    auto getPrayerConfig(Prayer prayerEnum, const QString &field) noexcept -> QVariant;
    auto getQiblaDegrees(const std::optional<QGeoCoordinate> &coordinates = std::nullopt) -> double;
    auto nextPrayer() noexcept -> Prayer;
    auto setCalcMethod(const QString &method) noexcept -> void;
    auto setMadhab(const QString &madhab) noexcept -> void;
    auto timeBetweenPrayers(Prayer current, Prayer next,
                            std::optional<QDate> current_date = std::nullopt,
                            std::optional<QDate> next_date    = std::nullopt) noexcept -> qint64;
    auto timeBetweenPrayers(const QString &currentPrayer, const QString &nextPrayer,
                            std::optional<QDate> current_date = std::nullopt,
                            std::optional<QDate> next_date    = std::nullopt) noexcept -> qint64;
    auto timeForPrayer(Prayer prayerEnum, std::optional<QDate> date = std::nullopt) noexcept
        -> QDateTime;
    auto timeForPrayer(const QString &prayerName, std::optional<QDate> date = std::nullopt) noexcept
        -> QDateTime;
    auto timeForSunnahTime(const QString       &sunnahName,
                           std::optional<QDate> date = std::nullopt) noexcept -> QDateTime;
    auto timeToNextPrayer(const std::optional<QDateTime> &since_ms = std::nullopt) noexcept
        -> qint64;

    Q_INVOKABLE void modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept;
    Q_INVOKABLE QDateTime
    timeForCurrentPrayer(const std::optional<QDateTime> &when = std::nullopt) noexcept;
    Q_INVOKABLE QDateTime
    timeForNextPrayer(const std::optional<QDateTime> &when = std::nullopt) noexcept;
    // NOLINTBEGIN(clazy-fully-qualified*-types)
    Q_INVOKABLE QDateTime      timeForSunnahTime(Sunnah               sunnahEnum,
                                                 std::optional<QDate> date = std::nullopt) noexcept;
    Q_INVOKABLE static QString toString(Prayer prayerEnum) noexcept;
    Q_INVOKABLE static QString toString(Sunnah sunnahEnum) noexcept;
    // NOLINTEND(clazy-fully-qualified*-types)

  protected:
    explicit PrayerEngine(QObject *parent = nullptr);

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static const QHash<QString, std::function<Adhan::CalculationParameters()>> s_calcMethods{
        {u"Dubai"_s,                 Adhan::CalculationMethod::Dubai                },
        {u"Egyptian"_s,              Adhan::CalculationMethod::Egyptian             },
        {u"Karachi"_s,               Adhan::CalculationMethod::Karachi              },
        {u"Kuwait"_s,                Adhan::CalculationMethod::Kuwait               },
        {u"MoonsightingCommittee"_s, Adhan::CalculationMethod::MoonsightingCommittee},
        {u"MuslimWorldLeague"_s,     Adhan::CalculationMethod::MuslimWorldLeague    },
        {u"NorthAmerica"_s,          Adhan::CalculationMethod::NorthAmerica         },
        {u"Other"_s,                 Adhan::CalculationMethod::Other                },
        {u"Qatar"_s,                 Adhan::CalculationMethod::Qatar                },
        {u"Singapore"_s,             Adhan::CalculationMethod::Singapore            },
        {u"Tehran"_s,                Adhan::CalculationMethod::Tehran               },
        {u"Turkey"_s,                Adhan::CalculationMethod::Turkey               },
        {u"UmmAlQura"_s,             Adhan::CalculationMethod::UmmAlQura            },
    };

    inline static const QHash<QString, Adhan::Madhab> s_madhab{
        {u"Hanafi"_s, Adhan::Madhab::Hanafi},
        {u"Shafi"_s,  Adhan::Madhab::Shafi },
    };

    inline static const QHash<Prayer, Adhan::Prayer> s_prayerEnumMap{
        {Prayer::Fajr,    Adhan::Prayer::Fajr   },
        {Prayer::Sunrise, Adhan::Prayer::Sunrise},
        {Prayer::Dhuhr,   Adhan::Prayer::Dhuhr  },
        {Prayer::Asr,     Adhan::Prayer::Asr    },
        {Prayer::Maghrib, Adhan::Prayer::Maghrib},
        {Prayer::Isha,    Adhan::Prayer::Isha   },
        {Prayer::None,    Adhan::Prayer::None   }
    };

    inline static const QHash<Adhan::Prayer, Prayer> s_revPrayerEnumMap{
        {Adhan::Prayer::Fajr,    Prayer::Fajr   },
        {Adhan::Prayer::Sunrise, Prayer::Sunrise},
        {Adhan::Prayer::Dhuhr,   Prayer::Dhuhr  },
        {Adhan::Prayer::Asr,     Prayer::Asr    },
        {Adhan::Prayer::Maghrib, Prayer::Maghrib},
        {Adhan::Prayer::Isha,    Prayer::Isha   },
        {Adhan::Prayer::None,    Prayer::None   }
    };

    inline static const QHash<Sunnah, Adhan::Sunnah> s_sunnahEnumMap{
        {Sunnah::None,           Adhan::Sunnah::None          },
        {Sunnah::Midnight,       Adhan::Sunnah::Midnight      },
        {Sunnah::LastThirdNight, Adhan::Sunnah::LastThirdNight}
    };

    inline static const QHash<Adhan::Sunnah, Sunnah> s_revSunnahEnumMap{
        {Adhan::Sunnah::None,           Sunnah::None          },
        {Adhan::Sunnah::Midnight,       Sunnah::Midnight      },
        {Adhan::Sunnah::LastThirdNight, Sunnah::LastThirdNight}
    };

    inline static const QMetaEnum s_metaPrayerEnum{QMetaEnum::fromType<Prayer>()};
    inline static const QMetaEnum s_metaSunnahEnum{QMetaEnum::fromType<Sunnah>()};

    QString                             m_calcMethodKey;
    QString                             m_madhabKey;
    QVariantHash                        m_prayerConfig;
    std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;
    std::atomic<double>                 m_qiblaDegrees;
    std::unique_ptr<Adhan::SunnahTimes> m_sunnahTimes;

    std::unique_ptr<Adhan::CalculationParameters> calcParams;
    std::unique_ptr<Adhan::Coordinates>           coordinates;
    QHash<QString, int>                           prayerConfigFields;
    QSet<QString>                                 prayerConfigKeys;
    qsizetype                                     prayerConfigEntrySize{};
    qsizetype                                     prayerConfigMapSize{};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto               connectToAthanAlarm() noexcept -> void;
    auto               connectToAlarmsReset() noexcept -> void;
    auto               connectToDateChanged() noexcept -> void;
    auto               connectToInvalidate() const noexcept -> void;
    auto               connectToLocationChanged() noexcept -> void;
    auto               connectToMinuteChanged() const noexcept -> void;
    auto               defaultConfigs() -> void override;
    auto               defaultPrayerConfig() noexcept -> void;
    [[nodiscard]] auto getPrayerConfig_p(Prayer prayerEnum, const QString &field) const noexcept
        -> QVariant;
    auto loadConfigs() noexcept -> void override;
    auto loadPrayerConfig(bool &configMigrated) noexcept -> void;
    auto saveConfigs() noexcept -> void override;
    auto setPrayerConfig(const QVariantHash &prayerConfig) noexcept -> void;
    auto updateCalc() -> void;

  signals:
    auto calcChanged() -> void;
    auto invalidate() -> void;
    auto prayerConfigChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    void onCoreOnline() noexcept override;
    void onInvalidate();
    void onMinuteChanged() noexcept;
    void onLocationEngineOnline() noexcept;
};

} // namespace Munadi