#pragma once

#include <QLoggingCategory>
#include <QTimer>

#include <CoreEngine.hpp>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define logDebug(logCategory)    qCDebug(logCategory).noquote().nospace() << __func__ << ": "
#define logInfo(logCategory)     qCInfo(logCategory).noquote().nospace()
#define logWarning(logCategory)  qCWarning(logCategory).noquote().nospace()
#define logCritical(logCategory) qCCritical(logCategory).noquote().nospace() << __func__ << ": "
#define logFatal(logCategory)    qCFatal(logCategory).noquote().nospace() << __func__ << ": "
// NOLINTEND(cppcoreguidelines-macro-usage)

template <typename T>
auto checkIndex(const QVariant &index, const QList<T> &refList,
                std::optional<int> idxVar = std::nullopt) noexcept -> bool
{
    const QLoggingCategory common{"Common"};

    if (index.isNull())
    {
        logDebug(common) << "No value was provided";

        return false;
    }

    const int idx{index.toInt()};

    if (idx < 0 || idx >= refList.size())
    {
        logCritical(common) << "Index " << idx << " out of range";

        return false;
    }

    if (idxVar.has_value() && idxVar == idx)
        logInfo(common) << "New value is identical to old";

    return true;
}

auto fetchAndSetListIdx(int index, const QVariantList &refList, int &idxVar,
                        const std::function<void(const QString &)> &setFunc) noexcept -> void;
auto isValidEntry(const QVariantHash &entryMaps, qsizetype refEntrySize,
                  const QHash<QString, int> &refFields, qsizetype refMapSize = 0) noexcept -> bool;
auto isValidEntry(const QVariantList &entryList, qsizetype refMapSize,
                  const QHash<QString, int> &refFields, qsizetype refListSize = 0) noexcept -> bool;
auto isValidMap(const QVariantHash &entry, qsizetype refMapSize,
                const QHash<QString, int> &refFields) noexcept -> bool;
auto loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   QObject *obj, const QString &property,
                   std::optional<bool> configMigrated = std::nullopt) noexcept -> void;
auto loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   const std::function<void(const QVariant &)> &setFunc,
                   std::optional<bool> configMigrated = std::nullopt) noexcept -> void;
auto preLoadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                      std::optional<bool> configMigrated) noexcept -> QVariant;
// auto textsRefresherReal() -> int;

/**
 * @brief Set `key` to `var` if it's a valid key from `table`, optionally set the logging category
 * to `category`
 *
 * @tparam Key  : QHash key type
 * @tparam T    : QHash value type
 * @param table : Reference QHash<Key, T> table to check against
 * @param key   : Key to be stored
 * @param var   : Variable where `key` will be stored to
 * @param category  : (Optional) QLogging category for logging purposes
 * @return true : If `var` is set to `key` or `var` and `key` are same
 * @return false : If `key` is an invalid key in `table`
 */
template <typename Key, typename T>
auto setConfig(const QHash<Key, T> &table, const QString &key, QString &var,
               const QLoggingCategory &category = QLoggingCategory{"Common"}) noexcept -> bool
{
    if (!table.contains(key))
    {
        logCritical(category) << "Key " << key << " not recognized";

        return false;
    }

    if (var == key)
    {
        logInfo(category) << "Skipping setting identical key";

        return false;
    }

    logDebug(category) << "Replacing " << var << " with " << key;
    var = key;

    return true;
}
