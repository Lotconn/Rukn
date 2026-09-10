#include <QVariant>

#include "internal-helpers.hpp"

Q_LOGGING_CATEGORY(common, "Common")

bool isValidMap(const QVariantMap &entry, const qsizetype &refMapSize,
                const QMap<QString, int> &refFields)
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

bool isValidEntry(const QVariantMap &entryMaps, const qsizetype &refEntrySize,
                  const QMap<QString, int> &refFields, const qsizetype &refMapSize)
{
    if (refMapSize > 0 && entryMaps.size() != refMapSize)
        return false;

    return std::ranges::all_of(entryMaps,
                               [&refEntrySize, &refFields](const QVariant &list) -> bool {
                                   return isValidMap(list.toMap(), refEntrySize, refFields);
                               });
}

bool isValidEntry(const QVariantList &entryList, const qsizetype &refMapSize,
                  const QMap<QString, int> &refFields, const qsizetype &refListSize)
{
    if (refListSize > 0 && entryList.size() != refListSize)
        return false;

    return std::ranges::all_of(entryList, [&refMapSize, &refFields](const QVariant &list) -> bool {
        return isValidMap(list.toMap(), refMapSize, refFields);
    });
}

int textsRefresherReal()
{ return 0; }