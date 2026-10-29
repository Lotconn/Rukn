#pragma once

#include <QQmlEngine>

#include <MunadiEngine.hpp>
#include <PrayerEngine.hpp>

namespace Munadi
{

class UiEngine : public MunadiBase, public MunadiEngine<UiEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool bgImage MEMBER m_bgImage NOTIFY bgImageChanged)
    Q_PROPERTY(double blur MEMBER m_blur NOTIFY blurChanged)
    Q_PROPERTY(int calcMethodDisplayIndex MEMBER m_calcMethodsDisplayIndex WRITE
                   setCalcMethodDisplayIdx NOTIFY prayerCalcChanged)
    Q_PROPERTY(bool isoClock MEMBER m_isoClock NOTIFY displayTimesChanged)
    Q_PROPERTY(
        QVariantList calcMethodsDisplayNames READ calcMethodsDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(QString calendarDateDisplay READ calendarDateDisplay NOTIFY calendarChanged)
    Q_PROPERTY(int calendarDisplayIndex MEMBER m_calendarDisplayIndex WRITE setCalendarDisplayIdx
                   NOTIFY calendarIdxChanged)
    Q_PROPERTY(
        QVariantList calendarDisplayNames READ calendarDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(int dateOffset MEMBER m_dateOffset WRITE setDateOffset RESET resetDateOffset NOTIFY
                   calendarChanged)
    Q_PROPERTY(
        double hijriOffset MEMBER m_hijriOffset WRITE setHijriOffset NOTIFY hijriOffsetChanged)
    Q_PROPERTY(int langDisplayIndex MEMBER m_langDisplayIndex WRITE setLangDisplayIdx NOTIFY
                   langIdxChanged)
    Q_PROPERTY(QVariantList langDisplayNames READ langDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(int madhabDisplayIndex MEMBER m_madhabDisplayIndex WRITE setMadhabDisplayIdx NOTIFY
                   prayerCalcChanged)
    Q_PROPERTY(QVariantList madhabDisplayNames READ madhabDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY opacityChanged)
    Q_PROPERTY(QVariantList prayerDisplayNames READ prayerDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(QVariantHash prayerDisplayTimes READ prayerDisplayTimes NOTIFY displayTimesChanged)
    Q_PROPERTY(bool showOnAthan MEMBER m_showOnAthan NOTIFY showOnAthanChanged)
    // Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
    Q_PROPERTY(int themeDisplayIndex MEMBER m_themeDisplayIndex NOTIFY themeChanged)
    Q_PROPERTY(QVariantList themeDisplayNames READ themeDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(QString timeLeftDisplay READ timeLeftDisplay NOTIFY timeChanged)
    Q_PROPERTY(double timeLeftProgress READ timeLeftProgress NOTIFY timeChanged)

  public:
    static auto create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> UiEngine *;
    static auto logCategory() noexcept -> const QLoggingCategory &;

    Q_INVOKABLE static QString getFormFactor() noexcept;

  protected:
    explicit UiEngine(QObject *parent = nullptr);

  public slots:
    void resetDateOffset() noexcept;

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    // Do not sort this list
    inline static const QVariantList oldCalcMethodsDisplayNames{
        u"MuslimWorldLeague"_s, u"Egyptian"_s, u"Karachi"_s,
        u"UmmAlQura"_s,         u"Dubai"_s,    u"MoonsightingCommittee"_s,
        u"NorthAmerica"_s,      u"Kuwait"_s,   u"Qatar"_s,
        u"Singapore"_s,         u"Turkey"_s,   u"Other"_s,
    };

    // Do not sort this list
    inline static const QVariantList oldCalendarDisplayNames{
        u"Both"_s,
        u"Hijri"_s,
        u"Gregorian"_s,
    };

    // Do not sort this list
    inline static const QVariantList oldMadhabDisplayNames{
        u""_s,
        u"Shafi"_s,
        u"Hanafi"_s,
    };

    // Do not sort this list
    inline static const QVariantList oldThemeDisplayNames{
        u"System"_s,
        u"Light"_s,
        u"Dark"_s,
    };

    inline static const QString calendarFormat{u"dd-MMM-yyyy"_s};

    std::atomic<bool>       m_bgImage;
    std::atomic<double>     m_blur;
    QAtomicInt              m_calcMethodsDisplayIndex;
    QVariantList            m_calcMethodsDisplayNames;
    QHash<QString, QString> m_calendarDateDisplay;
    QAtomicInt              m_calendarDisplayIndex;
    QVariantList            m_calendarDisplayNames;
    QAtomicInt              m_dateOffset;
    QAtomicInt              m_hijriOffset;
    std::atomic<bool>       m_isoClock;
    QAtomicInt              m_langDisplayIndex;
    QVariantList            m_langDisplayNames;
    QAtomicInt              m_madhabDisplayIndex;
    QVariantList            m_madhabDisplayNames;
    std::atomic<double>     m_opacity;
    QVariantList            m_prayerDisplayNames;
    std::atomic<bool>       m_showOnAthan;
    QAtomicInt              m_themeDisplayIndex;
    QVariantList            m_themeDisplayNames;

    QString calendarKey;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto indexOfValue(const QVariantList &list, const QString &value) noexcept -> int;
    // static auto textsRefresher() -> int;
    static auto timeLeftProgress() noexcept -> double;

    auto               calcMethodsDisplayNames() noexcept -> QVariantList &;
    auto               calendarDateDisplay() noexcept -> QString;
    auto               calendarDisplayNames() noexcept -> QVariantList &;
    auto               connectToAlarmsReset() const noexcept -> void;
    auto               connectToAthanPlayingChanged() const noexcept -> void;
    auto               connectToCalcChanged() const noexcept -> void;
    auto               connectToDateChanged() const noexcept -> void;
    auto               connectToLocaleChanged() const noexcept -> void;
    auto               connectToMinuteChanged() const noexcept -> void;
    auto               defaultConfigs() noexcept -> void override;
    [[nodiscard]] auto getCurrentLang() const noexcept -> QString;
    [[nodiscard]] auto getCurrentTheme() const noexcept -> QString;
    auto               initCalendarDates() noexcept -> void;
    auto               initConfigs() noexcept -> void;
    auto               initTexts() noexcept -> void;
    auto               langDisplayNames() noexcept -> QVariantList &;
    auto               loadConfigs() noexcept -> void override;
    auto               madhabDisplayNames() noexcept -> QVariantList &;
    auto               prayerDisplayNames() noexcept -> QVariantList &;
    [[nodiscard]] auto prayerDisplayTimes() const noexcept -> QVariantHash;
    auto               saveConfigs() noexcept -> void override;
    auto               setCalcMethodDisplayIdx(const QVariant &index) noexcept -> void;
    auto               setCalendarKey(const QString &name) noexcept -> void;
    auto               setCalendarDisplayIdx(const QVariant &index) noexcept -> void;
    auto               setDateOffset(int offset) noexcept -> void;
    auto               setHijriOffset(double offset) noexcept -> void;
    auto               setLangDisplayIdx(const QVariant &index) noexcept -> void;
    auto               setMadhabDisplayIdx(const QVariant &index) noexcept -> void;
    auto               themeDisplayNames() noexcept -> QVariantList &;
    [[nodiscard]] auto timeLeftDisplay() noexcept -> QString;

  signals:
    auto bgImageChanged() -> void;
    auto blurChanged() -> void;
    auto calendarChanged() -> void;
    auto calendarIdxChanged() -> void;
    auto displayTimesChanged() -> void;
    auto hijriOffsetChanged() -> void;
    auto langIdxChanged() -> void;
    auto opacityChanged() -> void;
    auto prayerCalcChanged() -> void;
    auto showOnAthanChanged() -> void;
    auto themeChanged() -> void;
    auto timeChanged() -> void;
    auto translationChanged() -> void;

  private slots:
    void onAlarmsReset() noexcept;
    void onAthanPlayingChanged(bool playing) noexcept;
    void onCalcChanged() noexcept;
    void onDateChanged() noexcept;
    void onMinuteChanged() noexcept;
    void onMultimediaEngineOnline() noexcept;
    void onPrayerEngineOnline() noexcept;
    void onCoreOnline() noexcept override;
    void onLocaleChanged() noexcept;
};

} // namespace Munadi