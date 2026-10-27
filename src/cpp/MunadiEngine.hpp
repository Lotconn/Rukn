#pragma once

#include <QObject>
#include <QReadWriteLock>
#include <QtQml/qqmlregistration.h>

namespace Munadi
{

using Qt::StringLiterals::operator""_s;
using Qt::StringLiterals::operator""_L1;

// Sole QObject base of every engine.
//
// moc/qmltyperegistrar records an engine's *first* base class verbatim as its QML prototype.
// Engines therefore MUST list MunadiBase first: `class X : public MunadiBase, public
// MunadiEngine<X>`. Listing MunadiEngine<X> first yields `prototype: "MunadiEngine<X>"` in the
// qmltypes, which is not a registered QML type. qmlls then loses the QObject chain and reports
// "Cannot assign binding of type X to QObject [incompatible-type]" (e.g. on Connections.target).
//
// Needs no QML_* macro: qmltyperegistrar emits it on its own as the base of the registered
// engines (verified in qmltypes). Do not make this a template or add a second QObject-derived base.
class MunadiBase : public QObject
{
    Q_OBJECT

    template <typename Engine_t> friend class MunadiEngine;

  protected:
    explicit MunadiBase(const QLoggingCategory &category, QObject *parent = nullptr) noexcept
        : QObject{parent}, m_logCategory{category}
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables
    ////////////////////////////////////////////////////////////////////////////////////////////////

    QReadWriteLock varLock{QReadWriteLock::Recursive}; // NOLINT(*non-private-*var*)

    ////////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions
    ////////////////////////////////////////////////////////////////////////////////////////////////

    auto               connectToCoreOffline() const noexcept -> void;
    [[nodiscard]] auto logCategory_p() const noexcept -> const QLoggingCategory &
    { return m_logCategory; }

    virtual auto defaultConfigs() -> void {};
    virtual auto loadConfigs() -> void {};
    virtual auto saveConfigs() -> void {};

  signals:
    auto engineOffline() -> void;
    auto engineOnline() -> void;

  protected slots:
    virtual auto onCoreOffline() -> void;
    virtual auto onCoreOnline() -> void;

  private:
    const QLoggingCategory &m_logCategory;
};

// CRTP mixin: per-engine singleton bookkeeping (instance(), isOnline(), onceOnline(), ...).
//
// Deliberately NOT a QObject and NOT derived from MunadiBase:
//  - Must stay non-QObject and non-first in the base list; see MunadiBase above.
//  - Deriving from MunadiBase would give each engine two QObject subobjects (ambiguous
//    conversions, wrong qmltypes). Reach MunadiBase through static_cast<Engine_t *>(this).
//  - Keep it free of per-object data. Statics only, so it stays a stateless helper.
//
// fuchsia-multiple-inheritance is disabled project-wide for this reason: the engines have one
// QObject chain and the mixin holds no per-object state, so the diamond/state-mixing hazard the
// check targets does not exist. Do not "fix" the warning by merging the two classes or by
// re-deriving the mixin from MunadiBase.
template <typename Engine_t> class MunadiEngine
{
  public:
    static auto instanceExists() noexcept -> bool;
    static auto instance() noexcept -> Engine_t *;
    static auto isOnline() noexcept -> bool;
    static auto onceOnline(QObject *context, std::function<void()> slot,
                           int retriesLeft = 19) noexcept -> void; // NOLINT(*-magic-numbers)

  protected:
    MunadiEngine()  = default;
    ~MunadiEngine() = default;

    [[nodiscard]] auto logCategory_p() const noexcept -> const QLoggingCategory &
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

  private:
    Q_DISABLE_COPY_MOVE(MunadiEngine)

    auto base() noexcept -> MunadiBase *
    { return static_cast<Engine_t *>(this); }
};
} // namespace Munadi