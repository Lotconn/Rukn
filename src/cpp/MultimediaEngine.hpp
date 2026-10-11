#pragma once

#include <QMediaPlayer>
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
    Q_PROPERTY(QVariantHash athanConfig MEMBER m_athanConfig WRITE setAthanConfig RESET
                   defaultAthanConfig NOTIFY athanConfigChanged)
    Q_PROPERTY(bool athanPlaying READ isAthanPlaying NOTIFY athanPlayingChanged)
    Q_PROPERTY(bool athanMuted MEMBER m_athanMuted NOTIFY athanMutedChanged)
    Q_PROPERTY(bool previewPlaying READ isPreviewPlaying NOTIFY previewPlayingChanged)

  public:
    explicit MultimediaEngine(QObject *parent = nullptr);
    ~MultimediaEngine() override;

    static auto instance() noexcept -> MultimediaEngine *;

    auto getAthanConfig(PrayerEngine::Prayer prayerEnum, QAnyStringView field) -> QVariant;

    Q_INVOKABLE void addAthan(const QUrl &fileUrl);
    Q_INVOKABLE void athanPlay();
    Q_INVOKABLE void athanStop();
    Q_INVOKABLE void modifyAthanConfig(const QString &key, const QVariantHash &entry);
    Q_INVOKABLE void previewPlay(int athanListIdx);
    Q_INVOKABLE void previewStop();
    Q_INVOKABLE void removeAthan(const qsizetype &rmIdx);

  private:
    Q_DISABLE_COPY_MOVE(MultimediaEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static QPointer<MultimediaEngine> s_singletonInstance;

    QVariantList      m_athanList;
    QVariantHash      m_athanConfig;
    std::atomic<bool> m_athanMuted{false};

    static constexpr int defaultAthan{0};
    static constexpr int defaultFajrAthan{2};
    static constexpr int defaultMaghribAthan{1};

    QHash<QString, int> athanConfigFields;
    QSet<QString>       athanConfigKeys;
    qsizetype           athanConfigEntrySize;
    qsizetype           athanConfigMapSize;
    QHash<QString, int> athanListKeys;
    qsizetype           athanListMapSize;
    QAudioOutput       *athanOutput;
    QMediaPlayer       *athanPlayer{nullptr};
    QHash<QString, int> configFields;
    qsizetype           defaultAthans;
    FileMan            *fileman{nullptr};
    QAudioOutput       *previewOutput;
    QMediaPlayer       *previewPlayer;
    QReadWriteLock      varLock;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto defaultAthanList() -> void;
    auto defaultAthanConfig() -> void;
    auto isAthanPlaying() -> bool;
    auto isPreviewPlaying() -> bool;
    auto loadAthanConfig(bool &configMigrated) -> void;
    auto loadAthanList(bool &configMigrated) -> void;
    auto loadConfigs() -> void;
    auto saveConfigs() -> void;
    auto setAthanConfig(const QVariantHash &athanConfig) -> void;
    auto setAthanList(const QVariantList &athanList) -> void;

  signals:
    auto athanConfigChanged(QVariantHash athanConfig) -> void;
    auto athanListChanged(QVariantList athanList) -> void;
    auto athanMutedChanged(bool athanMuted) -> void;
    auto athanPlayingChanged(bool playing) -> void;
    auto previewPlayingChanged(bool playing) -> void;

  private slots: // NOLINT(*-redundant-access-specifiers)
    auto onCoreOffline() -> void;
    auto onCoreOnline() -> void;
};

} // namespace Munadi