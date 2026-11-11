#include "internal-helpers.hpp"

#include <AppEngine.hpp>
#include <LocationEngine.hpp>
#include <MultimediaEngine.hpp>
#include <UiEngine.hpp>
#include <UpdateEngine.hpp>

namespace App
{

// Static
template <typename Engine_t> bool AppEngine<Engine_t>::instanceExists() noexcept
{ return AppEngine::s_singletonInstance != nullptr; }

// Static
template <typename Engine_t> Engine_t *AppEngine<Engine_t>::instance() noexcept
{
    if (!AppEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return AppEngine::s_singletonInstance;
}

// Static
template <typename Engine_t> bool AppEngine<Engine_t>::isOnline() noexcept
{ return AppEngine::s_online; }

// Static
template <typename Engine_t>
void AppEngine<Engine_t>::onceOnline(QObject *context, std::function<void()> slot,
                                     int retriesLeft) noexcept
{
    if (!AppEngine::instanceExists())
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
            AppEngine::onceOnline(context, std::move(slot), retriesLeft - 1);
        });

        return;
    }

    if (const auto conn =
            QObject::connect(AppEngine::instance(), &AppBase::engineOnline, context, slot);
        conn == nullptr)
        logWarning(Engine_t::logCategory()) << "engineOnline signal not connected";
    else
        logDebug(Engine_t::logCategory()) << "engineOnline signal connected";

    // Reconcile missed `engineOnline()` signal
    if (AppEngine::isOnline())
        slot();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void AppBase::connectToCoreOffline() const noexcept
{
    if (const auto conn =
            QObject::connect(core, &CoreEngine::engineOffline, this, &AppBase::onCoreOffline);
        conn == nullptr)
        logWarning(this->m_logCategory) << "coreOffline signal not connected";
    else
        logDebug(this->m_logCategory) << "coreOffline signal connected";
}

void AppBase::onCoreOffline()
{
    logDebug(this->m_logCategory) << "Going offline, executing offline hooks";
    this->saveConfigs();
}

void AppBase::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
}

template <typename Engine_t> void AppEngine<Engine_t>::bootEngine()
{
    if (AppEngine::instanceExists())
        logFatal(Engine_t::logCategory())
            << "A second instance was constructed; only one instance may exist per process";

    logDebug(Engine_t::logCategory()) << "Booting engine...";

    AppEngine::s_singletonInstance = static_cast<Engine_t *>(this);

    this->base()->defaultConfigs();
}

template <typename Engine_t> void AppEngine<Engine_t>::makeOnline()
{
    AppEngine::s_online = true;

    // The singleton's lifetime is owned by this TU (main()/first construction), not by the
    // QQmlEngine. Make that explicit so the engine never attempts to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(AppEngine::instance(), QQmlEngine::CppOwnership);

    logDebug(Engine_t::logCategory()) << "Engine online";
    emit this->base()->engineOnline();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace App

// List of all deriver engine classes

// NOLINTBEGIN(*-virtual-class-destructor)
template class App::AppEngine<App::CoreEngine>;
template class App::AppEngine<App::LocationEngine>;
template class App::AppEngine<App::MultimediaEngine>;
template class App::AppEngine<App::PrayerEngine>;
template class App::AppEngine<App::UiEngine>;
template class App::AppEngine<App::UpdateEngine>;
// NOLINTEND(*-virtual-class-destructor)