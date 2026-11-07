#include "internal-helpers.hpp"

#include <LocationEngine.hpp>
#include <MultimediaEngine.hpp>
#include <MunadiEngine.hpp>
#include <UiEngine.hpp>
#include <UpdateEngine.hpp>

namespace Munadi
{

// Static
template <typename Engine_t> bool MunadiEngine<Engine_t>::instanceExists() noexcept
{ return MunadiEngine::s_singletonInstance != nullptr; }

// Static
template <typename Engine_t> Engine_t *MunadiEngine<Engine_t>::instance() noexcept
{
    if (!MunadiEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return MunadiEngine::s_singletonInstance;
}

// Static
template <typename Engine_t> bool MunadiEngine<Engine_t>::isOnline() noexcept
{ return MunadiEngine::s_online; }

// Static
template <typename Engine_t>
void MunadiEngine<Engine_t>::onceOnline(QObject *context, std::function<void()> slot,
                                        int retriesLeft) noexcept
{
    if (!MunadiEngine::instanceExists())
    {
        if (retriesLeft <= 0)
        {
            logCritical(Engine_t::logCategory())
                << "Engine still not constructed after several retries";

            return;
        }

        logDebug(Engine_t::logCategory())
            << "Retrying connection... (attempts left: " << retriesLeft << ")";
        QTimer::singleShot(0, context, [context, slot, retriesLeft]() mutable {
            MunadiEngine::onceOnline(context, std::move(slot), retriesLeft - 1);
        });

        return;
    }

    if (const auto conn =
            QObject::connect(MunadiEngine::instance(), &MunadiBase::engineOnline, context, slot);
        conn == nullptr)
        logWarning(Engine_t::logCategory()) << "engineOnline signal not connected";
    else
        logDebug(Engine_t::logCategory()) << "engineOnline signal connected";

    // Reconcile missed `engineOnline()` signal
    if (MunadiEngine::isOnline())
        slot();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void MunadiBase::connectToCoreOffline() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::engineOffline, this, &MunadiBase::onCoreOffline);
        conn == nullptr)
        logWarning(this->m_logCategory) << "coreOffline signal not connected";
    else
        logDebug(this->m_logCategory) << "coreOffline signal connected";
}

void MunadiBase::onCoreOffline()
{
    logDebug(this->m_logCategory) << "Going offline, executing offline hooks";
    this->saveConfigs();
}

void MunadiBase::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
}

template <typename Engine_t> void MunadiEngine<Engine_t>::bootEngine()
{
    if (MunadiEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "A second instance was constructed; only one instance may exist per process";

    logDebug(Engine_t::logCategory()) << "Booting engine...";

    MunadiEngine::s_singletonInstance = static_cast<Engine_t *>(this);

    this->base()->defaultConfigs();
}

template <typename Engine_t> void MunadiEngine<Engine_t>::makeOnline()
{
    MunadiEngine::s_online = true;

    // The singleton's lifetime is owned by this TU (main()/first construction), not by the
    // QQmlEngine. Make that explicit so the engine never attempts to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(MunadiEngine::instance(), QQmlEngine::CppOwnership);

    logDebug(Engine_t::logCategory()) << "Engine online";
    emit this->base()->engineOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Munadi

// List of all deriver engine classes

// NOLINTBEGIN(*-virtual-class-destructor)
template class Munadi::MunadiEngine<Munadi::CoreEngine>;
template class Munadi::MunadiEngine<Munadi::LocationEngine>;
template class Munadi::MunadiEngine<Munadi::MultimediaEngine>;
template class Munadi::MunadiEngine<Munadi::PrayerEngine>;
template class Munadi::MunadiEngine<Munadi::UiEngine>;
template class Munadi::MunadiEngine<Munadi::UpdateEngine>;
// NOLINTEND(*-virtual-class-destructor)