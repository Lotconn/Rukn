#pragma once

#include <QObject>
#include <QtQml/qqmlregistration.h>

namespace App
{

using Qt::StringLiterals::operator""_s;
using Qt::StringLiterals::operator""_L1;

/**
 * @brief Sole QObject base of every engine. Contains all the common members among the engines.
 * Engines derive from this class directly to inherit these (QObject-based) members. Few of them are
 * intentionally kept empty because their bodies cannot be generalized or known up-front.
 * `AppEngine<Engine_t>` template class is "linked" to this base for the reason in the following
 * paragraph and for separating static members out.
 *
 * `moc`/`qmltyperegistrar` records an engine's _first_ base class verbatim as its QML prototype.
 * Engines therefore MUST list `AppBase` first: `class T : public AppBase, public
 * AppEngine<T>`. Listing `AppEngine<T>` first yields `prototype: "AppEngine<T>"` in the
 * qmltypes, which is not a registered QML type. `qmlls` then loses the QObject chain and reports
 * "Cannot assign binding of type T to QObject [incompatible-type]" (e.g. on Connections.target).
 * Needs no `QML_*` macro: qmltyperegistrar emits it on its own as the base of the registered
 * engines (verified in qmltypes). Do not make this a template or add a second QObject-derived base.
 */
class AppBase : public QObject
{
    Q_OBJECT

    template <typename Engine_t> friend class AppEngine;

  protected:
    /**
     * @brief Construct a new App Base object. Not default constructible. Requires a
     * QLoggingCategory object to be passed from the deriver class for message context.
     *
     * @param category: The reference to message context object to use for logging. For most
     * purposes, use `Q_STATIC_LOGGING_CATEGORY()` macro in implementation file and pass the object.
     * @param parent: Parent of the class. Usually left empty for auto-managed hierarchy
     */
    explicit AppBase(const QLoggingCategory &category, QObject *parent = nullptr) noexcept
        : QObject{parent}, m_logCategory{category}
    {
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions //////////////////////////////////////////////////////////////////////////////

    /**
     * @brief connects to the `engineOffline()` signal from CoreEngine, required by all engines.
     * Additionally handles connection success/failure logging
     */
    virtual void connectToCoreOffline() const noexcept;

    /**
     * @brief Sets all default values of members before engine onlines, especially the ones related
     * to class' state
     */
    virtual void defaultConfigs() {};

    /**
     * @brief Loads all relevant settings from persistent config file before engine onlines.
     * Requires CoreEngine's instance to exist.
     */
    virtual void loadConfigs() {};

    /**
     * @brief Saves all relevant settings to persistent config file before engine offline. Requires
     * CoreEngine's instance to exist.
     */
    virtual void saveConfigs() {};

  signals:
    /**
     * @brief Notifies that engine has effectively shutdown, but not necessarily destroyed. In
     * DirectConnection type, this means offlining is blocked until connected slots return, so this
     * signal effectively means "shutting down". In QueudConnection type however, the slot is
     * executed in callers thread, so engine may go offline (and possibly destroyed) before other
     * engines' slots dependent on this engine's instance return. It is highly recommended
     * therefore, to explicitly specify DiretConnection type while connecting to this signal.
     */
    void engineOffline();

    /**
     * @brief Notifies that engine has completed all its boot procedures since constructor was
     * called, has come online and ready to serve. This signal is immune to threading consequences
     * from previous brief.
     */
    void engineOnline();

  protected slots:
    /**
     * @brief Jobs to perform once CoreEngine is offline (QueudConnection) or while it is going
     * offline (DirectConnection). See `engineOffline()` signal's brief
     */
    virtual void onCoreOffline();

    /**
     * @brief Jobs to perform once CoreEngine is online. Immune to threading consequencing from
     * before
     */
    virtual void onCoreOnline();

  private:
    const QLoggingCategory &m_logCategory;
};

/**
 * @brief An F-bound/CRTP mixin class template for generalizing (static) typed-members of each
 * engine. Deliberately NOT a QObject and NOT dervied from `AppBase`, only contains statics
 * without any stateful members. `AppEngine<T>` calls through `base()` when it needs access to
 * members from `AppBase` (see its brief further below). Engines should be deriving this class
 * following AppBase to avoid tooling warnings (see the brief in `AppBase`). Due to the
 * design, there is effectively no state-mixing/diamond hazard issues when an engine derives from
 * both AppBase (first) and AppEngine<T> (second). Clangd check `fuchsia-multiple-inheritence`
 * was so disabled exactly for this reason.
 *
 * Do's
 * - Use with AppBase, alone won't work
 * - Preceed AppBase before this in derive-list
 * - Override or use the members in this class with appropriate type
 *
 * Don'ts
 * - Derive from AppBase
 * - Merge to AppBase
 * - Add stateful class members (i.e. members NOT static)
 * - Fix `*multiple-inheritence` class warnings by violating the above don'ts
 *
 * @tparam Engine_t: Replace with engine's class; i.e. `CoreEngine`, `PrayerEngine`, `MyEngine` etc
 */
template <typename Engine_t> class AppEngine
{
  public:
    /**
     * @brief Returns whether the engine class singleton pointer exists or not. Recommended check
     * before calling any engine's instance to avoid fatal quit. Not similar to `isOnline()` (see
     * its brief)
     *
     * @return true: singleton pointer exists
     * @return false: pointer is null
     */
    static auto instanceExists() noexcept -> bool;

    /**
     * @brief Returns the singleton pointer for the engine class. Fatal quit if called without
     * constructing an instance (i.e. when pointer is null)
     *
     * @return Engine_t*: Singleton pointer for the engine class
     */
    static auto instance() noexcept -> Engine_t *;

    /**
     * @brief Returns whether the engine is online or not. Not necessarily similar to
     * `instanceExists()`. Engines (should) set online (or offline) status after all their boot (or
     * shutdown) procedures are completed and ready (or closed) to serve. Therefore, there is a gap
     * in time from constructor (or destructor) being called, singleton instance possibly being set
     * and engines changing state. This is therefore a much safer check than `instanceExists()` for
     * callers needing the full operational state as it gurantees engine is genuinely ready (or
     * closed) to serve rather than singleton pointer barely being available (or not) while possibly
     * being in the midst of necessary boot (or cleanup) routines.
     *
     * @return true: Engine is online
     * @return false: Engine is offline
     */
    static auto isOnline() noexcept -> bool;

    /**
     * @brief A convenient static helper that runs in connectee engine's own class once its
     * singleton state exists to connect "context"'s "slot" to the its `engineOnline()` signal -
     * possibly before it's emitted - avoiding polls externally from the connector "context". If the
     * engine is offline when this static is called, a spin-table call to `instanceExists()` runs
     * for a specified number of retries (`retriesLeft`) to wait while engine state becomes
     * available before `slot` is executed. Returns without connecting if retries are exceeded. If
     * the function is called after engine is online and `engineOnline()` signal has possibly been
     * emitted, the `slot` is executed "by-hand" to reconcile the miss.
     *
     * @param context: One who owns `slot`. For most cases, use `this`
     * @param slot: The routine to be executed if `engineOnline()` signal is emitted
     * @param retriesLeft: Number of retries before failing
     */
    static auto onceOnline(QObject *context, std::function<void()> slot,
                           int retriesLeft = 19) noexcept -> void; // NOLINT(*-magic-numbers)

  protected:
    AppEngine()  = default;
    ~AppEngine() = default;

    ////////////////////////////////////////////////////////////////////////////////////////////
    //  Variables //////////////////////////////////////////////////////////////////////////////

    /**
     * @brief The singleton state pointer. Publicly accessed through `instance()` and checked
     * through `instanceExists()`. Available through the lifetime of the class.
     */
    inline static QPointer<Engine_t> s_singletonInstance;

    /**
     * @brief Holds the online state of "this" engine. Checked through `isOnline()`. Available
     * through the lifetime of the class, i.e. almost as long as `s_singletonInstance` exists.
     */
    inline static bool s_online{};

    ////////////////////////////////////////////////////////////////////////////////////////////
    //  Functions //////////////////////////////////////////////////////////////////////////////

    /**
     * @brief (Ideally) holds all boot-up routines.
     */
    virtual void bootEngine();

    /**
     * @brief DRY functional wrapping essential routines to make an engine online. Also takes care
     * of QML-module ownership control.
     */
    virtual void makeOnline();

  private:
    /**
     * @brief Doesn't get propagated to derivers so each engine should be including this in its
     * class. Mostly kept to pass linter checks
     */
    Q_DISABLE_COPY_MOVE(AppEngine)

    /**
     * @brief An accessor to AppBase's members. In simple sense, this digs a path through the
     * _constructed_ class' state to AppBase. This therefore won't work without a class deriving
     * _both_ AppBase followed by AppEngine<T>.
     *
     * @return AppBase*: The instance of AppBase's class
     */
    auto base() noexcept -> AppBase *
    { return static_cast<Engine_t *>(this); }
};
} // namespace App