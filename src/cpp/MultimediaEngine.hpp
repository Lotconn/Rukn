#pragma once

#include <QMediaPlayer>
#include <fileman>

#include <PrayerEngine.hpp>

#define mEngine App::MultimediaEngine::instance()

namespace App
{

class MultimediaEngine : public AppBase, public AppEngine<MultimediaEngine>
{
    Q_OBJECT

    // Enums must be access scoped in QML
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    // Construct a QML module from this class
    QML_ELEMENT

    // The QML module is a singleton
    QML_SINGLETON

    Q_PROPERTY(QVariantList adhanList READ getAdhanList NOTIFY adhanListChanged)
    Q_PROPERTY(QVariantHash adhanConfig MEMBER m_adhanConfig WRITE setAdhanConfig NOTIFY
                   adhanConfigChanged)
    Q_PROPERTY(bool adhanPlaying READ isAdhanPlaying NOTIFY adhanPlayingChanged)
    Q_PROPERTY(bool adhanMuted MEMBER m_adhanMuted NOTIFY adhanMutedChanged)
    Q_PROPERTY(bool previewPlaying READ isPreviewPlaying NOTIFY previewPlayingChanged)

  public:
    /**
     * @brief The QML factory, creating the singleton as a QML module, called by QmlEngine itself.
     * The created instance therefore is parented by the QmlEngine which can be accessed through
     * `qmlEngine`. This engine class here can request the QML engine to instantiate any other
     * dependent engine classes it requires through a call to
     * `qmlEngine->singletonInstance<OtherEngine *>()` which too would be parented by the QmlEngine,
     * avoiding loads of recurred problems, one among which is nested parenting if the dependent
     * engines would be instantiated right in this engine's constructor. However, QmlEngine prefers
     * the default-constructible `public` constructor over this if its available, so this never gets
     * called if it's present. The class constructor therefore needs to be hidden from `public` -
     * preferrably in `protected` segment for QmlEngine to call this factory. For reducing redundant
     * codes and actually using the class constructor preserving all the expected behaviors as
     * usual, this factory then needs to act as a wrapper around the "hidden" constructor which
     * artfully accomplishes both goals. Returns an instance of the engine class - in here, by
     * calling the `protected` constructor.
     *
     * @param qmlEngine: The QmlEngine instance, passed by the QmlEngine during its call
     * @param jsEngine: Present only for historical purposes, everything is done through the
     * QmlEngine instance these days. Not used here.
     * @return LocationEngine*: The class instance
     */
    static MultimediaEngine *create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */);
    ~MultimediaEngine() override = default;

    /**
     * @brief Accessor needed by AppEngine<T> for logging purposes
     *
     * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
     */
    static const QLoggingCategory &logCategory() noexcept;

    Q_INVOKABLE void addAdhan(const QUrl &fileUrl) noexcept;
    Q_INVOKABLE void adhanPlay() noexcept;
    Q_INVOKABLE void adhanStop() noexcept;
    QVariant         getAdhanConfig(PrayerEngine::Prayer prayerEnum, const QString &field) noexcept;
    bool             isAdhanPlaying() noexcept;
    Q_INVOKABLE void modifyAdhanConfig(const QString &key, const QVariantHash &entry) noexcept;
    Q_INVOKABLE void previewPlay(int adhanListIdx) noexcept;
    Q_INVOKABLE void previewStop() noexcept;
    Q_INVOKABLE void removeAdhan(qsizetype rmIdx) noexcept;

  protected:
    explicit MultimediaEngine(QObject *parent = nullptr);

  private:
    /**
     * @brief Singleton library designed to be only one instance, not to be copied, or transferred
     * ownership to other instance.
     */
    Q_DISABLE_COPY_MOVE(MultimediaEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QVariantList m_adhanList;
    QVariantHash m_adhanConfig;
    bool         m_adhanMuted{};

    static constexpr int defaultAdhan{0};
    static constexpr int defaultFajrAdhan{2};
    static constexpr int defaultMaghribAdhan{1};

    QHash<QString, int> adhanConfigFields;
    QSet<QString>       adhanConfigKeys;
    qsizetype           adhanConfigEntrySize{};
    qsizetype           adhanConfigMapSize{};
    QHash<QString, int> adhanListKeys;
    qsizetype           adhanListMapSize{};
    QAudioOutput       *adhanOutput;
    QMediaPlayer       *adhanPlayer{nullptr};
    QHash<QString, int> configFields;
    qsizetype           defaultAdhans{};
    FileMan            *fileman{nullptr};
    QAudioOutput       *previewOutput;
    QMediaPlayer       *previewPlayer;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    void connectToAdhanAlarm() const noexcept;
    void defaultAdhanList() noexcept;
    void defaultAdhanConfig() noexcept;

    /**
     * @brief Customized override of `AppBase::defaultConfigs()`. See
     * `AppBase::defaultConfigs()` brief
     */
    void          defaultConfigs() noexcept override;
    QVariantList &getAdhanList() noexcept;
    bool          isPreviewPlaying() noexcept;
    void          loadAdhanConfig(bool &configMigrated) noexcept;
    void          loadAdhanList(bool &configMigrated) noexcept;

    /**
     * @brief Customized override of `AppBase::loadConfigs()`. Additionally takes care of
     * old-settings migration.
     */
    void loadConfigs() noexcept override;

    /**
     * @brief Customized override of `AppBase::saveConfigs()`
     */
    void saveConfigs() noexcept override;
    void setAdhanConfig(const QVariantHash &adhanConfig) noexcept;
    void setAdhanList(const QVariantList &adhanList) noexcept;

  signals:
    void adhanConfigChanged();
    void adhanListChanged();
    void adhanMutedChanged();
    void adhanPlayingChanged(bool playing);
    void previewPlayingChanged(bool playing);

  private slots:
    void onAdhanAlarm() noexcept;
    void onAdhanPlaybackChanged(QMediaPlayer::PlaybackState state);
    /**
     * @brief Customized overridden extension of `AppBase::onCoreOnline`. See its brief for
     * details
     */
    void onCoreOnline() noexcept override;
    void onPreviewPlaybackChanged(QMediaPlayer::PlaybackState state);
};

} // namespace App