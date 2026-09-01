#pragma once

#include <QObject>
#include <QQmlEngine>
// #include <memory>

// #include <adhan/CalculationMethod.hpp>
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
    Q_PROPERTY(QStringList prayerNamesList READ prayerNamesList NOTIFY localeChanged)
    // Q_PROPERTY(PrayerEngine::Prayer currentPrayer READ currentPrayer NOTIFY timeChanged)

  signals:
    auto localeChanged() -> void;

  public:
    explicit PrayerEngine(QObject *parent = nullptr);
    ~PrayerEngine() override;

    static auto create(QQmlEngine *, QJSEngine *) -> PrayerEngine *;
    static auto instance() -> PrayerEngine &;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Types & Enums
    ////////////////////////////////////////////////////////////////////////////////////////////////

    enum Prayer : qint8
    {
        Fajr    = static_cast<qint8>(Adhan::Prayer::Fajr),
        Sunrise = static_cast<qint8>(Adhan::Prayer::Sunrise),
        Dhuhr   = static_cast<qint8>(Adhan::Prayer::Dhuhr),
        Asr     = static_cast<qint8>(Adhan::Prayer::Asr),
        Maghrib = static_cast<qint8>(Adhan::Prayer::Maghrib),
        Isha    = static_cast<qint8>(Adhan::Prayer::Isha),
        None    = static_cast<qint8>(Adhan::Prayer::None),
    };
    Q_ENUM(Prayer)

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    Q_INVOKABLE [[nodiscard]] QString prayerName(Prayer prayerEnum);
    // void timeChanged();

  public slots: // NOLINT(readability-redundant-access-specifiers)
    auto reInitTexts() -> void;

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static PrayerEngine *s_singletonInstance;

    const QMap<Prayer, Adhan::Prayer> m_prayerEnumMap{
        {   PrayerEngine::Fajr,    Adhan::Prayer::Fajr},
        {PrayerEngine::Sunrise, Adhan::Prayer::Sunrise},
        {  PrayerEngine::Dhuhr,   Adhan::Prayer::Dhuhr},
        {    PrayerEngine::Asr,     Adhan::Prayer::Asr},
        {PrayerEngine::Maghrib, Adhan::Prayer::Maghrib},
        {   PrayerEngine::Isha,    Adhan::Prayer::Isha},
        {   PrayerEngine::None,    Adhan::Prayer::None}
    };

    const QMap<Adhan::Prayer, Prayer> m_revPrayerEnumMap{
        {   Adhan::Prayer::Fajr,    PrayerEngine::Fajr},
        {Adhan::Prayer::Sunrise, PrayerEngine::Sunrise},
        {  Adhan::Prayer::Dhuhr,   PrayerEngine::Dhuhr},
        {    Adhan::Prayer::Asr,     PrayerEngine::Asr},
        {Adhan::Prayer::Maghrib, PrayerEngine::Maghrib},
        {   Adhan::Prayer::Isha,    PrayerEngine::Isha},
        {   Adhan::Prayer::None,    PrayerEngine::None}
    };

    QMap<Prayer, QString> m_prayerDisplayNames;
    // std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto               fromString(QStringView prayerName) -> Prayer;
    [[nodiscard]] auto toString(Prayer prayerEnum) -> QString;

    // auto currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    auto initTexts() -> void;
    auto prayerNamesList() -> QStringList;
};

} // namespace Munadi
