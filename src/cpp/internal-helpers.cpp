#include "internal-helpers.hpp"

Q_STATIC_LOGGING_CATEGORY(common, "Common")

void fetchAndSetListIdx(const int &index, const QVariantList &refList, QAtomicInt &idxVar,
                        const std::function<void(const QString &)> &setFunc) noexcept
{
    if (!checkIndex(index, refList, idxVar.loadRelaxed()))
        return;

    QVariant entry{refList.at(index)};

    switch (entry.typeId())
    {
    case QMetaType::QVariantHash:
        entry = entry.toHash().value("name"_L1);
        break;
    case QMetaType::QVariantMap:
        entry = entry.toMap().value("name"_L1);
        break;
    default:
        break;
    }

    logDebug(common) << entry;
    if (!entry.canConvert<QString>())
    {
        logCritical(common) << "Provided list is not of known type";

        return;
    }

    setFunc(entry.toString());

    logDebug(common) << "Replacing " << idxVar.fetchAndStoreRelaxed(index) << " with " << index;
}

bool isValidMap(const QVariantHash &entry, const qsizetype &refMapSize,
                const QHash<QString, int> &refFields) noexcept
{
    if (entry.size() < 0 || entry.size() > refMapSize)
        return false;

    const QStringList keys{entry.keys()};

    return std::ranges::all_of(
        keys.begin(), keys.end(), [&refFields, &entry](const QString &key) -> bool {
            // logDebug(common) << "Entry type: " << entry.value(key).typeId()
            //  << " vs reference: " << refFields.value(key);
            return refFields.contains(key) && entry.value(key).typeId() == refFields.value(key);
        });
}

bool isValidEntry(const QVariantHash &entryMaps, const qsizetype &refEntrySize,
                  const QHash<QString, int> &refFields, const qsizetype &refMapSize) noexcept
{
    if (refMapSize > 0 && entryMaps.size() != refMapSize)
        return false;

    return std::ranges::all_of(entryMaps,
                               [&refEntrySize, &refFields](const QVariant &list) -> bool {
                                   return isValidMap(list.toHash(), refEntrySize, refFields);
                               });
}

bool isValidEntry(const QVariantList &entryList, const qsizetype &refMapSize,
                  const QHash<QString, int> &refFields, const qsizetype &refListSize) noexcept
{
    if (refListSize > 0 && entryList.size() != refListSize)
        return false;

    return std::ranges::all_of(entryList, [&refMapSize, &refFields](const QVariant &list) -> bool {
        return isValidMap(list.toHash(), refMapSize, refFields);
    });
}

void loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   QObject *obj, QAnyStringView property,
                   std::optional<bool> configMigrated) noexcept
{
    if (const QVariant cfgVal{preLoadConfigKey(oldKey, newKey, migrateKey, configMigrated)};
        !cfgVal.isNull())
    {
        // logDebug(common) << "Value of provided key " << cfgVal;

        if (!obj->property(qPrintable(property.toString())).isValid())
            logCritical(common) << "Property \"" << property << "\" doesn't exist";
        else if (!obj->setProperty(qPrintable(property.toString()), cfgVal))
            logWarning(common) << "Failed to set config value \"" << cfgVal << "\" to property "
                               << property;

        return;
    }

    logWarning(common) << "Not setting null value";
}

void loadConfigKey(QAnyStringView oldKey, // NOLINT(bugprone-easily-swappable-parameters)
                   QAnyStringView newKey, const QString &migrateKey,
                   const std::function<void(const QVariant &)> &setFunc,
                   std::optional<bool>                          configMigrated) noexcept
{
    if (const QVariant cfgVal{preLoadConfigKey(oldKey, newKey, migrateKey, configMigrated)};
        !cfgVal.isNull())
    {
        // logDebug(common) << "Value of provided key " << cfgVal;

        setFunc(cfgVal);

        return;
    }

    logWarning(common) << "Not setting null value";
}

QVariant preLoadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                          std::optional<bool> configMigrated) noexcept
{
    if (oldKey.isEmpty() && newKey.isEmpty())
    {
        logCritical(common) << "Old key and new key cannot be both empty";

        return {};
    }

    const bool migrated{core->readSettings(migrateKey).toBool()};

    QVariant cfgVal;

    if (!migrated && !oldKey.isEmpty())
    {
        logDebug(common) << "Migrating from old config";

        cfgVal = core->readOldSettings(oldKey);
    }

    if (migrated || cfgVal.isNull())
    {
        if (!newKey.isEmpty())
            cfgVal = core->readSettings(newKey);

        if (configMigrated.has_value())
            configMigrated = false;
    }

    if (cfgVal.isNull())
    {
        if (!migrated)
            logCritical(common) << "Couldn't read valid value from config file";

        return {};
    }

    return cfgVal;
}

// int textsRefresherReal()
// { return 0; }