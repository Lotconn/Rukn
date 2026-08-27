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
    Q_PROPERTY(bool localeGuard READ localeGuard NOTIFY localeChanged)
    Q_PROPERTY(QVariantList prayerConfig READ prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)
    Q_PROPERTY(QStringList prayerNamesList READ prayerNamesList NOTIFY localeChanged)
    // Q_PROPERTY(PrayerEngine::Prayer currentPrayer READ currentPrayer NOTIFY timeChanged)

  public:
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

    explicit PrayerEngine(QObject *parent = nullptr);
    ~PrayerEngine() override;

    static auto create(QQmlEngine *, QJSEngine *) -> PrayerEngine *;
    static auto instance() -> PrayerEngine &;

    // auto        currentPrayer(Adhan::Instant when = Adhan::now()) -> PrayerEngine::Prayer;
    [[nodiscard]] auto localeGuard() const -> int;
    [[nodiscard]] auto prayerConfig() const -> QVariantList;
    auto               prayerNamesList() -> QStringList;

    Q_INVOKABLE [[nodiscard]] auto prayerName(Prayer) -> QString;
    Q_INVOKABLE auto               removeAthanIndex(int rmIdx) -> void;

  signals:
    auto localeChanged() -> void;
    auto prayerConfigChanged(const QVariantList &config) -> void;
    // void timeChanged();

  public slots:
    auto refreshText() -> void;
    auto setPrayerConfig(const QVariantList &config) -> void;

  private:
    Q_DISABLE_COPY_MOVE(PrayerEngine);

    /////////////////////////////////////////////////////////////
    //  Variables
    /////////////////////////////////////////////////////////////
    static PrayerEngine                     *s_singletonInstance;
    static const QMap<Prayer, Adhan::Prayer> m_prayerEnumMap;
    static const QMap<Adhan::Prayer, Prayer> m_revPrayerEnumMap;

    QAtomicInt                          m_localeChangeCount;
    QMap<Prayer, QString>               m_prayerDisplayNames;
    QVariantList                        m_prayerConfig;
    std::unique_ptr<Adhan::PrayerTimes> m_prayerTimes;

    /////////////////////////////////////////////////////////////
    //  Functions
    /////////////////////////////////////////////////////////////
    [[nodiscard]] static auto toString(Prayer) -> QString;
    static auto               fromString(QStringView) -> Prayer;

    auto init() -> void;
};

class UiEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList themeList READ themeList CONSTANT)
    Q_PROPERTY(
        int themeListIndex READ themeListIndex WRITE setThemeListIdx NOTIFY themeListIdxChanged)

  public:
    explicit UiEngine(QObject *parent = nullptr);
    ~UiEngine() override;

    static UiEngine *create(QQmlEngine *, QJSEngine *);
    static UiEngine &instance();

    QStringList themeList();
    int         themeListIndex();
    void        setThemeListIdx(int);

  signals:
    void themeListIdxChanged(int);

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    static UiEngine *s_singletonInstance;

    int m_themeListIdx;
};
} // namespace Munadi

// Q_DECLARE_OPERATORS_FOR_FLAGS(PrayerEngine::)
