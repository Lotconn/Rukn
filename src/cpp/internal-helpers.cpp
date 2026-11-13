#include "internal-helpers.hpp"

Q_STATIC_LOGGING_CATEGORY(common, "Common")

void fetchAndSetKey(int index, const QVariantList &refList, int &idxVar,
                    const std::function<void(const QString &)> &setFunc,
                    const QString                              &id) noexcept {
  if (!checkIndex(index, refList))
    return;

  QVariant entry{refList.at(index)};

  switch (entry.typeId()) {
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
  if (!entry.canConvert<QString>()) {
    logCritical(common) << "Provided list is not of known type";
    return;
  }

  setFunc(entry.toString());

  if (index == idxVar) {
    logDebug(common) << "Skipping setting identical values: " << idxVar << " vs " << index;
    return;
  }

  const int old{idxVar};
  idxVar = index;

  logDebug(common) << "Replaced " << old << " with " << index;
}

bool isValidEntry(const QVariantHash &entryMaps, qsizetype maxEntrySize,
                  const QHash<QString, int> &refFields, qsizetype refMapSize) noexcept {
  if (entryMaps.isEmpty() || maxEntrySize <= 0 || refFields.isEmpty() ||
      (refMapSize > 0 && entryMaps.size() != refMapSize))
    return false;

  return std::ranges::all_of(entryMaps, [&](const QVariant &list) -> bool {
    return isValidMap(list.toHash(), maxEntrySize, refFields);
  });
}

bool isValidEntry(const QVariantList &entryList, qsizetype maxMapSize,
                  const QHash<QString, int> &refFields, qsizetype refListSize) noexcept {
  if (entryList.isEmpty() || maxMapSize <= 0 || refFields.isEmpty() ||
      (refListSize > 0 && entryList.size() != refListSize))
    return false;

  return std::ranges::all_of(entryList, [&](const QVariant &list) -> bool {
    return isValidMap(list.toHash(), maxMapSize, refFields);
  });
}

bool isValidMap(const QVariantHash &entry, qsizetype maxMapSize,
                const QHash<QString, int> &refFields) noexcept {
  if (entry.size() < 0 || entry.size() > maxMapSize)
    return false;

  return std::ranges::all_of(entry.keyBegin(), entry.keyEnd(), [&](const QString &key) -> bool {
    return refFields.contains(key) && entry.value(key).typeId() == refFields.value(key);
  });
}

void loadConfigKey(QAnyStringView key, QObject *obj, const QString &property,
                   const QLoggingCategory &category) noexcept {
  if (const QVariant cfgVal{preLoadConfigKey(key, category)}; !cfgVal.isNull()) {
    // This guard is required to avoid Qt meta-system from creating the non-existent property
    // temporarily and assigning the value - a silent failure
    if (!obj->property(qPrintable(property)).isValid())
      logCritical(category) << "Property \"" << property << "\" doesn't exist";
    else if (!obj->setProperty(qPrintable(property), cfgVal))
      logWarning(category) << "Failed to set config value \"" << cfgVal << "\" to property "
                           << property;

    return;
  }

  logWarning(category) << "Not setting null value";
}

void loadConfigKey(QAnyStringView key, const std::function<void(const QVariant &)> &setFunc,
                   const QLoggingCategory &category) noexcept {
  if (const QVariant cfgVal{preLoadConfigKey(key, category)}; !cfgVal.isNull()) {
    setFunc(cfgVal);
    return;
  }

  logWarning(category) << "Not setting null value";
}

QVariant preLoadConfigKey(QAnyStringView key, const QLoggingCategory &category) noexcept {
  if (key.isEmpty()) {
    logCritical(category) << "Key cannot be empty";
    return {};
  }

  QVariant cfgVal{core->readSettings(key)};

  if (cfgVal.isNull()) {
    logWarning(category)
        << "Couldn't read valid value, possibly wrong or empty key, invalid value or read error";
    return {};
  }

  return cfgVal;
}