#pragma once

#include <QJsonArray>
#include <QLockFile>
#include <QMutex>
#include <QQmlEngine>
#include <QSettings>
#include <QTranslator>

#include <MunadiEngine.hpp>

#define core Munadi::CoreEngine::instance()

namespace Munadi
{

class CoreEngine : public MunadiEngine<CoreEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QLocale appLocale READ getCurrentLocale NOTIFY localeChanged)

  public:
    explicit CoreEngine(QObject *parent = nullptr);

    static auto logCategory() noexcept -> const QLoggingCategory &;

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

  protected:
    bool eventFilter(QObject *object, QEvent *event) noexcept override;

  private:
    Q_DISABLE_COPY_MOVE(CoreEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static const QHash<QString, QLocale> s_lang{
        { QLatin1StringView("System"),                   QLocale::system()},
        {QLatin1StringView("English"), QLocale{QLatin1StringView{"en_US"}}},
        { QLatin1StringView("Arabic"), QLocale{QLatin1StringView{"ar_SA"}}},
        { QLatin1StringView("Bangla"), QLocale{QLatin1StringView{"bn_BD"}}},
    };

    QLockFile  *configLock{nullptr};
    QSettings  *oldSettings{nullptr};
    QSettings  *settings{nullptr};
    QMutex      threadLock;
    QTranslator translator;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static auto getCurrentLocale() noexcept -> QLocale;
    static auto readValueOfKey(QSettings &settings, QAnyStringView key,
                               const QVariant &defaultValue = {}) noexcept -> QVariant;
    static auto setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                              bool flushOnWrite = false) noexcept -> void;

    // auto acquireConfigLock() -> int;
    auto applyLocale(const QLocale &locale = QLocale::system()) noexcept -> void;
    // auto releaseConfigLock() -> void;

  signals:
    auto localeChanged() -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onAboutToQuit() noexcept -> void;
};

} // namespace Munadi