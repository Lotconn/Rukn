#include "internal-helpers.hpp"

#include <LocationEngine.hpp>
#include <MultimediaEngine.hpp>
#include <MunadiEngine.hpp>
#include <UiEngine.hpp>

namespace Munadi
{

template <typename Engine_t> bool MunadiEngine<Engine_t>::instanceExists() noexcept
{ return MunadiEngine::s_singletonInstance != nullptr; }

template <typename Engine_t> Engine_t *MunadiEngine<Engine_t>::instance() noexcept
{
    if (!MunadiEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return MunadiEngine::s_singletonInstance;
}

template <typename Engine_t> bool MunadiEngine<Engine_t>::isOnline() noexcept
{ return MunadiEngine::s_online.load(); }

template <typename Engine_t>
void MunadiEngine<Engine_t>::onceOnline(QObject *context, std::function<void()> slot,
                                        int retriesLeft)
{
    if (!MunadiEngine::instanceExists())
    {
        if (retriesLeft <= 0)
        {
            logCritical(Engine_t::logCategory())
                << "Engine still not constructed after several retries";

            return;
        }

        QTimer::singleShot(0, context, [context, slot, retriesLeft]() mutable {
            MunadiEngine::onceOnline(context, std::move(slot), retriesLeft - 1);
        });

        return;
    }

    if (const auto conn = QObject::connect(MunadiEngine::instance(), &MunadiBase::engineOnline,
                                           context, slot, Qt::DirectConnection);
        conn == nullptr)
        logWarning(Engine_t::logCategory()) << "engineOnline signal not connected";
    else
        logDebug(Engine_t::logCategory()) << "engineOnline signal connected";

    if (MunadiEngine::isOnline())
        slot();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Engine_t> void MunadiEngine<Engine_t>::bootEngine()
{
    if (MunadiEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "A second instance was constructed; only one instance may exist per process";

    logDebug(Engine_t::logCategory()) << "Booting engine...";

    this->defaultConfigs();
}

void MunadiBase::connectToCoreOffline()
{
    if (const auto conn = QObject::connect(core, &CoreEngine::engineOffline, this,
                                           &MunadiBase::onCoreOffline, Qt::DirectConnection);
        conn == nullptr)
        logWarning(this->logCategory_p()) << "coreOffline signal not connected";
    else
        logDebug(this->logCategory_p()) << "coreOffline signal connected";
}

template <typename Engine_t> void MunadiEngine<Engine_t>::makeOnline()
{
    MunadiEngine::s_singletonInstance = static_cast<Engine_t *>(this);
    MunadiEngine::s_online.store(true);

    logDebug(Engine_t::logCategory()) << "Engine online";
    emit this->engineOnline();
}

void MunadiBase::onCoreOffline()
{
    logDebug(this->logCategory_p()) << "Going offline, executing offline hooks";
    this->saveConfigs();
}

void MunadiBase::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Munadi

// NOLINTBEGIN(*-virtual-class-destructor)
template class Munadi::MunadiEngine<Munadi::CoreEngine>;
template class Munadi::MunadiEngine<Munadi::LocationEngine>;
template class Munadi::MunadiEngine<Munadi::MultimediaEngine>;
template class Munadi::MunadiEngine<Munadi::PrayerEngine>;
template class Munadi::MunadiEngine<Munadi::UiEngine>;
// NOLINTEND(*-virtual-class-destructor)