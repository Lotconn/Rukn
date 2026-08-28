#include "internal-helpers.hpp"
#include <engine.hpp>
#include <fileman>

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

    this->initTexts();

    m_audioList = {
        QVariantMap{   {"name", "Makkah (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-makkah.ogg"}  },
        QVariantMap{{"name", "Ar-Rashid (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/athan-arRashid.ogg"}},
        QVariantMap{     {"name", "Fajr (built-in)"},
                    {"path", "qrc:/qt/qml/Munadi/data/fajr-athan.ogg"}    }
    };

    this->defaultAudioCount = m_audioList.size();

    m_prayerConfig = {
        QVariantMap{{"adjustment", 0}, {"volume", 0.2}, {"athan_idx", 2}}, // [0] Fajr
        QVariantMap{{"adjustment", 0}, {"volume", 0.0}, {"athan_idx", 0}}, // [1] Sunrise
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 0}}, // [2] Dhuhr
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 0}}, // [3] Asr
        QVariantMap{{"adjustment", 0}, {"volume", 1.0}, {"athan_idx", 1}}, // [4] Maghrib
        QVariantMap{{"adjustment", 0}, {"volume", 0.5}, {"athan_idx", 0}}  // [5] Isha
    };
}

PrayerEngine::~PrayerEngine()
{
    if (PrayerEngine::s_singletonInstance == this)
        PrayerEngine::s_singletonInstance = nullptr;
}

void PrayerEngine::initTexts()
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

auto PrayerEngine::addAthan(const QUrl &fileUrl, int fileDialogCategory) -> void
{
    FileMan::Categories categories = FileMan::None;

    switch (fileDialogCategory)
    {
    case 0:
        categories = FileMan::Audio;
        break;
    case 1:
        categories = FileMan::Video;
        break;
    default:
        categories = FileMan::Audio | FileMan::Video;
    }

    if (categories == FileMan::None)
    {
        logCritical << "Invalid category propagated from file dialog";
        return;
    }

    const add_file_t result = FileMan::instance().addFile(fileUrl, categories);

    if (result.status() != 0 && result.status() != EEXIST)
    {
        logCritical << "FileMan::addFile() failed with error: " << result.status();
        return;
    }

    const QVariantMap entry = {
        {"name",       FileMan::getFileName(fileUrl)},
        {"path", result.location().toDisplayString()}
    };

    QWriteLocker lock(&varLock);

    m_audioList.append(entry);

    emit audioListChanged(m_audioList);
}

// PrayerEngine::Prayer PrayerEngine::currentPrayer(Adhan::Instant when)
// { return Munadi::PrayerEngine::m_revPrayerEnumMap.value(m_prayerTimes.currentPrayer(when)); }

void PrayerEngine::modifyPrayerConfig(int prayerIndex, const QString &field, const QVariant &value)
{
    if (prayerIndex < 0 || prayerIndex >= m_prayerConfig.size())
        return;

    static const QSet<QString> validFields = {"adjustment", "volume", "athan_idx"};

    if (!validFields.contains(field))
    {
        logCritical << field << "is not a valid field in prayerConfig";
        return;
    }

    QWriteLocker lock(&varLock);

    QVariantMap config          = m_prayerConfig[prayerIndex].toMap();
    config[field]               = value;
    m_prayerConfig[prayerIndex] = config;

    emit prayerConfigChanged(m_prayerConfig);
}

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
    this->initTexts();
    ++m_localeChangeCount;
    emit localeChanged();
}

auto PrayerEngine::removeAthan(int selectedIndex) -> void
{
    if (selectedIndex < this->defaultAudioCount || selectedIndex >= m_audioList.size())
        return;

    const QString   filePath = this->m_audioList[selectedIndex].toMap().value("path").toString();
    const rm_file_t result   = FileMan::instance().rmFile(QUrl(filePath));

    if (result.status_l() != 0 && result.status_l() != ENOENT)
    {
        logCritical << "Fileman::rmFile() failed with error: " << result.status_l();
        return;
    }

    QWriteLocker lock(&varLock);

    // Iterate using enum values directly as indices
    for (int i = static_cast<int>(Prayer::Fajr); i <= static_cast<int>(Prayer::Isha); ++i)
    {
        QVariantMap cfg     = m_prayerConfig[i].toMap();
        const int   current = cfg.value("athan_idx").toInt();

        if (current == selectedIndex)
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
        else if (current > selectedIndex)
        {
            cfg["athan_idx"] = current - 1;
        }
        m_prayerConfig[i] = cfg;
    }

    m_audioList.removeAt(selectedIndex);
    m_audioList.squeeze();

    emit audioListChanged(m_audioList);
    emit prayerConfigChanged(m_prayerConfig);
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

bool PrayerEngine::isValidAudioEntry(const QVariantMap &entry)
{
    return entry.contains("name") && entry.contains("path") &&
           entry.value("name").typeId() == QMetaType::QString &&
           entry.value("path").typeId() == QMetaType::QString;
}

QString PrayerEngine::toString(PrayerEngine::Prayer prayer)
{ return QString::fromLatin1(Adhan::PrayerUtils::to_string(m_prayerEnumMap.value(prayer))); }

PrayerEngine::Prayer PrayerEngine::fromString(QStringView prayerName)
{
    return m_revPrayerEnumMap.value(
        Adhan::PrayerUtils::from_string(QStringView(prayerName).toLatin1()));
}

void PrayerEngine::setAudioList(const QVariantList &audioList)
{
    QWriteLocker lock(&varLock);

    QVariantList validated;
    validated.reserve(audioList.size());

    for (const QVariant &item : audioList)
    {
        const QVariantMap entry = item.toMap();

        if (!PrayerEngine::isValidAudioEntry(entry))
        {
            qWarning() << "PrayerEngine: dropping malformed audioList entry" << entry;
            continue;
        }

        validated.append(entry);
    }

    m_audioList = validated;

    emit audioListChanged(m_audioList);
}

void PrayerEngine::setPrayerConfig(const QVariantList &prayerConfig)
{
    QWriteLocker lock(&varLock);

    m_prayerConfig = prayerConfig;

    emit prayerConfigChanged(m_prayerConfig);
}
