#pragma once

#include <QQmlEngine>

#include <MunadiEngine.hpp>

namespace Munadi
{

class LocationEngine : public MunadiEngine<LocationEngine>
{
    Q_OBJECT
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    QML_ELEMENT
    QML_SINGLETON

  public:
    explicit LocationEngine(QObject *parent = nullptr);

    static auto logCategory() -> const QLoggingCategory &;

  private:
    Q_DISABLE_COPY_MOVE(LocationEngine);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto defaultConfigs() -> void override {};
    auto loadConfigs() -> void override {};
    auto saveConfigs() -> void override {};
};

} // namespace Munadi