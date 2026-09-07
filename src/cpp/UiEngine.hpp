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

    Q_PROPERTY(
        QStringList prayerDisplayNamesList READ prayerDisplayNamesList NOTIFY translationChanged)
    Q_PROPERTY(QStringList themeList MEMBER m_themeList WRITE dummy NOTIFY themeListChanged)
    Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
    Q_PROPERTY(int themeListIndex READ themeListIndex WRITE setThemeListIdx RESET defaultTheme
                   NOTIFY themeListIdxChanged)

  public:
    explicit UiEngine(QObject *parent = nullptr);
    ~UiEngine() override;

    static auto instance() -> UiEngine &;

    auto themeList() -> QStringList;
    auto themeListIndex() -> int;
    auto setThemeListIdx(int) -> void;

    Q_INVOKABLE [[nodiscard]] QString prayerDisplayName(PrayerEngine::Prayer prayerEnum);

  private:
    Q_DISABLE_COPY_MOVE(UiEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static UiEngine *s_singletonInstance{nullptr};

    QMap<PrayerEngine::Prayer, QString> m_prayerDisplayNames;
    int                                 m_themeListIdx = 0;
    QStringList                         m_themeList;

    QMetaObject::Connection coreOfflineConnection;
    QMetaObject::Connection coreOnlineConnection;
    QMetaObject::Connection localeChangedConnection;
    QReadWriteLock          varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto textsRefresher() -> int;
    static auto dummy(const QStringList &) -> void;

    auto connectToCoreOffline() -> void;
    auto connectToCoreOnline() -> void;
    auto connectToLocaleChanged() -> void;
    auto defaultTheme() -> void;
    auto prayerDisplayNamesList() -> QStringList;
    auto initTexts() -> void;
    auto loadConfigs() -> void;
    auto saveConfigs() -> void;

  signals:
    auto translationChanged() -> void;
    void themeListChanged();
    void themeListIdxChanged(int);

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
    auto onLocaleChanged() -> void;
};

} // namespace Munadi