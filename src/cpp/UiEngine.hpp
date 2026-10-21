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

    Q_PROPERTY(bool bgImage MEMBER m_bgImage NOTIFY guiConfigChanged)
    Q_PROPERTY(double blur MEMBER m_blur NOTIFY guiConfigChanged)
    Q_PROPERTY(int calcMethodDisplayIndex MEMBER m_calcMethodsDisplayIndex WRITE
                   setCalcMethodDisplayIdx NOTIFY configChanged)
    Q_PROPERTY(
        QVariantList calcMethodsDisplayNames READ calcMethodsDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(int langDisplayIndex MEMBER m_langDisplayIndex WRITE setLangDisplayIdx NOTIFY
                   uiConfigChanged)
    Q_PROPERTY(QVariantList langDisplayNames READ langDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(int madhabDisplayIndex MEMBER m_madhabDisplayIndex WRITE setMadhabDisplayIdx NOTIFY
                   configChanged)
    Q_PROPERTY(QVariantList madhabDisplayNames READ madhabDisplayNames NOTIFY translationChanged)
    Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY guiConfigChanged)
    Q_PROPERTY(QVariantList prayerDisplayNames READ prayerDisplayNames NOTIFY translationChanged)
    // Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
    Q_PROPERTY(int themeDisplayIndex MEMBER m_themeDisplayIndex NOTIFY uiConfigChanged)
    Q_PROPERTY(QVariantList themeDisplayNames READ themeDisplayNames NOTIFY translationChanged)

  public:
    explicit UiEngine(QObject *parent = nullptr);

    static auto logCategory() noexcept -> const QLoggingCategory &;

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static const QVariantList oldCalcMethodsDisplayNames{
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

    inline static const QVariantList oldMadhabDisplayNames{
        QLatin1StringView{},
        QLatin1StringView{"Shafi"},
        QLatin1StringView{"Hanafi"},
    };

    inline static const QVariantList oldThemeDisplayNames{
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

    static auto indexOfValue(const QVariantList &list, const QString &value) noexcept -> int;
    // static auto textsRefresher() -> int;

    auto               calcMethodsDisplayNames() noexcept -> QVariantList &;
    auto               connectToInvalidate() const noexcept -> void;
    auto               connectToLocaleChanged() const noexcept -> void;
    auto               defaultConfigs() noexcept -> void override;
    [[nodiscard]] auto getCurrentLang() const noexcept -> QString;
    [[nodiscard]] auto getCurrentTheme() const noexcept -> QString;
    auto               initConfigs() noexcept -> void;
    auto               initTexts() noexcept -> void;
    auto               langDisplayNames() noexcept -> QVariantList &;
    auto               loadConfigs() noexcept -> void override;
    auto               madhabDisplayNames() noexcept -> QVariantList &;
    auto               prayerDisplayNames() noexcept -> QVariantList &;
    auto               saveConfigs() noexcept -> void override;
    auto               setCalcMethodDisplayIdx(const QVariant &index) noexcept -> void;
    auto               setLangDisplayIdx(const QVariant &index) noexcept -> void;
    auto               setMadhabDisplayIdx(const QVariant &index) noexcept -> void;
    auto               themeDisplayNames() noexcept -> QVariantList &;

  signals:
    auto configChanged() -> void;
    auto guiConfigChanged() -> void;
    auto translationChanged() -> void;
    auto uiConfigChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    void onPrayerEngineOnline() noexcept;
    void onCoreOnline() noexcept override;
    void onInvalidate() noexcept;
    void onLocaleChanged() noexcept;
};

} // namespace Munadi