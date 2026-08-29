#include "internal-helpers.hpp"
#include <CoreEngine.hpp>
#include <PrayerEngine.hpp>
#include <fileman>

using CoreEngine = Munadi::CoreEngine;
using Prayer     = Munadi::PrayerEngine::Prayer;

CoreEngine *CoreEngine::create(QQmlEngine * /*unused*/, QJSEngine * /*unused*/)
{
    if (CoreEngine::s_singletonInstance == nullptr)
        qFatal("Munadi:CoreEngine: no instance constructed. Munadi must be explicitly "
               "instantiated before the QML engine loads");

    // The singleton's lifetime is owned by this TU (main()/first construction),
    // not by the QQmlEngine. Make that explicit so the engine never attempts
    // to garbage-collect or delete it.
    QQmlEngine::setObjectOwnership(CoreEngine::s_singletonInstance, QQmlEngine::CppOwnership);

    return CoreEngine::s_singletonInstance;
}

CoreEngine &CoreEngine::instance()
{
    if (CoreEngine::s_singletonInstance == nullptr)
        qFatal("Munadi:CoreEngine: no instance constructed. Munadi must be explicitly "
               "instantiated before "
               "invoking the instance");

    return *s_singletonInstance;
}

CoreEngine::CoreEngine(QObject *parent) : QObject(parent)
{
    if (s_singletonInstance != nullptr)
        qFatal("Munadi:CoreEngine: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    CoreEngine::s_singletonInstance = this;

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

CoreEngine::~CoreEngine()
{
    if (CoreEngine::s_singletonInstance == this)
        CoreEngine::s_singletonInstance = nullptr;
}

auto CoreEngine::addAthan(const QUrl &fileUrl, int fileDialogCategory) -> void
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

void CoreEngine::emitLocaleChanged()
{
    ++m_localeChangeCount;
    emit localeChanged();
}

void CoreEngine::modifyPrayerConfig(int prayerIndex, const QString &field, const QVariant &value)
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

auto CoreEngine::removeAthan(int selectedIndex) -> void
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

CoreEngine *CoreEngine::s_singletonInstance = nullptr;

bool CoreEngine::isValidAudioEntry(const QVariantMap &entry)
{
    return entry.contains("name") && entry.contains("path") &&
           entry.value("name").typeId() == QMetaType::QString &&
           entry.value("path").typeId() == QMetaType::QString;
}

void CoreEngine::setAudioList(const QVariantList &audioList)
{
    QWriteLocker lock(&varLock);

    QVariantList validated;
    validated.reserve(audioList.size());

    for (const QVariant &item : audioList)
    {
        const QVariantMap entry = item.toMap();

        if (!CoreEngine::isValidAudioEntry(entry))
        {
            qWarning() << "CoreEngine: dropping malformed audioList entry" << entry;
            continue;
        }

        validated.append(entry);
    }

    m_audioList = validated;

    emit audioListChanged(m_audioList);
}

void CoreEngine::setPrayerConfig(const QVariantList &prayerConfig)
{
    QWriteLocker lock(&varLock);

    m_prayerConfig = prayerConfig;

    emit prayerConfigChanged(m_prayerConfig);
}
