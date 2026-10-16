#pragma once

#include <QLoggingCategory>
#include <QTimer>

#include <CoreEngine.hpp>

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define logDebug(logCategory) qCDebug(logCategory).noquote().nospace() << __func__ << ": debug: "_L1
#define logInfo(logCategory)  qCInfo(logCategory).noquote().nospace() << __func__ << ": info: "_L1
#define logWarning(logCategory)                                                                    \
    qCWarning(logCategory).noquote().nospace() << __func__ << ": warning: "_L1
#define logCritical(logCategory)                                                                   \
    qCCritical(logCategory).noquote().nospace() << __func__ << ": error: "_L1
#define logFatal(logCategory) qCFatal(logCategory).noquote().nospace() << __func__ << ": fatal: "_L1
// NOLINTEND(cppcoreguidelines-macro-usage)

// Allows shorthand QLatin1StringView ""_L1 operators
// Included in this internal header for all sources to utilize
using Qt::StringLiterals::operator""_L1; // NOLINT(*global-names-in-headers)

bool checkIndex(const QVariant &index, const QVariantList &refList,
                const std::optional<int> &idxVar = std::nullopt);
auto fetchAndSetListIdx(const int &index, const QVariantList &refList, QAtomicInt &idxVar,
                        const std::function<void(const QString &)> &setFunc) -> void;
auto isValidEntry(const QVariantHash &entryMaps, const qsizetype &refEntrySize,
                  const QHash<QString, int> &refFields, const qsizetype &refMapSize = 0) -> bool;
auto isValidEntry(const QVariantList &entryList, const qsizetype &refMapSize,
                  const QHash<QString, int> &refFields, const qsizetype &refListSize = 0) -> bool;
auto isValidMap(const QVariantHash &entry, const qsizetype &refMapSize,
                const QHash<QString, int> &refFields) -> bool;
auto loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   QObject *obj, QAnyStringView property,
                   std::optional<bool> configMigrated = std::nullopt) -> void;
auto loadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                   const std::function<void(const QVariant &)> &setFunc,
                   std::optional<bool> configMigrated = std::nullopt) -> void;
auto preLoadConfigKey(QAnyStringView oldKey, QAnyStringView newKey, const QString &migrateKey,
                      std::optional<bool> configMigrated) -> QVariant;
auto textsRefresherReal() -> int;

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
               const QLoggingCategory &category = QLoggingCategory{"Common"}) -> bool
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
