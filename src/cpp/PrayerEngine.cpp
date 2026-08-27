#include <engine.hpp>

using PrayerEngine = Munadi::PrayerEngine;
using Prayer       = Munadi::PrayerEngine::Prayer;

PrayerEngine *PrayerEngine::create(QQmlEngine * /*unused*/, QJSEngine * /*unused*/)
{
    if (PrayerEngine::s_singletonInstance == nullptr)
        qFatal("Munadi: no instance constructed. Munadi must be explicitly "
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
        qFatal("Munadi: no instance constructed. Munadi must be explicitly instantiated before "
               "invoking the instance");

    return *s_singletonInstance;
}

PrayerEngine::PrayerEngine(QObject *parent) : QObject(parent)
{
    if (s_singletonInstance != nullptr)
        qFatal("Munadi: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    PrayerEngine::s_singletonInstance = this;

    this->init();
}

void PrayerEngine::init()
{
    m_prayerDisplayNames = {
        {   Prayer::Fajr,    tr("Fajr")},
        {Prayer::Sunrise, tr("Sunrise")},
        {  Prayer::Dhuhr,   tr("Dhuhr")},
        {    Prayer::Asr,     tr("Asr")},
        {Prayer::Maghrib, tr("Maghrib")},
        {   Prayer::Isha,    tr("Isha")}
    };
}

PrayerEngine::~PrayerEngine()
{
    if (PrayerEngine::s_singletonInstance == this)
        PrayerEngine::s_singletonInstance = nullptr;
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

QString PrayerEngine::prayerName(PrayerEngine::Prayer prayer)
{ return m_prayerDisplayNames.value(prayer); }

QStringList PrayerEngine::prayerNamesList()
{
    QStringList list;

    for (const auto &[key, value] : m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

PrayerEngine *PrayerEngine::s_singletonInstance = nullptr;

const QMap<PrayerEngine::Prayer, Adhan::Prayer> PrayerEngine::m_prayerEnumMap = {
    {   PrayerEngine::Fajr,    Adhan::Prayer::Fajr},
    {PrayerEngine::Sunrise, Adhan::Prayer::Sunrise},
    {  PrayerEngine::Dhuhr,   Adhan::Prayer::Dhuhr},
    {    PrayerEngine::Asr,     Adhan::Prayer::Asr},
    {PrayerEngine::Maghrib, Adhan::Prayer::Maghrib},
    {   PrayerEngine::Isha,    Adhan::Prayer::Isha},
    {   PrayerEngine::None,    Adhan::Prayer::None}
};

const QMap<Adhan::Prayer, PrayerEngine::Prayer> PrayerEngine::m_revPrayerEnumMap = {
    {   Adhan::Prayer::Fajr,    PrayerEngine::Fajr},
    {Adhan::Prayer::Sunrise, PrayerEngine::Sunrise},
    {  Adhan::Prayer::Dhuhr,   PrayerEngine::Dhuhr},
    {    Adhan::Prayer::Asr,     PrayerEngine::Asr},
    {Adhan::Prayer::Maghrib, PrayerEngine::Maghrib},
    {   Adhan::Prayer::Isha,    PrayerEngine::Isha},
    {   Adhan::Prayer::None,    PrayerEngine::None}
};

QString PrayerEngine::toString(PrayerEngine::Prayer prayer)
{ return QString::fromLatin1(Adhan::PrayerUtils::to_string(m_prayerEnumMap.value(prayer))); }

PrayerEngine::Prayer PrayerEngine::fromString(QStringView prayerString)
{
    return m_revPrayerEnumMap.value(
        Adhan::PrayerUtils::from_string(QStringView(prayerString).toLatin1()));
}