#pragma once

#include <QQmlEngine>

#include <MunadiEngine.hpp>
#include <PrayerEngine.hpp>

namespace Munadi
{

class UiEngine : public MunadiEngine<UiEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool bgImage MEMBER m_bgImage NOTIFY uiConfigChanged)
    Q_PROPERTY(double blur MEMBER m_blur NOTIFY uiConfigChanged)
    Q_PROPERTY(int calcMethodDisplayIndex MEMBER m_calcMethodsDisplayIndex WRITE
                   setCalcMethodDisplayIdx NOTIFY configChanged)
    Q_PROPERTY(QVariantList calcMethodsDisplayNames MEMBER m_calcMethodsDisplayNames NOTIFY
                   translationChanged)
    Q_PROPERTY(int langDisplayIndex MEMBER m_langDisplayIndex WRITE setLangDisplayIdx NOTIFY
                   uiConfigChanged)
    Q_PROPERTY(QVariantList langDisplayNames MEMBER m_langDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(int madhabDisplayIndex MEMBER m_madhabDisplayIndex WRITE setMadhabDisplayIdx NOTIFY
                   configChanged)
    Q_PROPERTY(
        QVariantList madhabDisplayNames MEMBER m_madhabDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY uiConfigChanged)
    Q_PROPERTY(
        QVariantList prayerDisplayNames MEMBER m_prayerDisplayNames NOTIFY translationChanged)
    // Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
    Q_PROPERTY(int themeDisplayIndex MEMBER m_themeDisplayIndex NOTIFY uiConfigChanged)
    Q_PROPERTY(QVariantList themeDisplayNames MEMBER m_themeDisplayNames NOTIFY translationChanged)

  public:
    explicit UiEngine(QObject *parent = nullptr);

    static auto logCategory() -> const QLoggingCategory &;

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

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

    inline static QVariantList oldThemeDisplayNames{
        QLatin1StringView{"System"},
        QLatin1StringView{"Light"},
        QLatin1StringView{"Dark"},
    };

    std::atomic<bool>   m_bgImage;
    std::atomic<double> m_blur; // NOLINT(*-magic-numbers)
    QAtomicInt          m_calcMethodsDisplayIndex;
    QVariantList        m_calcMethodsDisplayNames;
    QAtomicInt          m_langDisplayIndex;
    QVariantList        m_langDisplayNames;
    QAtomicInt          m_madhabDisplayIndex;
    QVariantList        m_madhabDisplayNames;
    std::atomic<double> m_opacity;
    QVariantList        m_prayerDisplayNames;
    QAtomicInt          m_themeDisplayIndex;
    QVariantList        m_themeDisplayNames;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto indexOfValue(const QVariantList &list, const QString &value) -> int;
    // static auto textsRefresher() -> int;
    static auto dummy(const QStringList &) -> void;

    auto connectToInvalidate() const -> void;
    auto connectToLocaleChanged() const -> void;
    auto defaultConfigs() -> void override;
    auto getCurrentLang() -> QString;
    auto getCurrentTheme() -> QString;
    auto initConfigs() -> void;
    auto initTexts() -> void;
    auto loadConfigs() -> void override;
    auto saveConfigs() -> void override;
    auto setCalcMethodDisplayIdx(const QVariant &index) -> void;
    auto setLangDisplayIdx(const QVariant &index) -> void;
    auto setMadhabDisplayIdx(const QVariant &index) -> void;

  signals:
    auto bgImageChanged() -> void;
    auto blurChanged() -> void;
    auto configChanged() -> void;
    auto opacityChanged() -> void;
    auto translationChanged() -> void;
    auto uiConfigChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    void onPrayerEngineOnline();
    void onCoreOnline() override;
    void onInvalidate();
    void onLocaleChanged();
};

} // namespace Munadi