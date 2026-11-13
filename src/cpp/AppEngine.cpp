#include <AppEngine.hpp>
#include <CoreEngine.hpp>

namespace App {

void AppBase::connectToCoreOffline() const noexcept {
  if (const auto conn =
          QObject::connect(core, &CoreEngine::engineOffline, this, &AppBase::onCoreOffline);
      conn == nullptr)
    logWarning(this->m_logCategory) << "coreOffline signal not connected";
  else
    logDebug(this->m_logCategory) << "coreOffline signal connected";
}

void AppBase::connectToCoreOnline() noexcept {
  if (const auto conn =
          QObject::connect(core, &CoreEngine::engineOnline, this, &AppBase::onCoreOnline);
      conn == nullptr)
    logWarning(this->m_logCategory) << "coreOnline signal not connected";
  else
    logDebug(this->m_logCategory) << "coreOnline signal connected";

  if (CoreEngine::isOnline())
    this->onCoreOnline();
}

void AppBase::onCoreOffline() {
  logDebug(this->m_logCategory) << "Going offline, executing offline hooks";

  this->saveConfigs();
  emit this->engineOffline();

  if (this->m_online != nullptr)
    *this->m_online = false;
}

void AppBase::onCoreOnline() {
  this->loadConfigs();
  this->connectToCoreOffline();
}

} // namespace App
