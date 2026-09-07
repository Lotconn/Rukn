#include "internal-helpers.hpp"
#include <UiEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "UiEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(uiEngine, "UiEngine")

namespace Munadi
{

using Prayer = PrayerEngine::Prayer;

UiEngine::UiEngine(QObject *parent) : QObject{parent}
{
    if (UiEngine::s_singletonInstance != nullptr)
        logFatal(uiEngine)
            << "A second instance was constructed; only one instance may exist per process";

    QTimer::singleShot(0, this, &UiEngine::connectToCoreOnline);
    this->initTexts();

    UiEngine::s_singletonInstance = this;
}

UiEngine::~UiEngine()
{
    QObject::disconnect(this->coreOfflineConnection);
    QObject::disconnect(this->coreOnlineConnection);
    QObject::disconnect(this->localeChangedConnection);

    if (UiEngine::s_singletonInstance == this)
        UiEngine::s_singletonInstance = nullptr;
}

UiEngine &UiEngine::instance()
{
    if (UiEngine::s_singletonInstance == nullptr)
        logFatal(uiEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *UiEngine::s_singletonInstance;
}

QString UiEngine::prayerDisplayName(Prayer prayerEnum)
{ return this->m_prayerDisplayNames.value(prayerEnum); }

QStringList UiEngine::prayerDisplayNamesList()
{
    QStringList list;

    for (const auto &[key, value] : this->m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::connectToCoreOffline()
{
    this->coreOfflineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOffline, this,
                         &UiEngine::onCoreOffline, Qt::DirectConnection);

    if (this->coreOfflineConnection == nullptr)
        logWarning(uiEngine) << "aboutToQuit signal not connected";
    else
        logDebug(uiEngine) << "aboutToQuit signal connected";
}

void UiEngine::connectToCoreOnline()
{
    if (!CoreEngine::exists())
    {
        logDebug(uiEngine) << "CoreEngine doesn't exist yet";
        // CoreEngine not constructed yet on this tick; retry next tick.
        QTimer::singleShot(0, this, &UiEngine::connectToCoreOnline);
        return;
    }

    this->coreOnlineConnection =
        QObject::connect(&CoreEngine::instance(), &CoreEngine::coreOnline, this,
                         &UiEngine::onCoreOnline, Qt::DirectConnection);

    if (this->coreOnlineConnection == nullptr)
        logWarning(uiEngine) << "coreOnline signal not connected";
    else
        logDebug(uiEngine) << "coreOnline signal connected";

    // coreOnline() already fired before we connected; manually reconcile
    logDebug(uiEngine) << "CoreEngine already online; catching up";

    this->onCoreOnline();
}

void UiEngine::connectToLocaleChanged()
{
    this->localeChangedConnection = QObject::connect(
        &CoreEngine::instance(), &CoreEngine::localeChanged, this, &UiEngine::onLocaleChanged);

    if (this->localeChangedConnection == nullptr)
        logWarning(uiEngine) << "localeChanged signal not connected";
    else
        logDebug(uiEngine) << "localeChanged signal connected";
}

void UiEngine::defaultTheme()
{ this->setThemeListIdx(0); }

void UiEngine::dummy(const QStringList & /*unused*/)
{
}

void UiEngine::initTexts()
{
    this->m_prayerDisplayNames = {
        {   Prayer::Fajr,    tr("Fajr")},
        {Prayer::Sunrise, tr("Sunrise")},
        {  Prayer::Dhuhr,   tr("Dhuhr")},
        {    Prayer::Asr,     tr("Asr")},
        {Prayer::Maghrib, tr("Maghrib")},
        {   Prayer::Isha,    tr("Isha")}
    };

    this->m_themeList = {tr("System"), tr("Light"), tr("Dark")};
}

void UiEngine::loadConfigs()
{ return; }

int UiEngine::textsRefresher()
{ return textsRefresherReal(); }

int UiEngine::themeListIndex()
{ return this->m_themeListIdx; }

void UiEngine::saveConfigs()
{ return; }

void UiEngine::setThemeListIdx(int idx)
{ this->m_themeListIdx = idx; }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Slots
////////////////////////////////////////////////////////////////////////////////////////////////////

void UiEngine::onCoreOffline()
{ this->saveConfigs(); }

void UiEngine::onCoreOnline()
{
    this->loadConfigs();
    this->connectToCoreOffline();
    this->connectToLocaleChanged();
}

void UiEngine::onLocaleChanged()
{
    this->initTexts();

    emit this->translationChanged();
}

} // namespace Munadi