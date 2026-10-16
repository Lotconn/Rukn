#include "internal-helpers.hpp"

#include <LocationEngine.hpp>

Q_STATIC_LOGGING_CATEGORY(munadi, "LocationEngine")

namespace Munadi
{

LocationEngine::LocationEngine(QObject *parent) : MunadiEngine<LocationEngine>{parent}
{
    this->bootEngine();
    CoreEngine::onceOnline(this, [this] -> void { this->onCoreOnline(); });
    this->makeOnline();
}

const QLoggingCategory &LocationEngine::logCategory()
{ return munadi(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Munadi