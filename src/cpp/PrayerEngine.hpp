#pragma once

#include <QObject>
#include <QQmlEngine>
#include <memory>

#include <adhan/CalculationMethod.hpp>
#include <adhan/Coordinates.hpp>
#include <adhan/DateUtils.hpp>
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

    Q_INVOKABLE [[nodiscard]] auto prayerName(Prayer prayerEnum) -> QString;
    // void timeChanged();

  public slots:
    auto reInitTexts() -> void;

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static PrayerEngine                     *s_singletonInstance;
    static const QMap<Prayer, Adhan::Prayer> m_prayerEnumMap;
    static const QMap<Adhan::Prayer, Prayer> m_revPrayerEnumMap;

    QMap<Prayer, QString>               m_prayerDisplayNames;
    std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto               fromString(QStringView prayerName) -> Prayer;
    [[nodiscard]] static auto toString(Prayer prayerEnum) -> QString;

    // auto currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    auto initTexts() -> void;
    auto prayerNamesList() -> QStringList;
};
} // namespace Munadi
