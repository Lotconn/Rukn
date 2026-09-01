#include "internal-helpers.hpp"
#include <CoreEngine.hpp>
#include <PrayerEngine.hpp>

namespace Munadi
{

using Prayer = Munadi::PrayerEngine::Prayer;

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

CoreEngine::CoreEngine(QObject *parent) : QObject{parent}
{
    if (s_singletonInstance != nullptr)
        qFatal("Munadi:CoreEngine: A second Munadi instance was constructed; only one "
               "instance may exist per process");

    logDebug << "FileMan::refreshStore() return: " << fileman.refreshStore();

    CoreEngine::s_singletonInstance = this;
}

CoreEngine::~CoreEngine()
{
    if (CoreEngine::s_singletonInstance == this)
        CoreEngine::s_singletonInstance = nullptr;
}

auto CoreEngine::addAthan(const QUrl &fileUrl, int fileDialogCategory) -> void
{
    FileMan::Categories categories{FileMan::None};

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

    const add_file_t result{fileman.addFile(fileUrl, categories)};

    if (result.status() != 0 && result.status() != EEXIST)
    {
        logCritical << "FileMan::addFile() failed with error: " << result.status();
        return;
    }

    const QVariantMap entry{
        {"name",       FileMan::getFileName(fileUrl)},
        {"path", result.location().toDisplayString()}
    };

    QWriteLocker lock{&this->varLock};

    this->m_audioList.append(entry);

    emit audioListChanged(this->m_audioList);
}

void CoreEngine::modifyPrayerConfig(int prayerIndex, const QString &field, const QVariant &value)
{
    if (prayerIndex < 0 || prayerIndex >= this->m_prayerConfig.size())
        return;

    static const QSet<QString> validFields{"adjustment", "volume", "athan_idx"};

    if (!validFields.contains(field))
    {
        logCritical << field << "is not a valid field in prayerConfig";
        return;
    }

    QWriteLocker lock{&this->varLock};

    QVariantMap config{this->m_prayerConfig[prayerIndex].toMap()};

    config[field]                     = value;
    this->m_prayerConfig[prayerIndex] = config;

    emit prayerConfigChanged(this->m_prayerConfig);
}

auto CoreEngine::removeAthan(int selectedIndex) -> void
{
    if (selectedIndex < this->defaultAudioCount || selectedIndex >= this->m_audioList.size())
        return;

    const QString   filePath{this->m_audioList[selectedIndex].toMap().value("path").toString()};
    const rm_file_t result{fileman.rmFile(QUrl(filePath))};

    if (result.status_l() != 0 && result.status_l() != ENOENT)
    {
        logCritical << "Fileman::rmFile() failed with error: " << result.status_l();
        return;
    }

    QWriteLocker lock{&this->varLock};

    // Iterate using enum values directly as indices
    for (int i{static_cast<int>(Prayer::Fajr)}; i <= static_cast<int>(Prayer::Isha); ++i)
    {
        QVariantMap cfg{this->m_prayerConfig[i].toMap()};
        const int   current{cfg.value("athan_idx").toInt()};

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

        this->m_prayerConfig[i] = cfg;
    }

    this->m_audioList.removeAt(selectedIndex);
    this->m_audioList.squeeze();

    emit this->audioListChanged(this->m_audioList);
    emit this->prayerConfigChanged(this->m_prayerConfig);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//  Private Members
////////////////////////////////////////////////////////////////////////////////////////////////////

CoreEngine *CoreEngine::s_singletonInstance{nullptr};

bool CoreEngine::isValidAudioEntry(const QVariantMap &entry)
{
    return entry.contains("name") && entry.contains("path") &&
           entry.value("name").typeId() == QMetaType::QString &&
           entry.value("path").typeId() == QMetaType::QString;
}

int CoreEngine::localeGuard()
{ return 0; }

void CoreEngine::setAudioList(const QVariantList &audioList)
{
    QWriteLocker lock{&this->varLock};
    QVariantList validated;

    validated.reserve(audioList.size());

    for (const QVariant &item : audioList)
    {
        const QVariantMap entry{item.toMap()};

        if (!CoreEngine::isValidAudioEntry(entry))
        {
            qWarning() << "CoreEngine: dropping malformed audioList entry" << entry;
            continue;
        }

        validated.append(entry);
    }

    this->m_audioList = validated;

    emit this->audioListChanged(this->m_audioList);
}

void CoreEngine::setPrayerConfig(const QVariantList &prayerConfig)
{
    QWriteLocker lock{&this->varLock};

    this->m_prayerConfig = prayerConfig;

    emit this->prayerConfigChanged(this->m_prayerConfig);
}
} // namespace Munadi