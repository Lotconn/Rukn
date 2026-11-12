#pragma once

#include <QGeoCoordinate>
#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>
#include <QSet>
#include <adhan/CalculationMethod.hpp>
#include <adhan/PrayerTimes.hpp>
#include <adhan/SunnahTimes.hpp>

#include <AppEngine.hpp>

#define pEngine App::PrayerEngine::instance()

namespace App
{

class PrayerEngine : public AppBase, public AppEngine<PrayerEngine>
{
    Q_OBJECT

    // Enums must be access scoped in QML
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    // Construct a QML module from this class
    QML_ELEMENT

    // The QML module is a singleton
    QML_SINGLETON

    // The current prayer expressed as PrayerEngine::Prayer enum. Read-only. See `currentPrayer()`
    // and `calcChanged()` briefs for details.
    Q_PROPERTY(Prayer currentPrayer READ currentPrayer NOTIFY calcChanged)

    // The next prayer expressed as PrayerEngine::Prayer enum. Read-only. See `nextPrayer()` and
    // `calcChanged()` briefs for details.
    Q_PROPERTY(Prayer nextPrayer READ nextPrayer NOTIFY calcChanged)

    // Provides user-adjustable calculation configurations for each prayer times. Individual
    // adjustments are mostly modified through `modifyPrayerConfig()`, and is the recommended
    // approach too. This property however allows writing the whole configuration at once. See
    // `m_prayerConfig()`, `setPrayerConfig()` and `prayerConfigChanged()` briefings for details.
    Q_PROPERTY(QVariantHash prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)

    // Provides the qibla bearing from North (0°) in degrees. Read-only. See `getQiblaDegrees()`
    // briefs for details.
    Q_PROPERTY(double qibla READ getQiblaDegrees NOTIFY calcChanged)

  public:
    /**
     * @brief The QML factory, creating the singleton as a QML module, called by QmlEngine itself.
     * The created instance therefore is parented by the QmlEngine which can be accessed through
     * `qmlEngine`. This engine class here can request the QML engine to instantiate any other
     * dependent engine classes it requires through a call to
     * `qmlEngine->singletonInstance<OtherEngine *>()` which too would be parented by the QmlEngine,
     * avoiding loads of recurred problems, one among which is nested parenting if the dependent
     * engines would be instantiated right in this engine's constructor. However, QmlEngine prefers
     * the default-constructible `public` constructor over this if its available, so this never gets
     * called if it's present. The class constructor therefore needs to be hidden from `public` -
     * preferrably in `protected` segment for QmlEngine to call this factory. For reducing redundant
     * codes and actually using the class constructor preserving all the expected behaviors as
     * usual, this factory then needs to act as a wrapper around the "hidden" constructor which
     * artfully accomplishes both goals. Returns an instance of the engine class - in here, by
     * calling the `protected` constructor.
     *
     * @param qmlEngine: The QmlEngine instance, passed by the QmlEngine during its call
     * @param jsEngine: Present only for historical purposes, everything is done through the
     * QmlEngine instance these days. Not used here.
     * @return LocationEngine*: The class instance
     */
    static PrayerEngine *create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */);
    ~PrayerEngine() override = default;

    /**
     * @brief Accessor needed by AppEngine<T> for logging purposes
     *
     * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
     */
    static const QLoggingCategory &logCategory() noexcept;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Types & Enums
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief A proxy in-class enum for `Adhan::Prayer`. Maps values 1:1 with `Adhan::Prayer`.
     * Classified to avoid clashes with other enums having same entries.
     */
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

    /**
     * @brief A proxy in-class enum for `Adhan::Sunnah`. Maps values 1:1 with `Adhan::Sunnah`.
     * Classified to avoid clashes with other enums having same entries.
     */
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

    /**
     * @brief The current prayer expressed as `PrayerEngine::Prayer` enum. Mimicks
     * `Adhan::PrayerUtils::currentPrayer()` but additionally handles inapplicable prayer times
     * (i.e. `None`) after date boundary, before Fajr. The method is inherently read-only.
     *
     * @return Prayer: The current prayer time expressed as `Prayer` enum
     */
    [[nodiscard]] Prayer currentPrayer() const noexcept;

    /**
     * @brief Translate a (known) prayer string name to actual enum. Mimicks
     * `Adhan::PrayerUtils::from_string()`. Returns `Prayer::None` on invalid or unrecognized string
     * names. String names for the enums are the element names themselves, i.e. "Fajr" =>
     * Prayer::Fajr, "Isha" => Prayer::Isha
     *
     * @param prayerName: the (known) string name of the prayer
     * @return Prayer: Corresponding `Prayer` enum for the string name, `None` if `prayerName` is
     * invalid or unrecognized
     */
    static Prayer fromString(const QString &prayerName) noexcept;

    /**
     * @brief Translate a (known) sunnah times string name to actual enum. Mimicks
     * `Adhan::SunnahUtils::from_string()`. Returns `Sunnah::None` on invalid or unrecognized string
     * names. String names for the enums are the element names themselves, i.e. "Midnight" =>
     * Sunnah::Midnight, "LastThirdNight" => Sunnah::LastThirdNight
     *
     * @param sunnahName: the (known) string name of the sunnah time
     * @return Sunnah: Corresponding `Sunnah` enum for the string name, `None` if `sunnahName` is
     * invalid or unrecognized
     */
    static Sunnah fromString_sunnahTime(const QString &sunnahName) noexcept;

    /**
     * @brief Get the string name of currently set calculation method for prayer-times calculation
     * in `m_calcMethodsKey`. The name is one among the string-keys in `s_calcMethods` table. This
     * method is read-only.
     *
     * @return QString: String-name of currently set calculation method
     */
    [[nodiscard]] QString getCalcMethod() const noexcept;

    /**
     * @brief Get the string name of currently set madhhab for prayer-times calculation in
     * `m_maddhabKey`. The name is one among the string-keys in `s_madhhab` table. This method is
     * read-only.
     *
     * @return QString: String-name of currently set maddhab
     */
    [[nodiscard]] QString getMadhhab() const noexcept;

    /**
     * @brief Fetch the current config value at the key `field` for the prayer `prayerEnum`. This
     * looks up the key `field` in the `m_prayerConfig` hash-table. Read-only.
     *
     * @param prayerEnum: The prayer to look up the config of
     * @param field: The desired field/key of the config in `prayerEnum`
     * @return QVariant: Value of the looked-up key, empty if `field` is invalid or empty.
     */
    [[nodiscard]] QVariant getPrayerConfig(Prayer prayerEnum, const QString &field) const noexcept;

    /**
     * @brief Get the qibla degrees from North (0°) for the currently set coordinates in the
     * Adhan::Coordinate member `coordinates`, or using the optionally passed coordinates as a
     * QGeoCoordinate object. Read-only.
     *
     * @param coordinates: (optional) Coordinates to get the qibla direction for
     * @return double: Bearing in degrees from North (0°)
     */
    [[nodiscard]] double getQiblaDegrees(
        const std::optional<QGeoCoordinate> &coordinates = std::nullopt) const;

    /**
     * @brief Read-only accessor to the QMetaEnum member `s_metaPrayerEnum` for other engines to
     * use. See `s_metaPrayerEnum` briefs for details.
     *
     * @return QMetaEnum: The reference to the `Prayer` enum metadata object, `s_metaPrayerEnum`
     */
    static QMetaEnum metaPrayerEnum() noexcept;

    /**
     * @brief Updates the configuration entry of prayer designated by `key` with `entry` in
     * `m_prayerConfigs`, where `entry` itself is a map for the (new) configuration. This method
     * ensures `entry` only modifies the configuration of the corresponding prayer as designated by
     * `key` and doesn't mutate the structure or size of the table in any way. Utilizes
     * `prayerConfigFields` and `prayerConfigEntrySize` members to guard unwanted modifications.
     *
     * @param key: The prayer string-name for which the configuration needs to be updated
     * @param entry: The configuration entry to update with. All keys of `entry` must remain intact
     * as original, only values can change. The whole entry would be discarded othewise.
     */
    Q_INVOKABLE void modifyPrayerConfig(const QString &key, const QVariantHash &entry) noexcept;

    /**
     * @brief The upcoming prayer expressed as `PrayerEngine::Prayer` enum. Mimicks
     * `Adhan::PrayerUtils::nextPrayer()` but additionally handles inapplicable prayer times
     * (i.e. `None`) before date boundary, during Isha. This method is inherently read-only.
     *
     * @return Prayer: The upcoming prayer time expressed as `Prayer` enum
     */
    [[nodiscard]] Prayer nextPrayer() const noexcept;

    /**
     * @brief Sets the calculation method to be used for calculating prayer-times. Write-only. Valid
     * names are one of `s_calcMethods` string-keys. Saves the set key in `m_calcMethodKey`.
     *
     * @param method: String-name of the calculation method.
     */
    void setCalcMethod(const QString &method) noexcept;

    /**
     * @brief Sets the madhhab to be used for calculating prayer-times. Write-only. Valid names are
     * one of `s_madhhab` string-keys. Saves the set key in `m_madhhabKey`.
     *
     * @param method: String-name of the madhhab.
     */
    void setMadhhab(const QString &madhhab) noexcept;

    /**
     * @brief Calculate the time since current prayer until next prayer in milliseconds. Utilizes
     * `timeForPrayer()`. The span between them can be optionally adjusted by providing dates for
     * each, otherwise, sets the dates to current local date when left out. When `current` or `next`
     * is `None`, following the behavior of `timeForPrayer()`, returns the span from or to
     * start-of-day time. See the briefing in `timeForPrayer()` for details.
     *
     * @param current: The prayer to start counting time from, expressed in `Prayer` enum
     * @param next: The prayer to count time until, expressed in `Prayer` enum
     * @param current_date: (optional) The date when `current` is at. Sets the effective time to
     * start counting from. Current local date by default.
     * @param next_date: (optional) The date when `next` is at. Sets the effective time to count
     * until. Current local date by default.
     * @return qint64: Time between effective `current` and effective `next` in milliseconds.
     * Negative if effective `next` is earlier than effective `current`. -1 if time for either of
     * `current` or `next` couldn't be calculated.
     */
    [[nodiscard]] qint64 timeBetweenPrayers(
        Prayer current, Prayer next, std::optional<QDate> current_date = std::nullopt,
        std::optional<QDate> next_date = std::nullopt) const noexcept;

    /**
     * @brief Overload of `timeBetweenPrayer()` taking string-names for prayers instead of enums.
     *
     * @param current: The prayer to start counting time from, expressed in string-names
     * @param next: The prayer to count time until, expressed in string-names
     * @param current_date: (optional) The date when `current` is at. Sets the effective time to
     * start counting from. Current local date by default.
     * @param next_date: (optional) The date when `next` is at. Sets the effective time to count
     * until. Current local date by default.
     * @return qint64: Time between effective `current` and effective `next` in milliseconds.
     * Negative if effective `next` is earlier than effective `current`. -1 if time for either of
     * `current` or `next` couldn't be calculated.
     */
    [[nodiscard]] qint64 timeBetweenPrayers(
        const QString &currentPrayer, const QString &nextPrayer,
        std::optional<QDate> current_date = std::nullopt,
        std::optional<QDate> next_date    = std::nullopt) const noexcept;

    /**
     * @brief A convenient utility that provides the time for the current prayer in local clock.
     * If the optional parameter `when` is omitted, defaults to showing current prayer time for
     * current date. When a value to `when` is passed, it provides the time for the current prayer
     * of that instance's date, not the current prayer of current date. When current prayer becomes
     * `None` (after date boundary 23:59:59 before Fajr), the calculation is handled to show time to
     * previous day's Isha instead. Utilizes `timeForPrayer()`. See also its brief for further
     * details.
     *
     * @param when: (optional) QDateTime instance to use for showing current prayer time of
     * @return QDateTime: Time for current prayer in local clock. Invalid construct on error.
     */
    Q_INVOKABLE [[nodiscard]] QDateTime timeForCurrentPrayer(
        const std::optional<QDateTime> &when = std::nullopt) const noexcept;

    /**
     * @brief Get the time for the provided prayer. Mimicks `Adhan::PrayerUtils::timeForPrayer()`
     * but also handles false-positive cases like after Isha and before Fajr around date-boundaries.
     * Returns the time in a QDateTime object in local clock or invalid construct if calculation for
     * the said prayer failed for some reason. Uses `m_prayerTimes`, `coordinates` and `calcParams`
     * members internally. When `Prayer::None` is passed explicitly, returns the midnight time of
     * the same local date (i.e. 00:00:00).
     *
     * @param prayerEnum: The prayer to get the time of, expressed in `Prayer` enum
     * @param date: (optional) The date when `prayerEnum` is at. Sets the effective date to
     * calculate the prayer-time of. Current local date by default.
     * @return QDateTime: Time of the prayer `prayerEnum` expressed in a QDateTime object in local
     * clock. Invalid QDateTime construct if time could not be calculated for `prayerEnum`.
     * Start-of-the-day time of `date` if `prayerEnum` is `None`.
     */
    [[nodiscard]] QDateTime timeForPrayer(Prayer               prayerEnum,
                                          std::optional<QDate> date = std::nullopt) const noexcept;

    /**
     * @brief Overload of `timeForPrayer()` taking string-names for prayer instead of enum.
     *
     * @param prayerName: The prayer to get the time of, expressed in string-names
     * @param date: (optional) The date when `prayerName` is at. Sets the effective date to
     * calculate the prayer-time of. Current local date by default.
     * @return QDateTime: Time of the prayer `prayerName` expressed in a QDateTime object in local
     * clock. Invalid QDateTime construct if time could not be calculated for `prayerName`.
     * Start-of-the-day time of `date` if `prayerName` is `None`.
     */
    [[nodiscard]] QDateTime timeForPrayer(const QString       &prayerName,
                                          std::optional<QDate> date = std::nullopt) const noexcept;

    /**
     * @brief A convenient utility that provides the time for the upcoming prayer in local clock.
     * If the optional parameter `when` is omitted, defaults to showing next prayer time for current
     * date. When a value to `when` is passed, it provides the time for the next prayer of that
     * instance's date, not the next prayer of current date. When next prayer becomes `None` (after
     * Isha before date boundary 23:59:59), the calculation is handled to show time to next day's
     * Fajr instead. Utilizes `timeForPrayer()`. See also its brief for further details.
     *
     * @param when: (optional) QDateTime instance to use for showing next prayer time of
     * @return QDateTime: Time for next prayer in local clock. Invalid construct on error.
     */
    Q_INVOKABLE [[nodiscard]] QDateTime timeForNextPrayer(
        const std::optional<QDateTime> &when = std::nullopt) const noexcept;

    /**
     * @brief Get the time for the provided sunnah time. Mimicks
     * `Adhan::SunnahUtils::timeForSunnahTime()`. Returns the time in a QDateTime object in local
     * clock or invalid construct if calculation for the said prayer failed for some reason. Uses
     * `m_sunnahTimes`, `coordinates` and `calcParams` members internally.
     *
     * @param sunnahEnum: The sunnah time to get the time of, expressed in `Sunnah` enum
     * @param date: (optional) The date when `sunnahEnum` is at. Sets the effective date to
     * calculate the sunnah time of. Current local date by default.
     * @return QDateTime: Time of the prayer `sunnahEnum` expressed in a QDateTime object in local
     * clock. Invalid QDateTime construct if time could not be calculated for `sunnahEnum`.
     */
    Q_INVOKABLE [[nodiscard]] QDateTime timeForSunnahTime(
        Sunnah sunnahEnum, std::optional<QDate> date = std::nullopt) const noexcept;

    /**
     * @brief Overload of `timeForSunnahTime()` taking string-names for sunnah time instead of enum.
     *
     * @param sunnahName: The sunnah time to get the time of, expressed in string-names
     * @param date: (optional) The date when `sunnahName` is at. Sets the effective date to
     * calculate the sunnah time of. Current local date by default.
     * @return QDateTime: Time of the prayer `sunnahName` expressed in a QDateTime object in local
     * clock. Invalid QDateTime construct if time could not be calculated for `sunnahName`.
     */
    [[nodiscard]] QDateTime timeForSunnahTime(
        const QString &sunnahName, std::optional<QDate> date = std::nullopt) const noexcept;

    /**
     * @brief Provides the time left from now to next imminent prayer in milliseconds. "Now" time is
     * calculated at the instance this method is called. An optional parameter `since_ms` calculates
     * the time left from the instance provided to the next prayer of that instance, not the next
     * prayer from "now". When next prayer becomes `None` (after Isha before date boundary
     * 23:59:59), the calculation is handled to show time to next day's Fajr instead.
     *
     * @param since_ms: (optional) QDateTime Instance to start calculating time left to next prayer
     * from
     * @return qint64: Time left to next prayer in milliseconds. -1 if error occurred, otherwise,
     * never negative.
     */
    [[nodiscard]] qint64 timeToNextPrayer(
        const std::optional<QDateTime> &since_ms = std::nullopt) const noexcept;

    /**
     * @brief Translate the provided prayer enum `prayerEnum` to its string name. Mimicks
     * `Adhan::PrayerUtils::to_string()` but unlike it, shows "None" for `Prayer::None`
     *
     * @param prayerEnum: the prayer enum
     * @return QString: Corresponding string name for `prayerEnum`, empty if `prayerName` is
     * invalid.
     */
    Q_INVOKABLE static QString toString(Prayer prayerEnum) noexcept;

    /**
     * @brief Translate the provided sunnah enum `sunnahEnum` to its string name. Overload of
     * `toString()`. Mimicks `Adhan::SunnahUtils::to_string()` but unlike it, shows "None" for
     * `Sunnah::None`
     *
     * @param sunnahEnum: the sunnah enum
     * @return QString: Corresponding string name for `sunnahEnum`, empty if `sunnahName` is
     * invalid.
     */
    Q_INVOKABLE static QString toString(Sunnah sunnahEnum) noexcept;

  protected:
    explicit PrayerEngine(QObject *parent = nullptr);

  private:
    /**
     * @brief Singleton library designed to be only one instance, not to be copied, or transferred
     * ownership to other instance.
     */
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief String-name versus corresponding callable methods hash-table. The desired method can
     * be conveniently called, for example, using the value stored in `m_calcMethodKey`:
     * `s_calcMethods.value(this->m_calcMethodKey)()`
     */
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

    /**
     * @brief A hash-table for madhhab string-names versus `Adhan::Madhab` enum. Employs a shortcut
     * over having an in-class enum vs lib's enum map and then using QMetaEnum object to extract key
     * string-names, since an in-class madhhab enum is not required at the time of writing the code.
     * However, when it _will_ be required, this hash-table needs to be replaced with appropriate
     * way to handle with QMetaEnum just as other enums.
     */
    inline static const QHash<QString, Adhan::Madhab> s_madhhab{
        {u"Hanafi"_s, Adhan::Madhab::Hanafi},
        {u"Shafi"_s,  Adhan::Madhab::Shafi },
    };

    /**
     * @brief An in-class `Prayer` enum vs lib's `Adhan::Prayer` enum hash-table. Used to resolve
     * corresponding lib's enum for the in-class `Prayer` enum where required. The other workaround
     * could've been to static cast the in-class enum to lib's enum call-time but this is much more
     * specific, safe and "typed".
     */
    inline static const QHash<Prayer, Adhan::Prayer> s_prayerEnumMap{
        {Prayer::Fajr,    Adhan::Prayer::Fajr   },
        {Prayer::Sunrise, Adhan::Prayer::Sunrise},
        {Prayer::Dhuhr,   Adhan::Prayer::Dhuhr  },
        {Prayer::Asr,     Adhan::Prayer::Asr    },
        {Prayer::Maghrib, Adhan::Prayer::Maghrib},
        {Prayer::Isha,    Adhan::Prayer::Isha   },
        {Prayer::None,    Adhan::Prayer::None   }
    };

    /**
     * @brief Reverse map of `s_prayerEnumMap`, used to resolve vice-versa between `Adhan::Prayer`
     * and in-class `Prayer`. See brief at `s_prayerEnumMap` for details.
     */
    inline static const QHash<Adhan::Prayer, Prayer> s_revPrayerEnumMap{
        {Adhan::Prayer::Fajr,    Prayer::Fajr   },
        {Adhan::Prayer::Sunrise, Prayer::Sunrise},
        {Adhan::Prayer::Dhuhr,   Prayer::Dhuhr  },
        {Adhan::Prayer::Asr,     Prayer::Asr    },
        {Adhan::Prayer::Maghrib, Prayer::Maghrib},
        {Adhan::Prayer::Isha,    Prayer::Isha   },
        {Adhan::Prayer::None,    Prayer::None   }
    };

    /**
     * @brief An in-class `Sunnah` enum vs lib's `Adhan::Sunnah` enum hash-table. Used to resolve
     * corresponding lib's enum for the in-class `Sunnah` enum where required. The other workaround
     * could've been to static cast the in-class enum to lib's enum call-time but this is much more
     * specific, safe and "typed".
     */
    inline static const QHash<Sunnah, Adhan::Sunnah> s_sunnahEnumMap{
        {Sunnah::None,           Adhan::Sunnah::None          },
        {Sunnah::Midnight,       Adhan::Sunnah::Midnight      },
        {Sunnah::LastThirdNight, Adhan::Sunnah::LastThirdNight}
    };

    /**
     * @brief Reverse map of `s_sunnahEnumMap`, used to resolve vice-versa between `Adhan::Sunnah`
     * and in-class `Sunnah`. See brief at `s_sunnahEnumMap` for details.
     */
    inline static const QHash<Adhan::Sunnah, Sunnah> s_revSunnahEnumMap{
        {Adhan::Sunnah::None,           Sunnah::None          },
        {Adhan::Sunnah::Midnight,       Sunnah::Midnight      },
        {Adhan::Sunnah::LastThirdNight, Sunnah::LastThirdNight}
    };

    /**
     * @brief Provides a metadata object for the `Prayer` enum. Has wide utility in the enum's
     * metadata-related tasks like extracting values from string keys and vice-versa, scope names
     * etc.
     */
    inline static const QMetaEnum s_metaPrayerEnum{QMetaEnum::fromType<Prayer>()};

    /**
     * @brief Provides a metadata object for the `Sunnah` enum. Has wide utility in the enum's
     * metadata-related tasks like extracting values from string keys and vice-versa, scope names
     * etc.
     */
    inline static const QMetaEnum s_metaSunnahEnum{QMetaEnum::fromType<Sunnah>()};

    /**
     * @brief Stores the currently used calculation method's string-name for prayer times
     * calculation. Usually modified from `calcMethod` property or `setCalcMethod()` method. Can be
     * used to invoke appropriate method from `s_calcMethod`.
     */
    QString m_calcMethodKey;

    /**
     * @brief Stores the currently used madhhab's string-name for prayer times calculation. Usually
     * modified from `madhhab` property or `setMadhhab()` method. Can be used to use corresponding
     * `Adhan::Madhab` enum from `s_madhhab` table
     */
    QString m_madhhabKey;

    /**
     * @brief A fixed size nested hash-table for storing prayer-wise user-configurable
     * configurations - a `QHash<QString, QHash>`-type. The structure and size as defined during
     * initiation in `defaultPrayerConfigs()` must remain intact throughout read/write/modify cycles
     * - both the top-level keys and configuration keys must stay fixed - new entries must not be
     * added or existing ones must not be removed - only the values at corresponding configuration
     * keys can be modified. Modify this either through `setPrayerConfig()` as a whole or
     * incrementally through `modifyPrayerConfig()`.
     */
    QVariantHash m_prayerConfig;

    /**
     * @brief Pointer to object, the heart of all prayer times calculations. Provides calculated
     * prayer times and utilities for currently set calculation parameters. Constructed using
     * `calcParams`, and `coordinates` members. See their briefs for further details.
     */
    std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;

    /**
     * @brief Stores the qibla degrees from North (0°). Constructed using currently set coordinates
     * as in member `coordinates` and `Adhan::qibla()` method
     */
    double m_qiblaDegrees{};

    /**
     * @brief Pointer to object, provides calculated sunnah time and utilities for currently
     * constructed `Adhan::PrayerTimes` object - in this case `m_prayerTimes`. See its brief for
     * further details.
     */
    std::unique_ptr<Adhan::SunnahTimes> m_sunnahTimes;

    /**
     * @brief Pointer to object, provides access to the corresponding `Adhan::CalculationMethods`
     * object. Set implicitly by `m_calcParamsKey`, in `setCalcParams()` method.
     */
    std::unique_ptr<Adhan::CalculationParameters> calcParams;

    /**
     * @brief Pointer to object, used in constructing `m_prayerTimes` and `m_qiblaDegrees`. Requires
     * valid latitude and longitude values for the construct.
     */
    std::unique_ptr<Adhan::Coordinates> coordinates;

    /**
     * @brief Stores the valid keys and their variant type-IDs for each configuration maps in
     * `m_prayerConfig`. Set once in `defaultPrayerConfigs()` and must not be modified past that.
     * Used for guarding unwanted structural modifications in `m_prayerConfig`
     */
    QHash<QString, int> prayerConfigFields;

    /**
     * @brief Stores the valid top-level keys in `m_prayerConfig`. Set once in
     * `defaultPrayerConfigs()` and must not be modified past that. Used for guarding unwanted
     * structural modifications in `m_prayerConfig`
     */
    QSet<QString> prayerConfigKeys;

    /**
     * @brief Stores the valid size for each configuration maps in `m_prayerConfig`. Set once in
     * `defaultPrayerConfigs()` and must not be modified past that. Used for guarding unwanted
     * structural modifications in `m_prayerConfig`
     */
    qsizetype prayerConfigEntrySize{};

    /**
     * @brief Stores the valid size for `m_prayerConfig`. Set once in `defaultPrayerConfigs()` and
     * must not be modified past that. Used for guarding unwanted structural modifications in
     * `m_prayerConfig`
     */
    qsizetype prayerConfigMapSize{};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Customized override of `AppBase::defaultConfigs()`. See
     * `AppBase::defaultConfigs()` brief
     */
    void defaultConfigs() noexcept override;

    /**
     * @brief Sets prayer config defaults, i.e. initiates `m_prayerConfig` with a pre-defined
     * structure and default values. Additionally initiates `prayerConfigKeys`,
     * `prayerConfigFields`, `prayerConfigMapSize` and `prayerConfigEntrySize` members with expected
     * (polymorphic, not hardcoded) values which will be used to maintain `m_prayerConfig`'s
     * structure during the whole lifetime of the engine.
     */
    void defaultPrayerConfig() noexcept;

    /**
     * @brief Customized override of `AppBase::loadConfigs()`. Additionally takes care of
     * old-settings migration.
     */
    void loadConfigs() noexcept override;

    /**
     * @brief Loads prayer config from persistent config file to memory, i.e. `m_prayerConfig`.
     * Additionally handles migration from old config conditionally.
     *
     * @param configMigrated: Mutable bool variable which will be modified to inform store migration
     * status - false if store was not migrated, untouched otherwise. See `loadConfigKeys()` for a
     * detail on the logical-AND chain behavior.
     */
    void loadPrayerConfig(bool &configMigrated) noexcept;

    /**
     * @brief Customized override of `AppBase::saveConfigs()`
     */
    void saveConfigs() noexcept override;

    /**
     * @brief Sets a full prayer config hash-table. Use this to assign a modified whole-table to
     * `m_prayerConfig`. For incremental and specific modifications, use `modifyPrayerConfig()`
     * instead. Guards are set to preserve the structure, size and type of the prayer config table.
     * If `prayerConfig` is invalid or identical to currently used table, the whole table is
     * ignored. Utilizes `isValidEntry()`. See also its briefs for further details.
     *
     * @param prayerConfig: The whole hash-table to be set for use, with modified values.
     */
    void setPrayerConfig(const QVariantHash &prayerConfig) noexcept;

    /**
     * @brief Establishes a relay-connection between `CoreEngine::alarmsReset()` and `invalidate()`
     * signals. A direct connection to `CoreEngine::alarmManager()` instead is deliberately not
     * established to prefer simpler setups and respect the chain of responsibility.
     */
    void setupAlarmsResetRelay() const noexcept;

    /**
     * @brief Establishes a relay-connection between `CoreEngine::adhanAlarm()` and `calcChanged()`
     * signals. A direct connection to `CoreEngine::alarmManager()` instead is deliberately not
     * established to prefer simpler setups and respect the chain of responsibility.
     */
    void setupAdhanAlarmRelay() const noexcept;

    /**
     * @brief Establishes a relay-connection between `CoreEngine::dateChanged()` and `invalidate()`
     * signals. A direct connection to `CoreEngine::alarmManager()` instead is deliberately not
     * established to prefer simpler setups and respect the chain of responsibility.
     */
    void setupDateChangedRelay() const noexcept;

    /**
     * @brief Establishes all internal relays, signals and slots. See the briefs of
     * `prayerConfigChanged()`, `invalidate()` and `onInvalidate() for details.
     */
    void setupInternalConnections() const noexcept;

    /**
     * @brief Establishes a relay-connection between `LocationEngine::locationChanged()` and
     * `invalidate()` signals. A direct connection to `CoreEngine::alarmManager()` instead is
     * deliberately not established to prefer simpler setups and respect the chain of
     * responsibility.
     */
    void setupLocationChangedRelay() const noexcept;

    /**
     * @brief Establishes a relay-connection between `CoreEngine::minuteChanged()` and
     * `calcChanged()` signals. A direct connection to `CoreEngine::alarmManager()` instead is
     * deliberately not established to prefer simpler setups and respect the chain of
     * responsibility.
     */
    void setupMinuteChangedRelay() const noexcept;

    /**
     * @brief Update prayer calculations with newly changed values of members. Should be called when
     * members related to prayer times calculations are modified (e.g. `m_prayerConfig`,
     * `m_calcMethodKey`, `m_madhhabKey`, `LocationEngine::getLocation()`) or local date changes.
     * Utilizes `getPrayerConfig()`, `s_calcMethods`, `s_madhhab`, and `CoreEngine::alarmManager()`
     * for get-set routines and clear-registering adhan alarms for the prayer times. Not recommended
     * to call bare by itself since it does not emit any change signals. Always wrap it in a slot or
     * a parent method that additionally handles corresponding change-signal notifications. See
     * `onInvalidate()` and `invalidate()` for further information.
     */
    void updateCalc();

  signals:
    /**
     * @brief Group signal notifying about prayer-times calculations being modified. Also implicitly
     * forces a re-read of `currentPrayer`, `nextPrayer` and `qibla` properties. Implicitly bound to
     * `CoreEngine::adhanAlarm()`, `CoreEngine::minuteChanged()` (as soft, re-read relays), and
     * `invalidate()` signal (as hard change-notifier). However, `invalidate()` signal is not
     * directly relayed; this signal must be emitted only after changes have been finalized and
     * committed. See `onInvalidate()` for further details.
     */
    void calcChanged();

    /**
     * @brief Notifies about a hard refresh / full re-calculation of prayer times because members
     * related to it has possibly been modified. Implicitly bound to `m_calcMethodKey`,
     * `m_madhhabKey`, `LocationEngine::getLocation()` (external) members and
     * `prayerConfigChanged()`, `CoreEngine::alarmsReset()`, `CoreEngine::dateChanged()` and
     * `LocationEngine::locationChanged()` signals. A `calcChanged()` signal should be followed once
     * the invalidated calcuations has been refreshed, finalized and committed.
     */
    void invalidate();

    /**
     * @brief Notifies about prayer-times configuration has changed. Implicitly bound to
     * `m_prayerConfig`, relayed to `invalidate()`. See also their briefs. This signal needs to be
     * independent owing to `prayerConfig` property being writtable.
     */
    void prayerConfigChanged();

  private slots: // NOLINT(*-redundant-access-specifiers)
    /**
     * @brief Customized overridden extension of `AppBase::onCoreOnline`. See its brief for
     * details
     */
    void onCoreOnline() noexcept override;

    /**
     * @brief Performs routines to update prayer-times calculations on `invalidate()` notification.
     * Utilizes `updateCalc()` for re-calculations. Emits `calcChanged()` once updates are
     * completed.
     */
    void onInvalidate();

    /**
     * @brief Executes `setupLocationChangedRelay()`. Trivial, but retains readability over a direct
     * lambda in `LocationEngine::onceOnline()`. Also allows extensibility in future.
     */
    void onLocationEngineOnline() noexcept;
};

} // namespace App