#pragma once

#include <QJsonArray>
#include <QLockFile>
#include <QQmlEngine>
#include <QSet>
#include <QSettings>
#include <QSocketNotifier>
#include <QTranslator>

#include <MunadiEngine.hpp>
#include <alarm.h>
#include <version.hpp>

#define core Munadi::CoreEngine::instance()

namespace Munadi
{

class CoreEngine : public MunadiBase, public MunadiEngine<CoreEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QLocale appLocale READ getCurrentLocale NOTIFY localeChanged)

  public:
    explicit CoreEngine(QObject *parent = nullptr);

    static auto logCategory() noexcept -> const QLoggingCategory &;
    static auto termSignalHandler(int /* unused */) -> void;

    Q_INVOKABLE static QString getAppVersion() noexcept;

    [[nodiscard]] auto alarmManager() const noexcept -> Alarm &;
    auto readOldSettings(QAnyStringView oldKey, const QVariant &defaultValue = {}) noexcept
        -> QVariant;
    auto readOldSettingsJsArray(QAnyStringView oldKey, const QJsonArray &defaultArray = {}) noexcept
        -> QVariantList;
    auto readSettings(QAnyStringView key, const QVariant &defaultValue = {}) noexcept -> QVariant;
    auto readSettingsArray(QAnyStringView group) noexcept -> QVariantList;
    // auto removeOldSettingsKey(QAnyStringView key) noexcept -> bool;
    auto setLang(const QString &lang) noexcept -> void;
    auto writeSettings(QAnyStringView key, const QVariant &value,
                       bool flushOnWrite = false) noexcept -> void;
    auto writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                            bool flushOnWrite = false) noexcept -> void;

    Q_INVOKABLE [[nodiscard]] bool isVersionNew() const noexcept;
    Q_INVOKABLE void               set60sTicker() noexcept;

  public slots: // NOLINT(*redundant-access-specifiers)
    void onReceivedMessage(quint32 instanceId, const QByteArray &message);
    void onUnixSigTerm();

  protected:
    bool eventFilter(QObject *object, QEvent *event) noexcept override;

  private:
    Q_DISABLE_COPY_MOVE(CoreEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static const QString                 s_appVersion{QStringLiteral(APP_VERSION)};
    inline static const QHash<QString, QLocale> s_lang{
        {u"System"_s,  QLocale::system()  },
        {u"English"_s, QLocale{u"en_US"_s}},
        {u"Arabic"_s,  QLocale{u"ar_SA"_s}},
        {u"Bangla"_s,  QLocale{u"bn_BD"_s}},
    };

    static const QHash<QString, std::function<void(CoreEngine *)>> doTrigger;

    inline static std::array<int, 2> sigtermFd{0, 0};

    bool m_isVersionNew{true};

    Alarm           *alarm{};
    QLockFile       *configLock{};
    QSettings       *oldSettings{};
    QSettings       *settings{};
    QSocketNotifier *snTerm;
    QTranslator      translator;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto installUnixSignalHandlers() -> int;
    static auto getCurrentLocale() noexcept -> QLocale;
    static auto readValueOfKey(QSettings &settings, QAnyStringView key,
                               const QVariant &defaultValue = {}) noexcept -> QVariant;
    static auto setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                              bool flushOnWrite = false) noexcept -> void;

    // auto acquireConfigLock() -> int;
    auto applyLocale(const QLocale &locale = QLocale::system()) noexcept -> void;
    auto connectToAboutToQuit() const noexcept -> void;
    auto connectToTrigger() const noexcept -> void;
    auto installTickers() noexcept -> void;
    auto loadConfigs() noexcept -> void override;
    // auto releaseConfigLock() -> void;
    auto saveConfigs() noexcept -> void override;

  signals:
    auto alarmsReset() -> void;
    auto athanAlarm() -> void;
    auto dateChanged() -> void;
    auto exposeInstance() -> void;
    auto minuteChanged() -> void;
    auto localeChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    void onAboutToQuit() noexcept;
    void onCoreOnline() noexcept override {};
    void onTrigger(const QString &id) noexcept;
};

} // namespace Munadi