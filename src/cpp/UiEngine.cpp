#include <UiEngine.hpp>

using UiEngine = Munadi::UiEngine;

UiEngine *UiEngine::s_singletonInstance = nullptr;

UiEngine *UiEngine::create(QQmlEngine * /*unused*/, QJSEngine * /*unused*/)
{

    if (UiEngine::s_singletonInstance == nullptr)
        qFatal("Munadi: no instance constructed. Munadi must be explicitly "
               "instantiated before the QML engine loads");

    // The singleton's lifetime is owned by this TU (main()/first construction),
    // not by the QQmlEngine. Make that explicit so the engine never attempts
    // to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(UiEngine::s_singletonInstance, QQmlEngine::CppOwnership);

    return UiEngine::s_singletonInstance;
}

UiEngine &UiEngine::instance()
{
    if (UiEngine::s_singletonInstance == nullptr)
        qFatal("Munadi: no instance constructed. Munadi must be explicitly instantiated before "
               "invoking the instance");

    return *s_singletonInstance;
}

UiEngine::UiEngine(QObject *parent) : QObject(parent)
{

    if (s_singletonInstance != nullptr)
        qFatal("Munadi: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    UiEngine::s_singletonInstance = this;
}

UiEngine::~UiEngine()
{

    if (UiEngine::s_singletonInstance == this)
        UiEngine::s_singletonInstance = nullptr;
}

QStringList UiEngine::themeList()
{ return {tr("System"), tr("Light"), tr("Dark")}; }

int UiEngine::themeListIndex()
{ return m_themeListIdx; }

void UiEngine::setThemeListIdx(int idx)
{ m_themeListIdx = idx; }
