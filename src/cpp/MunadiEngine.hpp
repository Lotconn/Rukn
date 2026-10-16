#pragma once

#include <QObject>
#include <QReadWriteLock>

namespace Munadi
{

class MunadiBase : public QObject
{
    Q_OBJECT

    template <typename> friend class MunadiEngine;

  protected:
    using QObject::QObject;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QReadWriteLock varLock{QReadWriteLock::Recursive}; // NOLINT(*non-private-*var*)

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto connectToCoreOffline() -> void;

    virtual auto               defaultConfigs() -> void {};
    virtual auto               loadConfigs() -> void {};
    [[nodiscard]] virtual auto logCategory_p() const -> const QLoggingCategory & = 0;
    virtual auto               saveConfigs() -> void {};

  signals:
    auto engineOffline() -> void;
    auto engineOnline() -> void;

  protected slots:
    virtual auto onCoreOffline() -> void;
    virtual auto onCoreOnline() -> void;
};

template <typename Engine_t> class MunadiEngine : public MunadiBase // NOLINT(*special-member-func*)
{
  public:
    using MunadiBase::MunadiBase;

    static auto               instanceExists() noexcept -> bool;
    static auto               instance() noexcept -> Engine_t *;
    [[nodiscard]] static auto isOnline() noexcept -> bool;
    static auto               onceOnline(QObject *context, std::function<void()> slot,
                                         int retriesLeft = 19) -> void; // NOLINT(*-magic-numbers)

  protected:
    ~MunadiEngine() override = default;

    [[nodiscard]] auto logCategory_p() const -> const QLoggingCategory & final
    { return Engine_t::logCategory(); }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    inline static QPointer<Engine_t> s_singletonInstance;
    inline static std::atomic<bool>  s_online{false};

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    virtual auto bootEngine() -> void;
    virtual auto makeOnline() -> void;
};

} // namespace Munadi