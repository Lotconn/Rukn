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

class LocationEngine : public MunadiEngine<LocationEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool isLoading READ isLoading NOTIFY loadingChanged)
    Q_PROPERTY(bool isLocationSet READ isLocationSet NOTIFY locationChanged)
    Q_PROPERTY(QVariantHash location READ getParsableLocation NOTIFY locationChanged)
    Q_PROPERTY(QStringList locationResults READ getLocationResults NOTIFY loadingChanged)
    Q_PROPERTY(int locationResultsIndex MEMBER m_locationResultsIndex WRITE setLocation NOTIFY
                   loadingChanged)

  public:
    explicit LocationEngine(QObject *parent = nullptr);

    static auto logCategory() noexcept -> const QLoggingCategory &;

    Q_INVOKABLE void abortQuery() noexcept;
    Q_INVOKABLE void search(const QString &query) noexcept;

    auto getLocation() noexcept -> const QGeoLocation &;

  private:
    Q_DISABLE_COPY_MOVE(LocationEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QGeoLocation m_location;
    QAtomicInt   m_locationResultsIndex;

    QGeoAddress             address;
    QGeoCoordinate          coordinates;
    QPointer<QGeoCodeReply> geoCodeReply;
    QList<QGeoLocation>     geoLocations;
    QGeoServiceProvider     geoProvider;
    QGeoCodingManager      *manager;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto connectToAborted() const noexcept -> void;
    auto connectToFinished() const noexcept -> void;
    auto connectToLocaleChanged() const noexcept -> void;
    auto defaultConfigs() noexcept -> void override;
    auto isLoading() noexcept -> bool;
    auto isLocationSet() noexcept -> bool;
    auto getLocationResults() noexcept -> QStringList;
    auto getParsableLocation() noexcept -> QVariantHash;
    auto loadConfigs() noexcept -> void override;
    auto saveConfigs() noexcept -> void override;
    auto setLocation(const QVariant &index) noexcept -> void;
    auto updateLocation() noexcept -> void;

  signals:
    auto loadingChanged() -> void;
    auto locationChanged() -> void;

  private slots:
    void onAborted() noexcept;
    void onCoreOnline() noexcept override;
    void onFinished() noexcept;
    void onLocaleChanged() noexcept;
};

} // namespace Munadi