#pragma once

#include <AppEngine.hpp>
#include <PrayerEngine.hpp>

namespace App {

class UiEngine : public AppBase, public AppEngine<UiEngine> {
  Q_OBJECT

  // Enums must be access scoped in QML
  Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")

  // Construct a QML module from this class
  QML_ELEMENT

  // The QML module is a singleton
  QML_SINGLETON

  Q_PROPERTY(bool bgImage MEMBER m_bgImage NOTIFY bgImageChanged)
  Q_PROPERTY(double blur MEMBER m_blur NOTIFY blurChanged)
  Q_PROPERTY(int calcMethodDisplayIndex MEMBER m_calcMethodsDisplayIndex WRITE
                 setCalcMethodDisplayIdx NOTIFY prayerCalcChanged)
  Q_PROPERTY(bool isoClock MEMBER m_isoClock NOTIFY displayTimesChanged)
  Q_PROPERTY(
      QVariantList calcMethodsDisplayNames READ calcMethodsDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(QString calendarDateDisplay READ calendarDateDisplay NOTIFY calendarChanged)
  Q_PROPERTY(int calendarDisplayIndex MEMBER m_calendarDisplayIndex WRITE setCalendarDisplayIdx
                 NOTIFY calendarIdxChanged)
  Q_PROPERTY(QVariantList calendarDisplayNames READ calendarDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(int dateOffset MEMBER m_dateOffset WRITE setDateOffset RESET resetDateOffset NOTIFY
                 calendarChanged)
  Q_PROPERTY(bool hideOnClose MEMBER m_hideOnClose NOTIFY hideOnCloseChanged)
  Q_PROPERTY(double hijriOffset MEMBER m_hijriOffset WRITE setHijriOffset NOTIFY hijriOffsetChanged)
  Q_PROPERTY(
      int langDisplayIndex MEMBER m_langDisplayIndex WRITE setLangDisplayIdx NOTIFY langIdxChanged)
  Q_PROPERTY(QVariantList langDisplayNames READ langDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(int madhhabDisplayIndex MEMBER m_madhhabDisplayIndex WRITE setMadhhabDisplayIdx NOTIFY
                 prayerCalcChanged)
  Q_PROPERTY(QVariantList madhhabDisplayNames READ madhhabDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(double opacity MEMBER m_opacity NOTIFY opacityChanged)
  Q_PROPERTY(QVariantList prayerDisplayNames READ prayerDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(QVariantHash prayerDisplayTimes READ prayerDisplayTimes NOTIFY displayTimesChanged)
  Q_PROPERTY(bool showOnAdhan MEMBER m_showOnAdhan NOTIFY showOnAdhanChanged)
  // Q_PROPERTY(int textsRefresher READ textsRefresher NOTIFY translationChanged)
  Q_PROPERTY(int themeDisplayIndex MEMBER m_themeDisplayIndex NOTIFY themeChanged)
  Q_PROPERTY(QVariantList themeDisplayNames READ themeDisplayNames NOTIFY translationChanged)
  Q_PROPERTY(QString timeLeftDisplay READ timeLeftDisplay NOTIFY timeChanged)
  Q_PROPERTY(double timeLeftProgress READ timeLeftProgress NOTIFY timeChanged)

public:
  /**
   * @brief The QML factory, creating the singleton as a QML module, called by QmlEngine itself.
   * The created instance therefore is parented by the QmlEngine which can be accessed through
   * `qmlEngine`. This engine class here can request the QML engine to instantiate any other
   * dependent engine classes it requires through a call to
   * `qmlEngine->singletonInstance<OtherEngine *>()` which too would be parented by the QmlEngine,
   * avoiding loads of recurred problems, one among which is nested parenting if the dependent
   * engines would be instantiated right in this engine's constructor. However, QmlEngine prefers
   * the default-constructible `public` constructor over this if its available, so this never gets
   * called if it's present. The class constructor therefore needs to be hidden from `public` -
   * preferrably in `protected` segment for QmlEngine to call this factory. For reducing redundant
   * codes and actually using the class constructor preserving all the expected behaviors as
   * usual, this factory then needs to act as a wrapper around the "hidden" constructor which
   * artfully accomplishes both goals. Returns an instance of the engine class - in here, by
   * calling the `protected` constructor.
   *
   * @param qmlEngine: The QmlEngine instance, passed by the QmlEngine during its call
   * @param jsEngine: Present only for historical purposes, everything is done through the
   * QmlEngine instance these days. Not used here.
   * @return LocationEngine*: The class instance
   */
  static UiEngine *create(QQmlEngine *qmlEngine, QJSEngine * /* jsEngine */);
  ~UiEngine() override = default;

  /**
   * @brief Accessor needed by AppEngine<T> for logging purposes
   *
   * @return const QLoggingCategory&: The instance created by Q_STATIC_LOGGING_CATEGORY()
   */
  static const QLoggingCategory &logCategory() noexcept;

  Q_INVOKABLE static QString getFormFactor() noexcept;

protected:
  explicit UiEngine(QObject *parent = nullptr);

public slots:
  void resetDateOffset() noexcept;

private:
  /**
   * @brief Singleton library designed to be only one instance, not to be copied, or transferred
   * ownership to other instance.
   */
  Q_DISABLE_COPY_MOVE(UiEngine);

  ////////////////////////////////////////////////////////////////////////////////////////////////
  //  Variables
  ////////////////////////////////////////////////////////////////////////////////////////////////

  inline static const QString calendarFormat{u"dd-MMM-yyyy"_s};

  bool                    m_bgImage{};
  double                  m_blur{};
  int                     m_calcMethodsDisplayIndex{};
  QVariantList            m_calcMethodsDisplayNames;
  QHash<QString, QString> m_calendarDateDisplay;
  int                     m_calendarDisplayIndex{};
  QVariantList            m_calendarDisplayNames;
  int                     m_dateOffset{};
  bool                    m_hideOnClose{};
  int                     m_hijriOffset{};
  bool                    m_isoClock{};
  int                     m_langDisplayIndex{};
  QVariantList            m_langDisplayNames;
  int                     m_madhhabDisplayIndex{};
  QVariantList            m_madhhabDisplayNames;
  double                  m_opacity{};
  QVariantList            m_prayerDisplayNames;
  bool                    m_showOnAdhan{};
  int                     m_themeDisplayIndex{};
  QVariantList            m_themeDisplayNames;

  QString calendarKey;

  ////////////////////////////////////////////////////////////////////////////////////////////////
  //  Functions
  ////////////////////////////////////////////////////////////////////////////////////////////////

  static auto indexOfValue(const QVariantList &list, const QString &value) noexcept -> int;
  static auto timeLeftProgress() noexcept -> double;

  auto calcMethodsDisplayNames() noexcept -> QVariantList &;
  auto calendarDateDisplay() noexcept -> QString;
  auto calendarDisplayNames() noexcept -> QVariantList &;
  void connectToAlarmsReset() const noexcept;
  void connectToCalcChanged() const noexcept;
  void connectToDateChanged() const noexcept;
  void connectToLocaleChanged() const noexcept;

  /**
   * @brief Customized override of `AppBase::defaultConfigs()`. See
   * `AppBase::defaultConfigs()` brief
   */
  void               defaultConfigs() noexcept override;
  [[nodiscard]] auto getCurrentLang() const noexcept -> QString;
  [[nodiscard]] auto getCurrentTheme() const noexcept -> QString;
  void               initCalendarDates() noexcept;
  void               initTexts() noexcept;
  auto               langDisplayNames() noexcept -> QVariantList &;

  /**
   * @brief Customized override of `AppBase::loadConfigs()`.
   */
  void               loadConfigs() noexcept override;
  auto               madhhabDisplayNames() noexcept -> QVariantList &;
  auto               prayerDisplayNames() noexcept -> QVariantList &;
  [[nodiscard]] auto prayerDisplayTimes() const noexcept -> QVariantHash;

  /**
   * @brief Customized override of `AppBase::saveConfigs()`
   */
  void               saveConfigs() noexcept override;
  void               setCalcMethodDisplayIdx(const QVariant &index) noexcept;
  void               setCalendarKey(const QString &name) noexcept;
  void               setCalendarDisplayIdx(const QVariant &index) noexcept;
  void               setDateOffset(int offset) noexcept;
  void               setHijriOffset(double offset) noexcept;
  void               setLangDisplayIdx(const QVariant &index) noexcept;
  void               setMadhhabDisplayIdx(const QVariant &index) noexcept;
  auto               themeDisplayNames() noexcept -> QVariantList &;
  [[nodiscard]] auto timeLeftDisplay() noexcept -> QString;
  void               updateConfigs() noexcept;

signals:
  void bgImageChanged();
  void blurChanged();
  void calendarChanged();
  void calendarIdxChanged();
  void displayTimesChanged();
  void hideOnCloseChanged();
  void hijriOffsetChanged();
  void langIdxChanged();
  void opacityChanged();
  void prayerCalcChanged();
  void showOnAdhanChanged();
  void themeChanged();
  void timeChanged();
  void translationChanged();

private slots:
  void onCalcChanged() noexcept;
  void onDateChanged() noexcept;
  void onPrayerEngineOnline() noexcept;
  /**
   * @brief Customized overridden extension of `AppBase::onCoreOnline`. See its brief for
   * details
   */
  void onCoreOnline() noexcept override;
  void onLocaleChanged() noexcept;
};

} // namespace App