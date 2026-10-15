#pragma once

#include <QJsonArray>
#include <QLockFile>
#include <QMutex>
#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>
#include <QSettings>
#include <QTranslator>

#define core Munadi::CoreEngine::instance()

namespace Munadi
{

class CoreEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

  public:
    explicit CoreEngine(QObject *parent = nullptr);
    ~CoreEngine() override;

    static auto               instance() noexcept -> CoreEngine *;
    static auto               instanceExists() noexcept -> bool;
    [[nodiscard]] static auto isOnline() noexcept -> bool;
    static auto               onceOnline(QObject *context, std::function<void()> slot,
                                         const QLoggingCategory &category,
                                         int retriesLeft = 19) -> void; // NOLINT(*-magic-numbers)

    auto readOldSettings(QAnyStringView oldKey, const QVariant &defaultValue = {}) -> QVariant;
    auto readOldSettingsJsArray(QAnyStringView oldKey, const QJsonArray &defaultArray = {})
        -> QVariantList;
    auto readSettings(QAnyStringView key, const QVariant &defaultValue = {}) -> QVariant;
    auto readSettingsArray(QAnyStringView group) -> QVariantList;
    // auto removeOldSettingsKey(QAnyStringView key) -> bool;
    auto setLang(const QString &lang) -> void;
    auto writeSettings(QAnyStringView key, const QVariant &value, bool flushOnWrite = true) -> void;
    auto writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                            bool flushOnWrite = true) -> void;

  protected:
    bool eventFilter(QObject *object, QEvent *event) override;

  private:
    Q_DISABLE_COPY_MOVE(CoreEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static QPointer<CoreEngine> s_singletonInstance;
    inline static bool                 s_online{false};

    inline static const QHash<QString, QLocale> s_lang{
        { QLatin1StringView("System"),                   QLocale::system()},
        {QLatin1StringView("English"), QLocale{QLatin1StringView{"en_US"}}},
        { QLatin1StringView("Arabic"), QLocale{QLatin1StringView{"ar_SA"}}},
        { QLatin1StringView("Bangla"), QLocale{QLatin1StringView{"bn_BD"}}},
    };

    // QString m_langKey;

    QLockFile     *configLock{nullptr};
    QSettings     *oldSettings{nullptr};
    QSettings     *settings{nullptr};
    QMutex         threadLock;
    QTranslator    translator;
    QReadWriteLock varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto readValueOfKey(QSettings &settings, QAnyStringView key,
                               const QVariant &defaultValue = {}) -> QVariant;
    static auto setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                              bool flushOnWrite = false) -> void;

    // auto acquireConfigLock() -> int;
    auto applyLocale(const QLocale &locale = QLocale::system()) -> void;
    // auto releaseConfigLock() -> void;

  signals:
    auto coreOffline() -> void;
    auto coreOnline() -> void;
    auto localeChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onAboutToQuit() -> void;
};

} // namespace Munadi