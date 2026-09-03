#include "internal-helpers.hpp"
#include <UiEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(uiEngine, "UiEngine")

namespace Munadi
{

UiEngine *UiEngine::s_singletonInstance{nullptr};

UiEngine &UiEngine::instance()
{
    if (UiEngine::s_singletonInstance == nullptr)
        logFatal(uiEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *UiEngine::s_singletonInstance;
}

UiEngine::UiEngine(QObject *parent) : QObject{parent}
{

    if (UiEngine::s_singletonInstance != nullptr)
        logFatal(uiEngine)
            << "A second instance was constructed; only one instance may exist per process";

    UiEngine::s_singletonInstance = this;

    this->initTexts();
}

UiEngine::~UiEngine()
{
    if (UiEngine::s_singletonInstance == this)
        UiEngine::s_singletonInstance = nullptr;
}

void UiEngine::initTexts()
{ this->m_themeList = {tr("System"), tr("Light"), tr("Dark")}; }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

} // namespace Munadi