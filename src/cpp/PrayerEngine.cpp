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

    m_prayerConfig = {
        QVariantMap{{"adjustment", 0}, {"volume", 0.2}, {"athan_idx", 2}}, // [0] Fajr
        QVariantMap{{"adjustment", 0}, {"volume", 0.0}, {"athan_idx", 0}}, // [1] Sunrise
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 0}}, // [2] Dhuhr
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 0}}, // [3] Asr
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 1}}, // [4] Maghrib
        QVariantMap{{"adjustment", 0}, {"volume", 0.5}, {"athan_idx", 0}}  // [5] Isha
    };
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

auto PrayerEngine::prayerConfig() const -> QVariantList
{ return m_prayerConfig; }

auto PrayerEngine::setPrayerConfig(const QVariantList &config) -> void
{
    m_prayerConfig = config;
    emit prayerConfigChanged(m_prayerConfig);
}

auto PrayerEngine::removeAthanIndex(int rmIdx) -> void
{
    // Iterate using enum values directly as indices
    for (int i = static_cast<int>(Prayer::Fajr); i <= static_cast<int>(Prayer::Isha); ++i)
    {
        QVariantMap cfg     = m_prayerConfig[i].toMap();
        const int   current = cfg.value("athan_idx").toInt();

        if (current == rmIdx)
        {
            switch (static_cast<Prayer>(i))
            {
            case Prayer::Fajr:
                cfg["athan_idx"] = 2;
                break;
            case Prayer::Maghrib:
                cfg["athan_idx"] = 1;
                break;
            default:
                cfg["athan_idx"] = 0;
                break;
            }
        }
        else if (current > rmIdx)
        {
            cfg["athan_idx"] = current - 1;
        }
        m_prayerConfig[i] = cfg;
    }

    emit prayerConfigChanged(m_prayerConfig);
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

int PrayerEngine::localeGuard() const
{ return this->m_localeChangeCount; }

QString PrayerEngine::prayerName(PrayerEngine::Prayer prayer)
{ return m_prayerDisplayNames.value(prayer); }

QStringList PrayerEngine::prayerNamesList()
{
    QStringList list;

    for (const auto &[key, value] : m_prayerDisplayNames.asKeyValueRange())
        list << value;

    return list;
}

void PrayerEngine::refreshText()
{
    this->init();
    ++m_localeChangeCount;
    emit localeChanged();
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