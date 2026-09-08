#pragma once

#include <fileman>

#include <PrayerEngine.hpp>

namespace Munadi
{

class MultimediaEngine : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList athanList MEMBER m_athanList WRITE setAthanList RESET defaultAthanList
                   NOTIFY athanListChanged)
    Q_PROPERTY(QVariantList athanConfig MEMBER m_athanConfig WRITE setAthanConfig RESET
                   defaultAthanConfig NOTIFY athanConfigChanged)

  public:
    explicit MultimediaEngine(QObject *parent = nullptr);
    ~MultimediaEngine() override;

    static auto instance() -> MultimediaEngine &;

    auto getAthanConfig(PrayerEngine::Prayer prayerEnum, QAnyStringView field) -> QVariant;

    Q_INVOKABLE void addAthan(const QUrl &fileUrl);
    Q_INVOKABLE void modifyAthanConfig(PrayerEngine::Prayer prayerEnum, const QVariantMap &entry);
    Q_INVOKABLE void removeAthan(const qsizetype &rmIdx);

  private:
    Q_DISABLE_COPY_MOVE(MultimediaEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static MultimediaEngine *s_singletonInstance{nullptr};

    QVariantList m_athanList;
    QVariantList m_athanConfig;

    static constexpr int defaultAthan{0};
    static constexpr int defaultFajrAthan{2};
    static constexpr int defaultMaghribAthan{1};

    QMap<QString, int>      athanConfigKeys;
    qsizetype               athanConfigListSize;
    qsizetype               athanConfigMapSize;
    QMap<QString, int>      athanListKeys;
    qsizetype               athanListMapSize;
    QMetaObject::Connection coreOfflineConnection;
    qsizetype               defaultAthans;
    FileMan                *fileman;
    QReadWriteLock          varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto connectToCoreOffline() -> void;
    auto defaultAthanList() -> void;
    auto defaultAthanConfig() -> void;
    auto loadAthanConfig(bool &configMigrated) -> void;
    auto loadAthanList(bool &configMigrated) -> void;
    auto loadConfigs() -> void;
    auto saveConfigs() -> void;
    auto setAthanConfig(const QVariantList &athanConfig) -> void;
    auto setAthanList(const QVariantList &athanList) -> void;

  signals:
    auto athanConfigChanged(QVariantList athanConfig) -> void;
    auto athanListChanged(QVariantList athanList) -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
};

} // namespace Munadi