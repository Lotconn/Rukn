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

    Q_PROPERTY(QVariantList prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig RESET
                   defaultPrayerConfig NOTIFY prayerConfigChanged)
    // Q_PROPERTY(PrayerEngine::Prayer currentPrayer READ currentPrayer NOTIFY timeChanged)

  public:
    explicit PrayerEngine(QObject *parent = nullptr);
    ~PrayerEngine() override;

    static auto instance() -> PrayerEngine &;

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
    static auto toString(Prayer prayerEnum) -> QString;
    static auto metaPrayerEnum() -> const QMetaEnum &;

    auto getPrayerConfig(Prayer prayerEnum, QAnyStringView field) -> QVariant;

    Q_INVOKABLE void modifyPrayerConfig(Prayer prayerEnum, const QVariantMap &entry);

    // void timeChanged();

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static PrayerEngine *s_singletonInstance{nullptr};

    inline static QMap<Prayer, Adhan::Prayer> s_prayerEnumMap{
        {   PrayerEngine::Fajr,    Adhan::Prayer::Fajr},
        {PrayerEngine::Sunrise, Adhan::Prayer::Sunrise},
        {  PrayerEngine::Dhuhr,   Adhan::Prayer::Dhuhr},
        {    PrayerEngine::Asr,     Adhan::Prayer::Asr},
        {PrayerEngine::Maghrib, Adhan::Prayer::Maghrib},
        {   PrayerEngine::Isha,    Adhan::Prayer::Isha},
        {   PrayerEngine::None,    Adhan::Prayer::None}
    };

    inline static QMap<Adhan::Prayer, Prayer> s_revPrayerEnumMap{
        {   Adhan::Prayer::Fajr,    PrayerEngine::Fajr},
        {Adhan::Prayer::Sunrise, PrayerEngine::Sunrise},
        {  Adhan::Prayer::Dhuhr,   PrayerEngine::Dhuhr},
        {    Adhan::Prayer::Asr,     PrayerEngine::Asr},
        {Adhan::Prayer::Maghrib, PrayerEngine::Maghrib},
        {   Adhan::Prayer::Isha,    PrayerEngine::Isha},
        {   Adhan::Prayer::None,    PrayerEngine::None}
    };

    // std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;
    inline static QMetaEnum s_metaPrayerEnum{QMetaEnum::fromType<Prayer>()};

    std::unique_ptr<Adhan::CalculationParameters> m_calcParams;
    QVariantList                                  m_prayerConfig;

    QMetaObject::Connection coreOfflineConnection;
    QMap<QString, int>      prayerConfigKeys;
    qsizetype               prayerConfigListSize;
    qsizetype               prayerConfigMapSize;
    QReadWriteLock          varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // auto currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    auto connectToCoreOffline() -> void;
    auto defaultCalcParams() -> void;
    auto defaultPrayerConfig() -> void;
    auto loadConfigs() -> void;
    auto loadPrayerConfig(bool &configMigrated) -> void;
    auto saveConfigs() -> void;
    auto setPrayerConfig(const QVariantList &prayerConfig) -> void;

  signals:
    auto invalidate() -> void;
    auto prayerConfigChanged(QVariantList prayerConfig) -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
    auto onInvalidate() -> void;
};

} // namespace Munadi
