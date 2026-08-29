#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QReadWriteLock>

namespace Munadi
{
class CoreEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(int localeGuard MEMBER m_localeChangeCount NOTIFY localeChanged)
    Q_PROPERTY(QVariantList audioList MEMBER m_audioList WRITE setAudioList NOTIFY audioListChanged)
    Q_PROPERTY(QVariantList prayerConfig MEMBER m_prayerConfig WRITE setPrayerConfig NOTIFY
                   prayerConfigChanged)

  signals:
    auto audioListChanged(const QVariantList &audioList) -> void;
    auto localeChanged() -> void;
    auto prayerConfigChanged(const QVariantList &prayerConfig) -> void;

  public:
    explicit CoreEngine(QObject *parent = nullptr);
    ~CoreEngine() override;

    static auto create(QQmlEngine *, QJSEngine *) -> CoreEngine *;
    static auto instance() -> CoreEngine &;

    auto emitLocaleChanged() -> void;

    Q_INVOKABLE auto addAthan(const QUrl &fileUrl, int fileDialogCategory) -> void;
    Q_INVOKABLE auto modifyPrayerConfig(int prayerIndex, const QString &field,
                                        const QVariant &value) -> void;

    Q_INVOKABLE auto removeAthan(int selectedIndex) -> void;

  private:
    Q_DISABLE_COPY_MOVE(CoreEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    static CoreEngine *s_singletonInstance;

    qsizetype      defaultAudioCount;
    QVariantList   m_audioList;
    QAtomicInt     m_localeChangeCount;
    QVariantList   m_prayerConfig;
    QReadWriteLock varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    [[nodiscard]] static auto isValidAudioEntry(const QVariantMap &entry) -> bool;

  private slots:
    auto setAudioList(const QVariantList &audioList) -> void;
    auto setPrayerConfig(const QVariantList &prayerConfig) -> void;
};
} // namespace Munadi