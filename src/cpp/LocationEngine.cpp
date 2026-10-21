#include "internal-helpers.hpp"
#include <LocationEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "LocationEngine")

namespace Munadi
{

LocationEngine::LocationEngine(QObject *parent)
    : MunadiEngine<LocationEngine>{parent}, geoProvider{"osm"_L1},
      manager{this->geoProvider.geocodingManager()}
{
    this->bootEngine();

    if (this->geoProvider.error() != QGeoServiceProvider::NoError)
        logCritical(munadi) << "Geocoding unavailable: [" << this->geoProvider.error() << "] "
                            << this->geoProvider.errorString();
    else
    {
        logDebug(munadi) << "Geolocation provider: " << this->manager->managerName()
                         << "; version: " << this->manager->managerVersion();
        logDebug(munadi) << "Using locale: " << this->manager->locale();
        logDebug(munadi) << "Features set: "
                         << QMetaEnum{QMetaEnum::fromType<QGeoServiceProvider::GeocodingFeatures>()}
                                .valueToKeys(this->geoProvider.geocodingFeatures());
    }

    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    this->makeOnline();
}

const QLoggingCategory &LocationEngine::logCategory() noexcept
{ return munadi(); }

void LocationEngine::abortQuery() noexcept
{
    {
        const QReadLocker lock{&this->varLock};

        if (this->geoCodeReply.isNull())
        {
            logInfo(munadi) << "No previous query running, nothing to abort.";
            return;
        }
    }

    this->connectToAborted();
    this->geoCodeReply->abort();
}

void LocationEngine::search(const QString &query) noexcept
{
    logDebug(munadi) << "Attempting geocoding with query: " << query;

    this->abortQuery();

    logDebug(munadi) << "Dispatching geocoder workers...";

    QGeoCodeReply *reply{this->manager->geocode(query)};

    {
        const QWriteLocker lock{&this->varLock};
        this->geoCodeReply = reply;
        this->connectToFinished();
    }

    // Handle early failures, otherwise `geocode()` already is working async in background
    if (reply->isFinished())
    {
        logWarning(munadi) << "Reply received too early, unknown failure expected";
        // Reminder: onFinished() has a QWriteLocker
        onFinished();
    }
    else
        emit this->loadingChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void LocationEngine::connectToAborted() const noexcept
{
    if (const auto conn = QObject::connect(this->geoCodeReply, &QGeoCodeReply::aborted, this,
                                           &LocationEngine::onAborted);
        conn == nullptr)
        logWarning(munadi) << "GeoCoder aborted signal not connected";
    else
        logDebug(munadi) << "GeoCoder aborted signal connected";
}

void LocationEngine::connectToFinished() const noexcept
{
    if (const auto conn = QObject::connect(this->geoCodeReply, &QGeoCodeReply::finished, this,
                                           &LocationEngine::onFinished);
        conn == nullptr)
        logWarning(munadi) << "GeoCoder finished signal not connected";
    else
        logDebug(munadi) << "GeoCoder finished signal connected";
}

void LocationEngine::connectToLocaleChanged() const noexcept
{
    if (const auto conn = QObject::connect(core, &CoreEngine::localeChanged, this,
                                           &LocationEngine::onLocaleChanged);
        conn == nullptr)
        logWarning(munadi) << "localeChanged signal not connected";
    else
        logDebug(munadi) << "localeChanged signal connected";
}

void LocationEngine::defaultConfigs() noexcept
{
    const QWriteLocker lock{&this->varLock};

    this->m_location = {};
    this->m_locationResultsIndex.storeRelaxed(0);
}

bool LocationEngine::isLoading() noexcept
{
    const QReadLocker lock{&this->varLock};

    return !this->geoCodeReply.isNull();
}

bool LocationEngine::isLocationSet() noexcept
{
    const QReadLocker lock{&this->varLock};

    return !this->m_location.isEmpty();
}

QVariantHash LocationEngine::getCurrentLocation() noexcept
{
    const QReadLocker lock{&this->varLock};

    return QVariantHash{
        {    "address"_L1,         this->m_location.address().text()},
        {  "streetNum"_L1, this->m_location.address().streetNumber()},
        {     "street"_L1,       this->m_location.address().street()},
        {       "city"_L1,         this->m_location.address().city()},
        {   "district"_L1,     this->m_location.address().district()},
        {      "state"_L1,        this->m_location.address().state()},
        {    "country"_L1,      this->m_location.address().country()},
        {"countryCode"_L1,  this->m_location.address().countryCode()},
        { "postalCode"_L1,   this->m_location.address().postalCode()},
        {   "latitude"_L1,  this->m_location.coordinate().latitude()},
        {  "longitude"_L1, this->m_location.coordinate().longitude()},
    };
}

QStringList LocationEngine::getLocationResults() noexcept
{
    const QReadLocker lock{&this->varLock};

    if (this->geoLocations.isEmpty())
        logWarning(munadi) << "Geocoding search result is empty";

    return this->geoLocations | std::views::transform([](const QGeoLocation &loc) -> QString {
               return loc.address().text();
           }) |
           std::ranges::to<QStringList>();
}

void LocationEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool          configMigrated{true};
    const QString migrateKey{"LocationEngineConfigMigrated"_L1};

    // Adding a QReadLocker/QWriteLocker here is strongly discouraged. Acquiring a locker here may
    // deadlock. Put it inside setter functions if necessary.

    loadConfigKey({}, "Location/Address"_L1, {}, [this](const QVariant &address) -> void {
        const QWriteLocker lock{&this->varLock};

        this->address.setText(address.toString());
    });

    loadConfigKey(
        "Adhan/cityName"_L1, {}, migrateKey,
        [this](const QVariant &city) -> void {
            const QWriteLocker lock{&this->varLock};

            this->address.setCity(city.toString());
        },
        configMigrated);

    loadConfigKey(
        "Adhan/countryName"_L1, {}, migrateKey,
        [this](const QVariant &country) -> void {
            const QWriteLocker lock{&this->varLock};

            this->address.setCountry(country.toString());
        },
        configMigrated);

    loadConfigKey(
        "Adhan/latitude"_L1, "Location/Latitude"_L1, migrateKey,
        [this](const QVariant &latitude) -> void {
            const QWriteLocker lock{&this->varLock};

            this->coordinates.setLatitude(latitude.toDouble());
        },
        configMigrated);

    loadConfigKey(
        "Adhan/longitude"_L1, "Location/Longitude"_L1, migrateKey,
        [this](const QVariant &longitude) -> void {
            const QWriteLocker lock{&this->varLock};

            this->coordinates.setLongitude(longitude.toDouble());
        },
        configMigrated);

    this->updateLocation();

    if (configMigrated)
        core->writeSettings("LocationEngineConfigMigrated"_L1, true);
}

void LocationEngine::saveConfigs() noexcept
{
    const QReadLocker lock{&this->varLock};

    logDebug(munadi) << "Saving location configurations";

    core->writeSettings("Location/Address"_L1, this->m_location.address().text());
    core->writeSettings("Location/Latitude"_L1, this->m_location.coordinate().latitude());
    core->writeSettings("Location/Longitude"_L1, this->m_location.coordinate().longitude());
}

void LocationEngine::setLocation(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;

    {
        const QWriteLocker lock{&this->varLock};

        if (!checkIndex(index, this->geoLocations, this->m_locationResultsIndex))
            return;

        this->m_locationResultsIndex.storeRelaxed(index.toInt());

        if (this->m_location.coordinate() ==
            this->geoLocations.at(this->m_locationResultsIndex).coordinate())
        {
            logDebug(munadi) << "Skipping setting identical location";
            return;
        }

        logDebug(munadi) << "Replacing previous location: " << this->m_location.address().text()
                         << "\ncoordinates: " << this->m_location.coordinate().toString();

        this->m_location = this->geoLocations.at(this->m_locationResultsIndex);

        logDebug(munadi) << "New location: " << this->m_location.address().text()
                         << "\ncoordinates: " << this->m_location.coordinate().toString();
    }

    emit this->locationChanged();
}

void LocationEngine::updateLocation() noexcept
{
    logDebug(munadi) << "Updating address to: " << this->address.text();
    logDebug(munadi) << "Updating location to: " << this->coordinates.toString();

    if (this->m_location.address() != this->address)
        this->m_location.setAddress(this->address);
    else
        logDebug(munadi) << "Skipping setting identical address";

    if (this->m_location.coordinate() != this->coordinates)
        this->m_location.setCoordinate(this->coordinates);
    else
        logDebug(munadi) << "Skipping setting identical coordinates";

    this->address.setText({});
    emit this->locationChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void LocationEngine::onAborted() noexcept
{
    logInfo(munadi) << "Previous query aborted";

    {
        const QWriteLocker lock{&this->varLock};

        if (!this->geoCodeReply.isNull())
            this->geoCodeReply->deleteLater();

        this->geoCodeReply.clear();
        this->geoLocations.clear();
        this->geoLocations.squeeze();
    }

    emit this->loadingChanged();
}

void LocationEngine::onCoreOnline() noexcept
{
    this->connectToLocaleChanged();
    MunadiBase::onCoreOnline();
}

void LocationEngine::onFinished() noexcept
{
    if (this->geoCodeReply.isNull())
    {
        logDebug(munadi) << "Late signal from an aborted or replaced query. Ignoring...";
        return;
    }

    {
        const QWriteLocker lock{&this->varLock};

        logDebug(munadi) << "Working on geocode query...";

        this->geoLocations.clear();
        this->geoLocations.squeeze();

        switch (this->geoCodeReply->error())
        {
        case QGeoCodeReply::NoError:
            logDebug(munadi) << "No error found in current reply";
            this->geoLocations = this->geoCodeReply->locations();
            break;
        default:
            logCritical(munadi) << "Could not geocode provided query";
            logDebug(munadi) << this->geoCodeReply->errorString();
            break;
        }

        this->geoCodeReply->deleteLater();
        this->geoCodeReply.clear();
    }

    emit this->loadingChanged();
}

void LocationEngine::onLocaleChanged() noexcept
{
    const QWriteLocker lock{&this->varLock};

    logDebug(munadi) << "Detected language change, refreshing locale...";

    this->manager->setLocale(core->property("locale").toLocale());

    logDebug(munadi) << "Using locale: " << this->manager->locale();
}

} // namespace Munadi