#include <QVariant>

#include "internal-helpers.hpp"

Q_LOGGING_CATEGORY(common, "Common")

bool isValidEntry(const QVariantMap &entry, const qsizetype &refMapSize,
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

bool isValidEntryList(const QVariantList &entryMaps, const qsizetype &refMapSize,
                      const QMap<QString, int> &refFields, const qsizetype &refListSize)
{
    if (refListSize > 0 && entryMaps.size() != refListSize)
        return false;

    const QVariantList maps{entryMaps};
    return std::ranges::all_of(maps.begin(), maps.end(),
                               [&refMapSize, &refFields](const QVariant &list) -> bool {
                                   return isValidEntry(list.toMap(), refMapSize, refFields);
                               });
}

int textsRefresherReal()
{ return 0; }