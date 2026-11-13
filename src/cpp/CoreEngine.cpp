#include <QApplication>
#include <QDir>
#include <QSaveFile>
#include <QStandardPaths>
#include <QSystemTrayIcon>

#include "internal-helpers.hpp"

#ifdef APP_FLATPAK_DIST
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QRandomGenerator>
#endif

#ifdef Q_OS_UNIX
#include <csignal>
#include <sys/socket.h>
#endif

Q_STATIC_LOGGING_CATEGORY(cat, "CoreEngine")

namespace App {

const QHash<QString, std::function<void(CoreEngine *)>> CoreEngine::triggerRelays{
    {u"24h_tick"_s,     &CoreEngine::dateChanged  },
    {u"60s_tick"_s,     &CoreEngine::minuteChanged},
    {u"alarms_reset"_s, &CoreEngine::alarmsReset  },
    {u"adhan"_s,        &CoreEngine::adhanAlarm   }
};

CoreEngine::CoreEngine(QObject *parent) : AppBase{cat(), parent}, m_alarm{createAlarm(this)} {
  this->bootEngine();

#ifdef Q_OS_UNIX
  logDebug(cat) << "Unix signal connection status: " << CoreEngine::installUnixSignalHandlers();

  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd.data()) != 0)
    logFatal(cat) << "Couldn't create TERM socketpair";

  snTerm = new QSocketNotifier{sigtermFd[1], QSocketNotifier::Read, this};

  QObject::connect(snTerm, &QSocketNotifier::activated, this, &CoreEngine::onUnixSigTerm);
#endif

  const QString appName{QCoreApplication::applicationName()};
  const QString orgName{QCoreApplication::organizationName()};
  const QString stateDir{QStandardPaths::writableLocation(QStandardPaths::StateLocation)};

  this->settings =
      new QSettings{QSettings::IniFormat, QSettings::UserScope, orgName, appName, this};
  this->stateFile = new QSettings{stateDir + u"rc"_s, QSettings::IniFormat, this};

  logDebug(cat) << "Settings file: " << this->settings->fileName();
  logDebug(cat) << "State file: " << this->stateFile->fileName();

  // These are in order of dependency, not in lexical order
  qApp->installEventFilter(this);
  this->connectToAboutToQuit();
  this->connectToTrigger();
  this->loadConfigs();

  this->makeOnline();
  this->applyLocale();
  this->installTickers();
}

const QLoggingCategory &CoreEngine::logCategory() noexcept { return cat(); }
Alarm                  &CoreEngine::alarmManager() const noexcept { return *this->m_alarm; }
QString                 CoreEngine::getAppVersion() noexcept { return CoreEngine::s_appVersion; }
bool                    CoreEngine::isAutoStartSet() const noexcept { return this->m_autoStartSet; }
bool                    CoreEngine::isVersionNew() const noexcept { return this->m_isVersionNew; }

QVariant CoreEngine::readSettings(QAnyStringView key, const QVariant &defaultValue) const noexcept {
  if (key.isEmpty()) {
    logCritical(cat) << "Provided key is empty";
    return {};
  }

  if (this->settings == nullptr)
    return defaultValue;

  return CoreEngine::readValueOfKey(*this->settings, key, defaultValue);
}

QVariantList CoreEngine::readSettingsArray(QAnyStringView group) const noexcept {
  if (group.isEmpty()) {
    logCritical(cat) << "Provided key group is empty";
    return {};
  }

  const qsizetype size{this->settings->beginReadArray(group)};
  QVariantList    list;

  if (this->settings == nullptr)
    return {};

  logDebug(cat) << "Array size for \"" << group << "\" is " << size;

  for (qsizetype i = 0; i < size; ++i) {
    this->settings->setArrayIndex(static_cast<int>(i));

    QVariantMap map;

    logDebug(cat) << "Child keys in array " << i << ": " << this->settings->childKeys();

    // NOLINTNEXTLINE(clazy-range-loop*)
    for (const QString &key : this->settings->childKeys()) {
      const QVariant value{CoreEngine::readValueOfKey(*this->settings, key)};

      logDebug(cat) << "Value of key \"" << key << "\": " << value;

      map.insert(key, value);
    }

    list.append(map);
  }

  this->settings->endArray();

  return list;
}

QVariant CoreEngine::readState(const QString &key, const QVariant &defaultValue) const noexcept {
  if (key.isEmpty()) {
    logCritical(cat) << "Provided key is empty";
    return {};
  }

  if (this->stateFile == nullptr)
    return defaultValue;

  return CoreEngine::readValueOfKey(*this->stateFile, key, defaultValue);
}

void CoreEngine::set60sTicker() const noexcept {
  logDebug(cat) << "Registering 60s_ticker";

  constexpr static quint8 minSecs{60};

  this->m_alarm->clear("60s_tick"_L1);
  // This is not regular arithmetic, do not simplify the math here
  this->m_alarm->add(
      "60s_tick"_L1,
      QDateTime::fromSecsSinceEpoch(((QDateTime::currentSecsSinceEpoch() / minSecs) + 1) * minSecs),
      minSecs);
}

void CoreEngine::setAutoStart(bool enabled) noexcept // NOLINT(*member*to-static)
{
  if (this->m_autoStartSet == enabled)
    return;

  const bool oldValue{this->m_autoStartSet};

#ifdef Q_OS_WIN
  this->setAutoStart_win(enabled);
#elifdef APP_FLATPAK_DIST
  this->setAutoStart_flatpak(enabled);
#elif defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
  CoreEngine::setAutoStart_linux(enabled);
#endif

  if (this->m_autoStartSet != oldValue)
    emit this->autoStartChanged(this->m_autoStartSet);
}

void CoreEngine::setLang(const QString &lang) noexcept {
  QString currLang{QLocale::languageToString(QLocale{}.language())};

  logDebug(cat) << "lang: " << lang;
  logDebug(cat) << "locale: " << CoreEngine::s_lang.value(lang);
  logDebug(cat) << "current lang: " << currLang;

  if (setConfig(CoreEngine::s_lang, lang, currLang))
    this->applyLocale(CoreEngine::s_lang.value(lang));
}

void CoreEngine::writeSettings(QAnyStringView key, const QVariant &value,
                               bool flushOnWrite) const noexcept {
  if (key.isEmpty()) {
    logCritical(cat) << "Provided key is empty";
    return;
  }

  if (this->settings == nullptr)
    return;

  CoreEngine::setValueOfKey(*this->settings, key, value, flushOnWrite);
}

void CoreEngine::writeSettingsArray(QAnyStringView group, const QVariantList &entryMaps,
                                    bool flushOnWrite) const noexcept {
  if (group.isEmpty()) {
    logCritical(cat) << "Provided key group is empty";
    return;
  }

  if (this->settings == nullptr)
    return;

  this->settings->remove(group);
  this->settings->beginWriteArray(group);

  for (qsizetype i = 0; i < entryMaps.size(); ++i) {
    const QVariantMap &map{entryMaps.at(i).toMap()};

    this->settings->setArrayIndex(static_cast<int>(i));

    for (const auto &[key, value] : map.asKeyValueRange())
      CoreEngine::setValueOfKey(*this->settings, key, value);
  }

  this->settings->endArray();

  if (flushOnWrite)
    this->settings->sync();
}

void CoreEngine::writeState(const QString &key, const QVariant &value,
                            bool flushOnWrite) const noexcept {
  if (key.isEmpty()) {
    logCritical(cat) << "Provided key is empty";
    return;
  }

  if (this->stateFile == nullptr)
    return;

  CoreEngine::setValueOfKey(*this->stateFile, key, value, flushOnWrite);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Public Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::onReceivedMessage(quint32 instanceId, const QByteArray &message) {
  logDebug(cat) << "Received message from instance: " << instanceId;
  logInfo(cat) << "Received Message: " << message;
  emit this->exposeInstance();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Members
////////////////////////////////////////////////////////////////////////////////////////////////////

bool CoreEngine::eventFilter(QObject *object, QEvent *event) noexcept {
  if (object != qApp)
    return QObject::eventFilter(object, event);

  switch (event->type()) {
  case QEvent::LocaleChange:
    // OS-level change
    logDebug(cat) << "Locale change detected";

    this->applyLocale();

    return true;
  case QEvent::LanguageChange:
    // App-level change, e.g. language selected from options
    logDebug(cat) << "Translations changed";
    return true;
  default:
    return QObject::eventFilter(object, event);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::applyLocale(const QLocale &locale) noexcept {
  logDebug(cat) << "Removing current translator: " << this->translator.language();
  qApp->removeTranslator(&this->translator); // no-op if none installed

  QLocale l{locale};

  if (CoreEngine::s_revLang.contains(l.language())) {
    l = QLocale{l.language(), l.territory()};

    QLocale::setDefault(l);

    if (this->translator.load(l, qAppName().toLower(), "_"_L1, ":/qt/qml/App/i18n"_L1)) {
      logDebug(cat) << "Installing updated translator: " << this->translator.language();

      qApp->installTranslator(&this->translator);
    }
  } else {
    logInfo(cat) << "Language not supported, using en_US as fallback";

    QLocale::setDefault(QLocale{"en_US"_L1});
  }

  // Querying the current default has its advantages
  QApplication::setLayoutDirection(QLocale{}.language() == QLocale::Arabic ? Qt::RightToLeft
                                                                           : Qt::LeftToRight);

  if (QQmlEngine *engine = qmlEngine(this)) {
    logDebug(cat) << "Reloading Qml translations";

    engine->retranslate();
  }

  emit this->localeChanged();
}

void CoreEngine::connectToAboutToQuit() const noexcept {
  if (const auto conn =
          QObject::connect(qApp, &QCoreApplication::aboutToQuit, this, &CoreEngine::onAboutToQuit);
      conn == nullptr)
    logWarning(cat) << "aboutToQuit signal not connected";
  else
    logDebug(cat) << "aboutToQuit signal connected";
}

void CoreEngine::connectToTrigger() const noexcept {
  if (const auto conn =
          QObject::connect(this->m_alarm, &Alarm::trigger, this, &CoreEngine::onTrigger);
      conn == nullptr)
    logWarning(cat) << "Trigger signal not connected";
  else
    logDebug(cat) << "Trigger signal connected";
}

QLocale CoreEngine::getCurrentLocale() noexcept { return QLocale{}; }

void CoreEngine::installTickers() const noexcept {
  constexpr static quint32 daySecs{86400};

  this->m_alarm->clear("24h_tick"_L1);
  this->m_alarm->add("24h_tick"_L1, QDate::currentDate().startOfDay().addDays(1), daySecs);

  if (!QSystemTrayIcon::isSystemTrayAvailable()) {
    logWarning(cat) << "System tray is not available";
    logDebug(cat) << "Not registering 60s_ticker";
    return;
  }

  this->set60sTicker();
}

int CoreEngine::installUnixSignalHandlers() {
#ifdef Q_OS_UNIX
  struct sigaction term{};

  term.sa_handler = CoreEngine::termSignalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags  = 0;
  term.sa_flags |= SA_RESTART;

  if (sigaction(SIGINT, &term, nullptr) != 0)
    return ENOTCONN;

  if (sigaction(SIGTERM, &term, nullptr) != 0)
    return ENOTCONN;

  return 0;
#endif
}

void CoreEngine::loadConfigs() noexcept {
  logDebug(cat) << "Loading configs from persistent config file";

  // Assign to the member directly to avoid resetting autostart to system on startup
  // WIP: Do not save state, use state from system directly
  loadConfigKey(
      "App/AutoStart"_L1,
      [this](const QVariant &enabled) -> void { this->m_autoStartSet = enabled.toBool(); }, cat());

  loadConfigKey(
      "App/LastVersion"_L1,
      [this](const QVariant &version) -> void {
        // the following string comparison does what it implies
        if (version.toString() >= CoreEngine::s_appVersion)
          this->m_isVersionNew = false;
      },
      cat());
}

QVariant CoreEngine::readValueOfKey(QSettings &settings, QAnyStringView key,
                                    const QVariant &defaultValue) noexcept {
  logDebug(cat) << "Fetching value of key \"" << key << "\"";

  QVariant value{settings.value(key)};

  switch (settings.status()) {
  case QSettings::AccessError:
    logWarning(cat) << "Failed to read config";
    return defaultValue;
  case QSettings::FormatError:
    logWarning(cat) << "Config for key \"" << key << "\" is malformed, returning default value";
    return defaultValue;
  default:
    break;
  }

  if (value.isNull()) {
    logWarning(cat) << "Failed to fetch a valid value for key \"" << key
                    << "\", returning default value";
    return defaultValue;
  }

  logDebug(cat) << "Key: \"" << key << "\"; Value: \"" << value << "\"";
  return value;
}

void CoreEngine::saveConfigs() noexcept {
  logDebug(cat) << "Saving configurations";

  this->writeSettings("App/AutoStart"_L1, this->m_autoStartSet);
  this->writeSettings("App/LastVersion"_L1, CoreEngine::s_appVersion, true);
}

void CoreEngine::setValueOfKey(QSettings &settings, QAnyStringView key, const QVariant &value,
                               bool flushOnWrite) noexcept {
  if (value.isNull())
    logWarning(cat) << "Value is empty. Make sure this is intended.";

  settings.setValue(key, value);

  if (flushOnWrite)
    settings.sync();

  switch (settings.status()) {
  case QSettings::AccessError:
    logWarning(cat) << "Failed to read config";
    break;
  case QSettings::FormatError:
    logWarning(cat) << "Config for key \"" << key << "\" is malformed, returning default value";
    break;
  default:
    break;
  }
}

void CoreEngine::setAutoStart_flatpak(bool enabled) noexcept { // NOLINT(*member-function*static)
  Q_UNUSED(enabled)
#ifdef APP_FLATPAK_DIST
  QDBusConnection bus{QDBusConnection::sessionBus()};

  QDBusMessage msg{QDBusMessage::createMethodCall(
      "org.freedesktop.portal.Desktop"_L1, "/org/freedesktop/portal/desktop"_L1,
      "org.freedesktop.portal.Background"_L1, "RequestBackground"_L1)};

  // NOLINTNEXTLINE(*magic-numbers)
  const int     token{QRandomGenerator::global()->bounded(1000, 9999)};
  const QString sender{bus.baseService().mid(1).replace(u'.', u'_')};
  const QString handleToken{qAppName() + u"_%1"_s.arg(token)};
  const QString requestPath{
      u"/org/freedesktop/portal/desktop/request/%1/%2"_s.arg(sender, handleToken)};

  interimStore = enabled;
  bus.connect("org.freedesktop.portal.Desktop"_L1, requestPath, "org.freedesktop.portal.Request"_L1,
              "Response"_L1, this,
              // NOLINTNEXTLINE(clazy-connect-not-normalized)
              SLOT(onAutoStartResponse(uint, QVariantMap)));

  const QVariantMap options{
      {u"autostart"_s,    enabled                                         },
      {u"background"_s,   enabled                                         },
      {u"commandline"_s,  QStringList{qAppName().toLower(), u"--hidden"_s}},
      {u"reason"_s,       u"Automatically launch application at login"_s  },
      {u"handle_token"_s, qAppName() + u"_%1"_s.arg(token)                },
  };

  msg << ""_L1 << options; // Empty parent window ID
  QDBusMessage response{bus.call(msg)};

  // Handle response and connect to signal for final status
  if (response.type() != QDBusMessage::ReplyMessage)
    logWarning(cat) << "Portal request failed: " << response.errorMessage();
#endif
}

void CoreEngine::setAutoStart_linux(bool enabled) noexcept { // NOLINT(*member-function*static)
  Q_UNUSED(enabled)
#ifndef APP_FLATPAK_DIST
  const QString desktopName{qApp->desktopFileName() + u"-sfx.desktop"_s};
  const QString autostartPath{
      QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + u"/autostart/"_s +
      desktopName};

  if (!enabled) {
    if (!QFileInfo::exists(autostartPath) || !QFile::remove(autostartPath))
      logWarning(cat) << "Couldn't remove autostart file " << autostartPath;
    else
      logInfo(cat) << "Autostart disabled successfully";

    this->m_autoStartSet = enabled;
    return;
  }

  const QString desktopPath{
      QStandardPaths::locate(QStandardPaths::ApplicationsLocation, desktopName)};

  if (desktopPath.isEmpty()) {
    logWarning(cat) << "No desktop file found installed in system";
    return;
  }

  logDebug(cat) << "Using desktop file at " << desktopPath;

  QFile desktopFile{desktopPath};

  if (!desktopFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    logCritical(cat) << "Couldn't open " << desktopPath << " for reading";
    return;
  }

  const QString cmdline{u"bin/"_s + qAppName().toLower()};
  const QString contents{QString{desktopFile.readAll()}.replace(cmdline, cmdline + " --hidden"_L1)};

  desktopFile.close();

  QSaveFile file{autostartPath};

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    logCritical(cat) << "Couldn't open " << autostartPath << " for writing";
    return;
  }

  file.write(contents.toUtf8());

  if (!file.commit())
    logCritical(cat) << "Failed to write at " << autostartPath;
  else {
    logDebug(cat) << "File " << autostartPath << " successfully written";

    this->m_autoStartSet = enabled;
  }
#endif
}

void CoreEngine::setAutoStart_win(bool enabled) noexcept { // NOLINT(*member-function*static)
  Q_UNUSED(enabled)
#ifdef Q_OS_WIN
  const QString appName{QCoreApplication::applicationName()};
  const QString appPath{QDir::toNativeSeparators(QCoreApplication::applicationFilePath())};
  QSettings     settings{R"(HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run)"_L1,
                         QSettings::NativeFormat};

  (enabled) ? settings.setValue(appName, appPath + " --hidden"_L1) : settings.remove(appName);

  switch (settings.status()) {
  case QSettings::AccessError:
    logWarning(cat) << "Failed to read config";
    return;
  case QSettings::FormatError:
    logWarning(cat) << "Value \"" << appPath << "\" for key \"" << appName << "\" is malformed";
    return;
  default:
    this->m_autoStartSet = enabled;
    break;
  }
#endif
}

void CoreEngine::termSignalHandler(int /* unused */) {
#ifdef Q_OS_UNIX
  const char    a{1};
  const ssize_t ret{::write(sigtermFd[0], &a, sizeof(a))};
  Q_UNUSED(ret)
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void CoreEngine::onAboutToQuit() noexcept {
  this->saveConfigs();
  emit this->engineOffline();
  CoreEngine::s_online = false;

#ifdef Q_OS_UNIX
  ::close(sigtermFd[0]);
  ::close(sigtermFd[1]);
#endif
}

void CoreEngine::onAutoStartResponse(uint ret, const QVariantMap & /* unused */) noexcept {
  if (ret > 0)
    logWarning(cat) << "Request to autostart was cancelled";
  else {
    logDebug(cat) << "Autostart set successfully";

    this->m_autoStartSet = this->interimStore;
  }
}

void CoreEngine::onTrigger(const QString &id) noexcept {
  logDebug(cat) << "trigger id: " << id;

  if (!CoreEngine::triggerRelays.contains(id)) {
    logWarning(cat) << "\"" << id << "\" is not a valid alarm ID";
    return;
  }

  // This is actually `emit signal()`
  std::invoke(CoreEngine::triggerRelays.value(id), this);

  if (id == u"alarms_reset"_s)
    this->installTickers();
}

void CoreEngine::onUnixSigTerm() const {
#ifdef Q_OS_UNIX
  this->snTerm->setEnabled(false);

  char          tmp{0};
  const ssize_t ret{::read(CoreEngine::sigtermFd[1], &tmp, sizeof(tmp))};
  Q_UNUSED(ret)

  qApp->quit();
#endif
}
} // namespace App