#include <QTimer>

#include "internal-helpers.hpp"
#include <CoreEngine.hpp>
#include <PrayerEngine.hpp>

// Q_STATIC_LOGGING_CATEGORY(munadi, "PrayerEngine") // Qt >= 6.9
Q_LOGGING_CATEGORY(prayerEngine, "PrayerEngine")

namespace Munadi
{

using Prayer = Munadi::PrayerEngine::Prayer;

PrayerEngine &PrayerEngine::instance()
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        logFatal(prayerEngine)
            << "No instance constructed. Explicit instantiation is required before invoking "
               "the instance";

    return *PrayerEngine::s_singletonInstance;
}

PrayerEngine::PrayerEngine(QObject *parent) : QObject{parent}
{
    if (PrayerEngine::s_singletonInstance != nullptr)
        logFatal(prayerEngine)
            << "A second instance was constructed; only one instance may exist per process";

    PrayerEngine::s_singletonInstance = this;

    this->initTexts();

    QTimer::singleShot(0, this, &PrayerEngine::connectLocaleChangedSignal);
}

PrayerEngine::~PrayerEngine()
{
    QObject::disconnect(m_localeChangedConnection);

    if (PrayerEngine::s_singletonInstance == this)
        PrayerEngine::s_singletonInstance = nullptr;
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

QString PrayerEngine::prayerName(PrayerEngine::Prayer prayerEnum)
{ return this->m_prayerDisplayNames.value(prayerEnum); }

QStringList PrayerEngine::prayerNamesList()
{
    QStringList list;

    for (const auto &[key, value] : this->m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

void PrayerEngine::refreshTexts()
{
    this->initTexts();

    emit this->textsChanged();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

PrayerEngine *PrayerEngine::s_singletonInstance{nullptr};

void PrayerEngine::connectLocaleChangedSignal()
{
    m_localeChangedConnection = QObject::connect(
        &CoreEngine::instance(), &CoreEngine::localeChanged, this, &PrayerEngine::refreshTexts);

    if (m_localeChangedConnection == nullptr)
        logWarning(prayerEngine) << "localeChanged signal not connected";
    else
        logDebug(prayerEngine) << "localeChanged signal connected";
}

void PrayerEngine::initTexts()
{
    this->m_prayerDisplayNames = {
        {   Prayer::Fajr,    tr("Fajr")},
        {Prayer::Sunrise, tr("Sunrise")},
        {  Prayer::Dhuhr,   tr("Dhuhr")},
        {    Prayer::Asr,     tr("Asr")},
        {Prayer::Maghrib, tr("Maghrib")},
        {   Prayer::Isha,    tr("Isha")}
    };
}

int PrayerEngine::textsRefresher()
{ return textsRefresherReal(); }

QString PrayerEngine::toString(PrayerEngine::Prayer prayer)
{ return QString::fromLatin1(Adhan::PrayerUtils::to_string(this->m_prayerEnumMap.value(prayer))); }

PrayerEngine::Prayer PrayerEngine::fromString(QStringView prayerName)
{
    return this->m_revPrayerEnumMap.value(
        Adhan::PrayerUtils::from_string(QStringView{prayerName}.toLatin1().toStdString()));
}
} // namespace Munadi