#include <PrayerEngine.hpp>

namespace Munadi
{

using Prayer = Munadi::PrayerEngine::Prayer;

PrayerEngine *PrayerEngine::create(QQmlEngine * /*unused*/, QJSEngine * /*unused*/)
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        qFatal("Munadi:PrayerEngine: no instance constructed. Munadi must be explicitly "
               "instantiated before the QML engine loads");

    // The singleton's lifetime is owned by this TU (main()/first construction),
    // not by the QQmlEngine. Make that explicit so the engine never attempts
    // to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(PrayerEngine::s_singletonInstance, QQmlEngine::CppOwnership);

    return PrayerEngine::s_singletonInstance;
}

PrayerEngine &PrayerEngine::instance()
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        qFatal("Munadi:PrayerEngine: no instance constructed. Munadi must be explicitly "
               "instantiated before "
               "invoking the instance");

    return *PrayerEngine::s_singletonInstance;
}

PrayerEngine::PrayerEngine(QObject *parent) : QObject(parent)
{
    if (PrayerEngine::s_singletonInstance != nullptr)
        qFatal("Munadi:PrayerEngine: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    PrayerEngine::s_singletonInstance = this;

    this->initTexts();
}

PrayerEngine::~PrayerEngine()
{
    if (PrayerEngine::s_singletonInstance == this)
        PrayerEngine::s_singletonInstance = nullptr;
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

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

QString PrayerEngine::prayerName(PrayerEngine::Prayer prayerEnum)
{ return this->m_prayerDisplayNames.value(prayerEnum); }

QStringList PrayerEngine::prayerNamesList()
{
    QStringList list;

    for (const auto &[key, value] : this->m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

void PrayerEngine::reInitTexts()
{ this->initTexts(); }

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

PrayerEngine *PrayerEngine::s_singletonInstance{nullptr};

QString PrayerEngine::toString(PrayerEngine::Prayer prayer)
{ return QString::fromLatin1(Adhan::PrayerUtils::to_string(this->m_prayerEnumMap.value(prayer))); }

PrayerEngine::Prayer PrayerEngine::fromString(QStringView prayerName)
{
    return this->m_revPrayerEnumMap.value(
        Adhan::PrayerUtils::from_string(QStringView{prayerName}.toLatin1().toStdString()));
}
} // namespace Munadi