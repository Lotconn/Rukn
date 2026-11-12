#pragma once

#include <QNetworkReply>
#include <QQmlEngine>

#include <AppEngine.hpp>

#define uEngine App::UpdateEngine::instance()

namespace App
{

class UpdateEngine : public AppBase, public AppEngine<UpdateEngine>
{
    Q_OBJECT

    // Enums must be access scoped in QML
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    // Construct a QML module from this class
    QML_ELEMENT

    // The QML module is a singleton
    QML_SINGLETON

    Q_PROPERTY(bool checkUpdates MEMBER m_checkUpdates NOTIFY checkUpdatesChanged)

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
    static UpdateEngine *create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */);
    ~UpdateEngine() override = default;

    /**
     * @brief Accessor needed by AppEngine<T> for logging purposes
     *
     * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
     */
    static const QLoggingCategory &logCategory() noexcept;

    auto updateCheck() const noexcept -> void;

  protected:
    explicit UpdateEngine(QObject *parent = nullptr);

  private:
    /**
     * @brief Singleton library designed to be only one instance, not to be copied, or transferred
     * ownership to other instance.
     */
    Q_DISABLE_COPY_MOVE(UpdateEngine)

    bool m_checkUpdates{};

    inline static const QUrl apiUrl{"https://lotconn.org/"_L1};

    QNetworkAccessManager *netMan;

    auto connectToNetMan() const noexcept -> void;

    /**
     * @brief Customized override of `AppBase::defaultConfigs()`. See
     * `AppBase::defaultConfigs()` brief
     */
    auto defaultConfigs() noexcept -> void override;

    /**
     * @brief Customized override of `AppBase::loadConfigs()`. Additionally takes care of
     * old-settings migration.
     */
    // void loadConfigs() noexcept override;

    /**
     * @brief Customized override of `AppBase::saveConfigs()`
     */
    // void saveConfigs() noexcept override;

  signals:
    void checkUpdatesChanged();
    void updateAvailable(const QString &info);

  private slots:
    void onFinished(QNetworkReply *reply);
};

} // namespace App