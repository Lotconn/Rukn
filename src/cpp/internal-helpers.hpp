#pragma once

#include <CoreEngine.hpp>

/**
 * @brief Validates `index` against a reference list `refList`. Passes if `index` has a valid value
 * and within `refList` 's range.
 *
 * @param index: The index to check, ideally a received value
 * @param refList: The reference QList list to validate `index` against
 * @return true: Validations passed, and `index` can be safely used for `refList`'s operations
 * @return false: Validations failed
 */
template <typename T> bool checkIndex(const QVariant &index, const QList<T> &refList) noexcept {
  const QLoggingCategory common{"Common"};

  if (index.isNull()) {
    logDebug(common) << "No value was provided";
    return false;
  }

  const int idx{index.toInt()};

  if (idx < 0 || idx >= refList.size()) {
    logCritical(common) << "Index " << idx << " out of range";
    return false;
  }

  return true;
}

/**
 * @brief Resolves the key-name indexed by `index` in the list `refList`, sets it using the setter
 * `setFunc`, and persists `index` to `idxVar`. If `refList` is a map-list, the value of `id` in the
 * map indexed by `index` is used as the key-name instead. In both cases, the resolved key-name must
 * be a string (QString). `index` is validated first before indexing `refList`.
 *
 * @param index: The candidate index where key-name could be found in the list `refList`, generally
 * a received value
 * @param refList: The list to fetch the key-name from
 * @param idxVar: The variable to persist `index` in, ideally a class member
 * @param setFunc: The method to set the key-name with
 * @param id: (optional) The key to lookup for resolving key-name if `refList` is a map-list
 */
void fetchAndSetKey(int index, const QVariantList &refList, int &idxVar,
                    const std::function<void(const QString &)> &setFunc,
                    const QString &id = QStringLiteral("name")) noexcept;

/**
 * @brief Validates a candidate nested hash/map-table `entryMaps`. Checks are performed for each
 * entry atom in `entryMaps` against a ceiling hash/map size `maxEntrySize`, and required fields and
 * their value types as in `refFields`. An optionally passed non-zero and positive `refMapSize`
 * additionally enables size validation for `entryMaps`. See `isValidMaps()` for further details.
 *
 * @param entryMaps: The candidate nested hash/map-table. Returns false if empty.
 * @param maxEntrySize: The maximum allowed size of each entry atoms in `entryMaps`. Returns false
 * if 0 or negative.
 * @param refFields: A map of required fields to exist in an ideal entry atom versus their value
 * type IDs (QMetaType enum), used to validate the entry atoms. Returns false if empty.
 * @param refMapSize: (optional) The exact allowed size of `entryMaps` required. Ignored if
 * non-positive or zero.
 * @return true: All validations passed
 * @return false: Validations failed for `entryMaps` or for any of its entry atoms, or parameters
 * are malformed.
 */
bool isValidEntry(const QVariantHash &entryMaps, qsizetype maxEntrySize,
                  const QHash<QString, int> &refFields, qsizetype refMapSize = 0) noexcept;

/**
 * @brief Map-list overload of `isValidEntry()`. Validates a candidate hash/map-list `entryList`
 * Checks are performed for each map entry in `entryList` against a ceiling hash/map size
 * `maxMapSize`, and required fields and their value types as in `refFields`. An optionally passed
 * non-zero and positive `refListSize` additionally enables size validation for `entryList`. See
 * `isValidMaps()` and `isValidEntry()` for further details.
 *
 * @param entryMaps: The candidate hash/map-list. Returns false if empty.
 * @param maxEntrySize: The maximum allowed size of each map entry in `entryList`. Returns false
 * if 0 or negative.
 * @param refFields: A map of required fields to exist in an ideal map entry versus their value
 * type IDs (QMetaType enum), used to validate the map entries. Returns false if empty.
 * @param refMapSize: (optional) The exact allowed size of `entryList` required. Ignored if
 * non-positive or zero.
 * @return true: All validations passed
 * @return false: Validations failed for `entryList` or for any of its map entries, or parameters
 * are malformed.
 */
bool isValidEntry(const QVariantList &entryList, qsizetype maxMapSize,
                  const QHash<QString, int> &refFields, qsizetype refListSize = 0) noexcept;

/**
 * @brief Validates a candidate hash-table `entry`. This is used for internal operations.
 * Recommended to use `isValidEntry()` instead. The hash is checked against the ceiling size
 * `maxMapSize` and required fields and their value types as in `refFields`.
 *
 * @param entry: The candidate hash-table/map-table. Could be an atom of a parent map/list or the
 * whole map itself. Returns false if empty.
 * @param maxMapSize: The maximum allowed size of `entry`.
 * @param refFields: A map of required fields to exist in an ideal hash/map-table versus their value
 * type IDs (QMetaType enum), used to validate `entry`.
 * @return true: All validations for `entry` passed successfully
 * @return false: Validations failed for `entry`
 */
bool isValidMap(const QVariantHash &entry, qsizetype maxMapSize,
                const QHash<QString, int> &refFields) noexcept;

/**
 * @brief Loads the value from the provided config key and sets it using `setFunc`. Skips
 * setting if value fetched is invalid. Uses `preLoadConfigKey()` method for the main routine.
 *
 * @param key: Key in config to read. Cannot be empty.
 * @param setFunc: A setter method that takes `const QVariant &` parameter and returns `void`. The
 * value read previously is set using this method.
 * @param category  : (Optional) QLogging category for logging purposes
 */
void loadConfigKey(QAnyStringView key, const std::function<void(const QVariant &)> &setFunc,
                   const QLoggingCategory &category = QLoggingCategory{"Common"}) noexcept;

/**
 * @brief Overload of `loadConfigKey()` using the `property` of `obj` to set the fetched value.
 * Behaviours otherwise are retained. See its brief for further details.
 *
 * @param key: Key in config to read. Cannot be empty.
 * @param obj: The object whose property is to be set.
 * @param property: The property to set the read value to. Does not assign to properties that
 * doesn't exist in `obj`.
 * @param category  : (Optional) QLogging category for logging purposes
 */
void loadConfigKey(QAnyStringView key, QObject *obj, const QString &property,
                   const QLoggingCategory &category = QLoggingCategory{"Common"}) noexcept;

/**
 * @brief The core routine for `loadConfigKey()`. All its behaviors inherently sprout from this
 * function. See `loadConfigKey()` briefings and inline comments of this function in the
 * implementation file for details.
 *
 * @param newKey: Key in config to read. Cannot be empty.
 * @param category  : (Optional) QLogging category for logging purposes
 * @return QVariant: The read value from `key` fetched. Empty on failure.
 */
QVariant preLoadConfigKey(QAnyStringView          key,
                          const QLoggingCategory &category = QLoggingCategory{"Common"}) noexcept;

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
bool setConfig(const QHash<Key, T> &table, const QString &key, QString &var,
               const QLoggingCategory &category = QLoggingCategory{"Common"}) noexcept {
  if (!table.contains(key)) {
    logCritical(category) << "Key " << key << " not recognized";

    return false;
  }

  if (var == key) {
    logInfo(category) << "Skipping setting identical key";

    return false;
  }

  logDebug(category) << "Replacing " << var << " with " << key;
  var = key;

  return true;
}
