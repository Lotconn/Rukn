#pragma once

#include <QSettings>
#include <QSocketNotifier>
#include <QTranslator>

#include <AppEngine.hpp>
#include <alarm.h>
#include <config.hpp>

/**
 * @brief Convenient instance accessor macro
 */
#define core App::CoreEngine::instance()

namespace App {

class CoreEngine : public AppBase, public AppEngine<CoreEngine> {
  Q_OBJECT

  // Enums must be access scoped in QML
  Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

  // Construct a QML module from this class
  QML_ELEMENT

  // The QML module is a singleton
  QML_SINGLETON

  // Returns the current default locale during call time. See `getCurrentLocale()` and
  // `localeChanged()` briefings
  Q_PROPERTY(QLocale appLocale READ getCurrentLocale NOTIFY localeChanged)

  // Returns whether auto start has been set in the system. See `isAutoStartSet()`,
  // `setAutoStartSet()` and `autoStartChanged()` briefings
  Q_PROPERTY(bool autoStart READ isAutoStartSet WRITE setAutoStart NOTIFY autoStartChanged)

public:
  explicit CoreEngine(QObject *parent = nullptr);
  ~CoreEngine() override = default;

  /**
   * @brief Accessor needed by AppEngine<T> for logging purposes
   *
   * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
   */
  static const QLoggingCategory &logCategory() noexcept;

  /**
   * @brief Provides RW access to the `m_alarm` object. Usually exists for other classes to
   * connect its signal to their appropriate slots, and manage alarms.
   *
   * @return Alarm&: The `m_alarm` object
   */
  [[nodiscard]] Alarm &alarmManager() const noexcept;

  /**
   * @brief Queries whether auto start is enabled in the system. For now, this is not a reliable
   * check for actual auto start state in the system as this just returns the "in-memory" value of
   * `m_autoStartSet` and doesn't actually query the current system state during call.
   *
   * @return true: Auto start is enabled (in-memory)
   * @return false: Auto start is disabled (in-memory)
   */
  [[nodiscard]] bool isAutoStartSet() const noexcept;

  /**
   * @brief Get the app version. Needless to be a property. Returns `s_appVersion` which is
   * constructed from `APP_VERSION` definition to avoid temporaries everytime this is called.
   *
   * @return QString: App's version as set in `APP_VERSION`
   */
  Q_INVOKABLE static QString getAppVersion() noexcept;

  /**
   * @brief Queries if the current app version is later than the value in "App/LastVersion" key
   * persisted in config. Returns the state as saved in `m_isVersionNew` which defaults to true.
   * Needless to be a property.
   *
   * @return true: App version is newer than the one saved in config, even if the value read is
   * empty.
   * @return false: App version is older or equal to the read version in config.
   */
  Q_INVOKABLE [[nodiscard]] bool isVersionNew() const noexcept;

  /**
   * @brief Reads and returns the value of `key` from the config file, or `defaultValue` if `key`
   * doesn't exist or failed to read.
   *
   * @param key: The key from the settings file to fetch value of
   * @param defaultValue: Fallback value to return if `key` doesn't exist or read fails for some
   * reason. By default, this is the default constructible QVariant empty value `{}`
   * @return QVariant: The read (or fallback) value of `key`. QSettings values are read and
   * written as QVariants, it's upto the caller to convert to the appropriate type accordingly.
   */
  [[nodiscard]] QVariant readSettings(QAnyStringView  key,
                                      const QVariant &defaultValue = {}) const noexcept;

  /**
   * @brief Reads the array value of the group `group` from the config file as a QSettings array,
   * and returns a parsable QVariantList. QSettings group is a section in the settings file, or a
   * nested key within a section that marks the begining of the array. Unlike JSON (or YAML)
   * arrays, the array items themselves begin with 1-indexed indices, and keys belonging to the
   * index are all preceeded with the same index "ID". These arrays are therefore position
   * independent. A `size` key at the end of the item list in the group dictates the size of the
   * array.
   *
   * @param group: The group to start reading the array from.
   * @return QVariantList: The array translated as a position-dependent list of QVariantMap KV
   * pairs, i.e. a QList<QVariantMap>.
   */
  [[nodiscard]] QVariantList readSettingsArray(QAnyStringView group) const noexcept;

  /**
   * @brief Similar sibling of readSettings, only it reads from the state file instead of settings
   * config (see `readSettings()`'s brief). Generally used in QML to restore window state.
   *
   * @param key: The key in the state file to read the value of
   * @param defaultValue: Fallback value to use if `key` doesn't exist or on some other failures.
   * Defaults to empty.
   * @return QVariant: The read value from `key` or `defaultValue`.
   */
  Q_INVOKABLE [[nodiscard]] QVariant readState(const QString  &key,
                                               const QVariant &defaultValue = {}) const noexcept;

  /**
   * @brief A wrapper to clear and fresh-register minute-boundary tracker in `m_alarm`. Called
   * from QML to more accurately show counters and time at certain UI states.
   */
  Q_INVOKABLE void set60sTicker() const noexcept;

  /**
   * @brief Save the auto-start state in memory (`m_autoStartSet`) and "request" the auto-start in
   * system. Selects the appropriate setter among the available ones according to the app build.
   *
   * @param enabled: The auto-start state to set. `true` to enable it, `false` to disable. Passing
   * the same state as before does nothing.
   */
  void setAutoStart(bool enabled) noexcept;

  /**
   * @brief Sets the default locale to one of `s_lang` entries. Skips re-setting if currently set
   * and `lang` are the same. Uses `applyLocale()` to set the passed locale.
   *
   * @param lang: The language to set, one among the availables in `s_lang` table.
   */
  void setLang(const QString &lang) noexcept;

  /**
   * @brief Writes `value` of `key` to the config file, with optionally forcing flush after write.
   *
   * @param key: The key in settings file to write `value` to
   * @param value: The value to write in `key` of the settings file
   * @param flushOnWrite: Commit right away after write and don't wait for QSettings to manage
   * flush. Defaults to false (let QSettings handle commits).
   */
  void writeSettings(QAnyStringView key, const QVariant &value,
                     bool flushOnWrite = false) const noexcept;

  /**
   * @brief Writes the array `entryMaps` at the `group` to the config file as a QSettings array.
   * QSettings group is a section in the settings file, or a nested key within a section that
   * marks the begining of the array. Unlike JSON (or YAML) arrays, the array items themselves
   * begin with 1-indexed indices, and keys belonging to the index are all preceeded with the same
   * index "ID". These arrays are therefore position independent. A `size` key at the end of the
   * item list in the group dictates the size of the array.
   *
   * @param group: The group to start writing the array to.
   * @param entryMaps: The position-dependent list of QVariantMap KV pairs, i.e. a
   * QList<QVariantMap> to save in the config file as a QSettings array.
   * @param flushOnWrite: Commit right away after write and don't wait for QSettings to manage
   * flush. Defaults to false (let QSettings handle commits).
   */
  void writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                          bool flushOnWrite = false) const noexcept;

  /**
   * @brief State-file sibling of writeSettings(). See the brief in `writeSettings()`. Generally
   * used in QML to save window state.
   *
   * @param key: The key in state file to write `value` to
   * @param value: The value to write in `key` of the state file
   * @param flushOnWrite: Commit right away after write and don't wait for QSettings to manage
   * flush. Defaults to false (let QSettings handle commits).
   */
  Q_INVOKABLE void writeState(const QString &key, const QVariant &value,
                              bool flushOnWrite = false) const noexcept;

protected:
  /**
   * @brief A (QObject `protected` member) overridden event handler to intercept desired QEvent
   * events. In here, this intercepts the `QEvent::LocaleChange` event (to work on OS-level
   * language change) and `QEvent::LanguageChange` event (to work on app-level language change;
   * currently no-op) and passes others through a call to base `QObject::eventFilter()`.
   * `LocaleChange` handler calls `applyLocale()` which implicitly triggers the `LanguageChange`
   * event
   *
   * @param object: [Param used by QObject internally] The object to intercept events from.
   * @param event: [Param used by QObject internally] The event to process.
   * @return true: Event intercepted.
   * @return false: Event was not intercepted.
   */
  bool eventFilter(QObject *object, QEvent *event) noexcept override;

public slots:
  /**
   * @brief Slot used for `receivedMessage()` signal from the SingleApplication instance in
   * `main.cpp`. Right now this is limited to trivially relaying the "second instance" signal to
   * QML to expose the instance and bringing it to focus (because it's much simpler this way for a
   * QGui-based app than from C++-side unlike QWidget-based). Could be extended in future.
   *
   * @param instanceId: The instance ID from where the signal was emitted (set by
   * SingleApplication)
   * @param message: The message sent by the instance (set by SingleApplication)
   */
  void onReceivedMessage(quint32 instanceId, const QByteArray &message);

private:
  /**
   * @brief Singleton library designed to be only one instance, not to be copied, or transferred
   * ownership to other instance.
   */
  Q_DISABLE_COPY_MOVE(CoreEngine)

  ////////////////////////////////////////////////////////////////////////////////////////////////
  //  Variables
  ////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief Member holding the macros string (in `const char *` type).
   */
  inline static const QString s_appVersion{QStringLiteral(APP_VERSION)};

  /**
   * @brief A hash-table for known language IDs/keys versus corresponding QLocale objects to
   * use/return
   */
  inline static const QHash<QString, QLocale> s_lang{
      {u"System"_s,  QLocale::system()  },
      {u"English"_s, QLocale{u"en_US"_s}},
      {u"Arabic"_s,  QLocale{u"ar_SA"_s}},
      {u"Bangla"_s,  QLocale{u"bn_BD"_s}},
  };

  /**
   * @brief A reverse hash-table for supported QLocale enum versus known language IDs/keys
   */
  inline static const QHash<QLocale::Language, QString> s_revLang{
      {QLocale::English, u"English"_s},
      {QLocale::Arabic,  u"Arabic"_s },
      {QLocale::Bangla,  u"Bangla"_s },
  };

  /**
   * @brief A hash-table for known alarm IDs carried by `trigger()` signal from `m_alarm` versus
   * signals to relay for
   */
  static const QHash<QString, std::function<void(CoreEngine *)>> triggerRelays;

  /**
   * @brief Array of two integers used in Unix signal handling. One of them represents the "read"
   * part of the opened signal handler socket and other the "write" part. See
   * `installUnixSignalHandlers()`, `termSignalHandler()` and `onUnixSigTerm()` briefs
   */
  inline static std::array<int, 2> sigtermFd{0, 0};

  /**
   * @brief Stores version newness state. Refer to `isVersionNew()` brief.
   */
  bool m_isVersionNew{true};

  /**
   * @brief The `Alarm` object for setting tickers and alarm notifier events in system
   */
  Alarm *m_alarm{};

  /**
   * @brief For storing auto-start state in memory
   */
  bool m_autoStartSet{};

  /**
   * @brief Used to store DBus message reply temporarily
   */
  bool interimStore{};

  /**
   * @brief New settings-file object
   */
  QSettings *settings{};

  /**
   * @brief State-file object
   */
  QSettings *stateFile{};

  /**
   * @brief Notifies when the read/write part of the Unix signal-handler socket becomes available
   * for read/write
   */
  QSocketNotifier *snTerm{};

  /**
   * @brief Manages translation load/unload for specified languages
   */
  QTranslator translator;

  ////////////////////////////////////////////////////////////////////////////////////////////////
  //  Functions
  ////////////////////////////////////////////////////////////////////////////////////////////////

  /**
   * @brief The function responsible for setting `locale` as the default locale application-wide,
   * loading appropriate translations and RTL/LTR layout for the locale and requesting
   * re-translate to QML. Sets to en_US if the requested or the system locale is not supported.
   * Implicitly triggers the `QEvent::LanguageChange` event (two times) upon language unload-load
   * cycle.
   *
   * @param locale: The locale to set the application-wide default to. Should be one among the
   * known in `s_lang`. Ignored and uses en_US as the fallback otherwise.
   */
  void applyLocale(const QLocale &locale = QLocale::system()) noexcept;

  /**
   * @brief Connects `onAboutToQuit()` slot to `QObject::aboutToQuit()` signal. Should
   * be called in constructor.
   */
  void connectToAboutToQuit() const noexcept;

  /**
   * @brief Null override to stop duplicate, needless and erroneous execution of its body since it
   * was more designed for engine classes dependent on CoreEngine.
   */
  void connectToCoreOnline() noexcept override {};

  /**
   * @brief Connects `onTrigger()` slot to `trigger()` signal from `m_alarm`. Should be called in
   * constructor, closest to when `m_alarm` has been registered signals.
   */
  void connectToTrigger() const noexcept;

  /**
   * @brief Get the current default locale
   *
   * @return QLocale
   */
  static QLocale getCurrentLocale() noexcept;

  /**
   * @brief Clears and (re-)registers "24h_tick" and "60s_tick" triggers in `m_alarm`. Should be
   * called in constructor, nearest to when engine has become online to avoid misfires, early
   * or missed calls to slots. Skips setting the "60s_tick" if system tray is not available.
   */
  void installTickers() const noexcept;

  /**
   * @brief Sets up the Unix signal handling. Connects the defined signal handler (i.e. the
   * "action"; `termSignalHandler()`) with set attributes to corresponding signals (e.g. SIGTERM,
   * SIGINT). Only defined in Unix OSs. Should be installed earliest in constructor for proper
   * lifetime assurance.
   *
   * @return int: 0 if connection was succesful, `ENOTCONN` if connection failed.
   */
  static int installUnixSignalHandlers();

  /**
   * @brief Customized override of `AppBase::loadConfigs()`.
   */
  void loadConfigs() noexcept override;

  /**
   * @brief A simple helper function to generalize KV fetch from a settings.
   *
   * @param settings: The settings file to read from
   * @param key: The key to read value of
   * @param defaultValue: Fallback value to use if `key` doesn't exist or failed to read. Defaults
   * to empty.
   * @return QVariant: The value read for `key` or idempotently, the `defaultValue`
   */
  static QVariant readValueOfKey(QSettings &settings, QAnyStringView key,
                                 const QVariant &defaultValue = {}) noexcept;

  /**
   * @brief Customized override of `AppBase::saveConfigs()`
   */
  void saveConfigs() noexcept override;

  /**
   * @brief Requests the auto-start state in system, using DBus. Defined only in Flatpak build. A
   * constructed message is sent to the desktop portal, at the `org.freedesktop.portal.Desktop`
   * bus requesting the "autostart" and "background" states to be changed. The response is
   * ultimately handled by the `onAutoStartResponse()` slot. On enable success, an autostart file
   * named with reverse-domain-name tuplet `domain.AppName.desktop` is created by Flatpak in
   * `$XDG_CONFIG_HOME/autostart/` with the default commandline or as requested. On disable
   * success, the same file is removed from the location. Several breakpoints exists where request
   * may silently fail, and the only trace of so are logs printed. As of now, this is a
   * scaffolding function to get the job done, will be further extended to handle failures more
   * concretely.
   *
   * @param enabled: Whether auto-start should be enabled (`true`|`false`)
   */
  void setAutoStart_flatpak(bool enabled) noexcept;

  /**
   * @brief Sets auto-start state in system by installing or removing the desktop file at
   * `$XDG_CONFIG_HOME/autostart/`. Defined only for "Default" Linux build other than Flatpak. On
   * enable success, a copy of `domain.AppName-sfx.desktop` available in expected places is
   * installed at the autostart location, and on disable, the same file is removed from there.
   * Utilizes QSaveFile class for atomic writes and auto-cleanup on failures. Similar to
   * `setAutoStart_flatpak()`, attempts may silently fail, and the only trace of error are the
   * logs. Will be further extended to handle and inform failures more concretely.
   *
   * @param enabled: Whether auto-start should be enabled (`true`|`false`)
   */
  void setAutoStart_linux(bool enabled) noexcept;

  /**
   * @brief Sets auto-start state in system using registry - defined only in Windows OSs. The
   * modified key in registry is `HKCU\Software\Microsoft\Windows\CurrentVersion\Run`. A temporary
   * QSettings object is create for interacting with the registry key. On enable success, the
   * program with full path and parameters is added under the key, and on disable success, it is
   * removed likewise. Fail-paths are defined but as of now, are only logged. Will be extended
   * further to handle and inform failures more concretely.
   *
   * @param enabled: Whether auto-start should be enabled (`true`|`false`)
   */
  void setAutoStart_win(bool enabled) noexcept;

  /**
   * @brief A simple helper function to generalize KV write to a settings.
   *
   * @param settings: The settings file to write to
   * @param key: The key to write value at
   * @param value: The value to persist
   * @param flushOnWrite: Force commit right after setting `value` to `key`. Default is to let
   * QSettings handle commits.
   */
  static void setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                            bool flushOnWrite = false) noexcept;

  /**
   * @brief Writes to the "write" part of the opened unix signal-handler socket when it is
   * available for write. This "opens" the file-descriptor associated with the captured Unix
   * signal. Only defined in Unix OSs.
   */
  static void termSignalHandler(int /* unused */);

signals:
  /**
   * @brief Relay of `trigger()` carrying "alarms_reset" ID. Implicitly bound to `trigger()`
   * signal.
   */
  void alarmsReset();

  /**
   * @brief Relay of `trigger()` carrying "adhan" ID. Implicitly bound to `trigger()` signal.
   */
  void adhanAlarm();

  /**
   * @brief Notifies about auto-start changed, carrying the state information along. Implicitly
   * bound to `m_autoStartSet`.
   *
   * @param enabled: The current auto-start state triggering the signal
   */
  void autoStartChanged(bool enabled);

  /**
   * @brief Relay of `trigger()` carrying "24h_tick" ID. Implicitly bound to `trigger()` signal.
   */
  void dateChanged();

  /**
   * @brief Relay of `receivedMessage()` signal from SingleApplication instance. Although the
   * `receivedMessage()` signal is more general than this purpose, it is coarsly implementated
   * only as a second instance notifier. Implicitly bound to `receivedMessage()` signal.
   */
  void exposeInstance();

  /**
   * @brief Relay of `trigger()` carrying "60s_tick" ID. Implicitly bound to `trigger()` signal.
   */
  void minuteChanged();

  /**
   * @brief Notifies about language (and locale) changes, either from underlying system or from
   * the app (e.g. from language options). Implicitly bound to `translate` member.
   */
  void localeChanged();

private slots:
  /**
   * @brief Corresponding slot for `QObject::aboutToQuit()` signal. Handles some housekeeping,
   * persistence and offlining notifications.
   */
  void onAboutToQuit() noexcept;

  /**
   * @brief Routine to handle DBus response requested in `setAutoStart_flatpak()`. Trivial as of
   * now, only logs and stores the intermediate value to state-member.
   *
   * @param ret: The response state as sent by DBus. 0 means success, non-zero means failure.
   */
  void onAutoStartResponse(uint ret, const QVariantMap &) noexcept;

  /**
   * @brief Executes the relay of `trigger()` signal from `m_alarm`. The corresponding relayer
   * signal is emitted from one of `triggerRelays()`
   *
   * @param id: The trigger ID emitting this signal
   */
  void onTrigger(const QString &id) noexcept;

  /**
   * @brief "Accepts" the signal by reading from the read-part of the file-descriptor and calling
   * `quit()` on the current QApplication instance. Defined only in Unix OSs.
   */
  void onUnixSigTerm() const;
};

} // namespace App