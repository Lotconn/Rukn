#pragma once

#include <QObject>
#include <QQmlEngine>

#include <PrayerEngine.hpp>

namespace Munadi
{

class UiEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool bgImage MEMBER m_bgImage NOTIFY bgImageChanged)
    Q_PROPERTY(double blur MEMBER m_blur NOTIFY blurChanged RESET resetBlur)
    Q_PROPERTY(int calcMethodDisplayIndex MEMBER m_calcMethodsDisplayIndex WRITE
                   setCalcMethodDisplayIdx NOTIFY configChanged)
    Q_PROPERTY(QVariantList calcMethodsDisplayNames MEMBER m_calcMethodsDisplayNames NOTIFY
                   translationChanged)
    Q_PROPERTY(int madhabDisplayIndex MEMBER m_madhabDisplayIndex WRITE setMadhabDisplayIdx NOTIFY
                   configChanged)
    Q_PROPERTY(
        QVariantList madhabDisplayNames MEMBER m_madhabDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY opacityChanged RESET resetOpacity)
    Q_PROPERTY(
        QVariantList prayerDisplayNames MEMBER m_prayerDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(QStringList themeList MEMBER m_themeList WRITE dummy NOTIFY themeListChanged)
    // Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
    Q_PROPERTY(int themeListIndex READ themeListIndex WRITE setThemeListIdx RESET resetTheme NOTIFY
                   themeListIdxChanged)

  public:
    explicit UiEngine(QObject *parent = nullptr);
    ~UiEngine() override;

    static auto instance() noexcept -> UiEngine *;

    auto themeList() -> QStringList;
    auto themeListIndex() -> int;
    auto setThemeListIdx(int) -> void;

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static QPointer<UiEngine> s_singletonInstance;

    inline static QVariantList oldCalcMethodsDisplayNames{
        QLatin1StringView{"MuslimWorldLeague"},
        QLatin1StringView{"Egyptian"},
        QLatin1StringView{"Karachi"},
        QLatin1StringView{"UmmAlQura"},
        QLatin1StringView{"Dubai"},
        QLatin1StringView{"MoonsightingCommittee"},
        QLatin1StringView{"NorthAmerica"},
        QLatin1StringView{"Kuwait"},
        QLatin1StringView{"Qatar"},
        QLatin1StringView{"Singapore"},
        QLatin1StringView{"Turkey"},
        QLatin1StringView{"Other"},
    };

    inline static QVariantList oldMadhabDisplayNames{
        QLatin1StringView{},
        QLatin1StringView{"Shafi"},
        QLatin1StringView{"Hanafi"},
    };

    constexpr static double defaultBlur{0.45};    // NOLINT(*-magic-numbers)
    constexpr static double defaultOpacity{0.77}; // NOLINT(*-magic-numbers)

    std::atomic<bool>   m_bgImage{true};
    std::atomic<double> m_blur{defaultBlur};      // NOLINT(*-magic-numbers)
    QAtomicInt          m_calcMethodsDisplayIndex;
    QVariantList        m_calcMethodsDisplayNames;
    QAtomicInt          m_madhabDisplayIndex;
    QVariantList        m_madhabDisplayNames;
    std::atomic<double> m_opacity{defaultOpacity};
    QVariantList        m_prayerDisplayNames;
    QAtomicInt          m_themeListIdx;
    QStringList         m_themeList;

    QReadWriteLock varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto indexOfValue(const QVariantList &list, const QString &value) -> int;
    // static auto textsRefresher() -> int;
    static auto dummy(const QStringList &) -> void;

    auto connectToCoreOffline() const -> void;
    auto connectToInvalidate() const -> void;
    auto connectToLocaleChanged() const -> void;
    auto initConfigs() -> void;
    auto initTexts() -> void;
    auto loadConfigs() -> void;
    auto resetBlur() -> void;
    auto resetOpacity() -> void;
    auto resetTheme() -> void;
    auto saveConfigs() -> void;
    auto setCalcMethodDisplayIdx(const QVariant &index) -> void;
    auto setMadhabDisplayIdx(const QVariant &index) -> void;

  signals:
    auto bgImageChanged() -> void;
    auto blurChanged() -> void;
    auto configChanged() -> void;
    auto opacityChanged() -> void;
    auto translationChanged() -> void;
    auto themeListChanged() -> void;
    auto themeListIdxChanged(int) -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onPrayerEngineOnline() -> void;
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
    auto onInvalidate() -> void;
    auto onLocaleChanged() -> void;
};

} // namespace Munadi