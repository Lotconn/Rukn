#include "internal-helpers.hpp"

Q_STATIC_LOGGING_CATEGORY(common, "Common")

void fetchAndSetKey(int index, const QVariantList &refList, int &idxVar,
                    const std::function<void(const QString &)> &setFunc, const QString &id) noexcept
{
    if (!checkIndex(index, refList))
        return;

    QVariant entry{refList.at(index)};

    switch (entry.typeId())
    {
    case QMetaType::QVariantHash:
        entry = entry.toHash().value(id);
        break;
    case QMetaType::QVariantMap:
        entry = entry.toMap().value(id);
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

    if (index == idxVar)
    {
        logDebug(common) << "Skipping setting identical values: " << idxVar << " vs " << index;
        return;
    }

    const int old{idxVar};
    idxVar = index;

    logDebug(common) << "Replaced " << old << " with " << index;
}

bool isValidEntry(const QVariantHash &entryMaps, qsizetype maxEntrySize,
                  const QHash<QString, int> &refFields, qsizetype refMapSize) noexcept
{
    if (entryMaps.isEmpty() || maxEntrySize <= 0 || refFields.isEmpty() ||
        (refMapSize > 0 && entryMaps.size() != refMapSize))
        return false;

    return std::ranges::all_of(entryMaps, [&](const QVariant &list) -> bool {
        return isValidMap(list.toHash(), maxEntrySize, refFields);
    });
}

bool isValidEntry(const QVariantList &entryList, qsizetype maxMapSize,
                  const QHash<QString, int> &refFields, qsizetype refListSize) noexcept
{
    if (entryList.isEmpty() || maxMapSize <= 0 || refFields.isEmpty() ||
        (refListSize > 0 && entryList.size() != refListSize))
        return false;

    return std::ranges::all_of(entryList, [&](const QVariant &list) -> bool {
        return isValidMap(list.toHash(), maxMapSize, refFields);
    });
}

bool isValidMap(const QVariantHash &entry, qsizetype maxMapSize,
                const QHash<QString, int> &refFields) noexcept
{
    if (entry.size() < 0 || entry.size() > maxMapSize)
        return false;

    return std::ranges::all_of(entry.keyBegin(), entry.keyEnd(), [&](const QString &key) -> bool {
        return refFields.contains(key) && entry.value(key).typeId() == refFields.value(key);
    });
}

void loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   QObject *obj, const QString &property,
                   std::optional<bool> configMigrated) noexcept
{
    if (const QVariant cfgVal{preLoadConfigKey(oldKey, newKey, migrateKey, configMigrated)};
        !cfgVal.isNull())
    {
        // This guard is required to avoid Qt meta-system from creating the non-existent property
        // temporarily and assigning the value - a silent failure
        if (!obj->property(qPrintable(property)).isValid())
            logCritical(common) << "Property \"" << property << "\" doesn't exist";
        else if (!obj->setProperty(qPrintable(property), cfgVal))
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

    // Both `migrated` == false and `oldKey` need to be supplied for a migration attempt. Migration
    // is skipped anyway if `migrated` == true. Note that the first empty-guard doesn't stop
    // `oldKey` from being empty.
    if (!migrated && !oldKey.isEmpty())
    {
        logDebug(common) << "Migrating from old config";

        cfgVal = core->readOldSettings(oldKey);
    }

    // If `cfgVal` is still null in the previous block, then it is either because old settings
    // simply doesn't exist or failure to fetch valid value from `oldKey`. At any rate, migration
    // does not apply anymore and we must read the present config file now. Note that the very first
    // empty-guard doesn't stop `newKey` from being empty.
    //
    // However, on an already-migrated case, the previous block is always skipped, and we must read
    // the present config file anyway.
    if (migrated || cfgVal.isNull())
    {
        if (!newKey.isEmpty())
            cfgVal = core->readSettings(newKey);

        // Once inside this block, we're sure migration did not happen and should not happen in
        // future, the logical-AND chain needs to be broken to stop top-level migration routines
        // from running further. Inform about this if applicable.
        if (configMigrated.has_value())
            configMigrated = false;
    }

    // If `cfgVal` is still null for a total failure then `migrateKey` should be false all along too
    // (because invalid/empty values evaluate to bool false). If that's the case, then something is
    // _actually_ wrong, whether we were reading from both old and present configs or either one.
    // However, if `migrateKey` read `true` a while ago, then present config file itself is ok, and
    // `cfgVal` being empty means the keys supplied could've been wrong all along.
    if (cfgVal.isNull())
    {
        if (!migrated)
            logCritical(common) << "Couldn't read valid value from config file";
        else
            logWarning(common)
                << "Couldn't read valid value, possibly wrong or empty key, or invalid value";

        return {};
    }

    return cfgVal;
}

// int textsRefresherReal()
// { return 0; }