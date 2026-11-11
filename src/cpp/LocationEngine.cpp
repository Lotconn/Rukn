#include "internal-helpers.hpp"
#include <LocationEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "LocationEngine")

namespace Munadi
{

LocationEngine *LocationEngine::create(QQmlEngine * /* qmlEngine */, QJSEngine * /* jsEngine */)
{
    logDebug(munadi) << "Creating instance from QML call";

    // CoreEngine is already set to init from main.cpp

    return new LocationEngine{};
}

const QLoggingCategory &LocationEngine::logCategory() noexcept
{ return munadi(); }

void LocationEngine::abortQuery() const noexcept
{
    if (this->geoCodeReply.isNull())
    {
        logInfo(munadi) << "No previous query running, nothing to abort.";
        return;
    }

    this->connectToAborted();
    this->geoCodeReply->abort(); // emits QGeoCodeReply::aborted() signal now
}

void LocationEngine::search(const QString &query) noexcept
{
    logDebug(munadi) << "Attempting geocoding with query: " << query;

    // Previous query must be aborted to avoid multiple queries simultaneously, all of which will be
    // drowned anyway otherwise except the first one to arrive
    this->abortQuery();

    logDebug(munadi) << "Dispatching geocoder workers...";

    // QGeoCodingManager::geocode() call is async in background
    this->geoCodeReply = this->manager->geocode(query);

    this->connectToFinished();

    // Handle early failures, reconcile "by-hand". Otherwise `geocode()` already is working in
    // background
    if (this->geoCodeReply->isFinished())
    {
        logWarning(munadi) << "Reply received too early, unknown failure expected";
        onFinished();
    }
    else
        emit this->loadingChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

LocationEngine::LocationEngine(QObject *parent)
    : MunadiBase{munadi(), parent}, geoProvider{u"osm"_s},
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
    // QGeoCodeReply::finished() is much easier to work with than QGeoCodingManager::finished() for
    // our purposes
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
    this->m_location             = {};
    this->m_locationResultsIndex = 0;
}

bool LocationEngine::isLoading() const noexcept
{ return !this->geoCodeReply.isNull(); }

bool LocationEngine::isLocationSet() const noexcept
{ return this->m_location.coordinate().isValid(); }

const QGeoLocation &LocationEngine::getLocation() const noexcept
{
    // Either make this function a R/W accessor with warning logs, or a read accessor with
    // returning location object on invalidity

    if (!this->isLocationSet())
        logCritical(munadi) << "Location isn't set, object is invalid";

    return this->m_location;
}

QStringList LocationEngine::getLocationResults() const noexcept
{
    if (this->geoLocations.isEmpty())
        logWarning(munadi) << "Geocoding search result is empty";

    // Make a string-list from the full-address text which is more faithful representation of the
    // original QGeoLocation object
    return this->geoLocations | std::views::transform([](const QGeoLocation &loc) -> QString {
               return loc.address().text();
           }) |
           std::ranges::to<QStringList>();
}

QVariantHash LocationEngine::getParsableLocation() const noexcept
{
    // Contains all the components of QGeoLocation object
    return QVariantHash{
        {u"address"_s,     this->m_location.address().text()        },
        {u"streetNum"_s,   this->m_location.address().streetNumber()},
        {u"street"_s,      this->m_location.address().street()      },
        {u"city"_s,        this->m_location.address().city()        },
        {u"district"_s,    this->m_location.address().district()    },
        {u"state"_s,       this->m_location.address().state()       },
        {u"country"_s,     this->m_location.address().country()     },
        {u"countryCode"_s, this->m_location.address().countryCode() },
        {u"postalCode"_s,  this->m_location.address().postalCode()  },
        {u"latitude"_s,    this->m_location.coordinate().latitude() },
        {u"longitude"_s,   this->m_location.coordinate().longitude()},
    };
}

void LocationEngine::loadConfigs() noexcept
{
    logDebug(munadi) << "Loading configs from persistent config file";

    bool           configMigrated{true};
    const QString  migrateKey{u"LocationEngineConfigMigrated"_s};
    QGeoAddress    address;
    QGeoCoordinate coordinates;

    loadConfigKey({}, "Location/Address"_L1, {},
                  [&address](const QVariant &text) -> void { address.setText(text.toString()); });

    loadConfigKey(
        "Adhan/cityName"_L1, {}, migrateKey,
        [&address](const QVariant &city) -> void { address.setCity(city.toString()); },
        configMigrated);

    loadConfigKey(
        "Adhan/countryName"_L1, {}, migrateKey,
        [&address](const QVariant &country) -> void { address.setCountry(country.toString()); },
        configMigrated);

    loadConfigKey(
        "Adhan/latitude"_L1, "Location/Latitude"_L1, migrateKey,
        [&coordinates](const QVariant &latitude) -> void {
            const double lat{latitude.toDouble()};

            if (lat < -90.0 || lat > 90.0) // NOLINT(*-magic-numbers)
            {
                logWarning(munadi) << "Not setting invalid latitude value: " << lat;
                return;
            }

            coordinates.setLatitude(lat);
        },
        configMigrated);

    loadConfigKey(
        "Adhan/longitude"_L1, "Location/Longitude"_L1, migrateKey,
        [&coordinates](const QVariant &longitude) -> void {
            const double lng{longitude.toDouble()};

            if (lng < -180.0 || lng > 180.0) // NOLINT(*-magic-numbers)
            {
                logWarning(munadi) << "Not setting invalid longitude value: " << lng;
                return;
            }

            coordinates.setLongitude(lng);
        },
        configMigrated);

    this->updateLocation(address, coordinates);

    if (configMigrated)
        core->writeSettings("LocationEngineConfigMigrated"_L1, true);
}

void LocationEngine::saveConfigs() noexcept
{
    logDebug(munadi) << "Saving location configurations";

    // Ensure the very last config to be saved has the third parameter set to "true". Otherwise
    // (later) settings are not saved

    core->writeSettings("Location/Address"_L1, this->m_location.address().text());
    core->writeSettings("Location/Latitude"_L1, this->m_location.coordinate().latitude());
    core->writeSettings("Location/Longitude"_L1, this->m_location.coordinate().longitude(), true);
}

void LocationEngine::setLocation(const QVariant &index) noexcept
{
    logDebug(munadi) << "Received index: " << index;

    if (!checkIndex(index, this->geoLocations))
        return;

    this->m_locationResultsIndex = index.toInt();

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

    emit this->locationChanged();
}

void LocationEngine::updateLocation(const QGeoAddress    &address,
                                    const QGeoCoordinate &coordinates) noexcept
{
    logDebug(munadi) << "Updating address to: " << address.text();
    logDebug(munadi) << "Updating location to: " << coordinates.toString();

    if (this->m_location.address() != address)
        this->m_location.setAddress(address);
    else
        logDebug(munadi) << "Skipping setting identical address";

    if (this->m_location.coordinate() != coordinates)
        this->m_location.setCoordinate(coordinates);
    else
        logDebug(munadi) << "Skipping setting identical coordinates";

    // this->address.setText({});
    emit this->locationChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void LocationEngine::onAborted() noexcept
{
    logInfo(munadi) << "Previous query aborted";

    if (!this->geoCodeReply.isNull())
        this->geoCodeReply->deleteLater();

    this->geoCodeReply.clear();
    this->geoLocations.clear();
    this->geoLocations.squeeze();
    emit this->loadingChanged();
}

void LocationEngine::onCoreOnline() noexcept
{
    this->connectToLocaleChanged();

    // The rest are already in here
    MunadiBase::onCoreOnline();
}

void LocationEngine::onFinished() noexcept
{
    // This mostly happens when abort() emits another finished() signal
    if (this->geoCodeReply.isNull())
    {
        logDebug(munadi) << "Late signal from an aborted or replaced query. Ignoring...";
        return;
    }

    logDebug(munadi) << "Processing geocode query...";

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
    emit this->loadingChanged();
}

void LocationEngine::onLocaleChanged() const noexcept
{
    logDebug(munadi) << "Detected language change, refreshing locale...";

    this->manager->setLocale(QLocale{});

    logDebug(munadi) << "Using locale: " << this->manager->locale();
}

} // namespace Munadi