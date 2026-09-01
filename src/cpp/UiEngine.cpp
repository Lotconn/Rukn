#include <CoreEngine.hpp>
#include <PrayerEngine.hpp>
#include <UiEngine.hpp>

namespace Munadi
{

UiEngine *UiEngine::s_singletonInstance{nullptr};

UiEngine *UiEngine::create(QQmlEngine *qmlEngine, QJSEngine * /*unused*/)
{

    if (UiEngine::s_singletonInstance == nullptr)
        qFatal("Munadi: no instance constructed. Munadi must be explicitly "
               "instantiated before the QML engine loads");

    // The singleton's lifetime is owned by this TU (main()/first construction),
    // not by the QQmlEngine. Make that explicit so the engine never attempts
    // to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(UiEngine::s_singletonInstance, QQmlEngine::CppOwnership);

    // Forces CoreEngine to exist through QML if it doesn't yet then wires up the locale-change
    // fan-out.
    auto *core = qmlEngine->singletonInstance<CoreEngine *>("Munadi", "CoreEngine");
    QObject::connect(core, &CoreEngine::localeChanged, UiEngine::s_singletonInstance,
                     &UiEngine::refreshTexts);

    // Forces PrayerEngine to exist through QML too if it doesn't yet for the same reason
    qmlEngine->singletonInstance<PrayerEngine *>("Munadi", "PrayerEngine");

    return UiEngine::s_singletonInstance;
}

UiEngine &UiEngine::instance()
{
    if (UiEngine::s_singletonInstance == nullptr)
        qFatal("Munadi: no instance constructed. Munadi must be explicitly instantiated before "
               "invoking the instance");

    return *UiEngine::s_singletonInstance;
}

UiEngine::UiEngine(QObject *parent) : QObject{parent}
{

    if (UiEngine::s_singletonInstance != nullptr)
        qFatal("Munadi: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    UiEngine::s_singletonInstance = this;

    this->m_themeListIdx = 0;
}

UiEngine::~UiEngine()
{

    if (UiEngine::s_singletonInstance == this)
        UiEngine::s_singletonInstance = nullptr;
}

void UiEngine::refreshTexts()
{ PrayerEngine::instance().reInitTexts(); }

QStringList UiEngine::themeList()
{ return {tr("System"), tr("Light"), tr("Dark")}; }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

} // namespace Munadi