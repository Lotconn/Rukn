#pragma once

#include <QMediaPlayer>
#include <fileman>

#include <PrayerEngine.hpp>

#define mEngine Munadi::MultimediaEngine::instance()

namespace Munadi
{

class MultimediaEngine : public MunadiBase, public MunadiEngine<MultimediaEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QVariantList athanList READ getAthanList NOTIFY athanListChanged)
    Q_PROPERTY(QVariantHash athanConfig MEMBER m_athanConfig WRITE setAthanConfig NOTIFY
                   athanConfigChanged)
    Q_PROPERTY(bool athanPlaying READ isAthanPlaying NOTIFY athanPlayingChanged)
    Q_PROPERTY(bool athanMuted MEMBER m_athanMuted NOTIFY athanMutedChanged)
    Q_PROPERTY(bool previewPlaying READ isPreviewPlaying NOTIFY previewPlayingChanged)

  public:
    static auto create(QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> MultimediaEngine *;
    static auto logCategory() noexcept -> const QLoggingCategory &;

    auto getAthanConfig(PrayerEngine::Prayer prayerEnum, const QString &field) noexcept -> QVariant;
    auto isAthanPlaying() noexcept -> bool;

    Q_INVOKABLE void addAthan(const QUrl &fileUrl) noexcept;
    Q_INVOKABLE void athanPlay() noexcept;
    Q_INVOKABLE void athanStop() noexcept;
    Q_INVOKABLE void modifyAthanConfig(const QString &key, const QVariantHash &entry) noexcept;
    Q_INVOKABLE void previewPlay(int athanListIdx) noexcept;
    Q_INVOKABLE void previewStop() noexcept;
    Q_INVOKABLE void removeAthan(qsizetype rmIdx) noexcept;

  protected:
    explicit MultimediaEngine(QObject *parent = nullptr);

  private:
    Q_DISABLE_COPY_MOVE(MultimediaEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QVariantList m_athanList;
    QVariantHash m_athanConfig;
    bool         m_athanMuted;

    static constexpr int defaultAthan{0};
    static constexpr int defaultFajrAthan{2};
    static constexpr int defaultMaghribAthan{1};

    QHash<QString, int> athanConfigFields;
    QSet<QString>       athanConfigKeys;
    qsizetype           athanConfigEntrySize{};
    qsizetype           athanConfigMapSize{};
    QHash<QString, int> athanListKeys;
    qsizetype           athanListMapSize{};
    QAudioOutput       *athanOutput;
    QMediaPlayer       *athanPlayer{nullptr};
    QHash<QString, int> configFields;
    qsizetype           defaultAthans{};
    FileMan            *fileman{nullptr};
    QAudioOutput       *previewOutput;
    QMediaPlayer       *previewPlayer;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto connectToAthanAlarm() const noexcept -> void;
    auto defaultAthanList() noexcept -> void;
    auto defaultAthanConfig() noexcept -> void;
    auto defaultConfigs() noexcept -> void override;
    auto getAthanList() noexcept -> QVariantList &;
    auto isPreviewPlaying() noexcept -> bool;
    auto loadAthanConfig(bool &configMigrated) noexcept -> void;
    auto loadAthanList(bool &configMigrated) noexcept -> void;
    auto loadConfigs() noexcept -> void override;
    auto saveConfigs() noexcept -> void override;
    auto setAthanConfig(const QVariantHash &athanConfig) noexcept -> void;
    auto setAthanList(const QVariantList &athanList) noexcept -> void;

  signals:
    auto athanConfigChanged() -> void;
    auto athanListChanged() -> void;
    auto athanMutedChanged() -> void;
    auto athanPlayingChanged(bool playing) -> void;
    auto previewPlayingChanged(bool playing) -> void;

  private slots:
    void onAthanAlarm() noexcept;
    void onAthanPlaybackChanged(QMediaPlayer::PlaybackState state);
    void onCoreOnline() noexcept override;
    void onPreviewPlaybackChanged(QMediaPlayer::PlaybackState state);
};

} // namespace Munadi