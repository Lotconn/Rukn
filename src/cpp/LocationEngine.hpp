#pragma once

#include <QGeoAddress>
#include <QGeoCodeReply>
#include <QGeoCodingManager>
#include <QGeoLocation>
#include <QGeoServiceProvider>
#include <QQmlEngine>

#include <MunadiEngine.hpp>

#define lEngine Munadi::LocationEngine::instance()

namespace Munadi
{

class LocationEngine : public MunadiBase, public MunadiEngine<LocationEngine>
{
    Q_OBJECT

    // Enums must be access scoped in QML
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

    // Construct a QML module from this class
    QML_ELEMENT

    // The QML module is a singleton
    QML_SINGLETON

    // Whether a query is running in background. See `isLoading()` and `loadingChanged()` briefs
    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)

    // Whether location is set. See `isLocationSet()` and `locationChanged()` briefs
    Q_PROPERTY(bool isLocationSet READ isLocationSet NOTIFY locationChanged)

    // Provides a string table carrying all the selected location information. See
    // `getParsableLocation()` briefs.
    Q_PROPERTY(QVariantHash location READ getParsableLocation NOTIFY locationChanged)

    // Provides a list of the geocoding result returned by the last query. See
    // `getLocationResults()` brief.
    Q_PROPERTY(QStringList locationResults READ getLocationResults NOTIFY loadingChanged)

    // Stores the current selected index for the displayed `locationResults`. See
    // `m_locationResultsIndex` and `setLocation()` briefs.
    Q_PROPERTY(int locationResultsIndex MEMBER m_locationResultsIndex WRITE setLocation NOTIFY
                   loadingChanged)

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
    static LocationEngine *create(QQmlEngine * /* qmlEngine */, QJSEngine * /* jsEngine */);
    ~LocationEngine() override = default;

    /**
     * @brief Accessor needed by MunadiEngine<T> for logging purposes
     *
     * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
     */
    static const QLoggingCategory &logCategory() noexcept;

    /**
     * @brief Abort any running geocoding query, no-op if no query is running at the time. Requests
     * abort on the `geoCodeReply` member. Aborting emits the `QGeoCodeReply::abort()` signal which
     * is connected to the `onAborted()` slot (dynamically right before the abort call) handling all
     * the required procedures and cleanups. Aborting additionally emits the
     * `QGeoCodeReply::finished()` signal which also triggers the (previously) connected
     * `onFinished()` slot, however, the redundant signal is ignored there.
     */
    Q_INVOKABLE void abortQuery() const noexcept;

    /**
     * @brief Initiate a geocode query request to the class-member `manager` with the string
     * `query`. Aborts any previously running query first before initiating a new one. Returns early
     * asynchronously as the geocoding continues in background. The reply object is saved and
     * required tasks are performed when `QGeoCodeReply::finished()` signal is received, in the
     * (request-time) connected `onFinished()` slot. Also emits a `loadingChanged()` signal to
     * notify about "working" status change, which directly affects a re-read of the `isLoading`
     * property
     *
     * @param query: The string to run the query on
     */
    Q_INVOKABLE void search(const QString &query) noexcept;

    /**
     * @brief Returns
     *
     * @return const QGeoLocation&
     */
    [[nodiscard]] const QGeoLocation &getLocation() const noexcept;

  protected:
    explicit LocationEngine(QObject *parent = nullptr);

  private:
    /**
     * @brief Singleton library designed to be only one instance, not to be copied, or transferred
     * ownership to other instance.
     */
    Q_DISABLE_COPY_MOVE(LocationEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Stores the currently selected QGeoLocation object, most of the time from one of
     * `geoLocations` list
     */
    QGeoLocation m_location;

    /**
     * @brief The index choice in the displayed `locationResults` list. Since both `locationResults`
     * property (i.e. `getLocationResults()`) and `geoLocations` contain the same 1:1 index map,
     * setting this from QML implies the corresponding item in `geoLocations` list to be selected.
     */
    int m_locationResultsIndex{};

    /**
     * @brief Stores the pointer to the current reply object (QGeoCodeReply) from the initiated
     * geocode query. Qt or parent object does not manage the reply object, this engine class is
     * solely responsible for it. Should be null (and its object deleted) after finishing any query,
     * successful or not.
     *
     * IMPORTANT: Do not call `delete` on the reply object since it's not thread-safe. Use
     * `deleteLater()` instead
     */
    QPointer<QGeoCodeReply> geoCodeReply;

    /**
     * @brief Stores the result received from last successful geocode query
     */
    QList<QGeoLocation> geoLocations;

    /**
     * @brief The master geocoder construct. The QGeoCodingManager member `manager` derives from
     * this
     */
    QGeoServiceProvider geoProvider;

    /**
     * @brief Manager of all geocoding affairs. Derived from the master QGeoServiceProvider member
     * `geoProvider`
     */
    QGeoCodingManager *manager;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    /**
     * @brief Connects the `onAborted()` slot to `QGeoCodeReply::aborted()` signal emitted from the
     * object pointed at by `geoCodeReply`. The connection should be established before
     * `geoCodeReply->abort()` is called and of-course, only if `geoCodeReply` points to a valid
     * reply object. The disconnection procedure is auto-managed by Qt when the reply object is
     * deleted.
     */
    void connectToAborted() const noexcept;

    /**
     * @brief Connects the `onFinished()` slot to `QGeoCodeReply::finished()` signal emitted from
     * the object pointed at by `geoCodeReply`. The connection should be established early right
     * after `geoCodeReply` is freshly set to point to a reply object. The disconnection procedure
     * is auto-managed by Qt when the reply object is deleted.
     */
    void connectToFinished() const noexcept;

    /**
     * @brief Connects the `onLocaleChanged()` slot to `CoreEngine::localeChange()` signal for
     * synchronizing locale changes in geocoding results. Should be called in constructor.
     */
    void connectToLocaleChanged() const noexcept;

    /**
     * @brief Customized override of `MunadiBase::defaultConfigs()`. See
     * `MunadiBase::defaultConfigs()` brief
     */
    void defaultConfigs() noexcept override;

    /**
     * @brief Returns the "busy" state of geocoding task. In effect, checks whether `geoCodeReply`
     * member has any QGeoCodeReply object or not. Its object ownership are managed in
     * `onFinished()`, `onAborted()` and `search()`.
     *
     * @return true: A geocoding task is running, `geoCodeReply` has a valid pointee.
     * @return false: No geocoding tasks are running, `geoCodeReply` is a null pointer.
     */
    [[nodiscard]] bool isLoading() const noexcept;

    /**
     * @brief Whether location is currently set or not. In effect, checks whether `m_location` has a
     * valid QGeoCoordinate object in itself. Unset coordinates, latitude beyond -90 <= lat <= 90
     * and longitude beyond -180 <= long <= 180 is considered invalid. Note 0.0N, 0.0E is considered
     * as a valid coordinate.
     *
     * @return true: Coordinates of `m_location` are valid.
     * @return false: Coordinates of `m_location` are not valid.
     */
    [[nodiscard]] bool isLocationSet() const noexcept;

    /**
     * @brief Provides location results of last geocode query in an easy-to-parse displayable
     * string-list rather than the `geoLocations` content verbatim, while preserving the index map
     * exactly 1:1. Therefore, an index set to choose among this list implies the same item in
     * `geoLocations` to be chosen.
     *
     * @return QStringList: A QList<QGeoLocations>-translated list of geocode query results
     */
    [[nodiscard]] QStringList getLocationResults() const noexcept;

    /**
     * @brief Provides a hash-table of currently selected location - not the `m_location` object
     * verbatim - for easy parsing and accessing each location component in QML.
     *
     * @return QVariantHash: A hash-table containing all readable components of a QGeoLocation
     * object
     */
    [[nodiscard]] QVariantHash getParsableLocation() const noexcept;

    /**
     * @brief Customized override of `MunadiBase::loadConfigs()`. Additionally takes care of
     * old-settings migration.
     */
    void loadConfigs() noexcept override;

    /**
     * @brief Customized override of `MunadiBase::saveConfigs()`
     */
    void saveConfigs() noexcept override;

    /**
     * @brief Sets the location in `geoLocations` list as indexed by `m_locationResultsIndex` as the
     * currently used location (i.e. sets in `m_location`). Mostly used by the meta property system
     * that passes `m_locationResultsIndex` value to the parameter.
     *
     * @param index: The item in the `geoLocations` list to choose, most of the time, as set in
     * `m_locationResultsIndex`.
     */
    void setLocation(const QVariant &index) noexcept;

    /**
     * @brief Convenient helper to modify `m_location` with address and coordinates individually.
     *
     * @param address: The QGeoAddress object to update `m_location` with
     * @param coordinates: The QGeoCoordinate object to update `m_location` with
     */
    void updateLocation(const QGeoAddress &address, const QGeoCoordinate &coordinates) noexcept;

  signals:
    /**
     * @brief Notify about geocoding task "busy" state change. In GUI, this can be used to display
     * wait spins or progressbars. Closely bound to `geoCodeReply` member having an object or being
     * null.
     */
    void loadingChanged();

    /**
     * @brief Notify about location has changed, i.e. `m_location` had its value modified. Closely
     * bound to `m_location`.
     */
    void locationChanged();

  private slots:
    /**
     * @brief Handles all the post-abort procedures. This includes deleting the stale object,
     * clearing the pointer, clearing the results list and notifying about geocoding work status
     * change (to finished), by emitting the `loadingChanged()` signal.
     */
    void onAborted() noexcept;

    /**
     * @brief Customized override of `MunadiBase::onCoreOnline()`. See the brief there for details.
     */
    void onCoreOnline() noexcept override;

    /**
     * @brief Handles the procedures upon response from last geocode query. Manages refreshing the
     * results list `geoLocations`, pointer-object cleanups and failure actions. Emits the
     * `loadingChanged()` signal to notify about geocoding work status change (to finished).
     */
    void onFinished() noexcept;

    /**
     * @brief This slot handles results localization on language change according to current
     * default. Only affects new queries, previous result list contains texts in old locale. Avoids
     * direct dependency on `CoreEngine::getCurrentLocale()` for trivial task by calling
     * `QLocale{}`.
     */
    void onLocaleChanged() const noexcept;
};

} // namespace Munadi